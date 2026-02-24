#include <QFile>
#include <QJsonParseError>
#include <QDebug>
#include <QFileInfo>
#include <QSet>
#include <QDir>
#include <QPainter>

#include "app_utility.h"

QJsonArray readJsonArrayFromFile(const QString &filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << file.errorString();
        return {};
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Ошибка парсинга JSON:" << parseError.errorString();
        return {};
    }

    if (!doc.isArray()) {
        qWarning() << "JSON не является массивом";
        return {};
    }

    return doc.array();
}

QJsonObject readQJsonObjectFromFile(const QString &filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << file.errorString();
        return {};
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Ошибка парсинга JSON:" << parseError.errorString();
        return {};
    }

    if (!doc.isObject()) {
        qWarning() << "JSON не является объектом";
        return {};
    }

    return doc.object();
}
