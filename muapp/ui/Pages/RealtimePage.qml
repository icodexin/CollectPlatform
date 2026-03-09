import QtQml
import QtQuick
import QtQuick.Layouts
import QtMultimedia
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

    property var emotionMetrics: [
        { name: "困惑", value: 20.0, color: "#5B8FF9" },
        { name: "无聊", value: 30.0, color: "#5AD8A6" },
        { name: "投入", value: 40.0, color: "#5D7092" },
        { name: "中性", value: 60.0, color: "#F6BD16" }
    ]

    property var cognitiveMetrics: [
        { name: "压力程度", value: 20.0, color: "#D03050" },
        { name: "脑力负荷", value: 30.0, color: "#389E0D" },
        { name: "专注程度", value: 40.0, color: "#1677FF" }
    ]

    function clampPercent(value) {
        return Math.max(0, Math.min(100, value));
    }

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

    ColumnLayout {
        id: mainPanels
        anchors.left: listView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        spacing: 5

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 5

            Item {
                id: cameraPanel
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    color: HusTheme.HusCard.colorBg
                    border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
                    opacity: 0.8
                    radius: 10
                }

                HusText {
                    id: camTitle
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 8
                    text: "摄像头画面"
                    font.weight: Font.Bold
                }

                HusText {
                    anchors.centerIn: parent
                    text: vpService.url + " 连接中..."
                    opacity: vpService.status === VideoPullService.Playing ? 0 : 1
                }

                VideoOutput {
                    id: videoOutput
                    anchors.left: parent.left
                    anchors.top: camTitle.bottom
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 8
                    opacity: vpService.status === VideoPullService.Playing ? 1 : 0
                }
            } // Item

            Item {
                id: insightPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                Rectangle {
                    anchors.fill: parent
                    color: HusTheme.HusCard.colorBg
                    border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
                    opacity: 0.8
                    radius: 10
                }

                HusText {
                    id: insightTitle
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 8
                    text: "情绪与认知面板"
                    font.weight: Font.Bold
                }

                HusDivider {
                    id: emoTitle
                    title: "情绪状态"
                    anchors.left: parent.left
                    anchors.top: insightTitle.bottom
                    anchors.right: parent.right
                    anchors.margins: 8
                    anchors.topMargin: 12
                }

                ColumnLayout {
                    id: emoView
                    anchors.left: parent.left
                    anchors.top: emoTitle.bottom
                    anchors.right: parent.right
                    anchors.margins: 8
                    Repeater {
                        model: emotionMetrics
                        delegate: Item {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: Math.min(800, emoView.width)
                            height: childrenRect.height

                            required property var modelData

                            HusText {
                                id: _emoName
                                text: modelData.name
                                anchors.left: parent.left
                            }

                            HusProgress {
                                Layout.fillWidth: true
                                anchors.left: _emoName.right
                                anchors.leftMargin: 8
                                anchors.right: parent.right
                                anchors.verticalCenter: _emoName.verticalCenter
                                percent: clampPercent(modelData.value)
                                colorBar: modelData.color
                            }
                        }
                    }
                }

                HusDivider {
                    id: cogTitle
                    title: "认知状态"
                    anchors.left: parent.left
                    anchors.top: emoView.bottom
                    anchors.right: parent.right
                    anchors.margins: 8
                    anchors.topMargin: parent.height < 400 ? 8 : 20
                }

                RowLayout {
                    id: cogRow
                    anchors.left: parent.left
                    anchors.top: cogTitle.bottom
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 8
                    spacing: 0

                    ColumnLayout {
                        Layout.fillHeight: true
                        Repeater {
                            model: cognitiveMetrics
                            delegate: Column {
                                required property var modelData
                                spacing: 2

                                HusProgress {
                                    id: _cogProgress
                                    width: height
                                    height: Math.min(120, Math.max(20, cogRow.height / 5))
                                    type: HusProgress.Type_Dashboard
                                    percent: clampPercent(modelData.value)
                                    colorBar: modelData.color
                                    font.pixelSize: 8
                                }

                                HusText {
                                    anchors.horizontalCenter: _cogProgress.horizontalCenter
                                    text: modelData.name
                                }
                            }
                        }
                    }

                    MultiLineChart {
                        id: cognitiveTrendChart
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        titleVisible: false
                        lineProperties: cognitiveMetrics
                        marginLeft: -25
                        legendVisible: false
                        autoScaleY: false
                        minY: 0
                        maxY: 100
                        minYRange: 100
                        yLabelFormat: "%.0f"
                    }
                }
            } // Item
        } // RowLayout

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 5

            BandView {
                id: bandView
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            EEGView {
                id: eegView
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        } // RowLayout
    } // ColumnLayout

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

    VideoPullService {
        id: vpService
        url: "rtsp://127.0.0.1:8554/live/10001"  // todo: replace with real student id
        videoOutput: videoOutput
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
