import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Window {
    width: 1500
    height: 800
    visible: true
    title: qsTr("Smart Personal Dashbaord")

    HomeComponent {
        anchors.fill: parent
    }
}
