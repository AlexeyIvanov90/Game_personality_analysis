#ifndef GAME_1_H
#define GAME_1_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class Game1;
}

enum class Game1Event {
    Hit,
    Miss
};

class Game1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Game1(QWidget *parent = nullptr);
    ~Game1();

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonInfo_clicked();

    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

    void initGame();
    void startNewLvl();
    void onHit();
    void onMiss();
    void levelCompleted();
    void autoLevelCalculation(Game1Event event);
    void startNewGame();
    void stopGame();
    void onTimeout();
    void sendMessage(QString message, int sec=0);
    void updateDisplayedGameTime();
private:
    Ui::Game1 *ui;

    int lvlVictoryPerMinuteCounter;
    int allHitCount;

    int lvlLossPerMinuteCounter;

    double accuracy;
    int accuracyPerMinuteCounter;
    double speed;    

    int lvl;
    bool autoLvl;

    int gameVictoryCounter;
    int gameLossCounter;

    QTimer gameTimer;
    QTimer displayedGameTimer;
    qint64 startGameTime;

    int gameTimerCounter;
    QObject* game;
    QString gameInfo = "Цель игры: оценка визуальной памяти(ПВК-тест)\n"
                       "Инструкция: на экране появляется серия разноцветных мишеней разной формы и размера."
                       "Необходимо как можно быстрее кликнуть мышью в зонах появления мишени";
};

#endif // GAME_1_H
