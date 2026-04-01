#include <QDebug>
#include <QQmlEngine>
#include <QQuickItem>
#include <QDateTime>
#include <QDir>
#include <QLibraryInfo>

#include "game_1.h"
#include "ui_game_1.h"
#include "../../GUI/info_dialog/info_dialog.h"
#include "../../app_setting.h"
#include "../../bio_signal/analysis/heart_rate_variability/heart_rate_variability.h"
#include "../../bio_signal/analysis/EEG/eeg.h"
#include "../../bio_signal/openBCI/openBCI_manager.h"

#define MAX_GAME_MINUTES  10
#define MIN_ACCURACY_PERCENT_PER_MINUTE 50
#define START_LVL 3
#define LEVEL_CORRECTION  0.8

namespace {
constexpr int kSampleRateHz = 250;
constexpr int kLogWindowSec = 5;
constexpr int kLogWindowSamples = kSampleRateHz * kLogWindowSec;
}

Game1::Game1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game1)
{
    ui->setupUi(this);

    ui->quickWidgetGame->setSource(QUrl("qrc:/game/1/game_1.qml"));

    if (ui->quickWidgetGame->status() == QQuickWidget::Error)
        qDebug() << "Ошибка загрузки QML";

    game = ui->quickWidgetGame->rootObject();

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

    connect(&logTimer, &QTimer::timeout, this, &Game1::writeGameLog);
    logTimer.setInterval(5000);
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
    startWriteGameLog();
}

void Game1::on_pushButtonStop_clicked()
{
    stopGame();
}

void Game1::onHit(){
    allHitCount++;
}

void Game1::onMiss(){
    lvlLossPerMinuteCounter++;
    gameLossCounter++;
    gameMaxVictoryStreak=std::max(gameMaxVictoryStreak, gameVictorystreak);
    gameVictorystreak=0;
    autoLevelCalculation(Game1Event::Miss);
    startNewLvl();
}

void Game1::initGame(){
    gameRun=true;
    lvlVictoryPerMinuteCounter=0;
    allHitCount=0;
    lvlLossPerMinuteCounter=0;
    accuracy=0.;
    accuracyPerMinuteCounter=0.;
    speed=0.;
    lvl=START_LVL;
    autoLvl=true;
    gameVictoryCounter=0;
    gameVictorystreak=0;
    gameMaxVictoryStreak=0;
    gameLossCounter=0;
    gameTimerCounter=0;
    startGameTime =  QDateTime::currentMSecsSinceEpoch();
    gameTimer.start();
    displayedGameTimer.start();
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
    gameVictoryCounter++;
    lvlVictoryPerMinuteCounter++;
    gameVictorystreak++;
    autoLevelCalculation(Game1Event::Hit);
    startNewLvl();
}

void Game1::autoLevelCalculation(Game1Event event){
    if(gameTimerCounter<1 && event == Game1Event::Hit){
        lvl++;
        qDebug() << "Уровень повышен до " <<  lvl;
    }

    if(autoLvl && gameTimerCounter==1){
        autoLvl=false;
        lvl = lvl*LEVEL_CORRECTION;
        qDebug() << "Уровень " <<  lvl << " зафиксирован";
    }
}

void Game1::startNewGame(){
    if(gameRun)
        return;

    sendMessage("Старт игры", 1000);
    OpenBCIManager::instance().start();
    initGame();
    startNewLvl();
}

void Game1::stopGame(){
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
        if (!success)
            qDebug() << "Не удалось вызвать функцию stopGame";
    }

    stopWriteGameLog();
    OpenBCIManager::instance().stop();
    gameRun=false;
}

void Game1::startWriteGameLog(){
    if(logWrite)
        return;
    logWrite=true;

    QDir().mkpath(DIR_GAME_LOG);
    gameLogfile = new QFile(DIR_GAME_LOG "game1_" + QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss")+".csv");

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

void Game1::writeHeader(){
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


void Game1::writeGameLog(){
    if (!gameLogStream)
        return;

    //canalECGOpenBCI canalEEGOpenBCI это данные с OpenBCI
    QVector<double> canalECGOpenBCI = OpenBCIManager::instance().getLatestEcgWindow(kLogWindowSamples);
    QVector<double> canalEEGOpenBCI = OpenBCIManager::instance().getLatestEegWindow(kLogWindowSamples);

    HeartRateVariability heartRateVariabilityAnalaiser;
    heartRateVariabilityAnalaiser.setDataFromSensor(canalECGOpenBCI);
    EEG eegAnalaiser;
    eegAnalaiser.setDataFromSensor(canalEEGOpenBCI);

    resultHeartRateVariability heartRateVariability = heartRateVariabilityAnalaiser.calculate();
    resultEEG EEG = eegAnalaiser.calculate();

    speed = allHitCount/(60.*(gameTimerCounter+1));

    *gameLogStream << QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss") << ","
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

void Game1::stopWriteGameLog(){       
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
    logWrite=false;
}

void Game1::sendMessage(QString message, int milliseconds){
    if (game) {
        bool success = QMetaObject::invokeMethod(game, "showTempMessage", Q_ARG(QVariant, message), Q_ARG(QVariant, milliseconds) );
        if (!success)
            qDebug() << "Не удалось вызвать функцию showTempMessage";
    }
}

void Game1::onTimeout()
{
    gameTimerCounter++;

    if(gameTimerCounter == MAX_GAME_MINUTES)
        stopGame();

    if((lvlVictoryPerMinuteCounter+lvlLossPerMinuteCounter!=0))
        accuracyPerMinuteCounter = ((double)lvlVictoryPerMinuteCounter/(lvlVictoryPerMinuteCounter+lvlLossPerMinuteCounter))*100.;
    else
        accuracyPerMinuteCounter = 0.;

    if(gameTimerCounter > 1 && accuracyPerMinuteCounter < MIN_ACCURACY_PERCENT_PER_MINUTE)
        stopGame();

    lvlVictoryPerMinuteCounter=0;
    lvlLossPerMinuteCounter=0;
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
