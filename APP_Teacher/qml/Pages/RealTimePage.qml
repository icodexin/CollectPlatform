import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtGraphs
import DelegateUI

import TeacherApp 1.0
import "../Controls/"
import "../Controls/Charts/"
import "../Views/"

MyPage {
    id: page
    titleIconSource: DelIcon.FundProjectionScreenOutlined
    titleText: qsTr("实时采集与分析")
    spacing: 20

    ObjectModel {
        id: wristbandViewModel
    }

    function createWristbandView(studentId) {
        const component = Qt.createComponent("../Views/WristbandView.qml");
        if (component.status === Component.Ready) {
            const obj = component.createObject(page, {
                studentId: studentId,
                width: Qt.binding(() => grid.cellWidth - 8),
                height: Qt.binding(() => grid.cellHeight - 8)
            });
            wristbandViewModel.append(obj);
        }
    }

    function removeWristbandView(studentId) {
        for (let i = 0; i < grid.count; i++) {
            var view = grid.itemAtIndex(i);
            if (view.studentId === studentId) {
                wristbandViewModel.remove(i);
                break;
            }
        }
    }

    function appendData(studentId, data) {
        for (let i = 0; i < grid.count; i++) {
            var view = grid.itemAtIndex(i);
            if (view.studentId === studentId) {
                view.append_data(data);
                break;
            }
        }
    }

    Component.onCompleted: {
        // createWristbandView("192.168.0.1")
    }


    GridView {
        id: grid
        clip: true
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: controlBar.top
        anchors.bottomMargin: 8
        model: wristbandViewModel
        cellWidth: width / 2
        cellHeight: height / 2
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: DelScrollBar {}

        add: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 400 }
            NumberAnimation { property: "scale"; from: 0; to: 1.0; duration: 400 }
        }
        remove: Transition {
            NumberAnimation { property: "opacity"; to: 0; duration: 400 }
            NumberAnimation { property: "x"; to: 0; duration: 400 }
        }
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.InOutCubic}
            // ensure opacity and scale values return to 1.0
            NumberAnimation { property: "opacity"; to: 1.0 }
            NumberAnimation { property: "scale"; to: 1.0 }
        }
    }

    Row {
        id: controlBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 8

        NumberInputer {
            id: portInput
            editable: !wristbandListener.listening
            value: 12345
            from: 1024
            to: 49152
        }

        DelButton {
            text: wristbandListener.listening ? qsTr("Stop") : qsTr("Start")
            onClicked: {
                const port = portInput.value;
                if (wristbandListener.listening) {
                    wristbandListener.stopServer();
                } else {
                    if (!wristbandListener.startServer(port))
                        message.error(qsTr("Failed to start server at port %1").arg(port));
                }
            }
        }
    }


    WristbandListener {
        id: wristbandListener
        onListeningChanged: (isListening) => {
            message.info(isListening ? qsTr("Server started at port %1").arg(port) : qsTr("Server stopped"))
        }
        onErrorOccurred: (error) => {
            if ("id" in error) {
                message.error(`${error.id}: ${error.msg}`);
                page.removeWristbandView(error.id);
            } else {
                message.error(error.msg)
            }
        }
        onClientConnected: (id) => {
            message.info(qsTr("Client connected: %1").arg(id))
            page.createWristbandView(id)
        }
        onClientDisconnected: (id) => {
            message.info(qsTr("Client disconnected: %1").arg(id))
            console.log(id)
            page.removeWristbandView(id)
        }
        onDataReceived: (id, data) => {
            page.appendData(id, data)
        }
    }
}
