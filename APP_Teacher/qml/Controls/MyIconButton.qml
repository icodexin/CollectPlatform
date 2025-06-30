import QtQuick
import QtQuick.Templates as T
import Qt5Compat.GraphicalEffects
import DelegateUI

T.Button {
    id: control

    enum Shape {
        Shape_Default = 0,
        Shape_Circle = 1
    }

    enum IconPosition {
        Position_Start = 0,
        Position_End = 1
    }

    property bool loading: false
    property string iconSource: ""
    property int iconSize: DelTheme.DelButton.fontSize
    property int iconSpacing: 5
    property int iconPosition: DelIconButton.Position_Start
    property color colorIcon: colorText

    property bool animationEnabled: DelTheme.animationEnabled
    property bool effectEnabled: true
    property int hoverCursorShape: Qt.PointingHandCursor
    property int type: DelButton.Type_Default
    property int shape: DelButton.Shape_Default
    property int radiusBg: 5
    property color colorText: enabled ? DelTheme.Primary.colorTextBase : DelTheme.DelButton.colorTextDisabled
    property color colorBg: {
        if (enabled) {
            return control.down ? DelTheme.DelCaptionButton.colorBgActive :
                control.hovered ? DelTheme.DelCaptionButton.colorBgHover : DelTheme.DelCaptionButton.colorBg;
        } else {
            return DelTheme.DelButton.colorBgDisabled;
        }
    }
    property string contentDescription: text

    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    padding: 2
    font {
        family: DelTheme.DelButton.fontFamily
        pixelSize: DelTheme.DelButton.fontSize
    }
    contentItem: Item {
        implicitWidth: __row.implicitWidth
        implicitHeight: Math.max(__icon.implicitHeight, __text.implicitHeight)

        Row {
            id: __row
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: control.iconSpacing
            layoutDirection: control.iconPosition === DelIconButton.Position_Start ? Qt.LeftToRight : Qt.RightToLeft

            // DelIconText {
            //     id: __icon
            //     anchors.verticalCenter: parent.verticalCenter
            //     color: control.colorIcon
            //     iconSize: control.iconSize
            //     iconSource: control.loading ? DelIcon.LoadingOutlined : control.iconSource
            //     verticalAlignment: Text.AlignVCenter
            //
            //     Behavior on color { enabled: control.animationEnabled; ColorAnimation { duration: DelTheme.Primary.durationFast } }
            //
            //     NumberAnimation on rotation {
            //         running: control.loading
            //         from: 0
            //         to: 360
            //         loops: Animation.Infinite
            //         duration: 1000
            //     }
            // }

            Image {
                id: __icon
                anchors.verticalCenter: parent.verticalCenter
                width: control.iconSize
                height: control.iconSize
                source: control.iconSource

                ColorOverlay {
                    anchors.fill: __icon
                    source: __icon
                    color: control.colorIcon
                }
            }


            Text {
                id: __text
                anchors.verticalCenter: parent.verticalCenter
                text: control.text
                font: control.font
                lineHeight: DelTheme.DelButton.fontLineHeight
                color: control.colorText
                elide: Text.ElideRight

                Behavior on color { enabled: control.animationEnabled; ColorAnimation { duration: DelTheme.Primary.durationFast } }
            }
        }
    }
    background: Item {
        Rectangle {
            id: __bg
            width: realWidth
            height: realHeight
            anchors.centerIn: parent
            radius: control.shape == MyIconButton.Shape_Default ? control.radiusBg : height * 0.5
            color: control.colorBg
            border.width: 0
            border.color: "transparent"

            property real realWidth: control.shape == MyIconButton.Shape_Default ? parent.width : parent.height
            property real realHeight: control.shape == MyIconButton.Shape_Default ? parent.height : parent.height

            Behavior on color { enabled: control.animationEnabled; ColorAnimation { duration: DelTheme.Primary.durationMid } }
        }
    }

    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: control.contentDescription
    Accessible.onPressAction: control.clicked();
}
