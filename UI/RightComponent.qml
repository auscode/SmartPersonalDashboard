import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1

    ColumnLayout{
        spacing: 2
        Text {
            id: drives
            text: qsTr("drives connected")
            color: "#ffffff"
        }

        Text {
            id: hardwares
            text: qsTr("hardwares connected")
            color: "#ffffff"
        }

        Text {
            id: peripherals
            text: qsTr("peripherals")
            color: "#ffffff"
        }
    }

}
