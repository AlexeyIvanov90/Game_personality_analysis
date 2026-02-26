#include <QQuickItem>
#include <QQmlEngine>

#include "game_2.h"
#include "ui_game_2.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game2::Game2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game2)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml"; // Укажите ВАШ путь!
    ui->quickWidgetGame->engine()->addImportPath(qmlPath);
    qDebug() << "Debug: добавлен путь к QML:" << qmlPath;
#endif
    ui->quickWidgetGame->setSource(QUrl("qrc:/game/2/game_2.qml"));

    if (ui->quickWidgetGame->status() == QQuickWidget::Error) {
        qDebug() << "Ошибка загрузки QML";
    }

    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        // Подключаем сигналы из QML к слотам C++
        connect(root, SIGNAL(onJump(int)), this, SLOT(onJump(int)));
        connect(root, SIGNAL(onCollision()), this, SLOT(onCollision()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }
    ui->quickWidgetGame->setFocus();
}

Game2::~Game2()
{
    delete ui;
}

void Game2::on_pushButtonClose_clicked()
{
    close();
}

void Game2::on_pushButtonInfo_clicked()
{
    InfoDialog id;
    id.setInfo(gameInfo);
    id.exec();
}

void Game2::on_pushButtonStart_clicked()
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "startGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
    ui->quickWidgetGame->setFocus();
}

void Game2::onJump(int accuracy){
    jumpCounter++;
    qDebug() << "Кол-во прыжков: " << jumpCounter << " точность: " << accuracy;
}

void Game2::onCollision(){
    collisionCounter++;
    qDebug() << "Кол-во столкновений: " << collisionCounter;
}

void Game2::on_spinBoxLVL_valueChanged(int arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        root->setProperty("speed", arg1); // установить сложность в 5
        qDebug() << "Новая скорость: " << arg1;
    }
}

void Game2::on_pushButtonStop_clicked()
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "stopGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
}

