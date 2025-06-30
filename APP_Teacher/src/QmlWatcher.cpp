#include "QmlWatcher.h"
#include <QDir>
#include <QDirIterator>
#include <QTimer>


QmlWatcher::QmlWatcher(QObject* parent) : QObject(parent) {
    m_watcher = new QFileSystemWatcher(this);

    // QStringList qmlFiles;
    // QDirIterator it(
    //     dirPath, QStringList() << "*.qml", QDir::Files, QDirIterator::Subdirectories
    // );
    //
    // while (it.hasNext()) {
    //     qmlFiles << it.next();
    // }
    //
    // m_watcher->addPaths(qmlFiles);

    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        QTimer::singleShot(100, [=]() {
            // qDebug() << "File changed:" << path;
            emit fileChanged(path);
        });
    });
}

void QmlWatcher::setDirPath(const QString& path) {
    if (m_dirPath == path) {
        return;
    }

    m_dirPath = path;
    emit dirPathChanged();

    // 先移除旧的路径
    if (!m_watcher->files().isEmpty()) {
        m_watcher->removePaths(m_watcher->files());
    }

    // 添加新的路径
    QDirIterator it(
        m_dirPath, QStringList() << "*.qml", QDir::Files, QDirIterator::Subdirectories
    );

    while (it.hasNext()) {
        m_watcher->addPath(it.next());
    }
}