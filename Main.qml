import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "UI"


Window {
    width: 1500
    height: 800
    visible: true
    title: qsTr("Smart Personal Dashboard")
    color: "#121212"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: 0
            
            background: Rectangle {
                color: "#1a1a1a"
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#333333"
                }
            }

            TabButton {
                text: qsTr("Dashboard")
                implicitWidth: 150
                implicitHeight: 40
                contentItem: Text {
                    text: parent.text
                    font.bold: true
                    color: parent.checked ? "#00ff00" : "#aaaaaa"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.checked ? "#112211" : "transparent"
                    border.color: parent.checked ? "#00ff00" : "transparent"
                    border.width: parent.checked ? 1 : 0
                    radius: 4
                }
            }

            TabButton {
                text: qsTr("File Events")
                implicitWidth: 150
                implicitHeight: 40
                contentItem: Text {
                    text: parent.text
                    font.bold: true
                    color: parent.checked ? "#00ff00" : "#aaaaaa"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.checked ? "#112211" : "transparent"
                    border.color: parent.checked ? "#00ff00" : "transparent"
                    border.width: parent.checked ? 1 : 0
                    radius: 4
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            HomeComponent {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            FileWatcherTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
