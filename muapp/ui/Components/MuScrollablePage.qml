import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic
import MuApp

MuPage {
    default property alias content: contentItem.data

    MuFlickable {
        id: flickable
        clip: true
        anchors.fill: parent
        contentHeight: contentItem.height
        ScrollBar.vertical: HusScrollBar {
            parent: flickable.parent
            anchors.top: flickable.top
            anchors.left: flickable.right
            anchors.bottom: flickable.bottom
        }

        Item {
            id: contentItem
            height: childrenRect.height
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }
}