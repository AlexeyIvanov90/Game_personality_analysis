#ifndef GAME_1_H
#define GAME_1_H

#include <QMainWindow>

namespace Ui {
class Game1;
}

class Game1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Game1(QWidget *parent = nullptr);
    ~Game1();

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonInfo_clicked();

private:
    QString gameInfo = "Цель игры: оценка визуальной памяти(ПВК-тест)\n"
                       "Инструкция: на экране появляется серия разноцветных мишеней разной формы и размера."
                       "Необходимо как можно быстрее кликнуть мышью в зонах появления мишени";
    Ui::Game1 *ui;
};

#endif // GAME_1_H
