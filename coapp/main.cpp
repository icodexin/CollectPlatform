#include <QApplication>
#include <QFile>
#include <QTranslator>

#include "views/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("perlab");
    app.setOrganizationDomain("perlab.edu");
    app.setApplicationVersion(APP_VERSION);

    if (app.platformName() == "windows") {
        auto font = QFont("Microsoft YaHei UI", 9);
        font.setHintingPreference(QFont::PreferNoHinting);
        app.setFont(font);
    }

    QFile file = QFile(":/res/style_light.qss");
    if (file.open(QFile::ReadOnly)) {
        app.setStyleSheet(file.readAll());
    } else {
        qDebug() << "Failed to load stylesheet.";
    }
    file.close();


    QTranslator translator;
    if (translator.load(QLocale(), "CoApp", "_", ":/i18n/")) {
        app.installTranslator(&translator);
    } else {
        qDebug() << "Failed to load translation file.";
    }

    app.setApplicationDisplayName(QObject::tr("Student Collection Terminal"));

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
