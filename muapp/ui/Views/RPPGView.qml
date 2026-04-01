import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

Item {
    id: root

    property real currentHeartRate: -1
    property real currentRespiratoryRate: -1

    Rectangle {
        id: background
        anchors.fill: parent
        color: HusTheme.HusCard.colorBg
        border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
        opacity: 0.8
        radius: 10
    }

    RowLayout  {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 0

        SingleLineChart {
            id: heartChart
            marginLeft: -10
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: root.currentHeartRate > 0
                ? qsTr("rPPG 心率波形  %1 BPM").arg(Number(root.currentHeartRate).toFixed(1))
                : qsTr("rPPG 心率波形")
            lineColor: "red"
        }

        SingleLineChart {
            id: respChart
            marginLeft: -10
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: root.currentRespiratoryRate > 0
                ? qsTr("rPPG 呼吸波形  %1 次/分").arg(Number(root.currentRespiratoryRate).toFixed(1))
                : qsTr("rPPG 呼吸波形")
            lineColor: "deepskyblue"
        }
    }

    function updateFrame(frame) {
        if (!frame)
            return

        if (frame.heartWavePoint)
            heartChart.append_point(frame.heartWavePoint)

        if (frame.respWavePoint)
            respChart.append_point(frame.respWavePoint)

        const hr = Number(frame.heartRate)
        if (Number.isFinite(hr) && hr > 0) {
            currentHeartRate = hr
        }

        const rr = Number(frame.respiratoryRate)
        if (Number.isFinite(rr) && rr > 0) {
            currentRespiratoryRate = rr
        }
    }

    function resetView() {
        if (heartChart.resetChart)
            heartChart.resetChart()
        if (respChart.resetChart)
            respChart.resetChart()

        currentHeartRate = -1
        currentRespiratoryRate = -1
    }
}