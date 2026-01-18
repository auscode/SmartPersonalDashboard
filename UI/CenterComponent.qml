import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Pane {
    id: centerRoot
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 2
   

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Text {
            text: "TOP PROCESSES RUNNING"
            font.bold: true
            font.pixelSize: 18
            color: "white"
            Layout.fillWidth: true
        }

        // Header Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            Rectangle { Layout.preferredWidth: 60; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "PID"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.fillWidth: true; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "NAME"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 100; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "MEMORY"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 80; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "CPU"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 150; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "ACTIONS"; color: "#aaa"; font.bold: true } }
        }

        ListView {
            id: processList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: metrics.processes
            spacing: 2

            delegate: Rectangle {
                width: processList.width
                height: 45
                color: index % 2 === 0 ? "#1a1a1a" : "#222222"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    spacing: 0

                    Text { Layout.preferredWidth: 60; text: modelData.pid; color: "#888"; horizontalAlignment: Text.AlignHCenter }
                    Text { Layout.fillWidth: true; text: modelData.name; color: "white"; font.bold: true; elide: Text.ElideRight }
                    Text { Layout.preferredWidth: 100; text: modelData.memory; color: "#00ff00"; horizontalAlignment: Text.AlignHCenter }
                    Text { Layout.preferredWidth: 80; text: modelData.cpu; color: "#00ccff"; horizontalAlignment: Text.AlignHCenter }

                    RowLayout {
                        Layout.preferredWidth: 150
                        spacing: 5
                        
                        Button {
                            text: "Kill"
                            flat: true
                            contentItem: Text { text: "Kill"; color: "#ff4444"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.pressed ? "#331111" : "#221111"; radius: 4; border.color: "#ff4444"; border.width: 1 }
                            onClicked: metrics.killProcess(modelData.pid)
                        }

                        Button {
                            text: "Stop"
                            flat: true
                            contentItem: Text { text: "Stop"; color: "#ffaa00"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.pressed ? "#332211" : "#221a11"; radius: 4; border.color: "#ffaa00"; border.width: 1 }
                            onClicked: metrics.suspendProcess(modelData.pid, true)
                        }
                        
                        Button {
                            text: "Run"
                            flat: true
                            contentItem: Text { text: "Run"; color: "#00ff00"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.pressed ? "#113311" : "#112211"; radius: 4; border.color: "#00ff00"; border.width: 1 }
                            onClicked: metrics.suspendProcess(modelData.pid, false)
                        }
                    }
                }
            }
        }
    }
}
