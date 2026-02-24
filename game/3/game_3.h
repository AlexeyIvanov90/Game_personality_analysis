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

private:
    Ui::Game3 *ui;
    QString gameInfo = "Цель игры: определение \"человеческого фактора\", связь его с биоритмами\n"
                       "Инструкция: управляйте шариком, избегая препятствия и собирая цели по траектории";
};

#endif // GAME_3_H
