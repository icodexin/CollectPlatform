import QtQuick
import HuskarUI.Basic

Item {
    id: page

    property alias titleIconSource: titleIcon.iconSource
    property alias titleText: titleText.text
    property real leftMargin: 20
    property real topMargin: 20
    property real rightMargin: 20
    property real bottomMargin: 8
    property real spacing: 20

    default property alias content: pageContent.data

    Row {
        id: titleBar
        anchors.left: parent.left
        anchors.leftMargin: parent.leftMargin
        anchors.top: parent.top
        anchors.topMargin: parent.topMargin
        anchors.right: parent.right
        anchors.rightMargin: parent.rightMargin
        spacing: 8

        HusIconText {
            id: titleIcon
            iconSize: HusTheme.Primary.fontPrimarySizeHeading3
            anchors.verticalCenter: titleBar.verticalCenter
        }

        HusText {
            id: titleText
            anchors.verticalCenter: titleBar.verticalCenter
            font {
                pixelSize: HusTheme.Primary.fontPrimarySizeHeading3
                weight: Font.DemiBold
            }
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