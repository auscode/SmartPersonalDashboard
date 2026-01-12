import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts 1.15
Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1

    ColumnLayout{
        spacing: 2

        Text {
            id: time
            text: qsTr("Time: 13:24:21")
            color: "#ffffff"
        }

        Text {
            id: network
            text: qsTr("Networks Up/Down")
            color: "#ffffff"
        }
        Text {
            id: cpu
            text: qsTr("cpu graphs and temps")
            color: "#ffffff"
        }
        Text {
            id: gpu
            text: qsTr("gpu graphs & temps")
            color: "#ffffff"
        }
        Text {
            id: memory
            text: qsTr("memory graphs")
            color: "#ffffff"
        }
    }
}
