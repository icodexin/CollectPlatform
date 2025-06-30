import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import DelegateUI

import "../Controls/"

MyPage {
    titleIconSource: DelIcon.HomeOutlined
    titleText: Qt.application.displayName
    rightMargin: 5

    Flickable {
        clip: true
        anchors.fill: parent
        rightMargin: 15
        contentHeight: column.height
        ScrollBar.vertical: DelScrollBar {}

        ColumnLayout {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 20

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                Layout.preferredHeight: 120
                color: DelTheme.DelCard.colorBg
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.8
                radius: 10

                Text {
                    text: "展示注册学生数量、在线人数"
                    anchors.centerIn: parent
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                Layout.preferredHeight: 120
                color: DelTheme.DelCard.colorBg
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.8
                radius: 10

                Text {
                    text: "正在进行的采集活动"
                    anchors.centerIn: parent
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                Layout.preferredHeight: 120
                color: DelTheme.DelCard.colorBg
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.8
                radius: 10

                Text {
                    text: "最近的采集活动"
                    anchors.centerIn: parent
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                Layout.preferredHeight: 120
                color: DelTheme.DelCard.colorBg
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.8
                radius: 10

                Text {
                    text: "中心服务器状态"
                    anchors.centerIn: parent
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                Layout.preferredHeight: 120
                color: DelTheme.DelCard.colorBg
                border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                opacity: 0.8
                radius: 10

                Text {
                    text: "数据库状态"
                    anchors.centerIn: parent
                }
            }

        }
    }
}
