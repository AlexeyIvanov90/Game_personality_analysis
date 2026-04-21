#ifndef OPEN_BCI_SETTING_H
#define OPEN_BCI_SETTING_H

#include <QDialog>
#include "openBCI_manager.h"

namespace Ui {
class DialogOpenBciSetting;
}

class QTimer;

class DialogOpenBciSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOpenBciSetting(openBCISetting setting, QWidget *parent = nullptr);
    ~DialogOpenBciSetting();
    openBCISetting getOpenBCISetting();

private slots:
    void on_pushButtonTestECG_clicked();

    void on_pushButtonEEG1_clicked();

    void on_pushButtonTestEEG2_clicked();

    void onTestPlotTimer();

private:
    enum class TestMode { None, Ecg, Eeg1, Eeg2 };

    void setupTestPlot();
    void toggleTest(TestMode mode);
    void stopTestAndRestoreBci();
    void applyTestUiState(TestMode activeOrNone);
    int channelForTestMode(TestMode mode) const;
    void updateTestGraph();

    Ui::DialogOpenBciSetting *ui;

    QTimer *testPlotTimer_ = nullptr;
    TestMode testMode_ = TestMode::None;
    bool bciWasRunningBeforeTest_ = false;
    openBCISetting bciSettingBackup_{};
};

bool loadOpenBciSettings(openBCISetting& out);
void saveOpenBciSettings(const openBCISetting& s);

#endif // OPEN_BCI_SETTING_H
