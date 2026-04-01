#include "SummaryController.h"

#include <QVariantMap>
#include <QDebug>

SummaryController::SummaryController(QObject* parent)
    : QObject(parent)
{
    initDefaultPhysioData();
    initDefaultEmotionData();
    initDefaultCognitionData();
}

DataStreamService* SummaryController::dataStreamService() const
{
    return m_dataStreamService;
}

void SummaryController::setDataStreamService(DataStreamService* service)
{
    if (m_dataStreamService == service) {
        return;
    }

    if (m_dataStreamService) {
        disconnect(m_dataStreamService, nullptr, this, nullptr);
    }

    m_dataStreamService = service;

    if (m_dataStreamService) {
        connect(m_dataStreamService, &DataStreamService::summaryReceived,
                this, &SummaryController::onSummaryReceived);
    }

    emit dataStreamServiceChanged();
}

QVariantList SummaryController::physioData() const
{
    return m_physioData;
}

QVariantList SummaryController::emotionData() const
{
    return m_emotionData;
}

QVariantList SummaryController::cognitionData() const
{
    return m_cognitionData;
}

void SummaryController::reset()
{
    initDefaultPhysioData();
    initDefaultEmotionData();
    initDefaultCognitionData();

    emit physioDataChanged();
    emit emotionDataChanged();
    emit cognitionDataChanged();

    qDebug() << "SummaryController reset done.";
}

void SummaryController::initDefaultPhysioData()
{
    m_physioData.clear();

    for (int i = 1; i <= 8; ++i) {
        m_physioData.append(QVariantMap{
            {"studentId", QString("1000%1").arg(i)},
            {"name", QString("Student %1").arg(i)},
            {"respiration", "0"},
            {"heartRate", "0"},
            {"diastolic", "0"},
            {"systolic", "0"}
        });
    }
}

void SummaryController::initDefaultEmotionData()
{
    m_emotionData.clear();

    for (int i = 1; i <= 8; ++i) {
        m_emotionData.append(QVariantMap{
            {"studentId", QString("1000%1").arg(i)},
            {"name", QString("Student %1").arg(i)},
            {"result", "-"}
        });
    }
}

void SummaryController::initDefaultCognitionData()
{
    m_cognitionData.clear();

    for (int i = 1; i <= 8; ++i) {
        m_cognitionData.append(QVariantMap{
            {"studentId", QString("1000%1").arg(i)},
            {"name", QString("Student %1").arg(i)},
            {"stress", "-"},
            {"fatigue", "-"}
        });
    }
}

int SummaryController::findStudentIndex(const QString& studentId) const
{
    for (int i = 0; i < m_physioData.size(); ++i) {
        const QVariantMap row = m_physioData[i].toMap();
        if (row.value("studentId").toString() == studentId) {
            return i;
        }
    }
    return -1;
}

int SummaryController::findEmotionIndex(const QString& studentId) const
{
    for (int i = 0; i < m_emotionData.size(); ++i) {
        const QVariantMap row = m_emotionData[i].toMap();
        if (row.value("studentId").toString() == studentId) {
            return i;
        }
    }
    return -1;
}

int SummaryController::findCognitionIndex(const QString& studentId) const
{
    for (int i = 0; i < m_cognitionData.size(); ++i) {
        const QVariantMap row = m_cognitionData[i].toMap();
        if (row.value("studentId").toString() == studentId) {
            return i;
        }
    }
    return -1;
}

QString SummaryController::studentNameOf(const QString& studentId) const
{
    if (studentId == "10001") return "Student 1";
    if (studentId == "10002") return "Student 2";
    if (studentId == "10003") return "Student 3";
    if (studentId == "10004") return "Student 4";
    if (studentId == "10005") return "Student 5";
    if (studentId == "10006") return "Student 6";
    if (studentId == "10007") return "Student 7";
    if (studentId == "10008") return "Student 8";
    return studentId;
}

QString SummaryController::jsonValueToString(const QJsonValue& value, const QString& fallback) const
{
    if (value.isUndefined() || value.isNull()) {
        return fallback;
    }

    const QString text = value.toVariant().toString();
    return text.isEmpty() ? fallback : text;
}

