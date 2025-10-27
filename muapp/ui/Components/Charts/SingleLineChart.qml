import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtGraphs
import HuskarUI.Basic
import MuApp

Item {
    id: control

    property alias titleText: title.text
    property alias titleVisible: title.visible
    property alias titleFont: title.font
    property alias titleFormat: title.textFormat
    property alias timeSize: axisX.timeSize
    property int timeTickCount: 3
    property alias xVisible: axisX.visible
    property alias yVisible: axisY.visible
    property alias xLineVisible: axisX.lineVisible
    property alias yLineVisible: axisY.lineVisible
    property alias minY: axisY.min
    property alias maxY: axisY.max
    property alias minYRange: axisY.minRange
    property int yTickCount: 5
    property alias yLabelFormat: axisY.labelFormat
    property bool autoScaleY: true
    property alias lineWidth: series.width
    property alias lineColor: series.color
    property alias marginLeft: view.marginLeft
    property alias marginTop: view.marginTop
    property alias marginRight: view.marginRight
    property alias marginBottom: view.marginBottom

    signal minXChanged(var min)
    signal maxXChanged(var max)

    HusText {
        id: title
        font.weight: Font.Bold
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        // anchors.horizontalCenterOffset: 10
    }

    GraphsView {
        id: view

        anchors.top: control.titleVisible ? title.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        marginLeft: 0
        marginBottom: 0

        theme: MuGraphsTheme {}

        axisX: MuDateTimeAxis {
            id: axisX
            // tickInterval: control.timeTickCount > 1 ? control.timeSize / (control.timeTickCount - 1) : 0
            tickInterval: control.timeTickCount - 1
            gridVisible: view.theme.gridVisible
            subGridVisible: false
            labelFont: view.theme.axisXLabelFont
            labelColor: view.theme.axisX.mainColor

            onMinChanged: (min) => control.minXChanged(min)
            onMaxChanged: (max) => control.maxXChanged(max)
        }

        axisY: MuValueAxis {
            id: axisY
            min: -1
            max: 1
            minRange: 2
            tickInterval: control.yTickCount > 1 ? (max - min) / (control.yTickCount - 1) : 0
            labelFormat: "%.1f"
            titleVisible: false
            gridVisible: view.theme.gridVisible
            subGridVisible: false
        }

        MuLineSeries {
            id: series
        }
    }

    /**
     * 添加一个新数据点
     * @param msecs - 毫秒时间戳
     * @param y - 数据点的值
     */
    function append_data(msecs, y) {
        append_point(Qt.point(msecs, y))
    }

    /**
     * 添加一个新数据点或多个数据点
     * @param {point | point[]} point - 数据点或数据点数组
     */
    function append_point(point) {
        // 添加数据点或多个数据点
        series.append(point)
        // 维护Y轴范围
        minMaxY.append(point)
        // 更新X轴范围
        axisX.rescaleBySeries(series)
        // 移除过旧的数据点
        series.removePointsBeforeTime(axisX.min)
        // 维护Y轴范围
        minMaxY.removeUntil(axisX.min)
    }

    MinMaxQueue {
        id: minMaxY

        onMinMaxChanged: {
            if (control.autoScaleY)
                axisY.rescale(min, max)
        }
    }

}