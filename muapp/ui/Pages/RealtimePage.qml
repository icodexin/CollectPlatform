import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MuApp

MuPage {
    title: "实时采集"
    titleIconSource: HusIcon.DotChartOutlined

    property bool summarySelected: false
    property string summaryStudentId: "__summary__"

    // todo: replace with real model
    property var stuModel: [
        { "studentId": "10001", "name": "Student 1", "status": "在线" },
        { "studentId": "10002", "name": "Student 2", "status": "在线" },
        { "studentId": "10003", "name": "Student 3", "status": "在线" },
        { "studentId": "10004", "name": "Student 4", "status": "在线" },
        { "studentId": "10005", "name": "Student 5", "status": "在线" },
        { "studentId": "10006", "name": "Student 6", "status": "在线" },
        { "studentId": "10007", "name": "Student 7", "status": "在线" },
        { "studentId": "10008", "name": "Student 8", "status": "在线" },
    ]

    function resetMonitorViews() {
        bcgViewController.reset()
        mmwavViewController.reset()
        rppgViewController.reset()
        bandViewController.reset()
    }

    function selectStudent(studentId, index) {
        summarySelected = false
        listView.currentIndex = index
        resetMonitorViews()
        service.subStudentId = studentId
    }

    function selectSummary() {
        summarySelected = true
        listView.currentIndex = -1
        resetMonitorViews()
        summaryController.reset()
        service.subStudentId = summaryStudentId
    }

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: summaryEntry.top
        width: 150

        model: stuModel
        delegate: stuDelegate
        highlight: Rectangle {
            radius: 10
            color: "lightsteelblue"
        }
        highlightMoveDuration: HusTheme.animationEnabled ? HusTheme.Primary.durationFast : 0
        focus: true
        clip: true
    }

    Rectangle {
        id: summaryEntry
        anchors.left: listView.left
        anchors.right: listView.right
        anchors.bottom: statusBar.top
        anchors.bottomMargin: 8
        height: 46
        radius: 10
        border.color: "#d9d9d9"

        color: {
            if (summarySelected) return "#595959"
            if (summaryMouse.pressed) return "#4a4a4a"
            if (summaryMouse.containsMouse) return "#737373"
            return "#f3f3f3"
        }

        Behavior on color {
            ColorAnimation { duration: 120 }
        }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            HusAvatar {
                anchors.verticalCenter: parent.verticalCenter
                size: 28
                iconSource: HusIcon.AppstoreOutlined
            }

            HusText {
                anchors.verticalCenter: parent.verticalCenter
                text: "结果展示"
                font.bold: true
                color: summarySelected || summaryMouse.containsMouse ? "white" : "#262626"
            }
        }

        MouseArea {
            id: summaryMouse
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                selectSummary()
            }
        }
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
                onClicked: {
                    selectStudent(modelData.studentId, index)
                }
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
            sizeHint: "small"
        }

        HusText {
            id: statusText
            Layout.alignment: Qt.AlignVCenter
            text: {
                switch (service.status) {
                    case DataStreamService.Offline:
                        return "服务已断开"
                    case DataStreamService.Online:
                        return "服务已连接"
                    case DataStreamService.Connecting:
                        return "服务已断开，连接中..."
                }
            }
        }
    }

    Item {
        id: monitorPanel
        visible: !summarySelected
        anchors.left: listView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10

        property int cardRadius: 10
        property int titleBarHeight: 42
        property color cardBorderColor: "#d9d9d9"
        property color cardBgColor: "#ffffff"
        property color titleBgColor: "#f5f5f5"

        GridLayout {
            anchors.fill: parent
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.preferredHeight: 1
                radius: monitorPanel.cardRadius
                color: monitorPanel.cardBgColor
                border.color: monitorPanel.cardBorderColor
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: monitorPanel.titleBarHeight
                        color: monitorPanel.titleBgColor

                        HusText {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: "PPG"
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: monitorPanel.cardBorderColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        BandView {
                            id: viewPPG
                            anchors.fill: parent
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.preferredHeight: 1
                radius: monitorPanel.cardRadius
                color: monitorPanel.cardBgColor
                border.color: monitorPanel.cardBorderColor
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: monitorPanel.titleBarHeight
                        color: monitorPanel.titleBgColor

                        HusText {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: "BCG"
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: monitorPanel.cardBorderColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        BCGView {
                            id: viewBCG
                            anchors.fill: parent
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.preferredHeight: 1
                radius: monitorPanel.cardRadius
                color: monitorPanel.cardBgColor
                border.color: monitorPanel.cardBorderColor
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: monitorPanel.titleBarHeight
                        color: monitorPanel.titleBgColor

                        HusText {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: "MMWAV"
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: monitorPanel.cardBorderColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        MMWAVView {
                            id: viewmmWAV
                            anchors.fill: parent
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.preferredHeight: 1
                radius: monitorPanel.cardRadius
                color: monitorPanel.cardBgColor
                border.color: monitorPanel.cardBorderColor
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: monitorPanel.titleBarHeight
                        color: monitorPanel.titleBgColor

                        HusText {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: "RPPG"
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: monitorPanel.cardBorderColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        RPPGView {
                            id: viewrPPG
                            anchors.fill: parent
                        }
                    }
                }
            }
        }
    }

    SummaryView {
        id: summaryPanel
        visible: summarySelected
        anchors.left: listView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10

        controller: summaryController
    }

    BandViewController {
        id: bandViewController

        onFrameUpdated: (frame) => {
            viewPPG.updateFrame(frame)
        }

        onResetRequested: {
            viewPPG.resetView()
        }
    }

    BcgViewController {
        id: bcgViewController

        onFrameUpdated: (frame) => {
            viewBCG.updateFrame(frame)
        }

        onResetRequested: {
            viewBCG.resetView()
        }
    }

    MmwavViewController {
        id: mmwavViewController

        onFrameUpdated: (frame) => {
            viewmmWAV.updateFrame(frame)
        }

        onResetRequested: {
            viewmmWAV.resetView()
        }
    }

    RppgViewController {
        id: rppgViewController

        onFrameUpdated: (frame) => {
            viewrPPG.updateFrame(frame)
        }

        onResetRequested: {
            viewrPPG.resetView()
        }
    }
    SummaryController {
        id: summaryController
        dataStreamService: service
    }

    DataStreamService {
        id: service
        subStudentId: "10001" // todo: replace with real student id

        onWristbandJsonReceived: (data, studentId) => {
            bandViewController.pushJsonData(data,studentId)
        }

        onBcgReceived: (data, studentId) => {
            bcgViewController.pushData(data, studentId)
        }

        onMmwavReceived: (data, studentId) => {
            mmwavViewController.pushMmwavData(data, studentId)
        }

        onRppgReceived: (data, studentId) => {
            rppgViewController.pushData(data, studentId)
        }

        onConnectTimesChanged: (times) => {
            if (times === 5) {
                AppController.notify.error(
                    "无法连接数据服务",
                    "连接异常，请检查网络或联系管理员。",
                    60000,
                    "datastream-service-connection-error"
                )
            } else if (times === 0) {
                AppController.notify.close("datastream-service-connection-error")
            }
        }
    }

    Component.onCompleted: {
        if (stuModel.length > 0) {
            selectStudent(stuModel[0].studentId, 0)
        }
    }
}