import QtQuick 2.12

Item {
    signal onHit()
    signal onCollision()

    visible: true
    width: 800
    height: 600

    // Параметры игры
    property int ballRadius: 20
    property double ballSpeed: 5          // импульс от клавиш
    property double friction: 0.98         // трение
    property bool gameActive: true

    // Скорость шарика по осям
    property double vx: 0
    property double vy: 0

    // Модели данных
    ListModel { id: obstacleModel }
    ListModel { id: targetModel }

    // ----- Функции проверки столкновений -----
    function circleRectCollision(cx, cy, r, rect) {
        var closestX = Math.max(rect.x, Math.min(cx, rect.x + rect.width));
        var closestY = Math.max(rect.y, Math.min(cy, rect.y + rect.height));
        var dx = cx - closestX;
        var dy = cy - closestY;
        return (dx * dx + dy * dy) < r * r;
    }

    function circleCircleCollision(x1, y1, r1, x2, y2, r2) {
        var dx = x1 - x2;
        var dy = y1 - y2;
        var dist = Math.sqrt(dx*dx + dy*dy);
        return dist < r1 + r2;
    }

    // ----- Инициализация / сброс -----
    function startGame() {
        obstacleModel.clear();
        targetModel.clear();

        obstacleModel.append({x: 200, y: 150, width: 80, height: 60});
        obstacleModel.append({x: 500, y: 300, width: 100, height: 40});
        obstacleModel.append({x: 300, y: 400, width: 60, height: 100});

        targetModel.append({x: 100, y: 100, radius: 15});
        targetModel.append({x: 700, y: 100, radius: 15});
        targetModel.append({x: 400, y: 200, radius: 15});
        targetModel.append({x: 600, y: 500, radius: 15});
        targetModel.append({x: 150, y: 500, radius: 15});

        ball.x = 400;
        ball.y = 300;
        vx = 0;
        vy = 0;
        gameActive = true;
    }

    // ----- Игровые объекты -----
    Rectangle {
        id: ball
        x: 400; y: 300
        width: ballRadius*2; height: ballRadius*2
        radius: ballRadius
        color: "blue"
        visible: gameActive
    }

    Repeater {
        model: obstacleModel
        Rectangle {
            x: model.x; y: model.y
            width: model.width; height: model.height
            color: "red"
            visible: gameActive
        }
    }

    Repeater {
        model: targetModel
        Rectangle {
            x: model.x - model.radius; y: model.y - model.radius
            width: model.radius*2; height: model.radius*2
            radius: model.radius
            color: "green"
            visible: gameActive
        }
    }

    // ----- Интерфейс: счёт и сообщение о проигрыше -----
    Text {
        id: gameOverText
        text: "Ты проиграл!"
        visible: !gameActive
        anchors.centerIn: parent
        font.pixelSize: 28
        color: "red"
        z: 10
    }

    // ----- Управление только с клавиатуры -----
    Item {
        id: inputHandler
        anchors.fill: parent
        focus: true   // захватываем фокус при старте

        Keys.onPressed: {
            if (!gameActive) return;
            if (event.key === Qt.Key_Left || event.key === Qt.Key_A) {
                vx -= ballSpeed;
            } else if (event.key === Qt.Key_Right || event.key === Qt.Key_D) {
                vx += ballSpeed;
            } else if (event.key === Qt.Key_Up || event.key === Qt.Key_W) {
                vy -= ballSpeed;
            } else if (event.key === Qt.Key_Down || event.key === Qt.Key_S) {
                vy += ballSpeed;
            }
        }
    }

    // ----- Таймер игрового цикла -----
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: {
            if (!gameActive) return;

            vx *= friction;
            vy *= friction;

            var newX = ball.x + vx;
            var newY = ball.y + vy;

            // Границы с отскоком
            if (newX < 0) { newX = 0; vx = -vx * 0.5; }
            if (newX > parent.width - ball.width) { newX = parent.width - ball.width; vx = -vx * 0.5; }
            if (newY < 0) { newY = 0; vy = -vy * 0.5; }
            if (newY > parent.height - ball.height) { newY = parent.height - ball.height; vy = -vy * 0.5; }

            ball.x = newX;
            ball.y = newY;

            // Проверка столкновений с препятствиями
            for (var i = 0; i < obstacleModel.count; i++) {
                var obs = obstacleModel.get(i);
                if (circleRectCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius,
                                        {x: obs.x, y: obs.y, width: obs.width, height: obs.height})) {
                    gameActive = false;
                    onCollision()
                    return;
                }
            }

            // Проверка сбора целей
            for (var j = targetModel.count - 1; j >= 0; j--) {
                var tar = targetModel.get(j);
                if (circleCircleCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius,
                                          tar.x, tar.y, tar.radius)) {
                    targetModel.remove(j);
                    onHit()
                    // Если цели кончились – добавляем новые случайные
                    if (targetModel.count === 0) {
                        for (var k = 0; k < 5; k++) {
                            targetModel.append({
                                x: 15 + Math.random() * (parent.width - 30),
                                y: 15 + Math.random() * (parent.height - 30),
                                radius: 15
                            });
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        inputHandler.forceActiveFocus(); // дополнительно форсируем фокус
    }
}
