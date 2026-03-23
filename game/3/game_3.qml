import QtQuick 2.12

Item {
    signal onHit()
    signal onCollision()
    signal levelCompleted()

    width: 981
    height: 641

    // Параметры игры
    property int ballRadius: 20
    property double ballSpeed: 10
    property double ballTremor: 0
    property int difficulty: 5

    property bool gameActive: false
    property bool controlsEnabled: false
    property bool movementEnabled: false

    property double directionX: 0
    property double directionY: 0

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

    // Функция для отображения сообщений
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

    // Таймер задержки управления
    Timer {
        id: controlsTimer
        interval: 1000
        onTriggered: controlsEnabled = true
    }

    // Функции столкновений
    function circleRectCollision(cx, cy, r, rect) {
        var closestX = Math.max(rect.x, Math.min(cx, rect.x + rect.width))
        var closestY = Math.max(rect.y, Math.min(cy, rect.y + rect.height))
        var dx = cx - closestX
        var dy = cy - closestY
        return (dx * dx + dy * dy) < r * r
    }

    function circleCircleCollision(x1, y1, r1, x2, y2, r2) {
        var dx = x1 - x2
        var dy = y1 - y2
        return Math.sqrt(dx*dx + dy*dy) < r1 + r2
    }

    function rectRectCollision(r1, r2) {
        return !(r2.x >= r1.x + r1.width ||
                 r2.x + r2.width <= r1.x ||
                 r2.y >= r1.y + r1.height ||
                 r2.y + r2.height <= r1.y)
    }

    function applyTremor(dx, dy) {
        if (ballTremor === 0) return {dx: dx, dy: dy}

        var len = Math.sqrt(dx*dx + dy*dy)
        if (len === 0) return {dx: 0, dy: 0}

        var angle = Math.atan2(dy/len, dx/len)
        angle += (Math.random() * 2 - 1) * ballTremor * Math.PI / 180
        return {dx: Math.cos(angle), dy: Math.sin(angle)}
    }

    function updateDirection(dx, dy) {
        if (!controlsEnabled || !movementEnabled) return
        if (dx === 0 && dy === 0) return

        var len = Math.sqrt(dx*dx + dy*dy)
        var tremor = applyTremor(dx/len, dy/len)

        directionX = tremor.dx
        directionY = tremor.dy

        len = Math.sqrt(directionX*directionX + directionY*directionY)
        if (len > 0) {
            directionX /= len
            directionY /= len
        }
    }

    function startGame(lvl) {
        movementEnabled = false
        controlsEnabled = false
        controlsTimer.stop()
        obstacleModel.clear()
        targetModel.clear()
        difficulty = lvl

        var ballPos = {x: width/2 - ballRadius, y: height/2 - ballRadius}
        var tempObstacles = []
        var tempTargets = []

        // Генерация препятствий
        for (var i = 0; i < difficulty; i++) {
            var placed = false
            for (var attempts = 0; attempts < 1000 && !placed; attempts++) {
                var w = 50 + Math.random() * 70
                var h = 40 + Math.random() * 60
                var x = 30 + Math.random() * (width - w - 60)
                var y = 30 + Math.random() * (height - h - 60)

                var valid = x >= 20 && x + w <= width - 20 && y >= 20 && y + h <= height - 20
                for (var j = 0; valid && j < tempObstacles.length; j++) {
                    if (rectRectCollision({x: x, y: y, width: w, height: h}, tempObstacles[j]))
                        valid = false
                }
                if (valid && !circleRectCollision(ballPos.x + ballRadius, ballPos.y + ballRadius, ballRadius*2,
                                                 {x: x, y: y, width: w, height: h})) {
                    tempObstacles.push({x: x, y: y, width: w, height: h})
                    placed = true
                }
            }
            if (!placed) {
                tempObstacles.push({x: 50 + Math.random() * (width - 150), y: 50 + Math.random() * (height - 150),
                                   width: 60, height: 50})
            }
        }

        // Генерация целей
        for (i = 0; i < difficulty; i++) {
            var placed = false
            for (var attempts = 0; attempts < 1000 && !placed; attempts++) {
                var r = 12 + Math.random() * 16
                var x = 40 + Math.random() * (width - 80)
                var y = 40 + Math.random() * (height - 80)

                var valid = x - r >= 10 && x + r <= width - 10 && y - r >= 10 && y + r <= height - 10
                for (j = 0; valid && j < tempTargets.length; j++) {
                    if (Math.hypot(x - tempTargets[j].x, y - tempTargets[j].y) < r + tempTargets[j].radius + 30)
                        valid = false
                }
                for (j = 0; valid && j < tempObstacles.length; j++) {
                    if (circleRectCollision(x, y, r + 20, tempObstacles[j]))
                        valid = false
                }
                if (valid && Math.hypot(x - (ballPos.x + ballRadius), y - (ballPos.y + ballRadius)) >= r + ballRadius + 50) {
                    tempTargets.push({x: x, y: y, radius: r})
                    placed = true
                }
            }
            if (!placed) {
                tempTargets.push({x: 60 + Math.random() * (width - 120), y: 60 + Math.random() * (height - 120), radius: 15})
            }
        }

        for (i = 0; i < tempObstacles.length; i++) obstacleModel.append(tempObstacles[i])
        for (i = 0; i < tempTargets.length; i++) targetModel.append(tempTargets[i])

        ball.x = ballPos.x
        ball.y = ballPos.y
        directionX = directionY = 0
        gameActive = true

        controlsTimer.start()
    }

    function stopGame() {
        gameActive = movementEnabled = controlsEnabled = false
        controlsTimer.stop()
    }

    // Игровые объекты
    Rectangle {
        id: ball
        x: parent.width/2 - ballRadius
        y: parent.height/2 - ballRadius
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
            color: "red"; border.color: "darkred"; border.width: 2
            visible: gameActive
        }
    }

    Repeater {
        model: targetModel
        Rectangle {
            x: model.x - model.radius; y: model.y - model.radius
            width: model.radius*2; height: model.radius*2
            radius: model.radius
            color: "green"; border.color: "darkgreen"; border.width: 2
            visible: gameActive
        }
    }

    // Управление
    Item {
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            if (!gameActive || !controlsEnabled) return

            var dx = 0, dy = 0
            switch(event.key) {
            case Qt.Key_Left: case Qt.Key_A: dx = -1; break
            case Qt.Key_Right: case Qt.Key_D: dx = 1; break
            case Qt.Key_Up: case Qt.Key_W: dy = -1; break
            case Qt.Key_Down: case Qt.Key_S: dy = 1; break
            default: return
            }

            if (!movementEnabled) movementEnabled = true
            updateDirection(dx, dy)
            event.accepted = true
        }
    }

    // Игровой цикл
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: {
            if (!gameActive || !movementEnabled) return

            ball.x += directionX * ballSpeed
            ball.y += directionY * ballSpeed

            // Отскок от стен
            if (ball.x < 0) { ball.x = 0; directionX = -directionX }
            if (ball.x > parent.width - ball.width) { ball.x = parent.width - ball.width; directionX = -directionX }
            if (ball.y < 0) { ball.y = 0; directionY = -directionY }
            if (ball.y > parent.height - ball.height) { ball.y = parent.height - ball.height; directionY = -directionY }

            // Проверка столкновений
            for (var i = 0; i < obstacleModel.count; i++) {
                var obs = obstacleModel.get(i)
                if (circleRectCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius, obs)) {
                    gameActive = movementEnabled = controlsEnabled = false
                    onCollision()
                    return
                }
            }

            // Сбор целей
            for (var j = targetModel.count - 1; j >= 0; j--) {
                var tar = targetModel.get(j)
                if (circleCircleCollision(ball.x + ballRadius, ball.y + ballRadius, ballRadius, tar.x, tar.y, tar.radius)) {
                    targetModel.remove(j)
                    onHit()
                }
            }

            if (targetModel.count === 0 && gameActive) {
                gameActive = movementEnabled = controlsEnabled = false
                levelCompleted()
            }
        }
    }
}
