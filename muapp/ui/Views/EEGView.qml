import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls.Basic
import HuskarUI.Basic
import MuApp

Item {
    id: view

    property var chNames: [
        "Fp1", "Fp2", "F7", "F3", "Fz", "F4", "F8", "A1", "T3", "C3",
        "Cz", "C4", "T4", "A2", "T5", "P3", "P4", "T6", "O1", "O2",
        "CM", "X1", "X2", "X3"
    ]

    Rectangle {
        id: background
        anchors.fill: parent
        color: HusTheme.HusCard.colorBg
        border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
        opacity: 0.8
        radius: 10
    }

    HusText {
        id: title
        text: "脑电 EEG (μV)"
        font.weight: Font.Bold
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Flickable {
        clip: true
        anchors.left: parent.left
        anchors.top: title.bottom
        anchors.right: parent.right
        anchors.bottom: startTimeLabel.top
        anchors.margins: 8
        rightMargin: 15
        contentHeight: chCol.height
        ScrollBar.vertical: HusScrollBar {}

        ColumnLayout {
            id: chCol
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 2
        }
    }

    HusText {
        id: startTimeLabel
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        text: "--:--:--"

        function setTime(time) {
            text = Qt.formatTime(time, "hh:mm:ss");
        }
    }

    HusText {
        id: endTimeLabel
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        text: "--:--:--"

        function setTime(time) {
            text = Qt.formatTime(time, "hh:mm:ss");
        }
    }

    Component {
        id: chComponent
        Item {
            property alias chName: chLabel.text
            property alias labelFont: chLabel.font
            property alias labelWidth: chLabel.width
            property alias chart: chChart
            property alias lineColor: chChart.lineColor

            HusText {
                id: chLabel
                text: "CH"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Shape {
                id: chBaseLine
                anchors.fill: chChart
                ShapePath {
                    strokeColor: "gray"
                    fillColor: "gray"
                    strokeWidth: 1
                    startX: 0
                    startY: chBaseLine.height / 2
                    PathLine {x: chBaseLine.width; relativeY: 0}
                }
            }

            SingleLineChart {
                id: chChart
                anchors.left: chLabel.right
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 4
                titleVisible: false
                xVisible: false
                yVisible: false
                xLineVisible: false
                yLineVisible: false
                marginTop: 0
                marginRight: 0
            }
        }
    }

    FontMetrics {
        id: labelFontMetrics
        font.family: HusTheme.Primary.fontPrimaryFamily
        font.pixelSize: HusTheme.Primary.fontPrimarySize
    }

    Component.onCompleted: {
        let maxLabelWidth = 0;
        for (const ch of chNames) {
            maxLabelWidth = Math.max(maxLabelWidth, labelFontMetrics.boundingRect(ch).width);
        }
        let bindTimeLabel = false;
        for (const ch of chNames) {
            let obj = chComponent.createObject(chCol, {
                chName: ch,
                "Layout.fillWidth": true,
                height: 50,
                labelFont: labelFontMetrics.font,
                labelWidth: maxLabelWidth,
                lineColor: Qt.rgba(
                    Math.floor(Math.random() * 128) / 255,
                    Math.floor(Math.random() * 128) / 255,
                    Math.floor(Math.random() * 128) / 255,
                    1
                )
            })
            if (!bindTimeLabel) {
                obj.chart.minXChanged.connect((min) => startTimeLabel.setTime(min));
                obj.chart.maxXChanged.connect((max) => endTimeLabel.setTime(max));
                bindTimeLabel = true;
            }
        }
    }

    Timer {
        interval: 33; running: true; repeat: true
        onTriggered: {
            for (let i = 0; i < chNames.length; i++) {
                const chItem = chCol.children[i];
                const chChart = chItem.chart;
                const now = Date.now();
                const value = Math.sin(now / 500) * 50; // 绘制带高斯噪声的 sin 曲线，范围在 -50 到 50 之间
                chChart.append_point(Qt.point(now, value));
            }
        }
    }
}