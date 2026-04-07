#include <functional>

#include <QtCore/QFile>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>

#include "network/HttpMgr.h"
#include "services/AuthService.h"
#include "services/CoSettingsMgr.h"
#include "services/UserApi.h"
#include "views/LoginDialog.h"
#include "views/MainWindow.h"

namespace {
    void applyQss(QApplication& app) {
        QFile file{":/res/style_light.qss"};
        if (file.open(QFile::ReadOnly)) {
            app.setStyleSheet(file.readAll());
        } else {
            qDebug() << "Failed to load stylesheet.";
        }
        file.close();
    }

    void applyTranslation(QApplication& app) {
        static QTranslator g_translator;
        if (g_translator.load(QLocale(), "CoApp", "_", ":/i18n/")) {
            app.installTranslator(&g_translator);
        } else {
            qDebug() << "Failed to load translation file.";
        }
    }

    void initApp(QApplication& app) {
        app.setQuitOnLastWindowClosed(false);
        app.setOrganizationName("perlab");
        app.setOrganizationDomain("perlab.edu");
        app.setApplicationVersion(APP_VERSION);

        if (app.platformName() == "windows") {
            auto font = QFont("Microsoft YaHei UI", 9);
            font.setHintingPreference(QFont::PreferNoHinting);
            app.setFont(font);
        }

        applyQss(app);
        applyTranslation(app);

        app.setApplicationDisplayName(QObject::tr("Student Collection Terminal"));
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    initApp(app);

    // 初始化HTTP管理器的基础URL
    HttpMgr::setBaseUrl(CoSettingsMgr::httpBaseUrl());

    MainWindow* mainWindow = nullptr;

    // 创建并显示主窗口 Lambda 函数
    auto showMainWindow = [&]() {
        if (!mainWindow) {
            mainWindow = new MainWindow;
            mainWindow->setAttribute(Qt::WA_DeleteOnClose);
            QObject::connect(mainWindow, &MainWindow::windowClosed, &app, [&](const bool shouldQuitApp) {
                mainWindow = nullptr;
                if (shouldQuitApp)
                    app.quit();
            });
        }
        mainWindow->setQuitOnClose(true);
        mainWindow->show();
        mainWindow->raise();          // 显示在最前
        mainWindow->activateWindow(); // 获得焦点
        if (AuthService::isAuthenticated())
            UserApi::fetchCurrentUser();
    };

    // 隐藏并删除主窗口
    auto hideAndDeleteMainWindow = [&]() {
        if (!mainWindow)
            return;
        mainWindow->setQuitOnClose(false);
        mainWindow->close();
        mainWindow = nullptr;
    };

    // 显示登录对话框
    const std::function<bool()> showLoginDialog = [&]() -> bool {
        hideAndDeleteMainWindow();

        LoginDialog loginDialog;
        const bool accepted = loginDialog.exec() == QDialog::Accepted;
        if (accepted)
            showMainWindow();
        return accepted;
    };

    // 响应认证状态更改
    QObject::connect(&AuthService::instance(), &AuthService::authenticationChanged, &app,
        [&](const bool authenticated) {
            if (authenticated)
                return;

            if (!showLoginDialog())
                app.quit();
        });

    if (AuthService::isAuthenticated()) {
        showMainWindow();
    } else if (!showLoginDialog()) {
        return 0;
    }

    return app.exec();
}
