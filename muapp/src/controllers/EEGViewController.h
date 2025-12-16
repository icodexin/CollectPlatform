#ifndef EEGVIEWCONTROLLER_H
#define EEGVIEWCONTROLLER_H

#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtQml/QQmlParserStatus>
#include "models/EEGViewFrame.h"

class EEGViewWorker final: public QObject {
    Q_OBJECT

public:
    explicit EEGViewWorker(QObject* parent = nullptr);

    signals:
        void frameReady(const EEGViewFrame&);

public slots:
    Q_INVOKABLE void pushData(const EEGPacket& data);
    Q_INVOKABLE void fetchNextFrame();

private:
    QQueue<EEGViewFrame> m_queue;
};

class EEGViewController : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit EEGViewController(QObject* parent = nullptr);
    ~EEGViewController() override;

    void classBegin() override;
    void componentComplete() override;

signals:
    void frameUpdated(const EEGViewFrame&);

public slots:
    void pushData(const EEGPacket& data);
    void startRendering();
    void stopRendering();

private slots:
    void onTimeout();

private:
    QThread* m_thread = nullptr;
    QPointer<EEGViewWorker> m_worker = nullptr;
    QTimer m_timer;
};

#endif //EEGVIEWCONTROLLER_H