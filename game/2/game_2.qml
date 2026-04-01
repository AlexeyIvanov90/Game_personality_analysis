import QtQuick 2.12

Item {
    signal onSuccess()
    signal onCollision()

    id: root
    width: 981
    height: 641
    visible: true

    // Игровые параметры
    property int groundY: height - 50

    property int dinoX: 100

    property real duckWidth: 59
    property real dinoDuckHeight: 30
    property real dinoNormalWidth: 44
    property real dinoNormalHeight: 47

    property int dinoWidth: dinoNormalWidth
    property int dinoHeight: dinoNormalHeight

    property real dinoY: groundY - dinoHeight
    property real dinoVy: 0

    property int cactusSmallWidth: 17
    property int cactusSmallHeight: 35

    property int cactusBigWidth: 51
    property int cactusBigHeight: 50

    property int pterodactylWidth: 44
    property int pterodactylHeight: 47

    property int cloudWidth: 96
    property int cloudHeight: 30

    property bool onGround: true
    property bool isDucking: false

    property real gravity: 0.8
    // Высота прыжка в пикселях (насколько выше «земли» поднимается низ динозавра)
    property real minJumpLiftPx: 52
    property real maxJumpLiftPx: 135
    // За это время удержания пробела достигается maxJumpLiftPx
    property real jumpMaxHoldMs: 320
    property real jumpHoldBoost: 0.42
    property real jumpMaxUpVelocity: -16
    property real jumpHoldTime: 0
    property bool jumpSpaceHeld: false

    property real speed: 5
    property bool gameActive: false

    // Одиночные препятствия
    property var currentSmallObstacle: null
    property var currentBigObstacle: null
    property var currentFlyingObstacle: null
    property var clouds: []

    property int framesSinceLastObstacle: 0
    property bool hasActiveObstacle: false

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

    function hideTempMessage() {
        tempMessageBox.visible = false
        messageTimer.stop()
    }

    // Анимация динозавра
    property Image dinoFrame1: Image { source: "qrc:/source/source/img/game_2/dino_1_tr.png"; asynchronous: true; visible: false }
    property Image dinoFrame2: Image { source: "qrc:/source/source/img/game_2/dino_2_tr.png"; asynchronous: true; visible: false }
    property Image dinoDuckFrame1: Image { source: "qrc:/source/source/img/game_2/dino_duck_1_tr.png"; asynchronous: true; visible: false }
    property Image dinoDuckFrame2: Image { source: "qrc:/source/source/img/game_2/dino_duck_2_tr.png"; asynchronous: true; visible: false }
    property var dinoFrames: [dinoFrame1, dinoFrame2]
    property var dinoDuckFrames: [dinoDuckFrame1, dinoDuckFrame2]
    property int currentDinoFrame: 0
    property int currentDuckFrame: 0

    // Анимация птеродактиля
    property Image pteroFrame1: Image { source: "qrc:/source/source/img/game_2/pterodactyl_1_tr.png"; asynchronous: true; visible: false }
    property Image pteroFrame2: Image { source: "qrc:/source/source/img/game_2/pterodactyl_2_tr.png"; asynchronous: true; visible: false }
    property var pteroFrames: [pteroFrame1, pteroFrame2]
    property int currentPteroFrame: 0

    // Остальные изображения
    property Image cactusSmallImg: Image { source: "qrc:/source/source/img/game_2/cactus_small_1_tp.png"; visible: false }
    property Image cactusBigImg: Image { source: "qrc:/source/source/img/game_2/cactus_big_1_tp.png"; visible: false }
    property Image cloudImg: Image { source: "qrc:/source/source/img/game_2/cloud_tp.png"; visible: false }

    function stopGame() {
        if (gameActive) {
            gameActive = false
        }
    }

    function resetGame() {
        dinoY = groundY - dinoHeight
        dinoVy = 0
        dinoWidth = dinoNormalWidth
        dinoHeight = dinoNormalHeight
        onGround = true
        isDucking = false
        hasActiveObstacle = false
        currentSmallObstacle = null
        currentBigObstacle = null
        currentFlyingObstacle = null
        clouds = []
        gameActive = true
        currentDinoFrame = 0
        currentDuckFrame = 0
        currentPteroFrame = 0
        framesSinceLastObstacle = 0
        jumpSpaceHeld = false
        jumpHoldTime = 0
        gameTimer.restart()
        dinoAnimTimer.restart()
        pteroAnimTimer.restart()
        canvas.requestPaint()
    }

    // Прыжок сразу при нажатии пробела; минимальная высота — через начальную скорость √(2·g·h_min)
    function beginJumpFromGround() {
        if (!gameActive || !onGround || isDucking)
            return
        jumpSpaceHeld = true
        jumpHoldTime = 0
        var g = gravity
        dinoVy = -Math.sqrt(Math.max(0.001, 2.0 * g * minJumpLiftPx))
        onGround = false
    }

    function endJumpHold() {
        jumpSpaceHeld = false
    }

    function cancelJumpHold() {
        jumpSpaceHeld = false
    }

    property bool isDuckKeyPressed: false

    // Функция пригибания
    function duck() {
        if (gameActive && onGround && !isDucking) {
            cancelJumpHold()
            isDucking = true
            currentDuckFrame = 0
            dinoWidth = duckWidth
            dinoHeight = dinoDuckHeight
            dinoY = groundY - dinoHeight
            duckAnimTimer.start()
            canvas.requestPaint()
        }
    }

    // Функция подъема
    function standUp() {
        if (gameActive && isDucking) {
            isDucking = false
            duckAnimTimer.stop()
            currentDuckFrame = 0
            dinoHeight = dinoNormalHeight
            dinoWidth = dinoNormalWidth
            dinoY = groundY - dinoHeight
            canvas.requestPaint()
        }
    }

    // Функция создания нового препятствия
    function createObstacle() {
        var obstacleType = Math.random()

        if (obstacleType < 0.33) { // 33% - малые наземные препятствия
            currentSmallObstacle = {
                x: width,
                width: cactusSmallWidth,
                height: cactusSmallHeight,
                type: "ground"
            }
            currentBigObstacle = null
            currentFlyingObstacle = null
        } else if (obstacleType < 0.66) { // 33% - большие наземные препятствия
            currentBigObstacle = {
                x: width,
                width: cactusBigWidth,
                height: cactusBigHeight,
                type: "ground"
            }
            currentSmallObstacle = null
            currentFlyingObstacle = null
        } else { // 33% - летающие препятствия
            var flyY = groundY - dinoDuckHeight - pterodactylHeight - Math.random() * 50
            currentFlyingObstacle = {
                x: width,
                y: flyY,
                width: pterodactylWidth,
                height: pterodactylHeight,
                type: "flying"
            }
            currentSmallObstacle = null
            currentBigObstacle = null
        }

        hasActiveObstacle = true
    }

    function updateGame() {
        if (!gameActive) return

        // Физика динозавра: удержание пробела увеличивает целевую высоту до max; отпускание — только гравитация
        if (!onGround && jumpSpaceHeld) {
            jumpHoldTime += 16
            var t = Math.min(1.0, jumpHoldTime / jumpMaxHoldMs)
            var liftTarget = minJumpLiftPx + t * (maxJumpLiftPx - minJumpLiftPx)
            var apexY = groundY - dinoHeight - liftTarget
            if (dinoVy < 0 && dinoY > apexY) {
                dinoVy -= jumpHoldBoost
                if (dinoVy < jumpMaxUpVelocity)
                    dinoVy = jumpMaxUpVelocity
            }
        }

        dinoY += dinoVy
        dinoVy += gravity

        if (dinoY >= groundY - dinoHeight) {
            dinoY = groundY - dinoHeight
            dinoVy = 0
            onGround = true
            jumpSpaceHeld = false
            jumpHoldTime = 0
        } else {
            onGround = false
        }

        // Обработка малого наземного препятствия
        if (currentSmallObstacle !== null) {
            var newX = currentSmallObstacle.x - speed

            // Проверка столкновения
            var currentDinoHeight = isDucking ? dinoDuckHeight : dinoNormalHeight
            var currentDinoY = isDucking ? groundY - dinoDuckHeight : dinoY

            if (rectCollision(dinoX, currentDinoY, dinoWidth, currentDinoHeight,
                             newX, groundY - currentSmallObstacle.height,
                             currentSmallObstacle.width, currentSmallObstacle.height)) {
                gameActive = false
                gameTimer.stop()
                dinoAnimTimer.stop()
                duckAnimTimer.stop()
                pteroAnimTimer.stop()
                cancelJumpHold()
                onCollision()
                return
            } else {
                currentSmallObstacle.x = newX
            }

            // Удаляем ушедшее препятствие
            if (currentSmallObstacle.x + currentSmallObstacle.width < 0) {
                currentSmallObstacle = null
                hasActiveObstacle = false
                onSuccess()
            }
        }

        // Обработка большого наземного препятствия
        if (currentBigObstacle !== null) {
            var newX = currentBigObstacle.x - speed

            // Проверка столкновения
            var currentDinoHeight = isDucking ? dinoDuckHeight : dinoNormalHeight
            var currentDinoY = isDucking ? groundY - dinoDuckHeight : dinoY

            if (rectCollision(dinoX, currentDinoY, dinoWidth, currentDinoHeight,
                             newX, groundY - currentBigObstacle.height,
                             currentBigObstacle.width, currentBigObstacle.height)) {
                gameActive = false
                gameTimer.stop()
                dinoAnimTimer.stop()
                duckAnimTimer.stop()
                pteroAnimTimer.stop()
                cancelJumpHold()
                onCollision()
                return
            } else {
                currentBigObstacle.x = newX
            }

            // Удаляем ушедшее препятствие
            if (currentBigObstacle.x + currentBigObstacle.width < 0) {
                currentBigObstacle = null
                hasActiveObstacle = false
                onSuccess()
            }
        }

        // Обработка летающего препятствия
        if (currentFlyingObstacle !== null) {
            var newFlyX = currentFlyingObstacle.x - speed

            // Проверка столкновения
            if (rectCollision(dinoX, dinoY, dinoWidth, dinoHeight,
                             newFlyX, currentFlyingObstacle.y,
                             currentFlyingObstacle.width, currentFlyingObstacle.height)) {
                gameActive = false
                gameTimer.stop()
                dinoAnimTimer.stop()
                duckAnimTimer.stop()
                pteroAnimTimer.stop()
                cancelJumpHold()
                onCollision()
                return
            } else {
                currentFlyingObstacle.x = newFlyX
            }

            // Удаляем ушедшее препятствие
            if (currentFlyingObstacle.x + currentFlyingObstacle.width < 0) {
                currentFlyingObstacle = null
                hasActiveObstacle = false
                onSuccess()
            }
        }

        // Движение облаков
        for (var k = clouds.length - 1; k >= 0; k--) {
            clouds[k].x -= speed * 0.5
            if (clouds[k].x + cloudWidth < 0) {
                clouds.splice(k, 1)
            }
        }

        // Генерация новых облаков
        if (Math.random() < 0.02 && clouds.length < 4) {
            clouds.push({
                x: width,
                y: 30 + Math.random() * 80,
                width: cloudWidth,
                height: cloudHeight
            })
        }

        // Генерация нового препятствия - только если нет активного
        if (!hasActiveObstacle) {
            framesSinceLastObstacle++

            // Случайный интервал между препятствиями (40-80 кадров)
            if (framesSinceLastObstacle > 40 + Math.random() * 40) {
                createObstacle()
                framesSinceLastObstacle = 0
            }
        }

        canvas.requestPaint()
    }

    function rectCollision(x1, y1, w1, h1, x2, y2, w2, h2) {
        return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2)
    }

    // Таймер анимации динозавра (бег)
    Timer {
        id: dinoAnimTimer
        interval: 500
        running: gameActive
        repeat: true
        onTriggered: {
            if (gameActive && !isDucking) {
                currentDinoFrame = (currentDinoFrame + 1) % dinoFrames.length
                canvas.requestPaint()
            }
        }
    }

    // Таймер анимации пригибания
    Timer {
        id: duckAnimTimer
        interval: 300
        running: false
        repeat: true
        onTriggered: {
            if (gameActive && isDucking) {
                currentDuckFrame = (currentDuckFrame + 1) % dinoDuckFrames.length
                canvas.requestPaint()
            }
        }
    }

    // Таймер для автоматического подъема
    Timer {
        id: autoStandUpTimer
        interval: 100
        onTriggered: {
            if (isDucking && !isDuckKeyPressed) {
                standUp()
            }
        }
    }

    // Таймер анимации птеродактиля
    Timer {
        id: pteroAnimTimer
        interval: 200
        running: gameActive
        repeat: true
        onTriggered: {
            if (gameActive) {
                currentPteroFrame = (currentPteroFrame + 1) % 2
                canvas.requestPaint()
            }
        }
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

    // Canvas для отрисовки
    Canvas {
        id: canvas
        anchors.fill: parent
        focus: true

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Space) {
                beginJumpFromGround()
                event.accepted = true
            } else if (event.key === Qt.Key_Down) {
                if (!isDuckKeyPressed) {
                    isDuckKeyPressed = true
                    autoStandUpTimer.stop()
                    duck()
                }
                event.accepted = true
            }
        }

        Keys.onReleased: (event) => {
            if (event.key === Qt.Key_Space) {
                endJumpHold()
                event.accepted = true
            } else if (event.key === Qt.Key_Down) {
                if (isDuckKeyPressed) {
                    isDuckKeyPressed = false
                    autoStandUpTimer.start()
                }
                event.accepted = true
            }
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
                ctx.drawImage(cloudImg, c.x, c.y, cloudWidth, cloudHeight)
            }

            // Земля
            ctx.fillStyle = "#a0a0a0"
            ctx.fillRect(0, groundY, width, 5)

            // Наземное малое препятствие (только одно)
            if (currentSmallObstacle !== null) {
                ctx.drawImage(cactusSmallImg, currentSmallObstacle.x, groundY - currentSmallObstacle.height,
                            currentSmallObstacle.width, currentSmallObstacle.height)
            }

            // Наземное большое препятствие (только одно)
            if (currentBigObstacle !== null) {
                ctx.drawImage(cactusBigImg, currentBigObstacle.x, groundY - currentBigObstacle.height,
                            currentBigObstacle.width, currentBigObstacle.height)
            }

            // Летающее препятствие с анимацией (только одно)
            if (currentFlyingObstacle !== null) {
                var currentPteroImg = pteroFrames[currentPteroFrame]
                if (currentPteroImg && currentPteroImg.status === Image.Ready) {
                    ctx.drawImage(currentPteroImg, currentFlyingObstacle.x, currentFlyingObstacle.y,
                                currentFlyingObstacle.width, currentFlyingObstacle.height)
                } else if (pteroFrame1.status === Image.Ready) {
                    ctx.drawImage(pteroFrame1, currentFlyingObstacle.x, currentFlyingObstacle.y,
                                currentFlyingObstacle.width, currentFlyingObstacle.height)
                }
            }

            // Динозавр с анимацией (бег или пригибание)
            if (isDucking) {
                var currentDuckImg = dinoDuckFrames[currentDuckFrame]
                if (currentDuckImg && currentDuckImg.status === Image.Ready) {
                    ctx.drawImage(currentDuckImg, dinoX, dinoY, dinoWidth, dinoHeight)
                } else if (dinoDuckFrame1.status === Image.Ready) {
                    ctx.drawImage(dinoDuckFrame1, dinoX, dinoY, dinoWidth, dinoHeight)
                }
            } else {
                var currentDinoImg = dinoFrames[currentDinoFrame]
                if (currentDinoImg && currentDinoImg.status === Image.Ready) {
                    ctx.drawImage(currentDinoImg, dinoX, dinoY, dinoWidth, dinoHeight)
                } else if (dinoFrame1.status === Image.Ready) {
                    ctx.drawImage(dinoFrame1, dinoX, dinoY, dinoWidth, dinoHeight)
                }
            }
        }
    }
}
