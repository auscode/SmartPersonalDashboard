import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts 1.15
Pane {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1

    ColumnLayout{
        spacing: 2

        // New metric displays bound to MetricsService context property
    Text {
        text: "Time: " + metrics.currentTime
        color: "#ffffff"
    }
    Text {
        text: "CPU: " + metrics.cpuUsage.toFixed(1) + "%"
        color: "#ffffff"
    }
    Text {
        text: "GPU Temp: " + metrics.gpuTemp.toFixed(1) + "°C"
        color: "#ffffff"
    }
    Text {
        text: "Memory: " + metrics.memoryUsage.toFixed(1) + "%"
        color: "#ffffff"
    }
    Text {
        text: "Upload: " + metrics.uploadSpeed.toFixed(1) + " MB/s"
        color: "#ffffff"
    }
    Text {
        text: "Download: " + metrics.downloadSpeed.toFixed(1) + " MB/s"
        color: "#ffffff"
    }    }
}
