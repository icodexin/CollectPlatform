#include "DataStreamService.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QLoggingCategory>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

#include "model/serialize.h"
#include "network/WebsocketMgr.h"

namespace {
    constexpr const char* kDataStreamKey = "datastream";
    constexpr const char* kWebsocketUrl = "ws://localhost:8000/datastream/ws";
}

Q_LOGGING_CATEGORY(dsService, "Services.DataStreamService")

DataStreamService::DataStreamService(QObject* parent) : QObject(parent) {
    m_emotion_model = new EmotionModel(this);
}
EmotionModel* DataStreamService::emotionModel() const {
    return m_emotion_model;
}
DataStreamService::~DataStreamService() {
    if (MuWebsocketMgr.hasConnection(kDataStreamKey)) {
        MuWebsocketMgr.removeConnection(kDataStreamKey);
    }
}

void DataStreamService::classBegin() {
}

void DataStreamService::componentComplete() {
    start();
}

QString DataStreamService::subStudentId() const {
    return m_subStudentId;
}

void DataStreamService::setSubStudentId(const QString& studentId) {
    if (m_subStudentId == studentId) {
        return;
    }

    const QString oldStudentId = m_subStudentId;
    m_subStudentId = studentId;
    emit subStudentIdChanged(studentId);

    qDebug() << "subStudentId changed:" << oldStudentId << "->" << m_subStudentId;

    if (m_status == Online && !m_subStudentId.isEmpty()) {
        subscribe(m_subStudentId, m_subDataType);
    }
}

DataStreamService::DataType DataStreamService::subDataType() const {
    return m_subDataType;
}

void DataStreamService::setSubDataType(const DataType type) {
    if (m_subDataType == type) {
        return;
    }

    const DataType oldType = m_subDataType;
    m_subDataType = type;
    emit subDataTypeChanged(type);

    if (m_status == Online && !m_subStudentId.isEmpty()) {
        subscribe(m_subStudentId, m_subDataType);
    }
}
void DataStreamService::resubscribe() {
    if (m_status != Online) {
        qDebug() << "Cannot resubscribe, websocket is offline.";
        return;
    }

    if (m_subStudentId.isEmpty()) {
        qDebug() << "Cannot resubscribe, subStudentId is empty.";
        return;
    }

    subscribe(m_subStudentId, m_subDataType);
}
DataStreamService::Status DataStreamService::status() const {
    return m_status;
}

int DataStreamService::connectTimes() const {
    return m_connectTimes;
}

void DataStreamService::setStatus(const Status status) {
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged(status);
}

void DataStreamService::setConnectTimes(const int times) {
    if (m_connectTimes == times)
        return;
    m_connectTimes = times;
    emit connectTimesChanged(times);
}

void DataStreamService::start() {
    if (!MuWebsocketMgr.hasConnection(kDataStreamKey)) {
        MuWebsocketMgr.setReconnectParam({.maxAttempts = 0, .baseNumber = 1}); // infinite reconnect
        MuWebsocketMgr.createConnection(kDataStreamKey, QUrl(kWebsocketUrl));
        MuWebsocketMgr.open(kDataStreamKey);
        attachClientSignals(kDataStreamKey);
    }
}

void DataStreamService::subscribe(const QString& studentId, const DataType type) {
    if (m_status == Offline) {
        qCDebug(dsService) << "Cannot subscribe, WebSocket is not connected.";
        return;
    }

    QString typeStr = QVariant::fromValue(type).toString().toLower();
    qCDebug(dsService) << "发送订阅请求：学生ID=" << studentId << "，类型=" << typeStr;

    MuWebsocketMgr.sendJson(kDataStreamKey, {
        {"msg_type", "subscribe"},
        {
            "subscriptions", QJsonObject{
                {studentId, QVariant::fromValue(type).toString().toLower()}
            }
        }
    });
}

