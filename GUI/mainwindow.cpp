#include <QDebug>
#include "bio_signal/openBCI/open_bci_setting.h"

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "app_setting.h"

#include "game/1/game_1.h"
#include "game/2/game_2.h"
#include "game/3/game_3.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    openBCISetting loaded;
    if (loadOpenBciSettings(loaded))
        OpenBCIManager::instance().setSetting(loaded);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonClose_clicked()
{
    close();
}

void MainWindow::on_pushButtonGame1_clicked()
{
    Game1* window = new Game1();
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->setStyleSheet(AppSetting::styleSheet);

    if(AppSetting::fullScreen)
        window->showFullScreen();
    else
        window->show();
}

void MainWindow::on_pushButtonGame2_clicked()
{
    Game2* window = new Game2();
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->setStyleSheet(AppSetting::styleSheet);

    if(AppSetting::fullScreen)
        window->showFullScreen();
    else
        window->show();
}

void MainWindow::on_pushButtonGame3_clicked()
{
    Game3* window = new Game3();
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->setStyleSheet(AppSetting::styleSheet);

    if(AppSetting::fullScreen)
        window->showFullScreen();
    else
        window->show();
}

void MainWindow::on_pushButtonSetSettingOpenBci_clicked()
{
    openBCISetting setting = OpenBCIManager::instance().getSetting();

    DialogOpenBciSetting dlg(setting, this);
    dlg.setStyleSheet(AppSetting::styleSheet);

    if (dlg.exec() == QDialog::Accepted) {
        setting = dlg.getOpenBCISetting();
        OpenBCIManager::instance().setSetting(setting);
        saveOpenBciSettings(setting);
    }
}

