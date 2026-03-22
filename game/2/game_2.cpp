#include <QQuickItem>
#include <QQmlEngine>
#include <QDateTime>

#include "game_2.h"
#include "ui_game_2.h"
#include "../../GUI/info_dialog/info_dialog.h"

#define MAX_GAME_MINUTES  10

#define MIN_LVL  5
#define MAX_LVL  38
#define SUCCSESS_TO_NEXT_LVL  0
#define PERCENT_OF_MAXIMUM_LEVEL 0.8

#define MIN_ACCURACY_PERCENT_PER_MINUTE 50

Game2::Game2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game2)
    , lvl(5)
    , accuracy(0.)
    , speed(0.)
    , successCounter(0)
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

    game = ui->quickWidgetGame->rootObject();
    if (game) {
        // Подключаем сигналы из QML к слотам C++
        connect(game, SIGNAL(onSuccess()), this, SLOT(onSuccess()));
        connect(game, SIGNAL(onCollision()), this, SLOT(onCollision()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game2::onTimeout);
    gameTimer.setInterval(60000);

    connect(&displayedGameTimer, &QTimer::timeout, this, &Game2::updateDisplayedGameTime);
    displayedGameTimer.setInterval(1000);

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
    sendMessage("Старт игры", 1000);
    initGame();
    restartGame();
    ui->quickWidgetGame->setFocus();
}

void Game2::on_pushButtonStop_clicked()
{
    stopGame();
}

void Game2::initGame(){
    successCounter = 0;
    successPerMinuteCounter = 0;

    collisionCounter = 0;
    collisionPerMinuteCounter = 0;

    accuracy=0.;
    accuracyPerMinuteCounter = 0.;

    speed=0.;

    lvl=MIN_LVL;
    lvlSuccessCounter=0;
    updateLvl();

    autoLvl=false;
    gameTimerCounter=0;
    gameTimer.start();
    startGameTime =  QDateTime::currentMSecsSinceEpoch();
    displayedGameTimer.start();
}

void Game2::restartGame(){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "resetGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
}

void Game2::stopGame(){
    if((successCounter+collisionCounter!=0))
        accuracy = ((double)successCounter/(successCounter+collisionCounter))*100.;
    else
        accuracy = 0.;

    speed = successCounter/(60.*(gameTimerCounter+1));

    sendMessage("Конец игры\nТочность " + QString::number(accuracy)+ " %\nСкорость " + QString::number(speed) + " объектов/сек");

    gameTimer.stop();
    displayedGameTimer.stop();
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "stopGame");
        if (!success)
            qDebug() << "Не удалось вызвать функцию stopGame";
    }
}

void Game2::onSuccess(){
    qDebug() << "onSuccess";
    successCounter++;
    successPerMinuteCounter++;
    lvlSuccessCounter++;
    autoLevelCalculation(Game2Event::Success);
}

void Game2::onCollision(){
    qDebug() << "onCollision";
    collisionCounter++;
    collisionPerMinuteCounter++;
    lvlSuccessCounter=0;
    restartGame();
    autoLevelCalculation(Game2Event::Collision);
}


void Game2::updateLvl(){
    if(lvl>MAX_LVL)
        lvl=MAX_LVL;

    if(lvl<MIN_LVL)
        lvl=MIN_LVL;

    if (game) {
        game->setProperty("speed", lvl);
        qDebug() << "Новая скорость: " << lvl;
        lvlSuccessCounter=0;
    }
}

void Game2::autoLevelCalculation(Game2Event event){
    if(gameTimerCounter<1 && event == Game2Event::Success){
        if(lvlSuccessCounter >= SUCCSESS_TO_NEXT_LVL){
            lvl++;
            updateLvl();
            qDebug() << "Уровень повышен до " <<  lvl;
        }
    }

    if(autoLvl && gameTimerCounter==1){
        autoLvl=false;
        lvl = lvl*PERCENT_OF_MAXIMUM_LEVEL;
        updateLvl();
        qDebug() << "Уровень " <<  lvl << " зафиксирован";
    }
}

void Game2::sendMessage(QString message, int sec){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "showTempMessage", Q_ARG(QVariant, message), Q_ARG(QVariant, sec));
        if (!success)
            qDebug() << "Не удалось вызвать функцию showTempMessage";
    }
}

void Game2::onTimeout(){
    gameTimerCounter++;

    if(gameTimerCounter == MAX_GAME_MINUTES)
        stopGame();

    if((successPerMinuteCounter+collisionPerMinuteCounter!=0))
        accuracyPerMinuteCounter = ((double)successPerMinuteCounter/(successPerMinuteCounter+collisionPerMinuteCounter))*100.;
    else
        accuracyPerMinuteCounter = 0.;

    if(gameTimerCounter > 1 && accuracyPerMinuteCounter < MIN_ACCURACY_PERCENT_PER_MINUTE)
        stopGame();

    qDebug() << "successPerMinuteCounter" << successPerMinuteCounter;
    qDebug() << "collisionPerMinuteCounter" << collisionPerMinuteCounter;

    qDebug() << "accuracyPerMinuteCounter" << accuracyPerMinuteCounter;


    successPerMinuteCounter=0;
    collisionPerMinuteCounter=0;
}

void Game2::updateDisplayedGameTime(){
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedSeconds = (currentTime - startGameTime) / 1000;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->labelGameTimer->setText( QString("%1:%2")
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0')));
}

