#include "game_3.h"
#include "ui_game_3.h"

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

