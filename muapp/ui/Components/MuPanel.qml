import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

Column {
    id: root
    property string title: ""
    property bool showTitle: true
    property alias titleFont: titleText.font
    property alias cardHeight: card.height
    default property alias content: contentItem.data

    spacing: 8

    HusText {
        id: titleText
        visible: root.showTitle && root.title !== ""
        text: root.title
        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading4
        font.weight: Font.DemiBold
    }

    Rectangle {
        id: card
        width: parent.width
        color: HusTheme.HusCard.colorBg
        border.color: HusTheme.isDark ? HusTheme.HusCard.colorBorderDark : HusTheme.HusCard.colorBorder
        opacity: 0.8
        radius: 10

        Item {
            id: contentItem
            anchors.fill: parent
            anchors.margins: 8
        }

        Component.onCompleted: {
            if (!height)
                implicitHeight = Math.max(contentItem.childrenRect.height + contentItem.anchors.margins * 2, 100);
        }
    }
}