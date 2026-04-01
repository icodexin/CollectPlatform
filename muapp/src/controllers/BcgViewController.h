#ifndef BCGVIEWCONTROLLER_H
#define BCGVIEWCONTROLLER_H

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtQml/QQmlParserStatus>
#include <QtQmlIntegration/qqmlintegration.h>

#include "models/BcgViewFrame.h"

class QThread;

class BcgViewWorker final : public QObject {
    Q_OBJECT

public:
    explicit BcgViewWorker(QObject* parent = nullptr);

    signals:
        void frameReady(const BcgViewFrame& frame);

public slots:
    Q_INVOKABLE void pushData(const QJsonObject& data);
    Q_INVOKABLE void fetchNextFrame();
    Q_INVOKABLE void reset();
private:
    QQueue<BcgViewFrame> m_queue;
};

class BcgViewController : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit BcgViewController(QObject* parent = nullptr);
    ~BcgViewController() override;

    void classBegin() override;
    void componentComplete() override;

    signals:
        void frameUpdated(const BcgViewFrame& frame);
        void resetRequested();

public slots:
    void pushData(const QJsonObject& data, const QString& studentId);
    void startRendering();
    void stopRendering();
    void reset();
private slots:
    void onTimeout();

private:
    QThread* m_thread = nullptr;
    QPointer<BcgViewWorker> m_worker = nullptr;
    QTimer m_timer;
};

#endif // BCGVIEWCONTROLLER_H