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

private:
    Ui::Game3 *ui;
};

#endif // GAME_3_H
