import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

HusWindow {
    id: app
    minimumWidth: 800
    minimumHeight: 600
    title: Qt.application.displayName
    captionBar.color: HusTheme.Primary.colorFillTertiary
    captionBar.themeButtonVisible: true

    onWidthChanged: {
        checkNavMenuCompactMode()
    }

    Item {
        id: content
        anchors.left: parent.left
        anchors.top: app.captionBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        HusMenu {
            id: navMenu
            defaultMenuWidth: 200
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: settingButton.top
            compactMode: true
            tooltipVisible: navMenu.compactMode
            defaultSelectedKey: ["home"]
            initModel: app.getNavMenuModel()
            onClickMenu: (deep, key, keyPath, data) => {
                console.debug('onClickMenu', deep, key, keyPath, JSON.stringify(data));
                if (data && data.source)
                    app.loadPage(data.source)
            }
        }

        HusDivider {
            width: navMenu.width
            height: 1
            anchors.bottom: settingButton.top
        }

        HusIconButton {
            id: settingButton
            width: navMenu.width
            height: 40
            anchors.bottom: parent.bottom
            type: HusButton.Type_Text
            text: navMenu.compactMode ? '' : qsTr('设置')
            colorText: HusTheme.Primary.colorTextBase
            effectEnabled: false
            iconSize: navMenu.defaultMenuIconSize
            iconSource: HusIcon.SettingOutlined
            onClicked: {
                navMenu.clearSelectedMenu()
                app.loadPage("ui/Pages/SettingPage.qml")
            }
        }

        Item {
            id: container
            anchors.left: navMenu.right
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 8
            anchors.leftMargin: 0
            clip: true

            // 背景遮罩
            Rectangle {
                anchors.fill: parent
                color: HusTheme.HusCard.colorBg
                border.color: HusTheme.HusCard.colorBorder
                opacity: 0.6
                radius: 10
            }

            Loader {
                id: containerLoader
                anchors.fill: parent
            }
        }
    }

    HusMessage {
        id: toast
        z: 999
        parent: app.captionBar
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.bottom

        Connections {
            target: AppController
            function onRequestShowMessage(msg) { toast.open(msg) }
        }
    }

    Component.onCompleted: {
        setWindowEffect()
        checkNavMenuCompactMode()
    }

    function setWindowEffect() {
        if (Qt.platform.os === 'windows') {
            if (setSpecialEffect(HusWindow.Win_MicaAlt)) return;
            if (setSpecialEffect(HusWindow.Win_Mica)) return;
            if (setSpecialEffect(HusWindow.Win_AcrylicMaterial)) return;
            setSpecialEffect(HusWindow.Win_DwmBlur)
        } else if (Qt.platform.os === 'osx') {
            setSpecialEffect(HusWindow.Mac_BlurEffect)
        }
    }

    // todo: 改为从配置文件中获取
    function getNavMenuModel() {
        return [
            {
                key: "home",
                label: qsTr("主页"),
                iconSource: HusIcon.HomeOutlined,
                source: "ui/Pages/HomePage.qml"
            }, {
                key: "realtime",
                label: qsTr("实时采集"),
                iconSource: HusIcon.FundProjectionScreenOutlined,
                source: "ui/Pages/RealtimePage.qml"
            }, {
                key: "offline",
                label: qsTr("离线分析"),
                iconSource: HusIcon.DotChartOutlined,
                source: "ui/Pages/OfflinePage.qml"
            }, {
                key: "storage",
                label: qsTr("存储库"),
                iconSource: HusIcon.CloudOutlined,
                source: "ui/Pages/StoragePage.qml"
            }
        ]
    }

    function checkNavMenuCompactMode() {
        navMenu.compactMode = width < 1100;
    }

    function loadPage(source) {
        if (containerLoader.source !== source) {
            containerLoader.source = source
        }
    }
}