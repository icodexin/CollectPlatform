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
        anchors.bottom: parent.bottom
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