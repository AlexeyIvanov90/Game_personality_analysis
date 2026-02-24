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

private:
    Ui::Game2 *ui;
    QString gameInfo = "Цель игры: моделирование различных функциональных состояний, "
                       "определение уровня работоспобности в смоделированных функциональных состояниях\n"
                       "Инструкция: избегайте препятствий, количество препятствий растет с течением времени";
};

#endif // GAME_2_H
