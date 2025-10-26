#include "BandServer.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <model/serialize.h>

BandClientHandler::BandClientHandler(qintptr socketDescriptor, QObject* parent)
    : QObject(parent), m_socketDescriptor(socketDescriptor) {
    m_buffer.setBuffer(&m_serialized);
    m_buffer.open(QIODevice::WriteOnly);
}

void BandClientHandler::start() {
    m_socket = new QTcpSocket(this);
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor" << m_socketDescriptor << ":" << m_socket->errorString();
        emit finished();
        return;
    }
    m_id = m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());
    emit clientConnected(m_id);

    connect(m_socket, &QTcpSocket::readyRead, this, &BandClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [=] {
        emit clientDisconnected(m_id);
        emit finished();
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [=](const QAbstractSocket::SocketError error) {
        emit errorOccurred(m_id, error, m_socket->errorString());
        emit finished();
    });
}

void BandClientHandler::stop() {
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

    qWarning() << "Stopping BandClientHandler for" << m_id;
    emit finished();
}

void BandClientHandler::onReadyRead() {
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

        auto data = WristbandPacket::fromJsonObject(doc.object());

        m_serialized.clear();
        m_buffer.seek(0);
        msgpack::pack(m_buffer, data);

        emit dataReceived(m_id, m_serialized);
    }
}

BandServer::BandServer(QObject* parent) : QTcpServer(parent) {
}

BandServer::~BandServer() {
    stop();
}

bool BandServer::start(const quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        emit errorOccurred({{"msg", errorString()}});
        return false;
    }
    setRunning(true);
    emit started();
    return true;
}

void BandServer::stop() {
    if (!m_running) return;
    setRunning(false);
    emit requestStop();
    close();
    emit stopped();
}

void BandServer::setRunning(const bool running) {
    if (m_running != running) {
        m_running = running;
        emit runningChanged(m_running);
    }
}

void BandServer::onClientErrorOccurred(const QString& id, QAbstractSocket::SocketError error,
                                       const QString& errorString) {
    emit errorOccurred({{"client_id", id}, {"error_type", error}, {"msg", errorString}});
}

void BandServer::incomingConnection(const qintptr socketDescriptor) {
    if (!m_running) return;

    auto* handler = new BandClientHandler(socketDescriptor, nullptr);
    const auto thread = new QThread(this);
    handler->moveToThread(thread);

    connect(handler, &BandClientHandler::clientConnected, this, &BandServer::clientConnected);
    connect(handler, &BandClientHandler::clientDisconnected, this, &BandServer::clientDisconnected);
    connect(handler, &BandClientHandler::dataReceived, this, &BandServer::dataReceived);
    connect(handler, &BandClientHandler::errorOccurred, this, &BandServer::onClientErrorOccurred);

    connect(thread, &QThread::started, handler, &BandClientHandler::start);
    connect(this, &BandServer::requestStop, handler, &BandClientHandler::stop);
    connect(handler, &BandClientHandler::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, handler, &BandClientHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}
