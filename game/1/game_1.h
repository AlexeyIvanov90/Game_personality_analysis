#ifndef GAME_1_H
#define GAME_1_H

#include <QMainWindow>
#include <QTimer>

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

    void on_pushButtonStop_clicked();

    void initGame();
    void startNewLvl();
    void onHit();
    void onMiss();
    void levelCompleted();
    void autoLevelCalculation();
    void startNewGame();
    void stopGame();
    void onTimeout();    
private:
    Ui::Game1 *ui;

    int hitCount;
    int hitPerMinuteCounter;
    int allHitCount;

    int missCount;
    int missPerMinuteCounter;
    int allMissCount;

    double accuracy;
    int accuracyPerMinuteCounter;
    double speed;    

    int lvl;
    bool autoLvl;
    int gameCount;

    QTimer gameTimer;
    int gameTimerCounter;
    QObject* game;
    QString gameInfo = "Цель игры: оценка визуальной памяти(ПВК-тест)\n"
                       "Инструкция: на экране появляется серия разноцветных мишеней разной формы и размера."
                       "Необходимо как можно быстрее кликнуть мышью в зонах появления мишени";
};

#endif // GAME_1_H
