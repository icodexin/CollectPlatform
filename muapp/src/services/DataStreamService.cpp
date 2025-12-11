#include "DataStreamService.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

#include "model/serialize.h"
#include "network/WebsocketMgr.h"

namespace {
    constexpr const char* kDataStreamKey = "datastream";
    constexpr const char* kWebsocketUrl = "ws://localhost:8765/datastream";
}

Q_LOGGING_CATEGORY(dsService, "Services.DataStreamService")

DataStreamService::DataStreamService(QObject* parent) : QObject(parent) {
}

DataStreamService::~DataStreamService() {
    if (MuWebsocketMgr.hasConnection(kDataStreamKey)) {
        MuWebsocketMgr.removeConnection(kDataStreamKey);
    }
}

void DataStreamService::classBegin() {
}

void DataStreamService::componentComplete() {
    if (!m_subStudentId.isEmpty()) {
        subscribe(m_subStudentId, m_subDataType);
    }
}

QString DataStreamService::subStudentId() const {
    return m_subStudentId;
}

void DataStreamService::setSubStudentId(const QString& studentId) {
    if (m_subStudentId == studentId) {
        return;
    }
    m_subStudentId = studentId;
    emit subStudentIdChanged(studentId);
}

DataStreamService::DataType DataStreamService::subDataType() const {
    return m_subDataType;
}

void DataStreamService::setSubDataType(const DataType type) {
    if (m_subDataType == type) {
        return;
    }
    m_subDataType = type;
    emit subDataTypeChanged(type);
}

DataStreamService::Status DataStreamService::status() const {
    return m_status;
}

void DataStreamService::setStatus(const Status status) {
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged(status);
}

void DataStreamService::subscribe(const QString& studentId, const DataType type) {
    if (!MuWebsocketMgr.hasConnection(kDataStreamKey)) {
        MuWebsocketMgr.setReconnectParam({.maxAttempts = 0, .baseNumber = 1}); // infinite reconnect
        MuWebsocketMgr.createConnection(kDataStreamKey, QUrl(kWebsocketUrl));
        MuWebsocketMgr.open(kDataStreamKey);
        attachClientSignals(kDataStreamKey);
    }
    MuWebsocketMgr.sendJson(kDataStreamKey, {
        {"msg_type", "subscribe"},
        {"student_id", studentId},
        {"data_type", QVariant::fromValue(type).toString().toLower()}
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
                break;
            }
            default: {
                setStatus(Connecting);
                break;
            }
        }
    });
    connect(client, &WebsocketClient::errorOccurred, this, [](const QAbstractSocket::SocketError error, const QString& errorString) {
        qCDebug(dsService).noquote() << QString("WebSocket error occurred: %1 - %2").arg(error).arg(errorString);
    });
}

void DataStreamService::handleTextMessage(const QString& text) {
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }
    emit msgReceived(doc.object());
}

void DataStreamService::handleBinaryMessage(const QByteArray& data) {
    try {
        const msgpack::object_handle oh = msgpack::unpack(data.constData(), data.size());
        auto msg_map = oh.get().as<std::map<QString, msgpack::object>>();

        if (msg_map.find("student_id") == msg_map.end()) {
            throw std::runtime_error("student_id field missing");
        }
        auto student_id = msg_map["student_id"].as<QString>();

        if (msg_map.find("data_type") == msg_map.end()) {
            throw std::runtime_error("data_type field missing");
        }
        auto data_type = msg_map["data_type"].as<QString>();

        if (msg_map.find("data") == msg_map.end()) {
            throw std::runtime_error("data field missing");
        }
        auto data_obj = msg_map["data"];

        if (data_type == "wristband") {
            auto wristband_data = data_obj.as<WristbandPacket>();
            emit wristbandReceived(wristband_data, student_id);
        }
        else if (data_type == "eeg") {
            auto eeg_data = data_obj.as<EEGSensorData>();
            emit eegReceived(eeg_data, student_id);
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
