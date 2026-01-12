import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Item {
    anchors.fill: parent

    RowLayout {
        anchors.fill: parent
        spacing:5

        LeftComponent {
            width: parent.width * 0.25
            Layout.fillHeight: true
        }

        CenterComponent {
            width: parent.width * 0.5
            Layout.fillHeight: true
        }

        RightComponent {
            width: parent.width * 0.25
            Layout.fillHeight: true
        }
    }

}
