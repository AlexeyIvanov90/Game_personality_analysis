import QtQuick 2.12

Item {
    signal onJump(int accuracy)
    signal onCollision()

    id: root
    width: 800
    height: 300
    visible: true

    // Игровые параметры
    property int groundY: height - 50
    property int dinoX: 100
    property int dinoWidth: 30
    property int dinoHeight: 30
    property real dinoY: groundY - dinoHeight
    property real dinoVy: 0
    property bool onGround: true

    property real gravity: 1.0
    property real jumpStrength: -15

    property real speed: 5
    property bool gameActive: false

    property var obstacles: []
    property var clouds: []
    property var completedJumps: []  // список высот для успешно пройденных препятствий

    property int nextObstacleDist: 300

    // Анимация динозавра
    property Image dinoFrame1: Image { source: "qrc:/source/source/img/game_2/dino_1_transparent.png"; asynchronous: true; visible: false }
    property Image dinoFrame2: Image { source: "qrc:/source/source/img/game_2/dino_2_transparent.png"; asynchronous: true; visible: false }
    property var dinoFrames: [dinoFrame1, dinoFrame2]
    property int currentDinoFrame: 0

    // Остальные изображения
    property Image cactusImg: Image { source: "qrc:/source/source/img/game_2/cactus_transparent.png"; visible: false }
    property Image cloudImg: Image { source: "qrc:/source/source/img/game_2/cloud_transparent.png"; visible: false }

    function startGame() {
        if (!gameActive) {
            resetGame()
            gameActive = true
        }
    }

    function stopGame() {
        if (gameActive) {
            gameActive = false
        }
    }

    // Функция сброса игры
    function resetGame() {
        dinoY = groundY - dinoHeight
        dinoVy = 0
        onGround = true
        obstacles = []
        clouds = []
        completedJumps = []
        gameActive = true
        currentDinoFrame = 0
        gameTimer.restart()
        canvas.requestPaint()
    }

    // Возвращает расстояние в пикселях до ближайшего препятствия впереди (оставлено для отладки)
    function distanceToNextObstacle() {
        var minDist = -1;
        for (var i = 0; i < obstacles.length; i++) {
            var o = obstacles[i];
            if (o.x > dinoX + dinoWidth) {
                var dist = o.x - (dinoX + dinoWidth);
                if (minDist === -1 || dist < minDist) {
                    minDist = dist;
                }
            }
        }
        return minDist;
    }

    // Прыжок
    function jump() {
        if (gameActive && onGround) {
            dinoVy = jumpStrength
            onGround = false
        }
    }

    // Обновление логики каждый кадр
    function updateGame() {
        if (!gameActive) return

        // Физика динозавра
        dinoY += dinoVy
        dinoVy += gravity
        var wasOnGround = onGround
        if (dinoY >= groundY - dinoHeight) {
            dinoY = groundY - dinoHeight
            dinoVy = 0
            onGround = true
        } else {
            onGround = false
        }

        // Если только что приземлились и есть завершённые прыжки – отправляем сигналы
        if (!wasOnGround && onGround && completedJumps.length > 0) {
            var maxHeight = (jumpStrength * jumpStrength) / (2 * gravity)
            for (var h = 0; h < completedJumps.length; h++) {
                var height = completedJumps[h]
                var accuracy = Math.min(100, Math.round((height / maxHeight) * 100))
                onJump(accuracy)
            }
            completedJumps = []
        }

        // Движение препятствий с предварительной проверкой столкновений
        for (var i = obstacles.length - 1; i >= 0; i--) {
            var o = obstacles[i]
            var newX = o.x - speed

            // Проверка столкновения
            if (rectCollision(dinoX, dinoY, dinoWidth, dinoHeight, newX, groundY - o.height, o.width, o.height)) {
                gameActive = false
                gameTimer.stop()
                dinoAnimTimer.stop()
                onCollision()
                return
            } else {
                o.x = newX
            }

            // Если препятствие полностью позади динозавра и у него есть высота – добавляем в completedJumps и удаляем
            if (o.x + o.width < dinoX && o.heightWhenOver !== undefined) {
                completedJumps.push(o.heightWhenOver)
                obstacles.splice(i, 1)
                continue
            }

            // Удаляем ушедшие за левый край (если не были обработаны выше)
            if (o.x + o.width < 0) {
                obstacles.splice(i, 1)
            }
        }

        // Запоминаем высоту в момент пролёта над центром препятствия (если ещё не записано)
        var dinoCenterX = dinoX + dinoWidth / 2
        for (var k = 0; k < obstacles.length; k++) {
            var obs = obstacles[k]
            if (obs.heightWhenOver === undefined) {
                var obsCenterX = obs.x + obs.width / 2
                // Проверяем, находится ли центр динозавра в пределах ширины препятствия
                if (Math.abs(dinoCenterX - obsCenterX) <= dinoWidth / 2) {
                    obs.heightWhenOver = groundY - (dinoY + dinoHeight) // текущая высота подъёма
                }
            }
        }

        // Движение облаков
        for (var j = clouds.length - 1; j >= 0; j--) {
            clouds[j].x -= speed * 0.5
            if (clouds[j].x + 60 < 0) {
                clouds.splice(j, 1)
            }
        }

        // Генерация новых облаков
        if (Math.random() < 0.02 && clouds.length < 4) {
            clouds.push({
                x: width,
                y: 30 + Math.random() * 80,
                width: 60 + Math.random() * 40,
                height: 20 + Math.random() * 10
            })
        }

        // Генерация новых препятствий
        if (obstacles.length === 0 || obstacles[obstacles.length - 1].x < width - nextObstacleDist) {
            var obs = {
                x: width,
                width: 15 + Math.floor(Math.random() * 15),
                height: 25 + Math.floor(Math.random() * 15),
                heightWhenOver: undefined
            }
            obstacles.push(obs)
            nextObstacleDist = 300 + Math.floor(Math.random() * 200)
        }

        canvas.requestPaint()
    }

    // Проверка пересечения прямоугольников
    function rectCollision(x1, y1, w1, h1, x2, y2, w2, h2) {
        return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2)
    }

    // Таймер игрового цикла
    Timer {
        id: gameTimer
        interval: 16
        running: gameActive
        repeat: true
        onTriggered: {
            if (gameActive) {
                updateGame()
            }
        }
    }

    // Таймер анимации динозавра
    Timer {
        id: dinoAnimTimer
        interval: 500
        running: gameActive
        repeat: true
        onTriggered: {
            if (gameActive) {
                currentDinoFrame = (currentDinoFrame + 1) % dinoFrames.length
                canvas.requestPaint()
            }
        }
    }

    // Canvas для отрисовки
    Canvas {
        id: canvas
        anchors.fill: parent
        focus: true

        Keys.onSpacePressed: jump()
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Space) {
                jump()
                event.accepted = true
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: jump()
        }

        onPaint: {
            var ctx = canvas.getContext('2d')
            ctx.clearRect(0, 0, width, height)

            // Небо
            ctx.fillStyle = "#f7f7f7"
            ctx.fillRect(0, 0, width, height)

            // Облака
            for (var i = 0; i < clouds.length; i++) {
                var c = clouds[i]
                ctx.drawImage(cloudImg, c.x, c.y, c.width, c.height)
            }

            // Земля
            ctx.fillStyle = "#a0a0a0"
            ctx.fillRect(0, groundY, width, 5)

            // Препятствия
            for (var j = 0; j < obstacles.length; j++) {
                var o = obstacles[j]
                ctx.drawImage(cactusImg, o.x, groundY - o.height, o.width, o.height)
            }

            // Динозавр (анимированный)
            ctx.drawImage(dinoFrames[currentDinoFrame], dinoX, dinoY, dinoWidth, dinoHeight)
        }
    }
}
