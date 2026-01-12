import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Smart Personal Dashbaord")

    HomeComponent {
        anchors.fill: parent
    }
}
