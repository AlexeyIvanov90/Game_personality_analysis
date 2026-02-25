import QtQuick 2.12


Item {
    id: root
    signal hitShape()
    signal missShape()

    property int score: 0
    property int difficulty: 4

    property bool waitingForNumbers: true
    property int nextExpectedNumber: 1

    width: 640
    height: 480
    visible: true

    ListModel { id: shapeModel }

    Timer {
        id: numberTimer
        interval: difficulty*500
        onTriggered: {
            waitingForNumbers = false
            console.log("Номера скрыты")
        }
    }

    // Генерация случайных параметров фигуры (без номера)
    function createRandomShape() {
        var newSize = 50 + Math.random() * 100
        var maxX = Math.max(0, root.width - newSize)
        var maxY = Math.max(0, root.height - newSize)
        return {
            x: Math.random() * maxX,
            y: Math.random() * maxY,
            width: newSize,
            height: newSize,
            color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1).toString(),
            isCircle: Math.random() < 0.5
        }
    }

    // Проверка пересечения двух фигур (по прямоугольникам)
    function shapesOverlap(a, b) {
        return !(a.x + a.width <= b.x ||
                 a.x >= b.x + b.width ||
                 a.y + a.height <= b.y ||
                 a.y >= b.y + b.height)
    }

    // Генерация фигуры, не пересекающейся с уже созданными
    function generateNonOverlappingShape(existingShapes) {
        var maxAttempts = 1000
        for (var attempt = 0; attempt < maxAttempts; ++attempt) {
            var candidate = createRandomShape()
            var overlapping = false
            for (var i = 0; i < existingShapes.length; ++i) {
                if (shapesOverlap(candidate, existingShapes[i])) {
                    overlapping = true
                    break
                }
            }
            if (!overlapping) {
                return candidate
            }
        }
        console.warn("Не удалось разместить фигуру без пересечения после", maxAttempts, "попыток")
        return createRandomShape()
    }

    // Заполнение модели непересекающимися фигурами со случайными номерами
    function populateModel(lvl) {
        difficulty=lvl
        shapeModel.clear()

        // Создаём массив номеров от 1 до difficulty и перемешиваем
        var numbers = []
        for (var i = 1; i <= difficulty; ++i) numbers.push(i)
        for (var j = numbers.length - 1; j > 0; --j) {
            var rand = Math.floor(Math.random() * (j + 1))
            var tmp = numbers[j]
            numbers[j] = numbers[rand]
            numbers[rand] = tmp
        }

        var newShapes = []
        for (var k = 0; k < difficulty; ++k) {
            var shape = generateNonOverlappingShape(newShapes)
            shape.number = numbers[k]   // присваиваем номер
            newShapes.push(shape)
            shapeModel.append(shape)
        }

        waitingForNumbers = true
        numberTimer.restart()
        nextExpectedNumber = 1
        console.log("Новый раунд, номера:", numbers)
    }

    // Обработка клика по фигуре
    function shapeClicked(index) {
        if (waitingForNumbers) return   // пока видны номера — игнорируем

        var number = shapeModel.get(index).number
        if (number === nextExpectedNumber) {
            // Правильный порядок
            shapeModel.remove(index)
            score += 1
            hitShape()
            nextExpectedNumber++
            console.log("Правильно, осталось фигур:", shapeModel.count)

            if (shapeModel.count === 0) {
                console.log("Раунд завершён!")
            }
        } else {
            missShape()
            shapeModel.clear()
            console.log("Ошибка порядка")
        }
    }


    Repeater {
        model: shapeModel
        delegate: Rectangle {
            x: model.x
            y: model.y
            width: model.width
            height: model.height
            color: model.color
            radius: model.isCircle ? width / 2 : 0
            z: 1

            // Отображение номера (видно только во время waitingForNumbers)
            Text {
                text: model.number
                visible: root.waitingForNumbers
                anchors.centerIn: parent
                color: "white"
                font.pointSize: 20
                font.bold: true
                z: 2
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    shapeClicked(index)
                }
            }
        }
    }

    // Фон: клик мимо фигур
    MouseArea {
        anchors.fill: parent
        z: 0
        onClicked: {
            missShape()
            shapeModel.clear()
            console.log("Промах по фону")
        }
    }

    Component.onCompleted: populateModel()
}
