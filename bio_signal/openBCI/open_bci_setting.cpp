#include "open_bci_setting.h"
#include "ui_open_bci_setting.h"

#include "qcustomplot.h"

#include <QSerialPortInfo>
#include <QSettings>
#include <QElapsedTimer>
#include <QTimer>
#include <QtGlobal>

namespace {
constexpr char kSettingsGroup[] = "OpenBCI";
constexpr int kTestDurationMs = 60 * 1000;
constexpr int kSampleRateHz = 250; // Cyton default (см. OpenBCIManager)
}

bool loadOpenBciSettings(openBCISetting& out)
{
    QSettings st;
    st.beginGroup(kSettingsGroup);
    if (!st.contains(QStringLiteral("comport"))) {
        st.endGroup();
        return false;
    }
    out.comport = st.value(QStringLiteral("comport")).toString();
    out.ECG = st.value(QStringLiteral("ecg"), 0).toUInt();
    out.EEG1 = st.value(QStringLiteral("eeg1"), 0).toUInt();
    out.EEG2 = st.value(QStringLiteral("eeg2"), 1).toUInt();
    st.endGroup();
    return true;
}

void saveOpenBciSettings(const openBCISetting& s)
{
    QSettings st;
    st.beginGroup(kSettingsGroup);
    st.setValue(QStringLiteral("comport"), s.comport);
    st.setValue(QStringLiteral("ecg"), static_cast<int>(s.ECG));
    st.setValue(QStringLiteral("eeg1"), static_cast<int>(s.EEG1));
    st.setValue(QStringLiteral("eeg2"), static_cast<int>(s.EEG2));
    st.endGroup();
}

DialogOpenBciSetting::DialogOpenBciSetting(openBCISetting setting, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogOpenBciSetting)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->pushButtonClose, &QPushButton::clicked, this, &QDialog::reject);

    ui->comboBoxComPort->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports)
        ui->comboBoxComPort->addItem(info.portName());
    if (!setting.comport.isEmpty() && ui->comboBoxComPort->findText(setting.comport) < 0)
        ui->comboBoxComPort->addItem(setting.comport);

    for(int i=0;i<8;i++){
        ui->comboBoxECG->addItem(QString::number(i));
        ui->comboBoxEEG1->addItem(QString::number(i));
        ui->comboBoxEEG2->addItem(QString::number(i));
    }

    int indexComport = ui->comboBoxComPort->findText(setting.comport);
    if (indexComport != -1)
        ui->comboBoxComPort->setCurrentIndex(indexComport);

    int indexECG = ui->comboBoxECG->findText(QString::number(setting.ECG));
    if (indexECG != -1)
        ui->comboBoxECG->setCurrentIndex(indexECG);

    int indexEEG1 = ui->comboBoxEEG1->findText(QString::number(setting.EEG1));
    if (indexEEG1 != -1)
        ui->comboBoxEEG1->setCurrentIndex(indexEEG1);

    int indexEEG2 = ui->comboBoxEEG2->findText(QString::number(setting.EEG2));
    if (indexEEG2 != -1)
        ui->comboBoxEEG2->setCurrentIndex(indexEEG2);

    setupTestPlot();
    testPlotTimer_ = new QTimer(this);
    testPlotTimer_->setInterval(40);
    connect(testPlotTimer_, &QTimer::timeout, this, &DialogOpenBciSetting::onTestPlotTimer);

}

DialogOpenBciSetting::~DialogOpenBciSetting()
{
    if (testMode_ != TestMode::None)
        stopTestAndRestoreBci();
    delete ui;
}

openBCISetting DialogOpenBciSetting::getOpenBCISetting(){
    openBCISetting setting;
    setting.comport = ui->comboBoxComPort->currentText();

    setting.ECG = ui->comboBoxECG->currentText().toInt();
    setting.EEG1 = ui->comboBoxEEG1->currentText().toInt();
    setting.EEG2 = ui->comboBoxEEG2->currentText().toInt();

    return setting;
}

void DialogOpenBciSetting::on_pushButtonSave_clicked()
{
    // Если вдруг тест запущен (в норме Save в это время disabled) — корректно останавливаем.
    if (testMode_ != TestMode::None)
        stopTestAndRestoreBci();

    const openBCISetting s = getOpenBCISetting();
    saveOpenBciSettings(s);

    // Применяем настройки сразу, чтобы дальнейшие обращения брали правильные каналы/порт.
    OpenBCIManager::instance().setSetting(s);

    accept();
}

