import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts 1.15
Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ColumnLayout {
            spacing: 2
            Text { text: "Time: " + metrics.currentTime; color: "#ffffff" }
            Text { text: "CPU: " + metrics.cpuUsage.toFixed(1) + "%"; color: "#ffffff" }
            Text { text: "GPU Temp: " + metrics.gpuTemp.toFixed(1) + "°C"; color: "#ffffff" }
            Text { text: "Memory: " + metrics.memoryUsage.toFixed(1) + "%"; color: "#ffffff" }
            Text { text: "Upload: " + metrics.uploadSpeed.toFixed(1) + " MB/s"; color: "#ffffff" }
            Text { text: "Download: " + metrics.downloadSpeed.toFixed(1) + " MB/s"; color: "#ffffff" }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333333"
        }

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
                        Text { text: modelData.name; color: "white"; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Text { text: modelData.usage.toFixed(1) + "%"; color: "#00ff00" }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        value: modelData.usage / 100.0
                        background: Rectangle { height: 4; color: "#333333"; radius: 2 }
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
                        text: modelData.available.toFixed(1) + " GB free of " + modelData.total.toFixed(1) + " GB"
                        Layout.fillWidth: true
                        font.pixelSize: 12
                        color: "#888888"
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }
        
        Item { Layout.fillHeight: true }
    }
}
