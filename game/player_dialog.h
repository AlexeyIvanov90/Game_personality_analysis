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

struct Player{
    static QString name;
    static QString type;
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
    Player getPlayerInfo();
private:
    Ui::PlayerDialog *ui;
    Player player;
};

#endif // PLAYER_DIALOG_H
