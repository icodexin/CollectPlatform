#ifndef WRISTBANDVIEWCONTROLLER_H
#define WRISTBANDVIEWCONTROLLER_H

#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtQml/QQmlParserStatus>
#include "models/BandViewFrame.h"

class BandViewWorker final : public QObject {
    Q_OBJECT

public:
    explicit BandViewWorker(QObject* parent = nullptr);

signals:
    void frameReady(const BandViewFrame&);

public slots:
    Q_INVOKABLE void pushData(const WristbandPacket& data);
    Q_INVOKABLE void fetchNextFrame();

private:
    QQueue<BandViewFrame> m_queue;
};

class BandViewController : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit BandViewController(QObject* parent = nullptr);
    ~BandViewController() override;

    void classBegin() override;
    void componentComplete() override;

signals:
    void frameUpdated(const BandViewFrame&);

public slots:
    void pushData(const WristbandPacket& data);
    void startRendering();
    void stopRendering();

private slots:
    void onTimeout();

private:
    QThread* m_thread = nullptr;
    QPointer<BandViewWorker> m_worker = nullptr;
    QTimer m_timer;
};

#endif //WRISTBANDVIEWCONTROLLER_H
