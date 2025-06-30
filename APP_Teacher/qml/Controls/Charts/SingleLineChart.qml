import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtGraphs
import DelegateUI

import TeacherApp 1.0
import "./util.js" as ChartUtil

Item {
    id: control

    property alias titleText: title.text
    property alias titleVisible: title.visible
    property alias titleFont: title.font
    property alias titleFormat: title.textFormat
    property real timeSize: 10 * 1000 // 10s
    property int timeTickCount: 3
    property real minY: -1
    property real maxY: 1
    property real minRange: 2
    property int yTickCount: 5
    property string yLabelFormat: "%.1f"
    property bool autoScaleY: true
    property alias lineWidth: lineSeries.width
    property alias lineColor: lineSeries.color

    property alias marginLeft: view.marginLeft
    property alias marginRight: view.marginRight

    /**
     * 添加一个新数据点
     * @param msecs - 毫秒时间戳
     * @param y - 数据点的值
     */
    function append_data(msecs, y) {
        // 添加数据点
        lineSeries.append(msecs, y)
        // 维护Y轴最小最大值
        minMaxY.append(y)
        // 更新X轴范围
        ChartUtil.rescaleTimeAxisBySeries(view.axisX, lineSeries, control.timeSize)
        // 移除过旧的数据点
        ChartUtil.removePointsBeforeTime(lineSeries, view.axisX.min.getTime(), minMaxY.remove)
    }

    /**
     * 添加一个新数据点或多个数据点
     * @param {point | point[]} point - 数据点或数据点数组
     */
    function append_point(point) {
        // 添加数据点或多个数据点
        lineSeries.append(point);

        if (Array.isArray(point)) {
            for (let p of point) {
                minMaxY.append(p.y);
            }
        } else {
            minMaxY.append(point.y);
        }

        // 更新X轴范围
        ChartUtil.rescaleTimeAxisBySeries(view.axisX, lineSeries, control.timeSize)
        // 移除过旧的数据点
        ChartUtil.removePointsBeforeTime(lineSeries, view.axisX.min.getTime(), minMaxY.remove)
    }

    MinMaxQueue {
        id: minMaxY

        onMinMaxChanged: {
            if (control.autoScaleY)
                axisY.rescale(min, max, control.minRange)
        }
    }

    Text {
        id: title
        font.family: view.theme.labelFont.family
        font.pixelSize: 12
        font.weight: Font.Bold
        color: view.theme.labelTextColor
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 10
    }

    GraphsView {
        id: view

        anchors.top: control.titleVisible ? title.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        marginLeft: 0
        marginBottom: 0

        theme: MyGraphsTheme {}

        axisX: MyDateTimeAxis {
            max: new Date(min.getTime() + control.timeSize)
            // tickInterval: control.timeTickCount > 1 ? control.timeSize / (control.timeTickCount - 1) : 0
            tickInterval: control.timeTickCount - 1
            gridVisible: view.theme.gridVisible
            subGridVisible: false
            labelFont: view.theme.axisXLabelFont
            labelColor: view.theme.axisX.mainColor
        }

        axisY: MyValueAxis {
            id: axisY
            min: control.minY
            max: control.maxY
            tickInterval: control.yTickCount > 1 ? (max - min) / (control.yTickCount - 1) : 0
            labelFormat: control.yLabelFormat
            titleVisible: false
            gridVisible: view.theme.gridVisible
            subGridVisible: false
        }

        LineSeries {
            id: lineSeries
        }
    }

}