import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 2

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Text {
            text: qsTr("Top softwares running")
            font.pixelSize: 18
            color: "white"
            Layout.fillWidth: true
        }

        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            columnWidthProvider: function (column) {
                return width / 5
            }

            rowHeightProvider: function (row) {
                return 40
            }

            model: TableModel {
                TableModelColumn { display: "name" }
                TableModelColumn { display: "memory" }
                TableModelColumn { display: "storage" }
                TableModelColumn { display: "cpu" }
                TableModelColumn { display: "action" }

                rows: [
                    { name: "Software", memory: "Memory", storage: "Storage", cpu: "CPU", action: "Action" },
                    { name: "App 1", memory: "200MB", storage: "1GB", cpu: "5%", action: "Kill" },
                    { name: "App 2", memory: "350MB", storage: "2GB", cpu: "8%", action: "Suspend" }
                ]
            }

            delegate: Rectangle {
                required property var model
                height: 40
                color: "#2b2b2b"
                border.color: "#555"

                Text {
                    text: model.display
                    color: "white"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
