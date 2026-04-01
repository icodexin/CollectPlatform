import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic

Item {
    id: root

    // 外部注入：
    // SummaryView { controller: summaryController }
    property var controller

    // =========================
    // 列宽统一配置（表头和内容共用，保证对齐）
    // =========================
    readonly property int colGap: 8
    readonly property int respWidth: 90
    readonly property int heartWidth: 90
    readonly property int diaWidth: 100
    readonly property int sysWidth: 100
    readonly property int emotionWidth: 110
    readonly property int stressWidth: 100
    readonly property int fatigueWidth: 100

    function fixedColumnsWidth() {
        return respWidth + heartWidth + diaWidth + sysWidth
            + emotionWidth + stressWidth + fatigueWidth
    }

    function studentColumnWidth(totalWidth) {
        return Math.max(220, totalWidth - fixedColumnsWidth() - colGap * 7)
    }

    // =========================
    // 默认数据（8个学生）
    // =========================
    property var defaultPhysioData: [
        { studentId: "10001", name: "Student 1", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10002", name: "Student 2", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10003", name: "Student 3", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10004", name: "Student 4", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10005", name: "Student 5", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10006", name: "Student 6", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10007", name: "Student 7", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { studentId: "10008", name: "Student 8", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" }
    ]

    property var defaultEmotionData: [
        { studentId: "10001", name: "Student 1", result: "-" },
        { studentId: "10002", name: "Student 2", result: "-" },
        { studentId: "10003", name: "Student 3", result: "-" },
        { studentId: "10004", name: "Student 4", result: "-" },
        { studentId: "10005", name: "Student 5", result: "-" },
        { studentId: "10006", name: "Student 6", result: "-" },
        { studentId: "10007", name: "Student 7", result: "-" },
        { studentId: "10008", name: "Student 8", result: "-" }
    ]

    property var defaultCognitionData: [
        { studentId: "10001", name: "Student 1", stress: "-", fatigue: "-" },
        { studentId: "10002", name: "Student 2", stress: "-", fatigue: "-" },
        { studentId: "10003", name: "Student 3", stress: "-", fatigue: "-" },
        { studentId: "10004", name: "Student 4", stress: "-", fatigue: "-" },
        { studentId: "10005", name: "Student 5", stress: "-", fatigue: "-" },
        { studentId: "10006", name: "Student 6", stress: "-", fatigue: "-" },
        { studentId: "10007", name: "Student 7", stress: "-", fatigue: "-" },
        { studentId: "10008", name: "Student 8", stress: "-", fatigue: "-" }
    ]

    // =========================
    // 外部数据优先，不改原有逻辑
    // =========================
    property var physioData: controller && controller.physioData !== undefined
        ? controller.physioData
        : defaultPhysioData

    property var emotionData: controller && controller.emotionData !== undefined
        ? controller.emotionData
        : defaultEmotionData

    property var cognitionData: controller && controller.cognitionData !== undefined
        ? controller.cognitionData
        : defaultCognitionData

    // =========================
    // 工具函数
    // =========================
    function normalizeValue(v) {
        return v === undefined || v === null ? "" : String(v).trim()
    }

    function hasPhysioValue(v) {
        var s = normalizeValue(v)
        return s !== "" && s !== "0" && s !== "0.0" && s !== "0.00"
    }

    function hasCommonValue(v) {
        var s = normalizeValue(v)
        return s !== "" && s !== "-"
    }

    function findEmotion(studentId, name, index) {
        for (var i = 0; i < emotionData.length; ++i) {
            var item = emotionData[i]
            if ((item.studentId !== undefined && item.studentId === studentId) ||
                (item.name !== undefined && item.name === name)) {
                return item
            }
        }
        return index < emotionData.length ? emotionData[index] : null
    }

    function findCognition(studentId, name, index) {
        for (var i = 0; i < cognitionData.length; ++i) {
            var item = cognitionData[i]
            if ((item.studentId !== undefined && item.studentId === studentId) ||
                (item.name !== undefined && item.name === name)) {
                return item
            }
        }
        return index < cognitionData.length ? cognitionData[index] : null
    }

    function rowHasData(p, e, c) {
        if (p) {
            if (hasPhysioValue(p.respiration)) return true
            if (hasPhysioValue(p.heartRate)) return true
            if (hasPhysioValue(p.diastolic)) return true
            if (hasPhysioValue(p.systolic)) return true
        }

        if (e && hasCommonValue(e.result)) return true

        if (c) {
            if (hasCommonValue(c.stress)) return true
            if (hasCommonValue(c.fatigue)) return true
        }

        return false
    }

    // =========================
    // 合并展示数据
    // =========================
    property var mergedStudentData: {
        var physioList = physioData || []
        var emotionList = emotionData || []
        var cognitionList = cognitionData || []
        var maxLen = Math.max(physioList.length, emotionList.length, cognitionList.length)
        var result = []

        for (var i = 0; i < maxLen; ++i) {
            var p = i < physioList.length ? physioList[i] : {}
            var studentId = p.studentId !== undefined ? p.studentId : ""
            var name = p.name !== undefined ? p.name : ""

            var e = findEmotion(studentId, name, i)
            var c = findCognition(studentId, name, i)

            if ((!name || name === "") && e && e.name !== undefined)
                name = e.name
            if ((!name || name === "") && c && c.name !== undefined)
                name = c.name

            if ((!studentId || studentId === "") && e && e.studentId !== undefined)
                studentId = e.studentId
            if ((!studentId || studentId === "") && c && c.studentId !== undefined)
                studentId = c.studentId

            result.push({
                studentId: studentId,
                name: name,
                respiration: p.respiration !== undefined ? p.respiration : "0",
                heartRate: p.heartRate !== undefined ? p.heartRate : "0",
                diastolic: p.diastolic !== undefined ? p.diastolic : "0",
                systolic: p.systolic !== undefined ? p.systolic : "0",
                emotionResult: e && e.result !== undefined ? e.result : "-",
                stress: c && c.stress !== undefined ? c.stress : "-",
                fatigue: c && c.fatigue !== undefined ? c.fatigue : "-",
                hasData: rowHasData(p, e, c)
            })
        }

        return result
    }

    Rectangle {
        anchors.fill: parent
        color: "#f5f5f5"
        border.color: "#d9d9d9"
        radius: 10
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 8
        clip: true
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 24
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ColumnLayout {
            id: contentColumn
            width: flick.width - 16
            spacing: 12

            HusText {
                text: "结果展示"
                font.pixelSize: 24
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                height: 4
                color: "#8c8c8c"
                radius: 2
            }

            // =========================
            // 第一块：综合表
            // =========================
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                HusText {
                    text: "1. 学生综合数据表"
                    font.pixelSize: 18
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    border.color: "#d9d9d9"
                    radius: 8
                    implicitHeight: tableBody.implicitHeight + 24

                    Column {
                        id: tableBody
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        // 表头
                        Rectangle {
                            width: parent.width
                            height: 42
                            color: "#fafafa"
                            border.color: "#d9d9d9"
                            radius: 6

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: root.colGap

                                Rectangle {
                                    width: root.studentColumnWidth(tableBody.width - 24)
                                    height: parent.height
                                    color: "transparent"

                                    HusText {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 4
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "学生"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.respWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "呼吸"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.heartWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "心率"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.diaWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "舒张压"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.sysWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "收缩压"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.emotionWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "情绪"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.stressWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "压力"
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    width: root.fatigueWidth
                                    height: parent.height
                                    color: "transparent"
                                    HusText {
                                        anchors.centerIn: parent
                                        text: "疲劳"
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // 数据行
                        Repeater {
                            model: root.mergedStudentData

                            delegate: Rectangle {
                                width: tableBody.width
                                height: 48
                                color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                border.color: "#e8e8e8"
                                radius: 6

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: root.colGap

                                    // 学生列
                                    Rectangle {
                                        id: studentCell
                                        width: root.studentColumnWidth(tableBody.width - 24)
                                        height: parent.height
                                        color: "transparent"

                                        Row {
                                            anchors.left: parent.left
                                            anchors.leftMargin: 4
                                            anchors.verticalCenter: parent.verticalCenter
                                            spacing: 10

                                            Rectangle {
                                                id: idBadge
                                                width: 94
                                                height: 32
                                                radius: 16
                                                color: modelData.hasData ? "#d9f7be" : "#f0f0f0"
                                                border.color: modelData.hasData ? "#95de64" : "#d9d9d9"

                                                HusText {
                                                    anchors.centerIn: parent
                                                    text: String(modelData.studentId ?? "")
                                                    font.bold: true
                                                }
                                            }

                                            HusText {
                                                width: studentCell.width - idBadge.width - 24
                                                anchors.verticalCenter: parent.verticalCenter
                                                text: String(modelData.name ?? "")
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: root.respWidth
                                        height: parent.height
                                        color: "transparent"
                                        HusText {
                                            anchors.centerIn: parent
                                            text: String(modelData.respiration ?? "0")
                                        }
                                    }

                                    Rectangle {
                                        width: root.heartWidth
                                        height: parent.height
                                        color: "transparent"
                                        HusText {
                                            anchors.centerIn: parent
                                            text: String(modelData.heartRate ?? "0")
                                        }
                                    }

                                    Rectangle {
                                        width: root.diaWidth
                                        height: parent.height
                                        color: "transparent"
                                        HusText {
                                            anchors.centerIn: parent
                                            text: String(modelData.diastolic ?? "0")
                                        }
                                    }

                                    Rectangle {
                                        width: root.sysWidth
                                        height: parent.height
                                        color: "transparent"
                                        HusText {
                                            anchors.centerIn: parent
                                            text: String(modelData.systolic ?? "0")
                                        }
                                    }

                                    Rectangle {
                                        width: root.emotionWidth
                                        height: parent.height
                                        color: "transparent"

                                        Rectangle {
                                            width: 88
                                            height: 30
                                            radius: 15
                                            anchors.centerIn: parent
                                            color: hasCommonValue(modelData.emotionResult) ? "#d9f7be" : "#f0f0f0"
                                            border.color: hasCommonValue(modelData.emotionResult) ? "#95de64" : "#d9d9d9"

                                            HusText {
                                                anchors.centerIn: parent
                                                text: String(modelData.emotionResult ?? "-")
                                                font.bold: true
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: root.stressWidth
                                        height: parent.height
                                        color: "transparent"

                                        Rectangle {
                                            width: 88
                                            height: 30
                                            radius: 15
                                            anchors.centerIn: parent
                                            color: hasCommonValue(modelData.stress) ? "#e6f4ff" : "#f0f0f0"
                                            border.color: hasCommonValue(modelData.stress) ? "#91caff" : "#d9d9d9"

                                            HusText {
                                                anchors.centerIn: parent
                                                text: String(modelData.stress ?? "-")
                                                font.bold: true
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: root.fatigueWidth
                                        height: parent.height
                                        color: "transparent"

                                        Rectangle {
                                            width: 88
                                            height: 30
                                            radius: 15
                                            anchors.centerIn: parent
                                            color: hasCommonValue(modelData.fatigue) ? "#fff7e6" : "#f0f0f0"
                                            border.color: hasCommonValue(modelData.fatigue) ? "#ffd591" : "#d9d9d9"

                                            HusText {
                                                anchors.centerIn: parent
                                                text: String(modelData.fatigue ?? "-")
                                                font.bold: true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 4
                color: "#8c8c8c"
                radius: 2
            }

            // =========================
            // 第二块：生理同步状态
            // =========================
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                HusText {
                    text: "2. 生理同步状态"
                    font.pixelSize: 18
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 72
                    color: "#ffffff"
                    border.color: "#d9d9d9"
                    radius: 8

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        HusText {
                            text: "同步状态："
                            font.pixelSize: 16
                            font.bold: true
                        }

                        Rectangle {
                            width: 110
                            height: 34
                            radius: 17
                            color: "#52c41a"

                            HusText {
                                anchors.centerIn: parent
                                text: "高同步"
                                color: "white"
                                font.bold: true
                            }
                        }

                        HusText {
                            text: "同步指数：0.82"
                            font.pixelSize: 15
                        }

                        HusText {
                            text: "趋势：稳定"
                            font.pixelSize: 15
                        }
                    }
                }
            }

            Item {
                width: 1
                height: 12
            }
        }
    }
}