import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic
import MuApp

MuPage {
    default property alias content: contentItem.data

    Flickable {
        clip: true
        anchors.fill: parent
        anchors.rightMargin: -15
        rightMargin: 15
        contentHeight: contentItem.height
        ScrollBar.vertical: HusScrollBar {}

        Item {
            id: contentItem
            height: childrenRect.height
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }
}