#include "info_dialog.h"
#include "ui_info_dialog.h"
#include "app_setting.h"

InfoDialog::InfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    setStyleSheet(AppSetting::styleSheet);
    setWindowFlags(Qt::FramelessWindowHint);
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::setInfo(const QString info){
    ui->labelInfo->setText(info);
}

void InfoDialog::on_pushButtonClose_clicked()
{
    close();
}

