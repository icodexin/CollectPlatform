pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Shapes 1.8
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
    property int yTickCount: 5
    property string yLabelFormat: "%.1f"
    property bool autoScaleY: true
    property real lineWidth: 2.0
    property var lineNames: []

    property real legendMarginRight: 5

    property alias marginLeft: view.marginLeft
    property alias marginRight: view.marginRight

    /**
     * 添加一个新数据点
     * @param msecs - 毫秒时间戳
     * @param y_list - 数据点的值列表
     */
    function append_data(msecs, y_list) {
        console.assert(view.seriesList.length === y_list.length, "view.seriesList.length !== y_list.length")

        // 遍历每个序列
        for (let i = 0; i < y_list.length; i++) {
            let series = view.seriesList[i]
            // 添加数据点
            series.append(msecs, y_list[i]);
            // 维护Y轴最小最大值
            minMaxY.append(y_list[i])
        }

        // 更新X轴范围
        ChartUtil.rescaleTimeAxisBySeries(view.axisX, view.seriesList[0], control.timeSize)
        // 移除过旧的数据点
        for (let series of view.seriesList) {
            ChartUtil.removePointsBeforeTime(series, view.axisX.min.getTime(), minMaxY.remove)
        }
    }

    function append_point(...args) {
        console.assert(view.seriesList.length === args.length, "view.seriesList.length !== args.length")

        // 遍历每个序列
        for (let i = 0; i < args.length; i++) {
            let series = view.seriesList[i]
            // 添加数据点
            series.append(args[i]);
            // 维护Y轴最小最大值
            if (Array.isArray(args[i])) {
                for (let p of args[i]) {
                    minMaxY.append(p.y);
                }
            } else {
                minMaxY.append(args[i].y);
            }
        }

        // 更新X轴范围
        ChartUtil.rescaleTimeAxisBySeries(view.axisX, view.seriesList[0], control.timeSize)
        // 移除过旧的数据点
        for (let series of view.seriesList) {
            ChartUtil.removePointsBeforeTime(series, view.axisX.min.getTime(), minMaxY.remove)
        }
    }

    // todo: MinMax队列一次性添加多个值
    MinMaxQueue {
        id: minMaxY

        onMinMaxChanged: {
            if (control.autoScaleY)
                axisY.rescale(min, max)
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

        theme: MyGraphsTheme {
            theme: GraphsTheme.Theme.MixSeries
        }

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
    }

    // 用于动态生成Legend
    Component {
        id: legendComponent
        Item {
            id: legend
            required property real rightMargin
            property real relativeY: 0.05
            y: (parent.height - height) * relativeY
            width: legendColumn.width
            height: legendColumn.height
            anchors.right: parent.right
            anchors.rightMargin: rightMargin

            Rectangle {
                color: DelTheme.Primary.colorBgContainer
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.7
                anchors.fill: legendColumn
                radius: 5
            }

            Column {
                id: legendColumn
                spacing: 2
                leftPadding: 5
                topPadding: 5
                rightPadding: 5
                bottomPadding: 5

                Repeater {
                    model: view.seriesList
                    delegate: Row {
                        id: legendDelegate
                        spacing: 8
                        required property var model
                        required property int index

                        property color lineColor: view.theme.seriesColors[index % view.theme.seriesColors.length]

                        Shape {
                            id: legendMark
                            width: 20
                            height: legendText.height
                            ShapePath {
                                strokeColor: legendDelegate.lineColor
                                fillColor: legendDelegate.lineColor
                                strokeWidth: legendDelegate.model.width
                                startX: 0
                                startY: legendMark.height / 2
                                PathLine {x: legendMark.width; relativeY: 0}
                            }
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            id: legendText
                            text: parent.model.name
                            font.family: view.theme.labelFont.family
                            font.pixelSize: 10
                            color: view.theme.labelTextColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: legend
                drag.target: legend
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: legend.parent.height - legend.height
                // 按下时解除图例对象y值的绑定
                onPressed: legend.y = legend.y
                // 释放时重新绑定
                onReleased: {
                    legend.relativeY = legend.y / (legend.parent.height - legend.height)
                    legend.y = Qt.binding(() => (legend.parent.height - legend.height) * legend.relativeY)
                }
            }
        }
    }

    // 用于动态生成LineSeries
    Component {
        id: lineSeriesComponent
        LineSeries { }
    }

    Component.onCompleted: {
        for (let lineName of control.lineNames) {
            const series = lineSeriesComponent.createObject(view, {
                name: lineName,
                width: lineWidth
            });
            view.addSeries(series);
        }
        legendComponent.createObject(control, {
            rightMargin: control.legendMarginRight,
        });
    }

}