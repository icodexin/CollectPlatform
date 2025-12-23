import QtQuick 2.15
import QtQuick.Layouts 1.15
import HuskarUI.Basic

Rectangle {
    id: root
    radius: 8
    color: HusTheme.HusCard.bgColor || "#ffffff"
    border.color: HusTheme.HusCard.borderColor || "#e5e5e5"
    border.width: 1

    // ===== 对外属性 =====
    property string title: ""          // EEG / PPG / 视频 / 汇总
    property string emotion: "未知"     // 高兴 / 难过 / 中性
    property color emotionColor: "#bfbfbf"


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12
        Layout.alignment: Qt.AlignHCenter

        // 标题部分
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            width: titleText.width + 20

            // 边框样式配置
            border.color: "#e5e5e5"
            border.width: 1
            radius: 4
            color: "transparent"

            // 标题文本
            HusText {
                id: titleText
                anchors.centerIn: parent
                text: root.title
                font.bold: true
                font.pixelSize: 14
            }
        }

        // 情绪文本部分
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            width: emotionText.width + 20

            // 边框样式配置
            border.color: root.emotionColor
            border.width: 2
            radius: 4
            color: "transparent"

            // 情绪文本
            HusText {
                id: emotionText
                anchors.centerIn: parent
                text: root.emotion
                font.pixelSize: 24
                font.bold: true
                color: root.emotionColor
            }
        }
    }
}