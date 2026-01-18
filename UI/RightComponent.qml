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

        // Drives Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Text {
                text: "DRIVES CONNECTED"
                Layout.fillWidth: true
                font.bold: true
                font.pixelSize: 14
                color: "#ffffff"
                horizontalAlignment: Text.AlignLeft
            }

            Repeater {
                model: metrics.drives
                delegate: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: modelData.name
                            color: "white"
                            font.bold: true
                            Layout.alignment: Qt.AlignLeft
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: modelData.usage.toFixed(1) + "%"
                            color: "#00ff00"
                            Layout.alignment: Qt.AlignLeft
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        value: modelData.usage / 100.0

                        background: Rectangle {
                            height: 4
                            color: "#333333"
                            radius: 2
                        }

                        contentItem: Item {
                            Rectangle {
                                width: parent.visualPosition * parent.width
                                height: 4
                                radius: 2
                                color: modelData.usage > 90 ? "#ff4444" : "#00ff00"
                            }
                        }
                    }

                    Text {
                        text: modelData.available.toFixed(1) + " GB free of " +
                              modelData.total.toFixed(1) + " GB"
                        Layout.fillWidth: true
                        font.pixelSize: 10
                        color: "#888888"
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333333"
        }

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
