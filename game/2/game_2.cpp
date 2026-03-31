#include <QQuickItem>
#include <QQmlEngine>
#include <QDateTime>

#include "game_2.h"
#include "ui_game_2.h"
#include "../../GUI/info_dialog/info_dialog.h"
#include "../../app_setting.h"
#include "../../bio_signal/analysis/heart_rate_variability/heart_rate_variability.h"
#include "../../bio_signal/analysis/EEG/eeg.h"

#define MAX_GAME_MINUTES  10

#define MIN_LVL  5
#define MAX_LVL  38
#define SUCCSESS_TO_NEXT_LVL  0
#define LEVEL_CORRECTION 0.8

#define MIN_ACCURACY_PERCENT_PER_MINUTE 50

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

    connect(&logTimer, &QTimer::timeout, this, &Game2::writeGameLog);
    logTimer.setInterval(5000);

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
    startNewGame();
    ui->quickWidgetGame->setFocus();
}

void Game2::on_pushButtonStop_clicked()
{
    stopGame();
}

void Game2::initGame(){
    gameRun=true;
    successCounter = 0;
    successPerMinuteCounter = 0;

    collisionCounter = 0;
    collisionPerMinuteCounter = 0;

    accuracy=0.;
    accuracyPerMinuteCounter = 0.;

    gameVictorystreak=0;
    gameMaxVictoryStreak=0;

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

void Game2::startNewGame(){
    if(gameRun)
        return;

    sendMessage("Старт игры", 1000);
    initGame();
    restartGame();
    startWriteGameLog();
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
    stopWriteGameLog();

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
    gameRun=false;
}

void Game2::onSuccess(){
    qDebug() << "onSuccess";
    gameVictorystreak++;
    successCounter++;
    successPerMinuteCounter++;
    lvlSuccessCounter++;
    autoLevelCalculation(Game2Event::Success);
}

void Game2::onCollision(){
    qDebug() << "onCollision";
    gameMaxVictoryStreak=std::max(gameMaxVictoryStreak, gameVictorystreak);
    gameVictorystreak=0;
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
        lvl = lvl*LEVEL_CORRECTION;
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

void Game2::startWriteGameLog(){
    gameLogfile = new QFile(DIR_GAME_LOG "game2_" +  QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss")+".csv");

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

void Game2::writeHeader(){
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

void Game2::writeGameLog(){
    if (!gameLogStream)
        return;

    //canalECGOpenBCI canalEEGOpenBCI это данные с OpenBCI
    QVector<double> canalECGOpenBCI;
    QVector<double> canalEEGOpenBCI;

    HeartRateVariability heartRateVariabilityAnalaiser;
    heartRateVariabilityAnalaiser.setData(canalECGOpenBCI);
    EEG eegAnalaiser;
    eegAnalaiser.setData(canalEEGOpenBCI);

    resultHeartRateVariability heartRateVariability = heartRateVariabilityAnalaiser.calculate();
    resultEEG EEG = eegAnalaiser.calculate();

    speed = successCounter/(60.*(gameTimerCounter+1));

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

                   << QString::number(successCounter) << ","
                   << QString::number(speed) << ","
                   << QString::number(gameMaxVictoryStreak) << ","
                   << QString::number(lvl) << "\n";

    gameLogStream->flush();
}

void Game2::stopWriteGameLog(){
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
