import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic
import MuApp

MuPage {
    default property alias content: contentItem.data

    Flickable {
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

        function scrollBy(deltaX, deltaY) {
            const minY = 0
            const maxY = Math.max(0, contentHeight - height)
            const minX = 0
            const maxX = Math.max(0, contentWidth - width)

            let newY = contentY + deltaY
            let newX = contentX + deltaX

            contentY = Math.max(minY, Math.min(maxY, newY))
            contentX = Math.max(minX, Math.min(maxX, newX))
        }

        WheelHandler {
            id: wheelHandler
            target: null
            acceptedDevices: PointerDevice.Mouse

            onWheel: (event) => {
                let dx = 0
                let dy = 0

                if (event.pixelDelta.x !== 0 || event.pixelDelta.y !== 0) {
                    dx = event.pixelDelta.x
                    dy = event.pixelDelta.y
                } else {
                    // 传统鼠标：angleDelta 单位是 1/8°，一般为 120 -> 步长 = 120 / 8 = 15
                    dx = event.angleDelta.x / 8
                    dy = event.angleDelta.y / 8
                }

                flickable.scrollBy(-dx, -dy)
                event.accepted = true
            }
        }
    }
}