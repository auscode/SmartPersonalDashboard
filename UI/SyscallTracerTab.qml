import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Pane {
    id: tracerTabRoot
    Layout.fillWidth: true
    Layout.fillHeight: true

    background: Rectangle {
        color: "#121212" // Dark background
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Title and Status Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            Text {
                text: "SYSCALL TRACER"
                font.bold: true
                font.pixelSize: 18
                color: "white"
            }

            Rectangle {
                Layout.preferredWidth: 320
                height: 32
                color: "#1e1e1e"
                border.color: syscallTracerService.active ? "#00ff00" : "#444"
                border.width: 1
                radius: 4

                Text {
                    anchors.centerIn: parent
                    text: {
                        if (syscallTracerService.active) {
                            if (syscallTracerService.targetPid !== -1) {
                                return "Target: [ PID: " + syscallTracerService.targetPid + " - " + syscallTracerService.targetName + " ]"
                            } else {
                                return "Target: [ Starting " + syscallTracerService.targetName + "... ]"
                            }
                        }
                        return "Target: [ Not Tracing ]"
                    }
                    font.bold: true
                    font.pixelSize: 12
                    color: syscallTracerService.active ? "#00ff00" : "#888"
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Detach / Kill"
                flat: true
                visible: syscallTracerService.active
                Layout.preferredWidth: 100
                contentItem: Text {
                    text: "Detach / Kill"
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
                onClicked: syscallTracerService.detach()
            }
        }

        // Trace Launch Controls (only visible when not active)
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: !syscallTracerService.active

            Text {
                text: "Binary Path:"
                color: "#ccc"
                font.bold: true
                font.pixelSize: 13
            }

            TextField {
                id: binaryPathInput
                Layout.preferredWidth: 200
                text: "/bin/ls"
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

            Text {
                text: "Args:"
                color: "#ccc"
                font.bold: true
                font.pixelSize: 13
            }

            TextField {
                id: binaryArgsInput
                Layout.fillWidth: true
                placeholderText: "e.g. -l -a (space separated)"
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
                text: "Launch & Trace"
                flat: true
                Layout.preferredWidth: 130
                contentItem: Text {
                    text: "Launch & Trace"
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
                    if (binaryPathInput.text.trim() !== "") {
                        var argsList = [];
                        var rawArgs = binaryArgsInput.text.trim();
                        if (rawArgs !== "") {
                            argsList = rawArgs.split(/\s+/);
                        }
                        syscallTracerService.traceBinary(binaryPathInput.text.trim(), argsList);
                    }
                }
            }
        }

        // Attach Manual PID Controls (only visible when not active)
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: !syscallTracerService.active

            Text {
                text: "Attach to PID:"
                color: "#ccc"
                font.bold: true
                font.pixelSize: 13
            }

            TextField {
                id: attachPidInput
                Layout.preferredWidth: 150
                placeholderText: "Enter PID to trace"
                color: "white"
                font.pixelSize: 13
                padding: 8
                validator: IntValidator { bottom: 1 }
                background: Rectangle {
                    color: "#1e1e1e"
                    border.color: parent.activeFocus ? "#00ff00" : "#444"
                    border.width: 1
                    radius: 4
                }
            }

            Button {
                text: "Attach to Process"
                flat: true
                Layout.preferredWidth: 150
                contentItem: Text {
                    text: "Attach to Process"
                    color: "#00ccff"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#112b33" : "#11222b"
                    radius: 4
                    border.color: "#00ccff"
                    border.width: 1
                }
                onClicked: {
                    if (attachPidInput.text.trim() !== "") {
                        var pidVal = parseInt(attachPidInput.text.trim());
                        syscallTracerService.attachPid(pidVal, "PID " + pidVal);
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        // Table Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            Rectangle { Layout.preferredWidth: 90; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "TIME"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 150; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "SYSCALL"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.fillWidth: true; height: 30; color: "#333"; Text { anchors.left: parent.left; anchors.leftMargin: 15; anchors.verticalCenter: parent.verticalCenter; text: "ARGUMENTS"; color: "#aaa"; font.bold: true } }
            Rectangle { Layout.preferredWidth: 150; height: 30; color: "#333"; Text { anchors.centerIn: parent; text: "RETURN"; color: "#aaa"; font.bold: true } }
        }

        // ListView for Syscall Log
        ListView {
            id: syscallList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: syscallTracerService.events
            spacing: 2

            delegate: Rectangle {
                width: syscallList.width
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

                    // Syscall name badge colored by category
                    Item {
                        Layout.preferredWidth: 150
                        height: 24
                        Layout.alignment: Qt.AlignVCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: 130
                            height: 22
                            radius: 4
                            color: {
                                var cat = modelData.category;
                                if (cat === "file") return "#112222";
                                if (cat === "network") return "#221d11";
                                if (cat === "process") return "#112211";
                                if (cat === "memory") return "#221122";
                                if (cat === "signal") return "#111d22";
                                return "#222"; // other
                            }
                            border.color: {
                                var cat = modelData.category;
                                if (cat === "file") return "#00ccff";
                                if (cat === "network") return "#ffa500";
                                if (cat === "process") return "#00ff00";
                                if (cat === "memory") return "#ff77ff";
                                if (cat === "signal") return "#3399ff";
                                return "#bbbbbb"; // other
                            }
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData.syscall
                                font.bold: true
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                color: {
                                    var cat = modelData.category;
                                    if (cat === "file") return "#00ccff";
                                    if (cat === "network") return "#ffa500";
                                    if (cat === "process") return "#00ff00";
                                    if (cat === "memory") return "#ff77ff";
                                    if (cat === "signal") return "#3399ff";
                                    return "#bbbbbb"; // other
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: modelData.args
                        color: "#ccc"
                        font.pixelSize: 12
                        anchors.leftMargin: 15
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 150
                        text: modelData.ret
                        color: modelData.ret.startsWith("-1") ? "#ff4444" : "#00ff00"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }
            }
        }

        // Bottom Controls / Export Bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Syscalls Traced: " + syscallTracerService.events.length
                color: "#888"
                font.bold: true
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 16
                color: "#444"
            }

            Text {
                text: "Export Path:"
                color: "#ccc"
                font.bold: true
                font.pixelSize: 13
            }

            TextField {
                id: exportPathInput
                Layout.fillWidth: true
                text: "/home/auscode/Downloads/seccomp_policy.json"
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
                text: "Export Seccomp Policy"
                flat: true
                Layout.preferredWidth: 180
                contentItem: Text {
                    text: "Export Seccomp Policy"
                    color: "#00ccff"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#112b33" : "#11222b"
                    radius: 4
                    border.color: "#00ccff"
                    border.width: 1
                }
                onClicked: {
                    if (exportPathInput.text.trim() !== "") {
                        syscallTracerService.exportSeccompPolicy(exportPathInput.text.trim());
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
                onClicked: syscallTracerService.clearEvents()
            }
        }
    }
}
