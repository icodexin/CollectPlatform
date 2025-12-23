import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

MuPage {
    title: "实时采集"
    titleIconSource: HusIcon.DotChartOutlined

    // todo: replace with real model
    property var stuModel: [
        {
            "name": "Student 1",
            "status": "在线"
        },
        {
            "name": "Student 2",
            "status": "离线"
        },
        {
            "name": "Student 3",
            "status": "在线"
        }
    ]

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: statusBar.top
        model: stuModel
        delegate: stuDelegate
        highlight: Rectangle { radius: 10; color: "lightsteelblue" }
        highlightMoveDuration: HusTheme.animationEnabled ? HusTheme.Primary.durationFast : 0
        width: 150
        focus: true
        clip: true
    }

    Component {
        id: stuDelegate
        Item {
            id: stuItem
            required property var modelData
            required property int index
            property int contentHeight: 30
            property int padding: 8
            width: listView.width
            height: contentHeight + 2 * padding

            Row {
                width: parent.width
                height: parent.height
                padding: parent.padding
                spacing: 8

                HusAvatar {
                    id: stuAvatar
                    size: stuItem.contentHeight
                    iconSource: HusIcon.UserOutlined
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    HusText {
                        text: modelData.name
                        font.bold: true
                    }
                    Row {
                        spacing: 4
                        Rectangle {
                            width: stuStatusText.height / 2
                            height: stuStatusText.height / 2
                            radius: width / 2
                            color: modelData.status === "在线" ? "green" : "gray"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        HusText {
                            id: stuStatusText
                            text: modelData.status
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: { listView.currentIndex = index; }
            }
        }
    }

    RowLayout {
        id: statusBar
        anchors.left: listView.left
        anchors.right: listView.right
        anchors.bottom: parent.bottom

        HusSpin {
            Layout.alignment: Qt.AlignVCenter
            spinning: service.status === DataStreamService.Connecting
            sizeHint: 'small'
        }

        HusText {
            Layout.alignment: Qt.AlignVCenter
            id: statusText
            text: {
                switch (service.status) {
                case DataStreamService.Offline:
                    return "服务已断开";
                case DataStreamService.Online:
                    return "服务已连接";
                case DataStreamService.Connecting:
                    return "服务已断开，连接中...";
                }
            }
        }
    }

    Row {
        anchors.left: listView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: bottomPanel.top

        anchors.leftMargin: 10

        BandView {
            id: bandView
            width: parent.width / 2
            height: parent.height
        }

        EEGView {
            id: eegView
            width: parent.width / 2
            height: parent.height
        }
    }

    Rectangle {
        id: bottomPanel
        anchors.left: listView.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 100
        color: HusTheme.HusCard.bgColor || "#ffffff"
        border.color: HusTheme.HusCard.borderColor || "#e5e5e5"
        border.width: 1

        GridLayout {
            anchors.fill: parent
            columns: 4


            EmotionResultCard {
                Layout.fillWidth: true  // 占满列的宽度（每列占1/4）
                Layout.fillHeight: true // 占满行的高度，垂直居中显示
                title: "EEG"
                emotion: "高兴"
                emotionColor: "#52c41a"
            }

            EmotionResultCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "PPG"
                // 绑定情绪名称：从service.emotionModel获取，默认显示"无"
                emotion: service.emotionModel.emotionName || "无"
                // 绑定情绪颜色：根据情绪名称动态匹配，默认灰色
                emotionColor: {
                    // 情绪与颜色的映射表（可根据需求扩展）
                    switch (service.emotionModel.emotionName) {
                        case "困惑": return "#52c41a"; // 绿色
                        case "中性": return "#1890ff"; // 蓝色
                        case "无聊": return "#f5222d"; // 红色
                        case "专注": return "#fa8c16"; // 橙色
                        default: return "#bfbfbf"; // 默认灰色
                    }
                }
            }

            EmotionResultCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "视频"
                emotion: "中性"
                emotionColor: "#bfbfbf"
            }

            EmotionResultCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "汇总结果"
                emotion: "高兴"
                emotionColor: "#faad14"
            }
        }
    }


    BandViewController {
        id: bandViewController

        onFrameUpdated: (frame) => {
            bandView.updateFrame(frame)
        }
    }

    EEGViewController {
        id: eegViewController

        onFrameUpdated: (frame) => {
            eegView.updateFrame(frame)
        }
    }

    DataStreamService {
        id: service
        subStudentId: "10001" // todo: replace with real student id

        onWristbandReceived: (data) => {
            bandViewController.pushData(data);
        }

        onEegReceived: (data) => {
            eegViewController.pushData(data);
        }

        onConnectTimesChanged: (times) =>{
            if (times === 5) {
                AppController.notify.error(
                    "无法连接数据服务",
                    `连接异常，请检查网络或联系管理员。`,
                    60000,
                    "datastream-service-connection-error"
                );
            } else if (times === 0) {
                AppController.notify.close("datastream-service-connection-error");
            }
        }
    }
}