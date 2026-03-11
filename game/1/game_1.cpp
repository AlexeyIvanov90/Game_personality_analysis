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
    ,accuracy(0.)
    ,speed(0.)
    ,lvl(5)
    ,gameTimerCounter(0)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    QString qmlPath = "C:/Qt/5.14.2/mingw73_64/qml";
    ui->quickWidgetGame1->engine()->addImportPath(qmlPath);
    qDebug() << "Debug: добавлен путь к QML:" << qmlPath;
#endif

    ui->quickWidgetGame1->setSource(QUrl("qrc:/game/1/game_1.qml"));

    if (ui->quickWidgetGame1->status() == QQuickWidget::Error) {
        qDebug() << "Ошибка загрузки QML";
    }

    QObject *root = ui->quickWidgetGame1->rootObject();
    if (root) {
        connect(root, SIGNAL(hitShape()), this, SLOT(onHit()));
        connect(root, SIGNAL(missShape()), this, SLOT(onMiss()));
        connect(root, SIGNAL(levelCompleted()), this, SLOT(restartGame()));
    } else {
        qDebug() << "Не удалось получить корневой объект QML";
    }

    connect(&gameTimer, &QTimer::timeout, this, &Game1::onTimeout);
}

Game1::~Game1()
{
    delete ui;
}

void Game1::onTimeout()
{
    gameTimerCounter++;

    if (gameTimerCounter >= 5) {
        on_pushButtonStop_clicked();
        qDebug() << "Игра окончена";
    }else{
        if((hitCount+missCount!=0))
            accuracy = ((double)hitCount/(hitCount+missCount))*100.;
        else
            accuracy = 0.;

        speed = hitCount/60.;

        ui->labelAccuracy->setText("Точность: " + QString::number(accuracy) + "%");
        ui->labelSpeed->setText("Попаданий/сек: " + QString::number(speed));

        hitCount=0;
        missCount=0;
    }
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
    lvl = arg1;
}

void Game1::on_pushButtonStart_clicked()
{
    gameTimer.start(60000);
    restartGame();
}

void Game1::onHit()
{
    hitCount++;
    qDebug() << "Попадание, всего попаданий за " << gameTimerCounter << " мин.: " << hitCount;
}

void Game1::onMiss()
{
    missCount++;       
    qDebug() << "Промах, всего промахов за " << gameTimerCounter << " мин.: " << missCount;
    if(gameTimerCounter!=0 && accuracy<50.)
        on_pushButtonStart_clicked();
}

void Game1::restartGame(){
    QObject *root = ui->quickWidgetGame1->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "startGame",
                                                 Q_ARG(QVariant, lvl));
        if (!success) {
            qDebug() << "Не удалось вызвать функцию startGame";
        }
    }
}

void Game1::on_pushButtonStop_clicked()
{
    gameTimer.stop();
    QObject *root = ui->quickWidgetGame1->rootObject();
    if (root) {
        bool success = QMetaObject::invokeMethod(root, "stopGame");
        if (!success) {
            qDebug() << "Не удалось вызвать функцию stopGame";
        }
    }
}
