import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

Item {
    id: root

    property real currentHeartRate: -1
    property real currentRespiratoryRate: -1

    property string heartTitle: currentHeartRate > 0
        ? qsTr("BCG 心率  %1 bpm").arg(Number(currentHeartRate).toFixed(1))
        : qsTr("BCG 心率")

    property string respTitle: currentRespiratoryRate > 0
        ? qsTr("BCG 呼吸波  %1 次/分").arg(Number(currentRespiratoryRate).toFixed(1))
        : qsTr("BCG 呼吸波")

    Rectangle {
        anchors.fill: parent
        color: HusTheme.HusCard.colorBg
        border.color: HusTheme.isDark
            ? HusTheme.HusCard.colorBorderDark
            : HusTheme.HusCard.colorBorder
        opacity: 0.8
        radius: 10
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 0

        SingleLineChart {
            id: bcgHeartChart
            Layout.fillWidth: true
            Layout.fillHeight: true
            titleText: root.heartTitle
            lineColor: "red"
        }

        SingleLineChart {
            id: bcgRespChart
            Layout.fillWidth: true
            Layout.fillHeight: true
            titleText: root.respTitle
            lineColor: "blue"
        }
    }

    function updateFrame(frame) {
        if (!frame)
            return

        const heartPoint = frame.heartWavePoint
        const respPoint  = frame.respWavePoint

        if (heartPoint && heartPoint.x > 0) {
            bcgHeartChart.append_data(heartPoint.x, heartPoint.y)
        }

        if (respPoint && respPoint.x > 0) {
            bcgRespChart.append_data(respPoint.x, respPoint.y)
        }

        if (frame.heartRate !== undefined && frame.heartRate > 0) {
            currentHeartRate = frame.heartRate
        }

        if (frame.respiratoryRate !== undefined && frame.respiratoryRate > 0) {
            currentRespiratoryRate = frame.respiratoryRate
        }
    }

    function resetView() {
        bcgHeartChart.resetChart()
        bcgRespChart.resetChart()

        currentHeartRate = -1
        currentRespiratoryRate = -1
    }
}