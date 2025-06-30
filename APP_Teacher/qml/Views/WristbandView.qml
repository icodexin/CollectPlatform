import QtQuick
import QtQuick.Layouts
import DelegateUI
import "../Controls/Charts/"

Item {
    required property string studentId

    function append_data(data) {
        const timestamp = data.timestamp;

        const pulseWavePoints = data.pulseWaves.map((wave, i) => Qt.point(timestamp + i * 20, wave.filtedVal));
        const hrPoint = Qt.point(timestamp, data.hr);
        const gsrPoints = data.gsrs.map((gsr, i) => Qt.point(timestamp + i * 20, gsr));

        const accXPoints = data.accs.map((acc, i) => Qt.point(timestamp + i * 20, acc.x));
        const accYPoints = data.accs.map((acc, i) => Qt.point(timestamp + i * 20, acc.y));
        const accZPoints = data.accs.map((acc, i) => Qt.point(timestamp + i * 20, acc.z));

        pulseWaveChart.append_point(pulseWavePoints);
        hrChart.append_point(hrPoint);
        gsrChart.append_point(gsrPoints);
        accChart.append_point(accXPoints, accYPoints, accZPoints);
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: DelTheme.DelCard.colorBg
        border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
        opacity: 0.8
        radius: 10
    }

    GridLayout {
        id: view
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: label.top
        anchors.margins: 8
        anchors.bottomMargin: 4
        columns: 2

        SingleLineChart {
            id: pulseWaveChart
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("脉搏波 Pulse Wave")
            lineColor: 'red'
        }

        SingleLineChart {
            id: hrChart
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("心率 HR (bpm)")
            lineColor: 'darkmagenta'
        }

        SingleLineChart {
            id: gsrChart
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("皮电 GSR (μS)")
            lineColor: 'green'
            minY: 0
            maxY: 0.01
            minRange: 0.01
            yLabelFormat: '%.3f'
        }

        MultiLineChart {
            id: accChart
            Layout.fillWidth: true
            Layout.fillHeight: true

            titleText: qsTr("加速度 ACC (m<sup>2</sup>/s)")
            titleFormat: Text.RichText
            lineNames: ["X", "Y", "Z"]
        }
    }

    Text {
        id: label
        text: parent.studentId
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        font.family: DelTheme.Primary.fontPrimaryFamily
        font.pixelSize: 12
        color: DelTheme.Primary.colorTextBase
    }
}
