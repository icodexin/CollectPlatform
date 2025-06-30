pragma ComponentBehavior: Bound
import QtQuick
import QtGraphs

DateTimeAxis {
    id: axis

    property font labelFont
    property color labelColor

    labelFormat: "yyyy/MM/dd/hh/mm/ss"
    labelDelegate: Item {
        property string text
        Text {
            text: {
                // 因为DateTimeAxis默认显示的时间为UTC时间，这里转换为本地时间显示
                const [year, month, day, hour, minute, second] = parent.text.split("/")
                const date = new Date(Date.UTC(year, month - 1, day, hour, minute, second))
                return Qt.formatDateTime(date, "hh:mm:ss")
            }
            font: axis.labelFont
            color: axis.labelColor
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}