#include <QApplication>
#include <QFile>

#include "views/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("perlab");
    app.setOrganizationDomain("perlab.edu");

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

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}