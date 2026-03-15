#ifndef GAME_3_H
#define GAME_3_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class Game3;
}

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

    void on_spinBoxBallImpulse_valueChanged(int arg1);

    void on_spinBoxBallTremor_valueChanged(int arg1);

    void on_spinBoxLvl_valueChanged(int arg1);

    void restartGame();
    void onHit();
    void onCollision();
    void onTimeout();

private:
    Ui::Game3 *ui;

    double accuracy;
    double speed;

    int hitCounter;
    int collisionCounter;

    int ballTremor; // дрожание шарика
    int ballImpulse; // импульс шарика от клавиш

    int lvl;

    int gameTimerCounter;
    QTimer gameTimer;

    QString gameInfo = "Цель игры: определение \"человеческого фактора\", связь его с биоритмами\n"
                       "Инструкция: управляйте шариком, избегая препятствия и собирая цели по траектории";
};

#endif // GAME_3_H
