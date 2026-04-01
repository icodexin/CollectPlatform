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

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 0

        SingleLineChart {
            id: breathWaveChart
            marginLeft: -10
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: root.currentRespiratoryRate > 0
                ? qsTr("mmWav 呼吸波形  %1 次/分").arg(Number(root.currentRespiratoryRate).toFixed(1))
                : qsTr("mmWav 呼吸波形")
            lineColor: "green"
            yLabelFormat: "%.3f"
        }

        SingleLineChart {
            id: heartWaveChart
            marginLeft: -10
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: root.currentHeartRate > 0
                ? qsTr("mmWav 心搏波形  %1 BPM").arg(Number(root.currentHeartRate).toFixed(1))
                : qsTr("mmWav 心搏波形")
            lineColor: "darkmagenta"
            yLabelFormat: "%.3f"
        }
    }

    function safeNumber(v) {
        const n = Number(v)
        return Number.isFinite(n) ? n : 0
    }

    function safePoint(p) {
        if (!p)
            return Qt.point(0, 0)

        const x = safeNumber(p.x)
        const y = safeNumber(p.y)
        return Qt.point(x, y)
    }

    function updateFrame(frame) {
        if (!frame)
            return

        const breathPoint = safePoint(frame["breathWavePoint"])
        const heartPoint = safePoint(frame["heartWavePoint"])

        breathWaveChart.append_point(breathPoint)
        heartWaveChart.append_point(heartPoint)

        const rr = Number(frame["respiratoryRate"])
        currentRespiratoryRate = (Number.isFinite(rr) && rr > 0) ? rr : -1

        const hr = Number(frame["heartRate"])
        currentHeartRate = (Number.isFinite(hr) && hr > 0) ? hr : -1
    }

    function resetView() {
        if (breathWaveChart.resetChart)
            breathWaveChart.resetChart()
        if (heartWaveChart.resetChart)
            heartWaveChart.resetChart()

        currentRespiratoryRate = -1
        currentHeartRate = -1
    }
}