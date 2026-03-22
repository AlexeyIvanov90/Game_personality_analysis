import QtQuick 2.12
import QtQml 2.12

Item {
    signal onHit()
    signal onCollision()
    signal levelCompleted()
    visible: true
    width: 981
    height: 641

    // Параметры игры
    property int ballRadius: 20
    property double ballSpeed: 5           // постоянная скорость шарика
    property double ballTremor: 0          // дрожание шарика (диапазон отклонения в градусах)
    property int difficulty: 5             // количество препятствий и целей

    property bool gameActive: false
    property bool controlsEnabled: false   // флаг разрешения управления (включится через 1 секунду)
    property bool movementEnabled: false   // флаг движения (включится при первом нажатии после controlsEnabled)

    // Направление движения шарика (единичный вектор)
    property double directionX: 0
    property double directionY: 0

    // Модели данных
    ListModel { id: obstacleModel }
    ListModel { id: targetModel }

    // Компонент для отображения временных сообщений
    Rectangle {
        id: tempMessageBox
        anchors.centerIn: parent
        color: "#88000000"
        radius: 10
        visible: false
        z: 9999

        property alias text: messageText.text

        width: messageText.width + 40
        height: messageText.height + 20

        Text {
            id: messageText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 18
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Timer {
            id: messageTimer
            onTriggered: {
                if (interval > 0) {
                    tempMessageBox.visible = false
                }
            }
        }
    }

    // Таймер для задержки перед включением управления
    Timer {
        id: controlsEnableTimer
        interval: 1000  // 1 секунда
        onTriggered: {
            controlsEnabled = true;
        }
    }

    // Универсальная функция для отображения сообщений
    function showTempMessage(message, duration = 2000) {
        tempMessageBox.text = message
        tempMessageBox.visible = true

        if (duration > 0) {
            messageTimer.interval = duration
            messageTimer.start()
        } else if (duration === 0) {
            messageTimer.stop()
        }
    }

    // Функция для скрытия сообщения
    function hideTempMessage() {
        tempMessageBox.visible = false
        messageTimer.stop()
    }

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

    function rectRectCollision(rect1, rect2) {
        return !(rect2.x >= rect1.x + rect1.width ||
                 rect2.x + rect2.width <= rect1.x ||
                 rect2.y >= rect1.y + rect1.height ||
                 rect2.y + rect2.height <= rect1.y);
    }

    function isValidObstaclePosition(x, y, width, height, obstacles, ballPos, ballRad, gameWidth, gameHeight) {
        if (x < 20 || x + width > gameWidth - 20 ||
            y < 20 || y + height > gameHeight - 20) {
            return false;
        }

        for (var i = 0; i < obstacles.length; i++) {
            var obs = obstacles[i];
            if (rectRectCollision({x: x, y: y, width: width, height: height},
                                 {x: obs.x, y: obs.y, width: obs.width, height: obs.height})) {
                return false;
            }
        }

        if (circleRectCollision(ballPos.x + ballRad, ballPos.y + ballRad, ballRad * 2,
                               {x: x, y: y, width: width, height: height})) {
            return false;
        }

        return true;
    }

    function isValidTargetPosition(x, y, radius, targets, ballPos, ballRad, obstacles, gameWidth, gameHeight) {
        if (x - radius < 10 || x + radius > gameWidth - 10 ||
            y - radius < 10 || y + radius > gameHeight - 10) {
            return false;
        }

        for (var i = 0; i < targets.length; i++) {
            var tar = targets[i];
            var dx = x - tar.x;
            var dy = y - tar.y;
            var dist = Math.sqrt(dx*dx + dy*dy);
            if (dist < radius + tar.radius + 30) {
                return false;
            }
        }

        for (var j = 0; j < obstacles.length; j++) {
            var obs = obstacles[j];
            if (circleRectCollision(x, y, radius + 20,
                                   {x: obs.x, y: obs.y, width: obs.width, height: obs.height})) {
                return false;
            }
        }

        var dxBall = x - (ballPos.x + ballRad);
        var dyBall = y - (ballPos.y + ballRad);
        var distBall = Math.sqrt(dxBall*dxBall + dyBall*dyBall);
        if (distBall < radius + ballRad + 50) {
            return false;
        }

        return true;
    }

    function getRandomObstacleSize() {
        return {
            width: 50 + Math.floor(Math.random() * 70),
            height: 40 + Math.floor(Math.random() * 60)
        };
    }

    function applyTremor(dx, dy) {
        if (ballTremor === 0) {
            return {dx: dx, dy: dy};
        }

        var length = Math.sqrt(dx*dx + dy*dy);
        if (length === 0) {
            return {dx: 0, dy: 0};
        }

        var normDx = dx / length;
        var normDy = dy / length;
        var currentAngle = Math.atan2(normDy, normDx);
        var tremorRad = (Math.random() * 2 - 1) * ballTremor * Math.PI / 180;
        var newAngle = currentAngle + tremorRad;
        var newDx = Math.cos(newAngle);
        var newDy = Math.sin(newAngle);

        return {dx: newDx, dy: newDy};
    }

    function updateDirection(newDirX, newDirY) {
        // Обновляем направление только если управление активно и движение разрешено
        if (!controlsEnabled || !movementEnabled) return;

        var tremorResult = applyTremor(newDirX, newDirY);
        directionX = tremorResult.dx;
        directionY = tremorResult.dy;

        var len = Math.sqrt(directionX * directionX + directionY * directionY);
        if (len > 0) {
            directionX /= len;
            directionY /= len;
        }
    }

    // Функция для старта движения (вызывается при первом нажатии клавиши)
    function startMovement(firstDirX, firstDirY) {
        if (!controlsEnabled) return;
        if (movementEnabled) return;

        movementEnabled = true;

        // Устанавливаем начальное направление
        var tremorResult = applyTremor(firstDirX, firstDirY);
        directionX = tremorResult.dx;
        directionY = tremorResult.dy;

        var len = Math.sqrt(directionX * directionX + directionY * directionY);
        if (len > 0) {
            directionX /= len;
            directionY /= len;
        }
    }

    function startGame(lvl) {
        // Сбрасываем все флаги
        movementEnabled = false;
        controlsEnabled = false;

        // Останавливаем таймер, если был запущен
        controlsEnableTimer.stop();

        obstacleModel.clear();
        targetModel.clear();

        difficulty = lvl;

        var ballStartX = width / 2 - ballRadius;
        var ballStartY = height / 2 - ballRadius;
        var ballPos = {x: ballStartX, y: ballStartY};

        var tempObstacles = [];
        var maxAttempts = 1000;
        var gameWidth = width;
        var gameHeight = height;

        for (var i = 0; i < difficulty; i++) {
            var placed = false;

            for (var attempts = 0; attempts < maxAttempts && !placed; attempts++) {
                var size = getRandomObstacleSize();
                var obsX = 30 + Math.random() * (gameWidth - size.width - 60);
                var obsY = 30 + Math.random() * (gameHeight - size.height - 60);

                if (isValidObstaclePosition(obsX, obsY, size.width, size.height, tempObstacles, ballPos, ballRadius, gameWidth, gameHeight)) {
                    tempObstacles.push({x: obsX, y: obsY, width: size.width, height: size.height});
                    placed = true;
                }
            }

            if (!placed) {
                for (var attempt2 = 0; attempt2 < 100 && !placed; attempt2++) {
                    var fallbackX = 50 + Math.random() * (gameWidth - 150);
                    var fallbackY = 50 + Math.random() * (gameHeight - 150);
                    var fallbackSize = {width: 60, height: 50};

                    if (!circleRectCollision(ballPos.x + ballRadius, ballPos.y + ballRadius, ballRadius * 2,
                                           {x: fallbackX, y: fallbackY, width: fallbackSize.width, height: fallbackSize.height})) {

                        var intersects = false;
                        for (var k = 0; k < tempObstacles.length; k++) {
                            if (rectRectCollision({x: fallbackX, y: fallbackY, width: fallbackSize.width, height: fallbackSize.height},
                                                 {x: tempObstacles[k].x, y: tempObstacles[k].y,
                                                  width: tempObstacles[k].width, height: tempObstacles[k].height})) {
                                intersects = true;
                                break;
                            }
                        }

                        if (!intersects) {
                            tempObstacles.push({x: fallbackX, y: fallbackY,
                                               width: fallbackSize.width, height: fallbackSize.height});
                            placed = true;
                        }
                    }
                }
            }
        }

        for (var n = 0; n < tempObstacles.length; n++) {
            obstacleModel.append(tempObstacles[n]);
        }

        var tempTargets = [];

        for (var j = 0; j < difficulty; j++) {
            var placedTarget = false;

            for (var attemptsTarget = 0; attemptsTarget < maxAttempts && !placedTarget; attemptsTarget++) {
                var tarX = 40 + Math.random() * (gameWidth - 80);
                var tarY = 40 + Math.random() * (gameHeight - 80);
                var tarRadius = 12 + Math.floor(Math.random() * 16);

                if (isValidTargetPosition(tarX, tarY, tarRadius, tempTargets, ballPos, ballRadius, tempObstacles, gameWidth, gameHeight)) {
                    tempTargets.push({x: tarX, y: tarY, radius: tarRadius});
                    placedTarget = true;
                }
            }

            if (!placedTarget) {
                for (var attempt3 = 0; attempt3 < 100 && !placedTarget; attempt3++) {
                    var fallbackTarX = 60 + Math.random() * (gameWidth - 120);
                    var fallbackTarY = 60 + Math.random() * (gameHeight - 120);
                    var fallbackRadius = 15;

                    var dxBall = fallbackTarX - (ballPos.x + ballRadius);
                    var dyBall = fallbackTarY - (ballPos.y + ballRadius);
                    var distBall = Math.sqrt(dxBall*dxBall + dyBall*dyBall);

                    if (distBall >= fallbackRadius + ballRadius + 50) {
                        var intersects = false;
                        for (var k3 = 0; k3 < tempTargets.length; k3++) {
                            var dx = fallbackTarX - tempTargets[k3].x;
                            var dy = fallbackTarY - tempTargets[k3].y;
                            if (Math.sqrt(dx*dx + dy*dy) < fallbackRadius + tempTargets[k3].radius + 20) {
                                intersects = true;
                                break;
                            }
                        }

                        if (!intersects) {
                            tempTargets.push({x: fallbackTarX, y: fallbackTarY, radius: fallbackRadius});
                            placedTarget = true;
                        }
                    }
                }
            }
        }

        for (var m = 0; m < tempTargets.length; m++) {
            targetModel.append(tempTargets[m]);
        }

        ball.x = ballStartX;
        ball.y = ballStartY;

        directionX = 0;
        directionY = 0;

        gameActive = true;

        // Запускаем таймер задержки управления
        controlsEnableTimer.start();
    }

    function stopGame() {
        gameActive = false;
        movementEnabled = false;
        controlsEnabled = false;
        controlsEnableTimer.stop();
    }

    // Игровые объекты
    Rectangle {
        id: ball
        x: parent.width / 2 - ballRadius
        y: parent.height / 2 - ballRadius
        width: ballRadius * 2
        height: ballRadius * 2
        radius: ballRadius
        color: "blue"
        visible: gameActive

        // Добавляем эффект пульсации во время ожидания управления
        scale: (gameActive && !controlsEnabled) ? 1.2 :
               (gameActive && controlsEnabled && !movementEnabled) ? 1.1 : 1.0
        Behavior on scale {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }

        // Добавляем свечение для индикации состояния
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: !controlsEnabled ? "yellow" :
                         (controlsEnabled && !movementEnabled) ? "orange" : "transparent"
            border.width: 3
        }
    }

    Repeater {
        id: obstacleRepeater
        model: obstacleModel
        Rectangle {
            x: model.x
            y: model.y
            width: model.width
            height: model.height
            color: "red"
            border.color: "darkred"
            border.width: 2
            visible: gameActive
        }
    }

    Repeater {
        id: targetRepeater
        model: targetModel
        Rectangle {
            x: model.x - model.radius
            y: model.y - model.radius
            width: model.radius * 2
            height: model.radius * 2
            radius: model.radius
            color: "green"
            border.color: "darkgreen"
            border.width: 2
            visible: gameActive
        }
    }

    // Управление с клавиатуры
    Item {
        id: inputHandler
        anchors.fill: parent
        focus: true

        property var activeKeys: ({})

        function updateCombinedDirection() {
            var sumX = 0;
            var sumY = 0;

            if (activeKeys[16777234] || activeKeys[65]) sumX -= 1;  // Left или A
            if (activeKeys[16777236] || activeKeys[68]) sumX += 1;  // Right или D
            if (activeKeys[16777235] || activeKeys[87]) sumY -= 1;  // Up или W
            if (activeKeys[16777237] || activeKeys[83]) sumY += 1;  // Down или S

            if (sumX !== 0 || sumY !== 0) {
                // Если движение еще не начато, начинаем его с первого нажатия
                if (!movementEnabled && controlsEnabled) {
                    startMovement(sumX, sumY);
                } else if (movementEnabled) {
                    updateDirection(sumX, sumY);
                }
            }
        }

        Keys.onPressed: {
            if (!gameActive || !controlsEnabled) return;

            // Запоминаем нажатую клавишу
            activeKeys[event.key] = true;

            // Для одиночных нажатий без комбинаций
            if (!movementEnabled) {
                var singleDirX = 0;
                var singleDirY = 0;

                if (event.key === Qt.Key_Left || event.key === Qt.Key_A) singleDirX = -1;
                else if (event.key === Qt.Key_Right || event.key === Qt.Key_D) singleDirX = 1;
                else if (event.key === Qt.Key_Up || event.key === Qt.Key_W) singleDirY = -1;
                else if (event.key === Qt.Key_Down || event.key === Qt.Key_S) singleDirY = 1;

                if (singleDirX !== 0 || singleDirY !== 0) {
                    startMovement(singleDirX, singleDirY);
                }
            } else {
                updateCombinedDirection();
            }
        }

        Keys.onReleased: {
            if (!gameActive || !controlsEnabled || !movementEnabled) return;
            delete activeKeys[event.key];
            updateCombinedDirection();
        }
    }

    // Таймер игрового цикла
    Timer {
        id: gameLoopTimer
        interval: 16
        running: true
        repeat: true
        onTriggered: {
            if (!gameActive || !movementEnabled) return;

            // Движение с постоянной скоростью
            var newX = ball.x + directionX * ballSpeed;
            var newY = ball.y + directionY * ballSpeed;

            // Границы с отскоком
            if (newX < 0) {
                newX = 0;
                directionX = -directionX;
            }
            if (newX > parent.width - ball.width) {
                newX = parent.width - ball.width;
                directionX = -directionX;
            }
            if (newY < 0) {
                newY = 0;
                directionY = -directionY;
            }
            if (newY > parent.height - ball.height) {
                newY = parent.height - ball.height;
                directionY = -directionY;
            }

            ball.x = newX;
            ball.y = newY;

            // Проверка столкновений с препятствиями
            for (var i = 0; i < obstacleModel.count; i++) {
                var obs = obstacleModel.get(i);
                if (circleRectCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius,
                                        {x: obs.x, y: obs.y, width: obs.width, height: obs.height})) {
                    gameActive = false;
                    movementEnabled = false;
                    controlsEnabled = false;
                    onCollision();
                    return;
                }
            }

            // Проверка сбора целей
            for (var j = targetModel.count - 1; j >= 0; j--) {
                var tar = targetModel.get(j);
                if (circleCircleCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius,
                                          tar.x, tar.y, tar.radius)) {
                    targetModel.remove(j);
                    onHit();
                }
            }

            // Если цели кончились – завершаем уровень
            if (targetModel.count === 0 && gameActive) {
                gameActive = false;
                movementEnabled = false;
                controlsEnabled = false;
                levelCompleted();
            }
        }
    }
}
