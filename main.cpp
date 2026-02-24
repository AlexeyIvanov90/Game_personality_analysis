#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "app_setting.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Game_personality_analysis_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    AppSetting appSetting;

    MainWindow w;
    w.setStyleSheet(AppSetting::styleSheet);
    w.setWindowTitle("Методы психофизиологии для определения типа личности v" APP_VERSION);
    w.show();
    return a.exec();
}
