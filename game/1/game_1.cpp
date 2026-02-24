#include "game_1.h"
#include "ui_game_1.h"

Game1::Game1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game1)
{
    ui->setupUi(this);
}

Game1::~Game1()
{
    delete ui;
}

void Game1::on_pushButtonClose_clicked()
{
    close();
}

