#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "HuskarUI/husapp.h"
#include "src/WristTest.h"


int main(int argc, char* argv[]) {
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
    engine.loadFromModule("MuApp", "App");

    // WristTest tester;
    // QObject::connect(&tester, &WristTest::dataFetched, [](const WristbandData& data) {
    //     qDebug() << data;
    // });

    return app.exec();
}
