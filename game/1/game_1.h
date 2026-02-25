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

    void on_spinBoxLVL_valueChanged(int arg1);

    void on_pushButtonStart_clicked();

    void onHit();
    void onMiss();
private:
    Ui::Game1 *ui;

    int hitCount;
    int hitTotal = 0;

    int missCount;
    int missTotal = 0;

    QString gameInfo = "Цель игры: оценка визуальной памяти(ПВК-тест)\n"
                       "Инструкция: на экране появляется серия разноцветных мишеней разной формы и размера."
                       "Необходимо как можно быстрее кликнуть мышью в зонах появления мишени";
};

#endif // GAME_1_H
