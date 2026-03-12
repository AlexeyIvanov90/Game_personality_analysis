#ifndef GAME_2_H
#define GAME_2_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class Game2;
}

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


    void restartGame();
    void onObstacleOvercome();
    void onCollision();
    void on_spinBoxLVL_valueChanged(int arg1);
    void onTimeout();
private:
    Ui::Game2 *ui;

    int lvl;

    double accuracy;
    double speed;

    int obstacleOvercome;
    int collisionCounter;

    int gameTimerCounter;
    QTimer gameTimer;

    QString gameInfo = "Цель игры: моделирование различных функциональных состояний, "
                       "определение уровня работоспобности в смоделированных функциональных состояниях\n"
                       "Инструкция: избегайте препятствий, количество препятствий растет с течением времени";

};

#endif // GAME_2_H
