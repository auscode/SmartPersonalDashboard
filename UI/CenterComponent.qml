import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 2

    Text {
        id: heroHeader
        text: qsTr("top softwares running")
    }

    TableView {
        anchors.fill: parent

        model: TableModel {
            TableModelColumn { display: "name" }
            TableModelColumn { display: "memory" }
            TableModelColumn { display: "Storage" }
            TableModelColumn { display: "CPU usage" }
            TableModelColumn { display: "Action kill/suspend" }

            rows: [
                { name: "name", memory: "memory", storage: "storage", cpu:"cPU", action:"kill/suspen"    },
                { name: "   ", memory: "memory", storage: "Storage", cpu:"CPU", action:"kill/suspen"    },
                { name: "name", memory: "memory", storage: "Storage", cpu:"CPU", action:"kill/suspen"    },
                { name: "name", memory: "memory", storage: "Storage", cpu:"CPU", action:"kill/suspen"    },
                { name: "name", memory: "memory", storage: "Storage", cpu:"CPU", action:"kill/suspen"    },
            ]
        }

        delegate: Rectangle {
            implicitWidth: 120
            implicitHeight: 40
            border.color: "lightgray"

            Text {
                text: display
                anchors.centerIn: parent
            }
        }
    }
}
