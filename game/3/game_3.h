#ifndef GAME_3_H
#define GAME_3_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class Game3;
}

enum class Game3Event {
    levelCompleted,
    Collision
};

class Game3 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Game3(QWidget *parent = nullptr);
    ~Game3();

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonInfo_clicked();

    void on_pushButtonStart_clicked();    

    void on_pushButtonStop_clicked();

    void initGame();
    void startNewGame();
    void startNewLvl();
    void autoLevelCalculation(Game3Event event);
    void levelCompleted();
    void stopGame();

    void onHit();
    void onCollision();
    void setBallTremor();
    void setBallSpeed();

    void sendMessage(QString message, int sec=0);

    void onTimeout();
    void updateDisplayedGameTime();

private:
    Ui::Game3 *ui;

    double accuracy;
    int accuracyPerMinuteCounter;

    double speed;

    int allHitCount;

    int lvlVictoryPerMinuteCounter;
    int lvlCollisionPerMinuteCounter;

    int gameVictoryCounter;
    int gameLossCounter;

    int ballTremor; // дрожание шарика
    int ballSpeed; // импульс шарика от клавиш

    int lvl;
    int autoLvl;

    int gameTimerCounter;
    QTimer gameTimer;

    QTimer displayedGameTimer;
    qint64 startGameTime;

    QObject *game;

    QString gameInfo = "Цель игры: определение \"человеческого фактора\", связь его с биоритмами\n"
                       "Инструкция: управляйте шариком, избегая препятствия и собирая цели по траектории";
};

#endif // GAME_3_H
