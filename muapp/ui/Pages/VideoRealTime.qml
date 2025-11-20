import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp
import VideoComponents 1.0

MuPage {
    id: page
    titleIconSource: HusIcon.DotChartOutlined
    titleText: "实时视频采集"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 黑色展示框（预览区）
        Rectangle {
            id: displayArea
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            radius: 8
            color: "#000000"            // 纯黑背景
            border.color: "#333333"     // 细边框，便于区分
            border.width: 1

            VideoPaintedItem {
                id: videoDisplay
                anchors.fill: parent
            }

            // 占位提示
            // Text {
            //     anchors.centerIn: parent
            //     text: "等待视频链接"
            //     color: "#777777"
            //     font.pixelSize: 16
            // }
            Connections {
                target: pull_work
                onImageReady: function(image) {  // 显式声明参数
                    videoDisplay.updateFrame(image)
                }
            }
        }

        // 占位伸缩，保证按钮贴底
        Item { Layout.fillHeight: true }

        // 底部按钮行
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                id: startBtn
                text: "开始"
                Layout.fillWidth: true
                onClicked: {
                    pull_work.isTrueGet();
                }
            }

            Button {
                id: pauseBtn
                text: "暂停"
                Layout.fillWidth: true
                onClicked:{
                //     暂停逻辑没写
                }
            }
        }
    }
}
