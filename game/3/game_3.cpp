#include <QQuickItem>
#include <QQmlEngine>

#include "game_3.h"
#include "ui_game_3.h"
#include "../../GUI/info_dialog/info_dialog.h"

Game3::Game3(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game3)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml"; // Укажите ВАШ путь!
    ui->quickWidgetGame->engine()->addImportPath(qmlPath);
    qDebug() << "Debug: добавлен путь к QML:" << qmlPath;
#endif
    ui->quickWidgetGame->setSource(QUrl("qrc:/game/3/game_3.qml"));

    if (ui->quickWidgetGame->status() == QQuickWidget::Error) {
        qDebug() << "Ошибка загрузки QML";
    }

    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        // Подключаем сигналы из QML к слотам C++
        connect(root, SIGNAL(onHit()), this, SLOT(onHit()));
        connect(root, SIGNAL(onCollision()), this, SLOT(onCollision()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }
    ui->quickWidgetGame->setFocus();
}

Game3::~Game3()
{
    delete ui;
}

void Game3::on_pushButtonClose_clicked()
{
    close();
}

void Game3::on_pushButtonInfo_clicked()
{
    InfoDialog id;
    id.setInfo(gameInfo);
    id.exec();
    ui->quickWidgetGame->setFocus();
}

void Game3::on_pushButtonStart_clicked()
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "startGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
    ui->quickWidgetGame->setFocus();
}

void Game3::onCollision(){
    collisionCounter++;
    qDebug() << "Кол-во столкновений: " << collisionCounter;
}

void Game3::onHit(){
    hitCounter++;
    qDebug() << "Кол-во попаданий: " << hitCounter;
}

void Game3::on_spinBoxBallSpeed_valueChanged(int arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        ballSpeed=arg1;
        root->setProperty("ballSpeed", ballSpeed); // установить импульс
        qDebug() << "Новый импульс: " << ballSpeed;
    }
}

void Game3::on_doubleSpinBoxFriction_valueChanged(double arg1)
{
    QObject *root = ui->quickWidgetGame->rootObject();
    if (root) {
        friction=arg1;
        root->setProperty("friction", friction); // установить трение
        qDebug() << "Новое трение: " << friction;
    }
}
