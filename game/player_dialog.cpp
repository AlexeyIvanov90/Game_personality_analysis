#include "player_dialog.h"
#include "ui_player_dialog.h"
#include "../app_setting.h"


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

    playerParam player = Player::getInstance().getPlayer();

    ui->lineEditPlayerName->setText(player.name);

    int index = ui->comboBoxPlayerType->findText(player.type);
    if (index != -1)
        ui->comboBoxPlayerType->setCurrentIndex(index);
    else
        ui->comboBoxPlayerType->setCurrentIndex(0);

    setStyleSheet(AppSetting::styleSheet);
    setWindowFlags(Qt::FramelessWindowHint);
}

PlayerDialog::~PlayerDialog()
{
    delete ui;
}

void PlayerDialog::on_pushButtonSave_clicked()
{
    playerParam player;
    player.name = ui->lineEditPlayerName->text();
    player.type = ui->comboBoxPlayerType->currentText();
    Player::getInstance().setPlayer(player);
}
