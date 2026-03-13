import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic

Item {
    id: root

    property var physioData: [
        { name: "Student 1", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { name: "Student 2", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { name: "Student 3", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { name: "Student 4", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { name: "Student 5", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" },
        { name: "Student 6", respiration: "0", heartRate: "0", diastolic: "0", systolic: "0" }
    ]

    property var emotionData: [
        { name: "Student 1", result: "专注" },
        { name: "Student 2", result: "专注" },
        { name: "Student 3", result: "专注" },
        { name: "Student 4", result: "专注" },
        { name: "Student 5", result: "专注" },
        { name: "Student 6", result: "专注" }
    ]

    property var cognitionData: [
        { name: "Student 1", stress: "低", fatigue: "低" },
        { name: "Student 2", stress: "中", fatigue: "低" },
        { name: "Student 3", stress: "高", fatigue: "中" },
        { name: "Student 4", stress: "中", fatigue: "高" },
        { name: "Student 5", stress: "低", fatigue: "中" },
        { name: "Student 6", stress: "中", fatigue: "中" }
    ]

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

            // 第一行：生理数据表
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                HusText {
                    text: "1. 生理数据表"
                    font.pixelSize: 18
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    border.color: "#d9d9d9"
                    radius: 8
                    implicitHeight: physioColumn.implicitHeight + 24

                    ColumnLayout {
                        id: physioColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#fafafa"
                            border.color: "#d9d9d9"
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16
                                spacing: 8

                                HusText {
                                    Layout.fillWidth: true
                                    text: "学生"
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "呼吸"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "心率"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "舒张压"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "收缩压"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }
                            }
                        }

                        Repeater {
                            model: root.physioData
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 44
                                color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                border.color: "#e8e8e8"
                                radius: 6

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 16
                                    anchors.rightMargin: 16
                                    spacing: 8

                                    HusText {
                                        Layout.fillWidth: true
                                        text: modelData.name
                                    }

                                    HusText {
                                        Layout.preferredWidth: 80
                                        text: modelData.respiration
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    HusText {
                                        Layout.preferredWidth: 80
                                        text: modelData.heartRate
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    HusText {
                                        Layout.preferredWidth: 80
                                        text: modelData.diastolic
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    HusText {
                                        Layout.preferredWidth: 80
                                        text: modelData.systolic
                                        horizontalAlignment: Text.AlignHCenter
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

            // 第二行：生理同步状态
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

            Rectangle {
                Layout.fillWidth: true
                height: 4
                color: "#8c8c8c"
                radius: 2
            }

            // 第三行：情绪结果表
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                HusText {
                    text: "3. 情绪结果"
                    font.pixelSize: 18
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    border.color: "#d9d9d9"
                    radius: 8
                    implicitHeight: emotionColumn.implicitHeight + 24

                    ColumnLayout {
                        id: emotionColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#fafafa"
                            border.color: "#d9d9d9"
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16

                                HusText {
                                    Layout.fillWidth: true
                                    text: "学生"
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 120
                                    text: "识别结果"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }
                            }
                        }

                        Repeater {
                            model: root.emotionData
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 44
                                color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                border.color: "#e8e8e8"
                                radius: 6

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 16
                                    anchors.rightMargin: 16

                                    HusText {
                                        Layout.fillWidth: true
                                        text: modelData.name
                                    }

                                    Rectangle {
                                        Layout.preferredWidth: 120
                                        height: 28
                                        radius: 14
                                        color: "#d9f7be"

                                        HusText {
                                            anchors.centerIn: parent
                                            text: modelData.result
                                            font.bold: true
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

            // 第四行：学生认知状态
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                HusText {
                    text: "4. 学生认知状态"
                    font.pixelSize: 18
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    border.color: "#d9d9d9"
                    radius: 8
                    implicitHeight: cognitionColumn.implicitHeight + 24

                    ColumnLayout {
                        id: cognitionColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#fafafa"
                            border.color: "#d9d9d9"
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16

                                HusText {
                                    Layout.fillWidth: true
                                    text: "学生"
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "压力"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }

                                HusText {
                                    Layout.preferredWidth: 80
                                    text: "疲劳"
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: true
                                }
                            }
                        }

                        Repeater {
                            model: root.cognitionData
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 44
                                color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                border.color: "#e8e8e8"
                                radius: 6

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 16
                                    anchors.rightMargin: 16

                                    HusText {
                                        Layout.fillWidth: true
                                        text: modelData.name
                                    }

                                    Rectangle {
                                        Layout.preferredWidth: 80
                                        height: 28
                                        radius: 14
                                        color: modelData.stress === "高" ? "#ffccc7"
                                            : modelData.stress === "中" ? "#ffe7ba"
                                                : "#d9f7be"

                                        HusText {
                                            anchors.centerIn: parent
                                            text: modelData.stress
                                            font.bold: true
                                        }
                                    }

                                    Rectangle {
                                        Layout.preferredWidth: 80
                                        height: 28
                                        radius: 14
                                        color: modelData.fatigue === "高" ? "#ffccc7"
                                            : modelData.fatigue === "中" ? "#ffe7ba"
                                                : "#d9f7be"

                                        HusText {
                                            anchors.centerIn: parent
                                            text: modelData.fatigue
                                            font.bold: true
                                        }
                                    }
                                }
                            }
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