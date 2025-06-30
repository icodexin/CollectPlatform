import QtQuick
import DelegateUI

Item {
    id: page

    property alias titleIconSource: titleIcon.iconSource
    property alias titleText: titleText.text
    property real leftMargin: 20
    property real topMargin: 20
    property real rightMargin: 20
    property real bottomMargin: 8
    property real spacing: 30

    default property alias content: pageContent.data

    Row {
        id: titleBar
        anchors.left: parent.left
        anchors.leftMargin: page.leftMargin
        anchors.top: parent.top
        anchors.topMargin: page.topMargin
        anchors.right: parent.right
        anchors.rightMargin: page.rightMargin
        spacing: 8

        DelIconText {
            id: titleIcon
            iconSize: DelTheme.Primary.fontPrimarySizeHeading2
            anchors.verticalCenter: titleBar.verticalCenter
        }

        Text {
            id: titleText
            anchors.verticalCenter: titleBar.verticalCenter
            font {
                family: DelTheme.Primary.fontPrimaryFamily
                pixelSize: DelTheme.Primary.fontPrimarySizeHeading2
                weight: Font.DemiBold
            }
            color: DelTheme.Primary.colorTextBase
        }
    }

    Item {
        id: pageContent
        anchors.left: parent.left
        anchors.leftMargin: page.leftMargin
        anchors.top: titleBar.bottom
        anchors.topMargin: page.spacing
        anchors.right: parent.right
        anchors.rightMargin: page.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: page.bottomMargin
    }
}