#include "WristbandHandler.h"

#include <QEventLoop>
#include <qjsondocument.h>
#include <QJsonObject>
#include <QTcpSocket>

#include "model/serialize.h"

WristbandHandler::WristbandHandler(const qintptr socketDescriptor, QObject* parent)
    : QObject(parent), m_socketDescriptor(socketDescriptor) {
    m_buffer.setBuffer(&m_serialized);
    m_buffer.open(QIODevice::WriteOnly);
}

void WristbandHandler::start() {
    m_socket = new QTcpSocket(this);
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Failed to set socket descriptor";
        emit finished();
        return;
    }
    m_id = m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());
    emit clientConnected(m_id);

    connect(m_socket, &QTcpSocket::readyRead, this, &WristbandHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [=] {
        emit clientDisconnected(m_id);
        emit finished();
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [=](const QAbstractSocket::SocketError error) {
        emit errorOccurred(m_id, error, m_socket->errorString());
        emit finished();
    });
}

void WristbandHandler::stop() {
    if (!m_socket) return;

    switch (m_socket->state()) {
        case QAbstractSocket::ConnectedState:
            m_socket->disconnectFromHost();
            break;
        case QAbstractSocket::ConnectingState:
            m_socket->abort();
            break;
        default:
            break;
    }

    qDebug() << "Stopping WristbandHandler for" << m_id;

    emit finished();
}

void WristbandHandler::onReadyRead() {
    if (m_socket->bytesAvailable() <= 0)
        return;

    const QByteArray raw = m_socket->readAll();
    int startPos = 0, pos;

    while ((pos = raw.indexOf('\n', startPos)) != -1) {
        QByteArray line = raw.mid(startPos, pos - startPos).trimmed();
        startPos = pos + 1;

        if (line.isEmpty()) continue;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        if (error.error != QJsonParseError::NoError) {
            continue;
        }

        auto data = WristbandData::fromJsonObject(doc.object());
        m_serialized.clear();
        m_buffer.seek(0);
        msgpack::pack(m_buffer, data);

        emit dataReceived(m_id, m_serialized);
    }
}
