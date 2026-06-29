import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Pane {
    id: watcherTabRoot
    Layout.fillWidth: true
    Layout.fillHeight: true

    background: Rectangle {
        color: "#121212" // Dark background
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Title Row
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "FILE EVENTS"
                font.bold: true
                font.pixelSize: 18
                color: "white"
            }
            Item { Layout.fillWidth: true }
        }

        // Table Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            Rectangle { Layout.preferredWidth: 90; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "TIME"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 90; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "PID"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 150; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "PROCESS"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 110; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "EVENT"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.fillWidth: true; height: 30; color: "#333"; Text { anchors.left: parent.left; anchors.leftMargin: 15; anchors.verticalCenter: parent.verticalCenter; text: "PATH"; color: "#aaa"; font.bold: true } }
        }

        // ListView for Event Log
        ListView {
            id: eventList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: fileWatcherService.events
            spacing: 2

            delegate: Rectangle {
                width: eventList.width
                height: 38
                color: index % 2 === 0 ? "#1a1a1a" : "#222222"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    spacing: 0

                    Text {
                        Layout.preferredWidth: 90
                        text: modelData.time
                        color: "#888"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        Layout.preferredWidth: 90
                        text: modelData.pid
                        color: "#bbb"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        Layout.preferredWidth: 150
                        text: modelData.process
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }

                    // Event Type Badge
                    Item {
                        Layout.preferredWidth: 110
                        height: 24
                        Layout.alignment: Qt.AlignVCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: 90
                            height: 22
                            radius: 4
                            color: {
                                if (modelData.event === "CREATE") return "#112211";
                                if (modelData.event === "MODIFY") return "#221a11";
                                if (modelData.event === "DELETE") return "#221111";
                                return "#112222"; // READ / OPEN / etc
                            }
                            border.color: {
                                if (modelData.event === "CREATE") return "#00ff00";
                                if (modelData.event === "MODIFY") return "#ffaa00";
                                if (modelData.event === "DELETE") return "#ff4444";
                                return "#00ccff";
                            }
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData.event
                                font.bold: true
                                font.pixelSize: 11
                                color: {
                                    if (modelData.event === "CREATE") return "#00ff00";
                                    if (modelData.event === "MODIFY") return "#ffaa00";
                                    if (modelData.event === "DELETE") return "#ff4444";
                                    return "#00ccff";
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: modelData.path
                        color: "#ccc"
                        font.pixelSize: 12
                        anchors.leftMargin: 15
                        elide: Text.ElideLeft // Elide from the left to see the filename on the right when it's long
                    }
                }
            }
        }

        // Watched Paths Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            
            Text {
                text: "ACTIVE WATCH PATHS"
                font.bold: true
                font.pixelSize: 12
                color: "#888"
                Layout.fillWidth: true
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8
                
                Repeater {
                    model: fileWatcherService.watchedPaths
                    delegate: Rectangle {
                        height: 28
                        width: pathLabel.width + 36
                        color: "#2a2a2a"
                        border.color: "#444"
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 8

                            Text {
                                id: pathLabel
                                text: modelData
                                color: "#eee"
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                            }

                            // Remove button
                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: removeMouseArea.containsMouse ? "#442222" : "transparent"
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    anchors.centerIn: parent
                                    text: "×"
                                    color: removeMouseArea.containsMouse ? "#ff4444" : "#888"
                                    font.bold: true
                                    font.pixelSize: 14
                                }

                                MouseArea {
                                    id: removeMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: fileWatcherService.removePath(modelData)
                                }
                            }
                        }
                    }
                }
            }
        }

        // Bottom Controls Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Watch Path:"
                color: "#ccc"
                font.bold: true
                font.pixelSize: 13
            }

            TextField {
                id: watchPathInput
                Layout.fillWidth: true
                text: "/home/harshit"
                color: "white"
                font.pixelSize: 13
                padding: 8
                
                background: Rectangle {
                    color: "#1e1e1e"
                    border.color: parent.activeFocus ? "#00ff00" : "#444"
                    border.width: 1
                    radius: 4
                }
            }

            Button {
                text: "Add Path"
                flat: true
                Layout.preferredWidth: 100
                contentItem: Text {
                    text: "Add Path"
                    color: "#00ff00"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#113311" : "#112211"
                    radius: 4
                    border.color: "#00ff00"
                    border.width: 1
                }
                onClicked: {
                    if (watchPathInput.text.trim() !== "") {
                        fileWatcherService.addPath(watchPathInput.text.trim())
                    }
                }
            }

            Button {
                text: "Clear"
                flat: true
                Layout.preferredWidth: 80
                contentItem: Text {
                    text: "Clear"
                    color: "#ff4444"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#331111" : "#221111"
                    radius: 4
                    border.color: "#ff4444"
                    border.width: 1
                }
                onClicked: fileWatcherService.clearEvents()
            }
        }
    }
}
