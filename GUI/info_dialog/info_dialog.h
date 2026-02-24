#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include <QDialog>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();
    void setInfo(const QString info);
private slots:
    void on_pushButtonClose_clicked();

private:
    Ui::InfoDialog *ui;
};

#endif // INFO_DIALOG_H
