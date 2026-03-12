#include <QQuickItem>
#include <QQmlEngine>

#include "game_2.h"
#include "ui_game_2.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game2::Game2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game2)
    , lvl(5)
    , accuracy(0.)
    , speed(0.)
    , obstacleOvercome(0)
    , collisionCounter(0)
    , gameTimerCounter(0)
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
        connect(root, SIGNAL(onObstacleOvercome()), this, SLOT(onObstacleOvercome()));
        connect(root, SIGNAL(onCollision()), this, SLOT(onCollision()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game2::onTimeout);

    ui->quickWidgetGame->setFocus();
}

Game2::~Game2()
{
    delete ui;
}

void Game2::onTimeout(){
    gameTimerCounter++;

    if (gameTimerCounter >= 5) {
        on_pushButtonStop_clicked();
        qDebug() << "Игра окончена";
    }else{
        if((obstacleOvercome+collisionCounter!=0))
            accuracy = ((double)obstacleOvercome/(obstacleOvercome+collisionCounter))*100.;
        else
            accuracy = 0.;

        speed = obstacleOvercome/60.;
        ui->labelAccuracy->setText("Точность: " + QString::number(accuracy) + "%");
        ui->labelSpeed->setText("Попаданий/сек: " + QString::number(speed));

        obstacleOvercome=0;
        collisionCounter=0;
    }
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

void Game2::restartGame(){
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "resetGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
}

void Game2::on_pushButtonStart_clicked()
{
    gameTimerCounter = 0;
    obstacleOvercome = 0;
    collisionCounter = 0;

    gameTimer.start(60000);
    restartGame();
    ui->quickWidgetGame->setFocus();
}

void Game2::on_pushButtonStop_clicked()
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

void Game2::onObstacleOvercome(){
    obstacleOvercome++;
    qDebug() << gameTimerCounter << " минута, препятсвий пройдено: " << obstacleOvercome;
}

void Game2::onCollision(){
    collisionCounter++;
    qDebug() << gameTimerCounter << " минута, кол-во столкновений: " << collisionCounter;
    restartGame();
}

void Game2::on_spinBoxLVL_valueChanged(int arg1)
{
    lvl=arg1;
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        root->setProperty("speed", lvl);
        qDebug() << "Новая скорость: " << lvl;
    }

    ui->quickWidgetGame->setFocus();
}
