#ifndef QMLWATCHER_H
#define QMLWATCHER_H

#include <QFileSystemWatcher>
#include <QtQml/qqml.h>


class QmlWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString dirPath READ dirPath WRITE setDirPath NOTIFY dirPathChanged)
    QML_ELEMENT

public:
    explicit QmlWatcher(QObject* parent = nullptr);

    QString dirPath() const { return m_dirPath; }

    void setDirPath(const QString& path);

signals:
    void dirPathChanged();
    void fileChanged(const QString& path);

private:
    QFileSystemWatcher* m_watcher;
    QString m_dirPath;
};


#endif //QMLWATCHER_H
