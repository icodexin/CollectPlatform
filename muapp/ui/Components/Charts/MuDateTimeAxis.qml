pragma ComponentBehavior: Bound
import QtQuick
import QtGraphs

DateTimeAxis {
    id: axis

    property real timeSize: 10000 // 10s
    property font labelFont
    property color labelColor

    labelFormat: "yyyy-MM-ddTHH:mm:ss"
    labelDelegate: Item {
        property string text
        property var cachedDate : new Date(0)
        property string localTime

        onTextChanged: {
            const parts = text.split(/[-T:]/);
            const [year, month, day, hour, minute, second] = parts.map(p => parseInt(p, 10));
            cachedDate.setUTCFullYear(year, month - 1, day); // month 0-based
            cachedDate.setUTCHours(hour, minute, second, 0);
            localTime = Qt.formatDateTime(cachedDate, "hh:mm:ss")
        }

        Text {
            text: parent.localTime
            font: axis.labelFont
            color: axis.labelColor
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Component.onCompleted: {
        axis.max = new Date(min.getTime() + timeSize);
    }

    /**
     * 根据数据点动态调整时间轴范围
     * @param series - 数据序列
     */
    function rescaleBySeries(series) {
        if (series.count === 0)
            return;

        const minTime = series.at(0).x;
        const maxTime = series.at(series.count - 1).x;

        let start, end;
        if (maxTime - minTime >= axis.timeSize) {
            start = maxTime - axis.timeSize;
            end = maxTime;
        } else {
            start = minTime;
            end = minTime + axis.timeSize;
        }

        axis.min = new Date(start);
        axis.max = new Date(end);
    }
}