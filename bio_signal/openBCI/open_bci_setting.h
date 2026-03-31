#ifndef OPEN_BCI_SETTING_H
#define OPEN_BCI_SETTING_H

#include <QDialog>
#include "openBCI_manager.h"

namespace Ui {
class DialogOpenBciSetting;
}

class DialogOpenBciSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOpenBciSetting(openBCISetting setting, QWidget *parent = nullptr);
    ~DialogOpenBciSetting();
    openBCISetting getOpenBCISetting();

private:
    Ui::DialogOpenBciSetting *ui;
};

#endif // OPEN_BCI_SETTING_H
