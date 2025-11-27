import QtQuick
import QtQuick.Controls.Basic
import HuskarUI.Basic

Page {
    id: page

    property alias titleIconSource: titleIcon.iconSource

    horizontalPadding: 15
    verticalPadding: 15
    spacing: 0
    background: Item {}

    header: Item {
        implicitHeight: __row.implicitHeight + page.verticalPadding

        Row {
            id: __row
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.leftMargin: page.leftPadding
            anchors.topMargin: page.topPadding
            anchors.rightMargin: page.rightPadding
            spacing: 8

            HusIconText {
                id: titleIcon
                iconSize: HusTheme.Primary.fontPrimarySizeHeading3
                anchors.verticalCenter: parent.verticalCenter
            }

            HusText {
                text: page.title
                anchors.verticalCenter: parent.verticalCenter
                font {
                    pixelSize: HusTheme.Primary.fontPrimarySizeHeading3
                    weight: Font.DemiBold
                }
            }
        }
    }
}
