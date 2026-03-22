#include <QQuickItem>
#include <QQmlEngine>
#include <QDateTime>

#include "game_3.h"
#include "ui_game_3.h"
#include "../../GUI/info_dialog/info_dialog.h"

#define MAX_GAME_MINUTES  10

#define MIN_LVL  5
#define MAX_LVL  38
#define SUCCSESS_TO_NEXT_LVL  0
#define PERCENT_OF_MAXIMUM_LEVEL 0.8

#define MIN_ACCURACY_PERCENT_PER_MINUTE 50

Game3::Game3(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game3)
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

    game = ui->quickWidgetGame->rootObject();

    if (game) {
        // Подключаем сигналы из QML к слотам C++
        connect(game, SIGNAL(onHit()), this, SLOT(onHit()));
        connect(game, SIGNAL(onCollision()), this, SLOT(onCollision()));
        connect(game, SIGNAL(levelCompleted()), this, SLOT(levelCompleted()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game3::onTimeout);
    gameTimer.setInterval(60000);

    connect(&displayedGameTimer, &QTimer::timeout, this, &Game3::updateDisplayedGameTime);
    displayedGameTimer.setInterval(1000);

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
    startNewGame();
    ui->quickWidgetGame->setFocus();
}

void Game3::on_pushButtonStop_clicked()
{
    stopGame();
}

void Game3::initGame(){
    accuracy=0.;
    accuracyPerMinuteCounter=0.;

    speed=0.;

    allHitCount=0;

    lvlVictoryPerMinuteCounter=0.;
    lvlCollisionPerMinuteCounter=0.;

    gameVictoryCounter=0;
    gameLossCounter=0;

    ballTremor=0; // дрожание шарика
    ballImpulse=0; // импульс шарика от клавиш

    lvl=MIN_LVL;
    autoLvl=true;

    gameTimerCounter=0;

    startGameTime =  QDateTime::currentMSecsSinceEpoch();

    gameTimer.start();
    displayedGameTimer.start();
}

void Game3::onHit(){
    allHitCount++;
    qDebug() << "Кол-во попаданий: " << allHitCount;
}

void Game3::onCollision(){
    sendMessage("Столкновение", 1000);
    gameLossCounter++;
    lvlCollisionPerMinuteCounter++;
    autoLevelCalculation(Game3Event::Collision);

    qDebug() << "Кол-во неудачных игр: " << gameLossCounter;
    startNewLvl();
}

void Game3::startNewGame(){
    sendMessage("Старт игры", 1000);
    initGame();
    startNewLvl();
}

void Game3::startNewLvl(){
    if(lvl>MAX_LVL)
        lvl=MAX_LVL;

    if(lvl<MIN_LVL)
        lvl=MIN_LVL;

    if (game) {
        bool success = QMetaObject::invokeMethod(game, "startGame", Q_ARG(QVariant, lvl));
        if (!success)
            qDebug() << "Не удалось вызвать функцию startGame";
    }
}

void Game3::autoLevelCalculation(Game3Event event){
    if(gameTimerCounter<1 && event == Game3Event::levelCompleted){
        lvl++;
        qDebug() << "Уровень повышен до " <<  lvl;
    }

    if(autoLvl && gameTimerCounter==1){
        autoLvl=false;
        lvl = lvl*PERCENT_OF_MAXIMUM_LEVEL;
        qDebug() << "Уровень " <<  lvl << " зафиксирован";
    }
}

void Game3::levelCompleted(){
    sendMessage("Уровень пройден", 1000);

    gameVictoryCounter++;
    lvlVictoryPerMinuteCounter++;
    autoLevelCalculation(Game3Event::levelCompleted);
    startNewLvl();
}

void Game3::stopGame(){
    if((gameVictoryCounter+gameLossCounter!=0))
        accuracy = ((double)gameVictoryCounter/(gameVictoryCounter+gameLossCounter))*100.;
    else
        accuracy = 0.;

    speed = allHitCount/(60.*(gameTimerCounter+1));

    sendMessage("Конец игры\nТочность " + QString::number(accuracy)+ " %\nСкорость " + QString::number(speed) + " объектов/сек");

    gameTimer.stop();
    displayedGameTimer.stop();
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "stopGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию stopGame";
        }
    }
}

void Game3::setBallTremor(){
    if (game) {
        game->setProperty("ballTremor", ballTremor); // угол дрожания
        qDebug() << "Угол дрожания: " << ballTremor;
    }
}
void Game3::setBallImpulse(){
    if (game) {
        game->setProperty("ballImpulse", ballImpulse); // импульс
        qDebug() << "Новый импульс: " << ballImpulse;
    }
}

void Game3::sendMessage(QString message, int sec){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "showTempMessage", Q_ARG(QVariant, message), Q_ARG(QVariant, sec) );
        if (!success)
            qDebug() << "Не удалось вызвать функцию showTempMessage";
    }
}

void Game3::onTimeout(){
    gameTimerCounter++;    

    if (gameTimerCounter >= MAX_GAME_MINUTES)
        stopGame();

    if((lvlVictoryPerMinuteCounter+lvlCollisionPerMinuteCounter!=0))
        accuracyPerMinuteCounter = ((double)lvlVictoryPerMinuteCounter/(lvlVictoryPerMinuteCounter+lvlCollisionPerMinuteCounter))*100.;
    else
        accuracyPerMinuteCounter = 0.;

    if(gameTimerCounter > 1 && accuracyPerMinuteCounter < MIN_ACCURACY_PERCENT_PER_MINUTE)
        stopGame();

    lvlVictoryPerMinuteCounter=0;
    lvlCollisionPerMinuteCounter=0;
}


void Game3::updateDisplayedGameTime(){
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedSeconds = (currentTime - startGameTime) / 1000;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->labelGameTimer->setText( QString("%1:%2")
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0')));
}

