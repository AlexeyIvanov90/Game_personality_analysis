#include "open_bci_setting.h"
#include "ui_open_bci_setting.h"

DialogOpenBciSetting::DialogOpenBciSetting(openBCISetting setting, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogOpenBciSetting)
{
    ui->setupUi(this);

    connect(ui->pushButtonSave, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &QDialog::reject);

    //необходимо добавить заполнение comboBoxComPort доступными портами

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