void SummaryController::onSummaryReceived(const QJsonObject& obj)
{
    qDebug() << "====== Summary Result Received ======";
    qDebug() << "payload:" << obj;

    const QString studentId = obj.value("student_id").toString();
    if (studentId.isEmpty()) {
        qWarning() << "SummaryController: student_id is empty";
        return;
    }

    const QString dataType = obj.value("data_type").toString();

    // 1. 生理数据
    if (dataType == "mmwav" || obj.contains("respiratory_rate") || obj.contains("heart_rate")) {
        int idx = findStudentIndex(studentId);

        QVariantMap physioRow;
        if (idx >= 0) {
            physioRow = m_physioData[idx].toMap();
        } else {
            physioRow["studentId"] = studentId;
            physioRow["name"] = studentNameOf(studentId);
            physioRow["respiration"] = "0";
            physioRow["heartRate"] = "0";
            physioRow["diastolic"] = "0";
            physioRow["systolic"] = "0";
        }

        if (obj.contains("respiratory_rate")) {
            physioRow["respiration"] = jsonValueToString(
                obj.value("respiratory_rate"),
                physioRow.value("respiration").toString()
            );
        }

        if (obj.contains("heart_rate")) {
            physioRow["heartRate"] = jsonValueToString(
                obj.value("heart_rate"),
                physioRow.value("heartRate").toString()
            );
        }

        if (obj.contains("diastolic")) {
            physioRow["diastolic"] = jsonValueToString(
                obj.value("diastolic"),
                physioRow.value("diastolic").toString()
            );
        }

        if (obj.contains("systolic")) {
            physioRow["systolic"] = jsonValueToString(
                obj.value("systolic"),
                physioRow.value("systolic").toString()
            );
        }

        if (idx >= 0) {
            m_physioData[idx] = physioRow;
        } else {
            m_physioData.append(physioRow);
        }

        emit physioDataChanged();
        qDebug() << "SummaryController physio updated:" << physioRow;
    }

    // 2. 情绪数据
    if (dataType == "wristbandemotion" || obj.contains("pred_name")) {
        int eidx = findEmotionIndex(studentId);

        QVariantMap emotionRow;
        if (eidx >= 0) {
            emotionRow = m_emotionData[eidx].toMap();
        } else {
            emotionRow["studentId"] = studentId;
            emotionRow["name"] = studentNameOf(studentId);
            emotionRow["result"] = "-";
        }

        emotionRow["result"] = jsonValueToString(
            obj.value("pred_name"),
            emotionRow.value("result").toString()
        );

        if (eidx >= 0) {
            m_emotionData[eidx] = emotionRow;
        } else {
            m_emotionData.append(emotionRow);
        }

        emit emotionDataChanged();
        qDebug() << "SummaryController emotion updated:" << emotionRow;
    }

    // 3. 认知数据
    if (dataType == "rppg" || dataType == "mmwav" || obj.contains("stress_state") || obj.contains("fatigue_state")) {
        int cidx = findCognitionIndex(studentId);

        QVariantMap cognitionRow;
        if (cidx >= 0) {
            cognitionRow = m_cognitionData[cidx].toMap();
        } else {
            cognitionRow["studentId"] = studentId;
            cognitionRow["name"] = studentNameOf(studentId);
            cognitionRow["stress"] = "-";
            cognitionRow["fatigue"] = "-";
        }

        if (obj.contains("stress_state")) {
            cognitionRow["stress"] = jsonValueToString(
                obj.value("stress_state"),
                cognitionRow.value("stress").toString()
            );
        }

        if (obj.contains("fatigue_state")) {
            cognitionRow["fatigue"] = jsonValueToString(
                obj.value("fatigue_state"),
                cognitionRow.value("fatigue").toString()
            );
        }

        if (cidx >= 0) {
            m_cognitionData[cidx] = cognitionRow;
        } else {
            m_cognitionData.append(cognitionRow);
        }

        emit cognitionDataChanged();
        qDebug() << "SummaryController cognition updated:" << cognitionRow;
    }
}