import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

MuFlickablePage {
    titleIconSource: HusIcon.HomeOutlined
    titleText: Qt.application.displayName

    ColumnLayout {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 20

        MuPanel {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "注册学生数量、在线人数"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "正在进行的采集活动"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "最近的采集活动"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "中心服务器状态"
                anchors.centerIn: parent
            }
        }

        MuPanel {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(1000, column.width)
            HusText {
                text: "数据库状态"
                anchors.centerIn: parent
            }
        }

    } // ColumnLayout
}