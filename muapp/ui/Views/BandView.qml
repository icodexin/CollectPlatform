import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

Item {

    Rectangle {
        id: background
        anchors.fill: parent
        color: HusTheme.HusCard.colorBg
        border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
        opacity: 0.8
        radius: 10
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 8
        columns: 2

        SingleLineChart {
            id: pulseWaveChart
            marginLeft: -10
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("脉搏波 Pulse Wave")
            lineColor: 'red'
        }

        SingleLineChart {
            id: hrChart
            marginLeft: -10
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("心率 HR (bpm)")
            lineColor: 'darkmagenta'
        }

        SingleLineChart {
            id: gsrChart
            marginLeft: -10
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("皮电 GSR (μS)")
            lineColor: 'green'
            minY: 0
            maxY: 0.01
            minYRange: 0.01
            yLabelFormat: '%.3f'
        }

        MultiLineChart {
            id: accChart
            marginLeft: -10
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("加速度 ACC (m<sup>2</sup>/s)")
            titleFormat: Text.RichText
            lineNames: ["X", "Y", "Z"]
        }
    }

    function updateFrame(frame) {
        pulseWaveChart.append_point(frame.pulseWavePoint)
        hrChart.append_point(frame.hrPoint)
        gsrChart.append_point(frame.gsrPoint)
        accChart.append_point(frame.accXPoint, frame.accYPoint, frame.accZPoint)
    }
}