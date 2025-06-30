#include <QApplication>
#include <QFile>

#include "windows/MainWindow.h"
#include "components/BaseWidget.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("perlab");
    QApplication::setApplicationName("Collect Client");

    if (app.platformName() == "windows") {
        QFont font = QFont("Microsoft YaHei UI", 9);
        font.setHintingPreference(QFont::PreferNoHinting);
        app.setFont(font);
    }

    MainWindow window = MainWindow(nullptr);
    window.resize(1000, 800);


    // 载入样式表
    QFile file = QFile(":/res/style_light.qss");
    if (file.open(QFile::ReadOnly)) {
        app.setStyleSheet(file.readAll());
    } else {
        qDebug() << "Failed to load stylesheet.";
    }
    file.close();

    window.show();

    return app.exec();
}
