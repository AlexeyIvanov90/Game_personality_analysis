#ifndef APP_UTILITY_H
#define APP_UTILITY_H

#include <QJsonArray>
#include <QJsonObject>

QJsonArray readJsonArrayFromFile(const QString &filePath);
QJsonObject readQJsonObjectFromFile(const QString &filePath);

#endif // APP_UTILITY_H