void DataStreamService::attachClientSignals(const QString& key) {
    const QPointer<WebsocketClient> client = MuWebsocketMgr.client(key);
    connect(client, &WebsocketClient::textReceived, this, &DataStreamService::handleTextMessage);
    connect(client, &WebsocketClient::binaryReceived, this, &DataStreamService::handleBinaryMessage);

    connect(client, &WebsocketClient::statusChanged, this, [this](const WebsocketClient::Status status) {
        switch (status) {
            case WebsocketClient::Closed:
            case WebsocketClient::Closing: {
                setStatus(Offline);
                break;
            }
            case WebsocketClient::Open: {
                setStatus(Online);
                if (!m_subStudentId.isEmpty())
                    // todo: change subscription when studentId or dataType changed at runtime
                    subscribe(m_subStudentId, m_subDataType);
                break;
            }
            default: {
                setStatus(Connecting);
                break;
            }
        }
    });
    connect(client, &WebsocketClient::reconnectAttemptsChanged, this, &DataStreamService::setConnectTimes);
    connect(client, &WebsocketClient::errorOccurred, this,
        [](const QAbstractSocket::SocketError error, const QString& errorString) {
            qCDebug(dsService).noquote() << QString("WebSocket error occurred: %1 - %2").arg(error).arg(errorString);
        });
}
void DataStreamService::handleBcg(const QJsonObject& obj, const QString& student_id)
{
    // qDebug() << "====== BCG Received ======";
    //
    // QJsonArray heart_wave = obj["heart_waveform"].toArray();
    // QJsonArray resp_wave  = obj["resp_waveform"].toArray();
    //
    // qDebug() << "heart_wave size:" << heart_wave.size();
    // qDebug() << "resp_wave size:" << resp_wave.size();

    emit bcgReceived(obj, student_id);
}
void DataStreamService::handleMmwav(const QJsonObject& obj, const QString& student_id)
{
    // qDebug() << "====== mmWav Received ======";
    //
    // QJsonArray heart_wave = obj["heart_wave_filtered"].toArray();
    // QJsonArray breath_wave = obj["breath_wave_filtered"].toArray();
    //
    // qDebug() << "heart_wave_filtered size:" << heart_wave.size();
    // qDebug() << "breath_wave_filtered size:" << breath_wave.size();

    emit mmwavReceived(obj, student_id);
}
void DataStreamService::handleRppg(const QJsonObject& obj, const QString& student_id)
{
    // qDebug() << "====== rPPG Received ======";
    //
    // QJsonArray heart_wave = obj["heart_waveform"].toArray();
    // QJsonArray resp_wave  = obj["resp_waveform"].toArray();

    // qDebug() << "heart_waveform size:" << heart_wave.size();
    // qDebug() << "resp_waveform size:" << resp_wave.size();
    // qDebug() << "face_detected:" << obj["face_detected"].toBool();
    // qDebug() << "heart_rate:" << obj["heart_rate"].toVariant();
    // qDebug() << "respiratory_rate:" << obj["respiratory_rate"].toVariant();
    // qDebug() << "stress_state:" << obj["stress_state"].toString();

    emit rppgReceived(obj, student_id);
}
void DataStreamService::handleWristband(const QJsonObject& obj, const QString& student_id)
{
    // qDebug() << "====== Wristband Received ======";
    // qDebug().noquote() << "payload:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);

    // 后面教师端可以从 obj 里直接取：
    // obj["ppg_filtered"].toArray()
    // obj["accel_x"].toArray()
    // obj["accel_y"].toArray()
    // obj["accel_z"].toArray()
    // obj["time_intervals"].toArray()

    emit wristbandJsonReceived(obj, student_id);
}
void DataStreamService::handleTextMessage(const QString& text)
{
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());

    if (!doc.isObject())
        return;

    QJsonObject obj = doc.object();

    QString student_id = obj["student_id"].toString();
    QString data_type  = obj["data_type"].toString();
    bool isSummary     = obj["summary"].toBool(false);

    if (isSummary) {
        qDebug() << "====== Summary Result Received ======";
        qDebug().noquote() << "payload:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        emit summaryReceived(obj);
        return;
    }

    if (data_type == "bcg") {
        handleBcg(obj, student_id);
    }
    else if (data_type == "mmwav") {
        handleMmwav(obj, student_id);
    }
    else if (data_type == "rppg") {
        handleRppg(obj, student_id);
    }
    else if (data_type == "wristband") {
        handleWristband(obj, student_id);
    }
}

void DataStreamService::handleBinaryMessage(const QByteArray& data) {
    try {
        const msgpack::object_handle oh = msgpack::unpack(data.constData(), data.size());
        auto msg_map = oh.get().as<std::map<QString, msgpack::object>>();

        if (!msg_map.contains("student_id")) {
            throw std::runtime_error("student_id field missing");
        }
        auto student_id = msg_map["student_id"].as<QString>();

        if (!msg_map.contains("data_type")) {
            throw std::runtime_error("data_type field missing");
        }
        auto data_type = msg_map["data_type"].as<QString>();

        if (!msg_map.contains("data")) {
            throw std::runtime_error("data field missing");
        }
        auto data_obj = msg_map["data"];

        if (data_type == "eeg") {
            auto eeg_data = data_obj.as<EEGPacket>();
            emit eegReceived(eeg_data, student_id);
        }
        else if (data_type == "wristbandemotion") {
            auto emotion_map = data_obj.as<std::map<std::string, msgpack::object>>();

            int pred_class = emotion_map["pred_class"].as<int>();
            QString pred_name = QString::fromStdString(emotion_map["pred_name"].as<std::string>());

            qDebug().noquote() << QString(
                "====== Wristband Emotion Received ======\n"
                "student_id: %1\n"
                "pred_name: %3"
            ).arg(student_id)
             .arg(pred_name);

            QMetaObject::invokeMethod(m_emotion_model, "updateEmotionData",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, pred_name));
        }
        else {
            throw std::runtime_error("unknown data_type: " + data_type.toStdString());
        }
    }
    catch (const msgpack::type_error& e) {
        qCDebug(dsService) << QString("MessagePack type error: %1").arg(e.what());
    }
    catch (const std::exception& e) {
        qCDebug(dsService) << QString("Error processing binary message: %1").arg(e.what());
    }
}