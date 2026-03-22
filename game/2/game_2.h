#ifndef GAME_2_H
#define GAME_2_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class Game2;
}

enum class Game2Event {
    Success,
    Collision
};

class Game2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Game2(QWidget *parent = nullptr);
    ~Game2();

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonInfo_clicked();

    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

    void initGame();
    void restartGame();
    void autoLevelCalculation(Game2Event event);
    void updateLvl();
    void onSuccess();
    void onCollision();
    void onTimeout();
    void updateDisplayedGameTime();
    void sendMessage(QString message, int sec=0);
    void stopGame();

private:
    Ui::Game2 *ui;

    int lvl;
    int lvlSuccessCounter;
    bool autoLvl;

    double accuracy;
    double speed;

    int successCounter;
    int collisionCounter;

    int successPerMinuteCounter;
    int collisionPerMinuteCounter;
    double accuracyPerMinuteCounter;

    int gameTimerCounter;
    QTimer gameTimer;

    QTimer displayedGameTimer;
    qint64 startGameTime;

    QObject* game;

    QString gameInfo = "Цель игры: моделирование различных функциональных состояний, "
                       "определение уровня работоспобности в смоделированных функциональных состояниях\n"
                       "Инструкция: избегайте препятствий, количество препятствий растет с течением времени";
};

#endif // GAME_2_H
