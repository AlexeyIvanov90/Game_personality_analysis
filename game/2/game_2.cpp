#include "game_2.h"
#include "ui_game_2.h"

Game2::Game2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game2)
{
    ui->setupUi(this);
}

Game2::~Game2()
{
    delete ui;
}

void Game2::on_pushButtonClose_clicked()
{
    close();
}

