#include <QQuickItem>
#include <QQmlEngine>

#include "game_3.h"
#include "ui_game_3.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game3::Game3(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game3)
    , accuracy(0)
    , speed(0)
    , hitCounter(0)
    , collisionCounter(0)
    , ballTremor(0)
    , ballImpulse(5)
    , lvl(1)
    , gameTimerCounter(0)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml"; // Укажите ВАШ путь!
    ui->quickWidgetGame->engine()->addImportPath(qmlPath);
    qDebug() << "Debug: добавлен путь к QML:" << qmlPath;
#endif
    ui->quickWidgetGame->setSource(QUrl("qrc:/game/3/game_3.qml"));

    if (ui->quickWidgetGame->status() == QQuickWidget::Error) {
        qDebug() << "Ошибка загрузки QML";
    }

    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        // Подключаем сигналы из QML к слотам C++
        connect(root, SIGNAL(onHit()), this, SLOT(onHit()));
        connect(root, SIGNAL(onCollision()), this, SLOT(onCollision()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game3::onTimeout);

    ui->quickWidgetGame->setFocus();
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
    ui->quickWidgetGame->setFocus();
}

void Game3::on_pushButtonStart_clicked()
{
    gameTimerCounter = 0;
    hitCounter = 0;
    collisionCounter = 0;

    gameTimer.start(60000);
    restartGame();
    ui->quickWidgetGame->setFocus();
}

void Game3::on_pushButtonStop_clicked()
{
    gameTimer.stop();

    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "stopGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию stopGame";
        }
    }
}

void Game3::on_spinBoxBallImpulse_valueChanged(int arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        ballImpulse=arg1;
        root->setProperty("ballImpulse", ballImpulse); // импульс
        qDebug() << "Новый импульс: " << ballImpulse;
    }
    ui->quickWidgetGame->setFocus();
}

void Game3::on_spinBoxBallTremor_valueChanged(int arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        ballTremor=arg1;
        root->setProperty("ballTremor", ballTremor); // угол дрожания
        qDebug() << "Угол дрожания: " << ballTremor;
    }
    ui->quickWidgetGame->setFocus();
}

void Game3::on_spinBoxLvl_valueChanged(int arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        lvl=arg1;
        root->setProperty("lvl", lvl); // уровень
        qDebug() << "Новый уровень: " << lvl;
    }
    ui->quickWidgetGame->setFocus();
}

void Game3::onHit(){
    hitCounter++;
    qDebug() << "Кол-во попаданий: " << hitCounter;
}

void Game3::onCollision(){
    collisionCounter++;
    qDebug() << "Кол-во столкновений: " << collisionCounter;
    restartGame();
}

void Game3::restartGame(){
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "restartGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию restartGame";
        }
    }
}

void Game3::onTimeout(){
    gameTimerCounter++;

    if (gameTimerCounter >= 5) {
        on_pushButtonStop_clicked();
        qDebug() << "Игра окончена";
    }else{
        if((hitCounter+collisionCounter!=0))
            accuracy = ((double)hitCounter/(hitCounter+collisionCounter))*100.;
        else
            accuracy = 0.;

        speed = hitCounter/60.;
        ui->labelAccuracy->setText("Точность: " + QString::number(accuracy) + "%");
        ui->labelSpeed->setText("Попаданий/сек: " + QString::number(speed));

        hitCounter=0;
        collisionCounter=0;
    }
}
