#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "HuskarUI/husapp.h"
#include "./src/VideoRealTime/PullWork.h"
#include <qqmlcontext.h>
#include "./src/VideoRealTime/VideoPaintedItem.h"
int main(int argc, char* argv[]) {
    //使用cpu渲染
    qmlRegisterType<VideoPaintedItem>("VideoComponents", 1, 0, "VideoPaintedItem");

#ifndef Q_OS_MAC
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif
    QQuickWindow::setDefaultAlphaBuffer(true);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("perlab");
    app.setOrganizationDomain("perlab.edu");
    app.setApplicationVersion(APP_VERSION);
    app.setApplicationDisplayName(QObject::tr("多模态数据采集与学习者状态实时感知平台"));

    QQmlApplicationEngine engine;
    HusApp::initialize(&engine);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QGuiApplication::exit(-1); },
        Qt::QueuedConnection
    );

    //创建拉流的类，注入到调用界面
    PullWork pull_work;
    engine.rootContext()->setContextProperty("pull_work",&pull_work);

    engine.loadFromModule("MuApp", "App");

    return app.exec();
}
