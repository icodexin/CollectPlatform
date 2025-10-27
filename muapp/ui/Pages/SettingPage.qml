import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic
import MuApp

MuFlickablePage {
    titleIconSource: HusIcon.SettingOutlined
    titleText: qsTr("设置")

    ColumnLayout {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 20

        MuPanel {
            title: "网络"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "此栏目设置：中心服务器地址"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            title: "个性化"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "此栏目设置：主题色、背景透明度、深/浅色模式、动画开关等"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            title: "关于"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)

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

            HusText {
                anchors.left: logo.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 20
                text: qsTr(Qt.application.displayName + '\n' +
                    "版本: " + Qt.application.version + '\n' +
                    Qt.application.organization + '\n'
                    + "协议、隐私政策、使用条款"
                )
            }
        }
    } // ColumnLayout
}