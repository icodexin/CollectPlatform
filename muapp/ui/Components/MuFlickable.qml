import QtQuick

Flickable {
    id: _flickable

    WheelHandler {
        target: null
        acceptedDevices: PointerDevice.Mouse
        property int baseDegrees: 15 // 鼠标滚动一个步长对应的
        property int basePixel: 10

        onWheel: event => {
            let dx = 0
            let dy = 0

            if (event.pixelDelta.x !== 0 || event.pixelDelta.y !== 0) {
                // 高分辨率设备
                dx = event.pixelDelta.x
                dy = event.pixelDelta.y
            } else {
                const baseDegrees = 15  // 鼠标滚动一个步长对应的角度
                const basePixel = 10    // 鼠标滚动一个步长对应在屏幕上滚动的像素
                // 传统鼠标：angleDelta 单位是 1/8°
                dx = event.angleDelta.x / 8 / baseDegrees * basePixel
                dy = event.angleDelta.y / 8 / baseDegrees * basePixel
            }

            _flickable.scrollBy(-dx, -dy)
            event.accepted = true
        }
    }

    function scrollBy(deltaX, deltaY) {
        const maxY = Math.max(0, contentHeight - height)
        const maxX = Math.max(0, contentWidth - width)

        let newY = contentY + deltaY
        let newX = contentX + deltaX

        contentY = Math.max(0, Math.min(maxY, newY))
        contentX = Math.max(0, Math.min(maxX, newX))
    }
}
