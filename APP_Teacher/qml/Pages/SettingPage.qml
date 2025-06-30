import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import DelegateUI

import "../Controls/"

MyPage {
    titleIconSource: DelIcon.SettingOutlined
    titleText: qsTr("设置")
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

            Column {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                spacing: 8

                Text {
                    font {
                        family: DelTheme.Primary.fontPrimaryFamily
                        pixelSize: DelTheme.Primary.fontPrimarySizeHeading4
                        weight: Font.DemiBold
                    }
                    color: DelTheme.Primary.colorTextBase
                    text: qsTr("网络")
                }

                Rectangle {
                    width: parent.width
                    height: 120
                    color: DelTheme.DelCard.colorBg
                    border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                    opacity: 0.8
                    radius: 10

                    Text {
                        text: "此栏目设置：中心服务器地址"
                        anchors.centerIn: parent
                    }
                }
            }

            Column {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                spacing: 8

                Text {
                    font {
                        family: DelTheme.Primary.fontPrimaryFamily
                        pixelSize: DelTheme.Primary.fontPrimarySizeHeading4
                        weight: Font.DemiBold
                    }
                    color: DelTheme.Primary.colorTextBase
                    text: qsTr("个性化")
                }

                Rectangle {
                    width: parent.width
                    height: 120
                    color: DelTheme.DelCard.colorBg
                    border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                    opacity: 0.8
                    radius: 10

                    Text {
                        text: "此栏目设置：主题色、背景透明度、深/浅色模式、动画开关等"
                        anchors.centerIn: parent
                    }
                }
            }

            Column {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(1000, column.width)
                spacing: 8

                Text {
                    font {
                        family: DelTheme.Primary.fontPrimaryFamily
                        pixelSize: DelTheme.Primary.fontPrimarySizeHeading4
                        weight: Font.DemiBold
                    }
                    color: DelTheme.Primary.colorTextBase
                    text: qsTr("关于")
                }

                Rectangle {
                    width: parent.width
                    height: 120
                    color: DelTheme.DelCard.colorBg
                    border.color: DelTheme.isDark ? DelTheme.DelCard.colorBorderDark : DelTheme.DelCard.colorBorder
                    opacity: 0.8
                    radius: 10

                    Image {
                        id: logo
                        source: "qrc:/res/images/app_logo.png"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        width: 80
                        height: 80
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        anchors.left: logo.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 20
                        font {
                            family: DelTheme.Primary.fontPrimaryFamily
                            pixelSize: DelTheme.Primary.fontPrimarySize
                        }
                        color: DelTheme.Primary.colorTextBase
                        text: qsTr(Qt.application.displayName + '\n' +
                            "版本: " + Qt.application.version + '\n' +
                            Qt.application.organization + '\n'
                            + "<b>协议、隐私政策、使用条款等补充好</b>"
                        )
                    }

                }
            }


        }
    }
}