void DialogOpenBciSetting::setupTestPlot()
{
    ui->widgetTest->addGraph();
    ui->widgetTest->xAxis->setLabel(QStringLiteral("mSec"));
    ui->widgetTest->yAxis->setLabel(QStringLiteral("mV"));
    ui->widgetTest->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

int DialogOpenBciSetting::channelForTestMode(TestMode mode) const
{
    switch (mode) {
    case TestMode::Ecg:
        return ui->comboBoxECG->currentText().toInt();
    case TestMode::Eeg1:
        return ui->comboBoxEEG1->currentText().toInt();
    case TestMode::Eeg2:
        return ui->comboBoxEEG2->currentText().toInt();
    default:
        return 0;
    }
}

void DialogOpenBciSetting::applyTestUiState(TestMode active)
{
    const bool idle = (active == TestMode::None);
    ui->comboBoxComPort->setEnabled(idle);
    ui->comboBoxECG->setEnabled(idle);
    ui->comboBoxEEG1->setEnabled(idle);
    ui->comboBoxEEG2->setEnabled(idle);
    ui->pushButtonSave->setEnabled(idle);
    ui->pushButtonClose->setEnabled(idle);

    const QString testStr = QStringLiteral("Test");
    const QString cancelStr = QStringLiteral("Cancel");

    ui->pushButtonTestECG->setText(active == TestMode::Ecg ? cancelStr : testStr);
    ui->pushButtonEEG1->setText(active == TestMode::Eeg1 ? cancelStr : testStr);
    ui->pushButtonTestEEG2->setText(active == TestMode::Eeg2 ? cancelStr : testStr);

    ui->pushButtonTestECG->setEnabled(idle || active == TestMode::Ecg);
    ui->pushButtonEEG1->setEnabled(idle || active == TestMode::Eeg1);
    ui->pushButtonTestEEG2->setEnabled(idle || active == TestMode::Eeg2);
}

void DialogOpenBciSetting::stopTestAndRestoreBci()
{
    if (testPlotTimer_)
        testPlotTimer_->stop();
    if (testMode_ == TestMode::None)
        return;

    testMode_ = TestMode::None;

    if (bciWasRunningBeforeTest_) {
        OpenBCIManager::instance().setSetting(bciSettingBackup_);
        OpenBCIManager::instance().start();
    } else {
        OpenBCIManager::instance().stop();
    }

    applyTestUiState(TestMode::None);
    ui->labelInfo->clear();
    if (ui->widgetTest->graphCount() > 0) {
        ui->widgetTest->graph(0)->setData(QVector<double>(), QVector<double>());
        ui->widgetTest->replot();
    }
}

void DialogOpenBciSetting::updateTestGraph()
{
    if (testMode_ == TestMode::None)
        return;
    const int ch = qBound(0, channelForTestMode(testMode_), 7);
    const int windowSamples = (kTestDurationMs * kSampleRateHz) / 1000;
    const QVector<double> y = OpenBCIManager::instance().getLatestChannelSamples(ch, windowSamples);
    if (y.isEmpty()) {
        return;
    }
    QVector<double> x(y.size());
    const double stepMs = 1000.0 / double(kSampleRateHz);
    for (int i = 0; i < y.size(); ++i)
        x[i] = stepMs * double(i);
    ui->widgetTest->graph(0)->setData(x, y);
    ui->widgetTest->xAxis->setRange(0.0, double(kTestDurationMs));
    ui->widgetTest->yAxis->rescale(true);
    ui->widgetTest->replot();
}

void DialogOpenBciSetting::onTestPlotTimer()
{
    if (testMode_ == TestMode::None)
        return;
    const qint64 elapsed = testElapsed_.elapsed();
    const qint64 remainingMs = qMax<qint64>(0, qint64(kTestDurationMs) - elapsed);
    ui->labelInfo->setText(QStringLiteral("Test: %1 s left").arg((remainingMs + 999) / 1000));
    if (elapsed >= kTestDurationMs) {
        stopTestAndRestoreBci();
        return;
    }
    updateTestGraph();
}

void DialogOpenBciSetting::toggleTest(TestMode mode)
{
    if (testMode_ != TestMode::None) {
        if (testMode_ == mode)
            stopTestAndRestoreBci();
        return;
    }

    bciSettingBackup_ = OpenBCIManager::instance().getSetting();
    bciWasRunningBeforeTest_ = OpenBCIManager::instance().isRunning();

    OpenBCIManager::instance().setSetting(getOpenBCISetting());
    OpenBCIManager::instance().start();
    if (!OpenBCIManager::instance().isRunning()) {
        if (bciWasRunningBeforeTest_) {
            OpenBCIManager::instance().setSetting(bciSettingBackup_);
            OpenBCIManager::instance().start();
        }
        ui->labelInfo->setText(QStringLiteral("Failed to open COM port"));
        return;
    }

    testMode_ = mode;
    ui->labelInfo->clear();
    applyTestUiState(mode);
    testElapsed_.restart();
    testPlotTimer_->start();
}

void DialogOpenBciSetting::on_pushButtonTestECG_clicked()
{
    toggleTest(TestMode::Ecg);
}

void DialogOpenBciSetting::on_pushButtonEEG1_clicked()
{
    toggleTest(TestMode::Eeg1);
}

void DialogOpenBciSetting::on_pushButtonTestEEG2_clicked()
{
    toggleTest(TestMode::Eeg2);
}
