import QtQuick 2.0

Rectangle {
    width: 360
    height: 360
    Text {
        x: 8
        y: 16
        text: qsTr("МОЯ ПЕРВАЯ ПРОГА В QT")
        anchors.verticalCenterOffset: -155
        anchors.horizontalCenterOffset: -80
        anchors.centerIn: parent
    }
    states: [
        State {
            name: "State1"
        }
    ]
}
