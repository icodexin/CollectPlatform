#ifndef BANDVIEWCONTROLLER_H
#define BANDVIEWCONTROLLER_H

#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <QtQml/QQmlParserStatus>
#include "models/BandViewFrame.h"

class BandViewWorker : public QObject {
    Q_OBJECT

public:
    explicit BandViewWorker(QObject* parent = nullptr);

    signals:
        void frameReady(const BandViewFrame& frame);

public slots:
    Q_INVOKABLE void pushJsonData(const QVariantMap& data, const QString& studentId);
    Q_INVOKABLE void fetchNextFrame();
    Q_INVOKABLE void reset();

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
        void frameUpdated(const BandViewFrame& frame);
    void resetRequested();

public slots:
    void pushJsonData(const QVariantMap& data, const QString& studentId);
    void startRendering();
    void stopRendering();
    void reset();

private slots:
    void onTimeout();

private:
    QThread* m_thread = nullptr;
    QPointer<BandViewWorker> m_worker = nullptr;
    QTimer m_timer;
};

#endif // BANDVIEWCONTROLLER_H