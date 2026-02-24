#include "game_3.h"
#include "ui_game_3.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game3::Game3(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game3)
{
    ui->setupUi(this);
}

Game3::~Game3()
{
    delete ui;
}

void Game3::on_pushButtonClose_clicked()
{
    close();
}


void Game3::on_pushButtonInfo_clicked()
{
    InfoDialog id;
    id.setInfo(gameInfo);
    id.exec();
}

