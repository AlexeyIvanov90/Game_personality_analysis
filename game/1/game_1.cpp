#include <QDebug>
#include <QQmlEngine>
#include <QQuickItem>
#include <QDateTime>

#include "game_1.h"
#include "ui_game_1.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game1::Game1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game1)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml";
    ui->quickWidgetGame1->engine()->addImportPath(qmlPath);
#endif

    ui->quickWidgetGame1->setSource(QUrl("qrc:/game/1/game_1.qml"));

    if (ui->quickWidgetGame1->status() == QQuickWidget::Error)
        qDebug() << "Ошибка загрузки QML";

    game = ui->quickWidgetGame1->rootObject();
    if (game) {
        connect(game, SIGNAL(hitShape()), this, SLOT(onHit()));
        connect(game, SIGNAL(missShape()), this, SLOT(onMiss()));
        connect(game, SIGNAL(levelCompleted()), this, SLOT(levelCompleted()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game1::onTimeout);
    gameTimer.setInterval(60000);

    connect(&displayedGameTimer, &QTimer::timeout, this, &Game1::updateDisplayedGameTime);
    displayedGameTimer.setInterval(1000);
}

Game1::~Game1()
{
    stopGame();
    delete ui;
}

void Game1::on_pushButtonClose_clicked()
{
    close();
}

void Game1::on_pushButtonInfo_clicked()
{
    InfoDialog id;
    id.setInfo(gameInfo);
    id.exec();
}

void Game1::on_pushButtonStart_clicked()
{
    startNewGame();
}

void Game1::on_pushButtonStop_clicked()
{
    stopGame();
}

void Game1::onHit()
{
    allHitCount++;
    hitPerMinuteCounter++;
    hitCount++;
}

void Game1::onMiss()
{
    missCount++;
    missPerMinuteCounter++;
    allMissCount++;
}

void Game1::initGame(){
    hitCount=0;
    hitPerMinuteCounter=0;
    allHitCount=0;
    missCount=0;
    missPerMinuteCounter=0;
    allMissCount=0;
    accuracy=0.;
    accuracyPerMinuteCounter=0.;
    speed=0.;
    lvl=2;
    autoLvl=true;
    gameCount=0;
    gameTimerCounter=0;
    startGameTime =  QDateTime::currentMSecsSinceEpoch();
}

void Game1::startNewLvl(){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "startGame", Q_ARG(QVariant, lvl));
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
}

void Game1::levelCompleted(){
    autoLevelCalculation();
    startNewLvl();

    hitCount=0;
    missCount=0;
}

void Game1::autoLevelCalculation(){
    if(gameTimerCounter<1){
        double lvlAccuracy;
        if((hitCount+missCount!=0))
            lvlAccuracy = ((double)hitCount/(hitCount+missCount))*100.;
        else
            lvlAccuracy = 0.;

        if(lvlAccuracy>90.){
            lvl++;
            sendMessage("Уровень изменён на " + QString::number(lvl), 1000);
        }

        qDebug() << "Точность за уровень: " << lvlAccuracy << " авто уровень: " << lvl;
    }else if(autoLvl){
        autoLvl=false;
        lvl = lvl*0.85;
        qDebug() << "Уровень " <<  lvl << " зафиксирован";
    }
}

void Game1::startNewGame(){
    sendMessage("Старт игры", 1000);

    initGame();
    gameTimer.start();
    displayedGameTimer.start();
    startNewLvl();
}

void Game1::stopGame(){
    sendMessage("Конец игры\nТочность " + QString::number(accuracy)+ " %\nСкорость " + QString::number(speed) + " объектов/сек");

    gameTimer.stop();
    displayedGameTimer.stop();
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "stopGame");
        if (!success)
            qDebug() << "Не удалось вызвать функцию stopGame";
    }
}

void Game1::sendMessage(QString message, int sec){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "showTempMessage", Q_ARG(QVariant, message), Q_ARG(QVariant, sec) );
        if (!success)
            qDebug() << "Не удалось вызвать функцию showTempMessage";
    }
}

void Game1::onTimeout()
{
    gameTimerCounter++;

    if(gameTimerCounter == 10)
        stopGame();

    if((allHitCount+allMissCount!=0))
        accuracy = ((double)allHitCount/(allHitCount+allMissCount))*100.;
    else
        accuracy = 0.;

    speed = allHitCount/(60.*(gameTimerCounter+1));

    if((hitPerMinuteCounter+missPerMinuteCounter!=0))
        accuracyPerMinuteCounter = ((double)hitPerMinuteCounter/(hitPerMinuteCounter+missPerMinuteCounter))*100.;
    else
        accuracyPerMinuteCounter = 0.;

    if(gameTimerCounter > 1 && accuracyPerMinuteCounter < 50.)
        stopGame();

    hitPerMinuteCounter=0;
    missPerMinuteCounter=0;
}

void Game1::updateDisplayedGameTime(){
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedSeconds = (currentTime - startGameTime) / 1000;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->labelGameTimer->setText( QString("%1:%2")
                       .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0')));
}

