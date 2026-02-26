#ifndef GAME_2_H
#define GAME_2_H

#include <QMainWindow>

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

    void onJump(int accuracy);
    void onCollision();
    void on_spinBoxLVL_valueChanged(int arg1);
private:
    Ui::Game2 *ui;
    QString gameInfo = "Цель игры: моделирование различных функциональных состояний, "
                       "определение уровня работоспобности в смоделированных функциональных состояниях\n"
                       "Инструкция: избегайте препятствий, количество препятствий растет с течением времени";

    int jumpCounter=0;
    int collisionCounter=0;
};

#endif // GAME_2_H
