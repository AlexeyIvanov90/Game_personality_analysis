#include <QQuickItem>
#include <QQmlEngine>
#include <QDateTime>

#include "game_3.h"
#include "ui_game_3.h"
#include "../../GUI/info_dialog/info_dialog.h"
#include "../../app_setting.h"
#include "../../bio_signal/analysis/heart_rate_variability/heart_rate_variability.h"

#define MAX_GAME_MINUTES  10
#define WINDOW_SIZE  10

#define MIN_LVL  1
#define MAX_LVL  38

#define MAX_BALL_TREMOR 60
#define MAX_BALL_SPEED 10

#define SUCCSESS_TO_NEXT_LVL  0
#define LEVEL_CORRECTION 0.8

#define MIN_ACCURACY_PERCENT_PER_MINUTE 50
#define STATE_BOUNDARY 200
#define THRESHOLD_VARIABILITI_EEG 0.5

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

    connect(&logTimer, &QTimer::timeout, this, &Game3::writeGameLog);
    logTimer.setInterval(5000);

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
    gameRun=true;
    accuracy=0.;
    accuracyPerMinuteCounter=0.;

    speed=0.;

    allHitCount=0;

    lvlVictoryPerMinuteCounter=0.;
    lvlCollisionPerMinuteCounter=0.;

    gameVictoryCounter=0;
    gameVictorystreak=0;
    gameMaxVictoryStreak=0;
    gameLossCounter=0;

    ballTremor=0; // дрожание шарика
    ballSpeed=0; // импульс шарика от клавиш

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
    gameMaxVictoryStreak=std::max(gameMaxVictoryStreak, gameVictorystreak);
    gameVictorystreak=0;
    gameLossCounter++;
    lvlCollisionPerMinuteCounter++;
    autoLevelCalculation(Game3Event::Collision);

    qDebug() << "Кол-во неудачных игр: " << gameLossCounter;
    startNewLvl();
}

void Game3::startNewGame(){
    if(gameRun)
        return;

    sendMessage("Старт игры", 1000);
    initGame();
    startNewLvl();
    startWriteGameLog();

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
        lvl = lvl*LEVEL_CORRECTION;
        qDebug() << "Уровень " <<  lvl << " зафиксирован";
    }
}

void Game3::levelCompleted(){
    sendMessage("Уровень пройден", 1000);
    gameVictorystreak++;
    gameVictoryCounter++;
    lvlVictoryPerMinuteCounter++;
    autoLevelCalculation(Game3Event::levelCompleted);
    startNewLvl();
}

void Game3::stopGame(){
    stopWriteGameLog();

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

    gameRun=false;
}

void Game3::setBallTremor(){
    if (game) {
        game->setProperty("ballTremor", ballTremor); // угол дрожания
        qDebug() << "Угол дрожания: " << ballTremor;
    }
}

void Game3::setBallSpeed(){
    if (game) {
        game->setProperty("ballSpeed", ballSpeed); // скорость
        qDebug() << "Новая скорость: " << ballSpeed;
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

void Game3::startWriteGameLog(){
    gameLogfile = new QFile(DIR_GAME_LOG "game3_" + QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss")+".csv");

    if (!gameLogfile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug() << "Файл не создан";
        delete gameLogfile;
        gameLogfile = nullptr;
        return;
    }

    gameLogStream = new QTextStream(gameLogfile);

    writeHeader();
    logTimer.start();
}

/*Выход: показатели БОС, подсчет попаданий, время реакции, количество игр подряд, сложность.
 * ЧСС (ЭКГ, ФПГ), показатели вариативности сердечного ритма (Mean RR, StDev RR, VSR общая мощность ритмов, HF, LF, VLF, ULF, индекс напряжения по Баевскому, все это на каждые 5 сек исследования),
*ЭЭГ (мощность альфа ритма, бетта-ритма, % альфа и бетта ритмов, соотношение, все показатели на каждые 5 сек исследования)
*/

void Game3::writeHeader(){
    if (!gameLogStream)
        return;

    *gameLogStream << "\"Time\","
                      "\"Mean RR\","
                      "\"StDev RR\","
                      "\"VSR\","
                      "\"HF\","
                      "\"LF\","
                      "\"VLF\","
                      "\"ULF\","
                      "\"Stress index\","
                      "\"power of alpha rhythm\","
                      "\"power of beta rhythm\","
                      "\"% of alpha rhythms\","
                      "\"% of beta rhythms\","
                      "\"Ratio\","
                      "\"Hit count\","
                      "\"Reaction time\","
                      "\"Number of games in a row\","
                      "\"Difficulty\"\n";

    gameLogStream->flush();
}

bool Game3::isLowVariability(resultEEG newEEG){
    previousEEG.push_back(newEEG);
    if (previousEEG.size() > WINDOW_SIZE) {
        previousEEG.erase(previousEEG.begin());
    }

    if (previousEEG.size() < WINDOW_SIZE)
        return false;

    double minVal = previousEEG[0].ratio;
    double maxVal = previousEEG[0].ratio;

    for (int i=0;i<previousEEG.size();i++) {
        if (previousEEG.at(i).ratio < minVal) minVal = previousEEG.at(i).ratio;
        if (previousEEG.at(i).ratio > maxVal) maxVal = previousEEG.at(i).ratio;
    }

    double range = maxVal - minVal;
    return range < THRESHOLD_VARIABILITI_EEG;
}

void Game3::writeGameLog(){
    if (!gameLogStream)
        return;

    speed = allHitCount/(60.*(gameTimerCounter+1));

    resultHeartRateVariability heartRateVariability;
    resultEEG EEG;

    if(heartRateVariability.SI>STATE_BOUNDARY)
        ballTremor++;
    else
        ballTremor--;
    setBallTremor();

    if(isLowVariability(EEG))
        ballSpeed++;
    else
        ballSpeed--;
    setBallSpeed();

    *gameLogStream << QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss")
                   << QString::number(heartRateVariability.M) << ","
                   << QString::number(heartRateVariability.SDNN) << ","
                   << QString::number(heartRateVariability.TP) << ","
                   << QString::number(heartRateVariability.HF) << ","
                   << QString::number(heartRateVariability.LF) << ","
                   << QString::number(heartRateVariability.VLF) << ","
                   << QString::number(heartRateVariability.ULF) << ","
                   << QString::number(heartRateVariability.SI) << ","

                   << QString::number(EEG.powerAlphaRhythm) << ","
                   << QString::number(EEG.powerBetaRhythm) << ","
                   << QString::number(EEG.alphaRhythms_percent) << ","
                   << QString::number(EEG.betaRhythms_percent) << ","
                   << QString::number(EEG.ratio) << ","

                   << QString::number(allHitCount) << ","
                   << QString::number(speed) << ","
                   << QString::number(gameMaxVictoryStreak) << ","
                   << QString::number(lvl) << "\n";

    gameLogStream->flush();
}

void Game3::stopWriteGameLog(){
    if (logTimer.isActive())
        logTimer.stop();

    if (gameLogStream) {
        gameLogStream->flush();
        delete gameLogStream;
        gameLogStream = nullptr;
    }

    if (gameLogfile) {
        if (gameLogfile->isOpen())
            gameLogfile->close();
        delete gameLogfile;
        gameLogfile = nullptr;
    }
}
