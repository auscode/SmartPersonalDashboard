import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Pane {
    id: rightRoot
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1

    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        // Hardware & Peripherals Section
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 5

            Text {
                text: "HARDWARE & PERIPHERALS"
                Layout.fillWidth: true
                font.bold: true
                font.pixelSize: 14
                color: "#aaaaaa"
                horizontalAlignment: Text.AlignLeft
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 8
                model: metrics.hardware

                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Rectangle {
                        width: 32
                        height: 32
                        radius: 4
                        color: "#222222"

                        Text {
                            anchors.centerIn: parent
                            text: modelData.type === "USB Device" ? "🔌" : "🖱️"
                            font.pixelSize: 16
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                        }

                        Text {
                            text: modelData.type +
                                  (modelData.vendor ? " | " + modelData.vendor : "")
                            color: "#666666"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }
        }
    }
}
