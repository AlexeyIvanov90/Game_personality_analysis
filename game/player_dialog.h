#ifndef PLAYER_DIALOG_H
#define PLAYER_DIALOG_H

#include <QDialog>


enum class PlayerType {
    Choleric,
    Sanguine,
    Phlegmatic,
    Melancholic
};

inline QString playerTypeToString(PlayerType key) {
    switch(key) {
    case PlayerType::Choleric:   return "Холерик";
    case PlayerType::Sanguine:   return "Сангвиник";
    case PlayerType::Phlegmatic: return "Флегматик";
    case PlayerType::Melancholic:return "Меланхолик";
    default:                     return "";
    }
}

struct playerParam{
    QString name;
    QString type;
};

class Player {
private:
    Player() = default;

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    playerParam pP;
public:
    static Player& getInstance() {
        static Player instance;
        return instance;
    }

    void setPlayer(playerParam in) {
        pP = in;
    }

    const playerParam getPlayer() {
        return pP;
    }
};

namespace Ui {
class PlayerDialog;
}

class PlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayerDialog(QWidget *parent = nullptr);
    ~PlayerDialog();
private slots:
    void on_pushButtonSave_clicked();

private:
    Ui::PlayerDialog *ui;
};

#endif // PLAYER_DIALOG_H
