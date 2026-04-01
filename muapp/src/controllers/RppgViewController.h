//
// Created by Lenovo on 2026/3/17.
//

#ifndef RPPGVIEWCONTROLLER_H
#define RPPGVIEWCONTROLLER_H

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QVariantMap>
#include <QtQml/QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

#include "RppgViewFrame.h"

class RppgViewWorker final : public QObject {
    Q_OBJECT

public:
    explicit RppgViewWorker(QObject* parent = nullptr);

    signals:
        void frameReady(const RppgViewFrame& frame);

public slots:
    Q_INVOKABLE void pushData(const QJsonObject& data);
    Q_INVOKABLE void fetchNextFrame();
    Q_INVOKABLE void reset();

private:
    QQueue<RppgViewFrame> m_queue;
};

class RppgViewController : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit RppgViewController(QObject* parent = nullptr);
    ~RppgViewController() override;

    void classBegin() override;
    void componentComplete() override;

public slots:
    void pushData(const QJsonObject& data, const QString& studentId);
    Q_INVOKABLE void reset();

    signals:
        void frameUpdated(const QVariantMap& frame);
    void resetRequested();

private slots:
    void onTimeout();
    void onFrameReady(const RppgViewFrame& frame);

private:
    void startRendering();
    void stopRendering();

private:
    QPointer<RppgViewWorker> m_worker;
    QPointer<QThread> m_thread;
    QTimer m_timer;
};

#endif // RPPGVIEWCONTROLLER_H