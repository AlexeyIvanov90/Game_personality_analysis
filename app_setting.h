#ifndef APPSETTING_H
#define APPSETTING_H

#include <QString>

#define PATH_SETTING_APP "setting_app.json"

class AppSetting{
public:
    AppSetting();
    ~AppSetting();

    static bool fullScreen;
    static QString styleSheet;

private:
    void loadAppSetting();
    void print();
};

#endif // APPSETTING_H
