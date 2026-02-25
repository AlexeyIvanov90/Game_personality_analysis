#include <QDebug>
#include <QQmlEngine>
#include <QQuickItem>

#include "game_1.h"
#include "ui_game_1.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game1::Game1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game1)
    , hitCount(0)
    , missCount(0)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml"; // Укажите ВАШ путь!
    ui->quickWidgetGame1->engine()->addImportPath(qmlPath);
    qDebug() << "Debug: добавлен путь к QML:" << qmlPath;
#endif

    ui->quickWidgetGame1->setSource(QUrl("qrc:/game/1/game_1.qml"));

    if (ui->quickWidgetGame1->status() == QQuickWidget::Error) {
        qDebug() << "Ошибка загрузки QML";
    }

    QObject *root = ui->quickWidgetGame1->rootObject();
    if (root) {
        // Подключаем сигналы из QML к слотам C++
        connect(root, SIGNAL(hitShape()), this, SLOT(onHit()));
        connect(root, SIGNAL(missShape()), this, SLOT(onMiss()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }
}

Game1::~Game1()
{
    delete ui;
}

void Game1::on_pushButtonClose_clicked()
{
    close();
}


void Game1::on_pushButtonInfo_clicked()
{
    InfoDialog id;
    id.setInfo(gameInfo);
    id.exec();
}

void Game1::on_spinBoxLVL_valueChanged(int arg1)
{
}


void Game1::on_pushButtonStart_clicked()
{
    QObject *root = ui->quickWidgetGame1->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "populateModel",
                                                 Q_ARG(QVariant, ui->spinBoxLVL->text().toInt()));
        if (!success) {
            qDebug() << "Не удалось вызвать функцию resetGame";
        }
    }
}

void Game1::onHit()
{
    hitCount++;
    qDebug() << "Попадание: " << hitCount << "/" << ui->spinBoxLVL->text();
    if(hitCount == ui->spinBoxLVL->text().toInt()){
        hitCount=0;
        hitTotal++;
        ui->labelHit->setText(QString::number(hitTotal));
    }
}

void Game1::onMiss()
{
    missCount++;
    qDebug() << "Промах: " << hitCount << "/" << ui->spinBoxLVL->text();
    missCount=0;
    hitCount=0;

    missTotal++;
    ui->labelMiss->setText(QString::number(missTotal));
}
