#include "player_dialog.h"
#include "ui_player_dialog.h"

QString Player::name="";
QString Player::type="";

PlayerDialog::PlayerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PlayerDialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonSave, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &QDialog::reject);

    ui->comboBoxPlayerType->addItem(playerTypeToString(PlayerType::Choleric));
    ui->comboBoxPlayerType->addItem(playerTypeToString(PlayerType::Melancholic));
    ui->comboBoxPlayerType->addItem(playerTypeToString(PlayerType::Phlegmatic));
    ui->comboBoxPlayerType->addItem(playerTypeToString(PlayerType::Sanguine));


    ui->lineEditPlayerName->setText(player.name);

    int index = ui->comboBoxPlayerType->findText(player.type);
    if (index != -1)
        ui->comboBoxPlayerType->setCurrentIndex(index);
    else
        ui->comboBoxPlayerType->setCurrentIndex(0);
}

PlayerDialog::~PlayerDialog()
{
    delete ui;
}

Player PlayerDialog::getPlayerInfo(){
    player.name = ui->lineEditPlayerName->text();
    player.type = ui->comboBoxPlayerType->currentText();
    return player;
}
