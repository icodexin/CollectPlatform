import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl
import QtQuick.Shapes
import DelegateUI

T.SpinBox {
    id: control

    signal accepted

    property color colorBg: enabled ? DelTheme.DelInput.colorBg : DelTheme.DelInput.colorBgDisabled
    property color colorBorder: enabled ? activeFocus || hovered ? DelTheme.DelInput.colorBorderHover :
            DelTheme.DelInput.colorBorder : DelTheme.DelInput.colorBorderDisabled

    // Note: the width of the indicators are calculated into the padding
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             up.implicitIndicatorHeight + down.implicitIndicatorHeight)

    horizontalPadding: 10
    verticalPadding: 5
    leftPadding: horizontalPadding + (control.mirrored ? (up.indicator ? up.indicator.width : 0) : 0)
    rightPadding: horizontalPadding + (!control.mirrored ? (up.indicator ? up.indicator.width : 0) : 0)

    textFromValue: (value) => value.toString()
    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: TextInput {
        z: 2
        text: control.displayText

        font {
            family: DelTheme.DelInput.fontFamily
            pixelSize: DelTheme.DelInput.fontSize
        }
        color: enabled ? DelTheme.DelInput.colorText : DelTheme.DelInput.colorTextDisabled
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        selectByMouse: control.editable
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        clip: width < implicitWidth

        onAccepted: control.accepted()
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 1 : control.width - width - 1
        y: 1
        width: 16
        height: control.height / 2 - 1

        enabled: control.editable
        topRightRadius: 4
        color: control.colorBg

        Rectangle { // left border
            width: 1
            height: parent.height
            color: DelTheme.DelInput.colorBorder
            anchors.left: parent.left
            // anchors.margins: -1
        }

        Shape {
            width: 7
            height: 4
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 1
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeWidth: control.up.pressed ? 2 : 1
                strokeColor: !(control.value === control.to) && control.up.hovered || control.up.pressed ?
                        DelTheme.DelInput.colorBorderHover : DelTheme.DelInput.colorBorder
                fillColor: 'transparent'
                startX: 0; startY: 4

                PathLine {x: 3.5; y: 0}
                PathLine {x: 7; y: 4}
            }
        }

        Rectangle { // bottom border
            width: parent.width
            height: 0.5
            color: DelTheme.DelInput.colorBorder
            anchors.bottom: parent.bottom
        }
    }

    down.indicator: Rectangle {
        x: control.mirrored ? 1 : control.width - width - 1
        y: control.height - height - 1
        width: 16
        height: control.height / 2 - 1

        enabled: control.editable
        bottomRightRadius: 4
        clip: true
        color: control.colorBg

        Rectangle { // left border
            width: 1
            height: parent.height
            color: DelTheme.DelInput.colorBorder
            anchors.left: parent.left
            // anchors.margins: -1
        }

        Shape {
            width: 7
            height: 4
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 1
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeWidth: control.down.pressed ? 2 : 1
                strokeColor: !(control.value === control.from) && control.down.hovered || control.down.pressed ?
                        DelTheme.DelInput.colorBorderHover : DelTheme.DelInput.colorBorder
                fillColor: 'transparent'
                startX: 0; startY: 0

                PathLine {x: 3.5; y: 4}
                PathLine {x: 7; y: 0}
            }
        }

        Rectangle { // top border
            width: parent.width
            height: 0.5
            color: DelTheme.DelInput.colorBorder
            anchors.top: parent.top
        }
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 24

        color: control.colorBg
        border.color: control.colorBorder
        border.width: 1
        radius: 4

        Behavior on border.color {
            ColorAnimation {
                duration: 100
            }
        }
    }
}
