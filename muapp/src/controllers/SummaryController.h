#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <QJsonObject>
#include <QJsonValue>
#include <qqml.h>

#include "DataStreamService.h"

class SummaryController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(DataStreamService* dataStreamService
               READ dataStreamService
               WRITE setDataStreamService
               NOTIFY dataStreamServiceChanged)

    Q_PROPERTY(QVariantList physioData
               READ physioData
               NOTIFY physioDataChanged)

    Q_PROPERTY(QVariantList emotionData
               READ emotionData
               NOTIFY emotionDataChanged)

    Q_PROPERTY(QVariantList cognitionData
               READ cognitionData
               NOTIFY cognitionDataChanged)

public:
    explicit SummaryController(QObject* parent = nullptr);

    DataStreamService* dataStreamService() const;
    void setDataStreamService(DataStreamService* service);

    QVariantList physioData() const;
    QVariantList emotionData() const;
    QVariantList cognitionData() const;

    Q_INVOKABLE void reset();

    signals:
        void dataStreamServiceChanged();
    void physioDataChanged();
    void emotionDataChanged();
    void cognitionDataChanged();

private slots:
    void onSummaryReceived(const QJsonObject& obj);

private:
    void initDefaultPhysioData();
    void initDefaultEmotionData();
    void initDefaultCognitionData();

    int findStudentIndex(const QString& studentId) const;
    int findEmotionIndex(const QString& studentId) const;
    int findCognitionIndex(const QString& studentId) const;

    QString studentNameOf(const QString& studentId) const;
    QString jsonValueToString(const QJsonValue& value, const QString& fallback = "-") const;

private:
    QPointer<DataStreamService> m_dataStreamService;
    QVariantList m_physioData;
    QVariantList m_emotionData;
    QVariantList m_cognitionData;
};