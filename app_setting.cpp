#include <QJsonObject>
#include <QDebug>

#include "app_setting.h"
#include "app_utility.h"

bool AppSetting::fullScreen = false;
QString AppSetting::styleSheet = "";

AppSetting::AppSetting(){
    loadAppSetting();
}


AppSetting::~AppSetting(){
}

void AppSetting::loadAppSetting(){
    QJsonObject setting = readQJsonObjectFromFile(PATH_SETTING_APP);
    fullScreen = setting["FULL_SCREEN"].toBool();
    styleSheet = setting["STYLE_SHEET"].toString();
}

void AppSetting::print(){
    qDebug() << "fullScreen: " << fullScreen;
    qDebug() << "styleSheet: " << styleSheet;
}
