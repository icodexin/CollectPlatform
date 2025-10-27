import QtQuick
import QtGraphs
import QtQuick.Shapes
import HuskarUI.Basic
import MuApp
import "./util.js" as ChartUtil

Item {
    id: control

    property alias titleText: title.text
    property alias titleVisible: title.visible
    property alias titleFont: title.font
    property alias titleFormat: title.textFormat
    property alias timeSize: axisX.timeSize
    property int timeTickCount: 3
    property alias minY: axisY.min
    property alias maxY: axisY.max
    property alias minYRange: axisY.minRange
    property int yTickCount: 5
    property alias yLabelFormat: axisY.labelFormat
    property bool autoScaleY: true
    property real lineWidth: 2.0
    property var lineNames: []
    property alias legendRightMargin: legend.anchors.rightMargin
    property alias marginLeft: view.marginLeft
    property alias marginTop: view.marginTop
    property alias marginRight: view.marginRight
    property alias marginBottom: view.marginBottom

    HusText {
        id: title
        font.weight: Font.Bold
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    GraphsView {
        id: view

        anchors.top: control.titleVisible ? title.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        marginLeft: 0
        marginBottom: 0

        theme: MuGraphsTheme {
            theme: GraphsTheme.Theme.MixSeries
        }

        axisX: MuDateTimeAxis {
            id: axisX
            tickInterval: control.timeTickCount - 1
            gridVisible: view.theme.gridVisible
            subGridVisible: false
            labelFont: view.theme.axisXLabelFont
            labelColor: view.theme.axisX.mainColor
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
    }

    Item {
        id: legend
        anchors.right: parent.right
        anchors.rightMargin: 5
        width: legendCol.width
        height: legendCol.height
        property real relativeY: 0.05
        y: (parent.height - height) * relativeY

        Rectangle {
            anchors.fill: parent
            color: HusTheme.Primary.colorBgContainer
            border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
            opacity: 0.7
            radius: 2
        }

        Column {
            id: legendCol
            spacing: 2
            padding: 5

            Repeater {
                model: legendModel
                delegate: legendEntryDelegate
            }
        }

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: control.height - legend.height
            cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            onPositionChanged: legend.relativeY = legend.y / (control.height - legend.height)
        }
    }

    ListModel {
        id: legendModel
    }

    Component {
        id: legendEntryDelegate
        Row {
            id: legendEntry
            spacing: 8
            required property var model
            required property int index
            property color lineColor: model.color
            property real lineWidth: model.width
            property string lineName: model.name

            Shape {
                id: legendMark
                width: 20
                height: legendText.height
                anchors.verticalCenter: parent.verticalCenter
                ShapePath {
                    strokeColor: legendEntry.lineColor
                    fillColor: legendEntry.lineColor
                    strokeWidth: legendEntry.lineWidth
                    startX: 0
                    startY: legendMark.height / 2
                    PathLine {x: legendMark.width; relativeY: 0}
                }
            }
            HusText {
                id: legendText
                text: parent.lineName
                font.pixelSize: 10
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component.onCompleted: {
        let component = Qt.createComponent("MuApp", "MuLineSeries");
        for (let i = 0; i < lineNames.length; i++) {
            const lineName = lineNames[i]
            ChartUtil.createObject(component, view, {name: lineName, width: lineWidth})
                .then(series => {
                    view.addSeries(series)
                    legendModel.append({
                        name: series.name,
                        width: series.width,
                        color: view.theme.seriesColors[i % view.theme.seriesColors.length]
                    })
                })
                .catch(err => {
                    console.error("Error:", err)
                })
        }
    }

    function append_point(...pointsArray) {
        console.assert(view.seriesList.length === pointsArray.length, "view.seriesList.length !== pointsArray.length")
        console.assert(
            // 情况1：所有元素都是数组，且长度一致
            (pointsArray.every(p => "length" in p) &&
                pointsArray.every(p => p.length === pointsArray[0].length)) ||
            // 情况2：所有元素都是对象，且都有 x 和 y 属性
            pointsArray.every(p => typeof p === "object" && p !== null && "x" in p && "y" in p),
            "All points must be either arrays of the same length or objects with both 'x' and 'y' properties"
        );

        // 遍历每个序列，添加数据点
        for (let i = 0; i < pointsArray.length; i++) {
            let series = view.seriesList[i]
            series.append(pointsArray[i])
        }
        // 维护Y轴范围
        if ("x" in pointsArray[0])
            minMaxY.appendParallelPoints(pointsArray)
        else
            minMaxY.appendParallelLines(pointsArray)
        // 更新X轴范围
        axisX.rescaleBySeries(view.seriesList[0])
        // 移除过旧的数据点
        for (let series of view.seriesList) {
            series.removePointsBeforeTime(axisX.min)
        }
        // 维护Y轴范围
        minMaxY.removeUntil(axisX.min)
    }

    function append_line_point(index, point) {
        view.seriesList[index].append(point)
    }

    function auto_rescale(minMaxPoints) {
        minMaxY.append(minMaxPoints)
        axisX.rescaleBySeries(view.seriesList[0])
        for (let series of view.seriesList) {
            series.removePointsBeforeTime(axisX.min)
        }
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