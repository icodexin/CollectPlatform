import QtQuick
import HuskarUI.Basic
import MuApp

MuPage {
    title: "实时采集"
    titleIconSource: HusIcon.DotChartOutlined
    titleText: "实时采集"

    BandView {
        id: bandView
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2
    }

    EEGView {
        anchors.left: bandView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}