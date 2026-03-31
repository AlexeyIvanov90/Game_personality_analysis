#include "open_bci_setting.h"
#include "ui_open_bci_setting.h"

#include <QSerialPortInfo>
#include <QSettings>

namespace {
constexpr char kSettingsGroup[] = "OpenBCI";
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

    connect(ui->pushButtonSave, &QPushButton::clicked, this, &QDialog::accept);
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
}

DialogOpenBciSetting::~DialogOpenBciSetting()
{
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
