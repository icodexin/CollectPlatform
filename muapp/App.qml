import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

HusWindow {
    id: app
    minimumWidth: 800
    minimumHeight: 600
    title: Qt.application.displayName
    captionBar.color: HusTheme.Primary.colorFillTertiary
    captionBar.showThemeButton: true

    Item {
        id: content
        anchors.left: parent.left
        anchors.top: app.captionBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        MenuButton {
            id: navModeButton
            anchors.left: navMenu.left
            anchors.top: parent.top
            anchors.right: navMenu.right
            anchors.margins: 5
            anchors.rightMargin: 8
            iconSource: HusIcon.MenuOutlined

            onClicked: navMenu.changeCompactMode()
        }

        HusMenu {
            id: navMenu
            defaultMenuWidth: 200
            anchors.left: parent.left
            anchors.top: navModeButton.bottom
            anchors.topMargin: -5
            anchors.bottom: settingButton.top
            compactMode: MuSettingsMgr.navMenuCompactMode
            showToolTip: compactMode === HusMenu.Mode_Compact
            defaultSelectedKey: ["home"]
            initModel: app.getNavMenuModel()
            onCompactModeChanged: {
                MuSettingsMgr.navMenuCompactMode = compactMode
            }
            onClickMenu: (deep, key, keyPath, data) => {
                console.debug('onClickMenu', deep, key, keyPath, JSON.stringify(data));
                if (data && data.source)
                    app.loadPage(data.source)
            }

            function changeCompactMode() {
                if (compactMode === HusMenu.Mode_Compact)
                    compactMode = HusMenu.Mode_Relaxed
                else
                    compactMode = HusMenu.Mode_Compact
            }
        }

        HusDivider {
            height: 1
            anchors.left: navMenu.left
            anchors.right: navMenu.right
            anchors.bottom: settingButton.top
            anchors.margins: 5
        }

        MenuButton {
            id: settingButton
            anchors.left: navMenu.left
            anchors.right: navMenu.right
            anchors.bottom: parent.bottom
            anchors.margins: 5
            anchors.rightMargin: 8
            text: qsTr("设置")
            iconSource: HusIcon.SettingOutlined
            isCompact: navMenu.compactMode === HusMenu.Mode_Compact

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
            function onRequestCloseMessage(key) { toast.close(key) }
            function onRequestSetMessageProperty(key, property, value) { toast.setProperty(key, property, value) }
        }
    }

    HusNotification {
        id: notify
        parent: Overlay.overlay
        anchors.fill: parent
        anchors.topMargin: captionBar.height
        position: HusNotification.Position_TopRight

        Connections {
            target: AppController
            function onRequestShowNotification(msg) { notify.open(msg) }
            function onRequestCloseNotification(key) { notify.close(key) }
            function onRequestSetNotificationProperty(key, property, value) { notify.setProperty(key, property, value) }
        }
    }

    Component.onCompleted: {
        setWindowEffect()
        HusTheme.darkMode = MuSettingsMgr.appDarkMode
    }

    Component.onDestruction: {
        MuSettingsMgr.appDarkMode = HusTheme.darkMode
    }

    component MenuButton: HusButton {
        id: __menuBtn

        property int iconSize: HusTheme.HusMenu.fontSize
        property var iconSource: 0 ?? ''
        property int iconSpacing: 8
        property bool isCompact: false

        effectEnabled: false
        colorBorder: "transparent"
        colorBg: hovered ? HusTheme.HusMenu.colorBgHover : HusTheme.HusMenu.colorBg
        horizontalPadding: 12
        verticalPadding: 10
        radiusBg: HusRadius { all: HusTheme.HusMenu.radiusMenuBg }

        contentItem: RowLayout {
            spacing: parent.iconSpacing

            HusIconText {
                color: HusTheme.Primary.colorTextBase
                iconSize: __menuBtn.iconSize
                iconSource: __menuBtn.iconSource
            }

            HusText {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                text: __menuBtn.text
                visible: !__menuBtn.isCompact
            }
        }
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

    function loadPage(source) {
        if (containerLoader.source !== source) {
            containerLoader.source = source
        }
    }
}
