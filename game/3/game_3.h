#ifndef GAME_3_H
#define GAME_3_H

#include <QMainWindow>

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

    void onCollision();
    void onHit();

    void on_spinBoxBallSpeed_valueChanged(int arg1);

    void on_doubleSpinBoxFriction_valueChanged(double arg1);

private:
    Ui::Game3 *ui;
    QString gameInfo = "Цель игры: определение \"человеческого фактора\", связь его с биоритмами\n"
                       "Инструкция: управляйте шариком, избегая препятствия и собирая цели по траектории";
    int collisionCounter=0;
    int hitCounter=0;
    int ballSpeed=5; // импульс от клавиш
    double friction=0.98; // трение (замедление)
};

#endif // GAME_3_H
