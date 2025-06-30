#include "WristbandServer.h"

#include <QEventLoop>
#include <qjsondocument.h>
#include <QTcpSocket>
#include <QThreadPool>

#include "WristbandHandler.h"
#include "model/serialize.h"

WristbandServer::WristbandServer(QObject* parent)
    : QTcpServer(parent) {
}

WristbandServer::~WristbandServer() {
    stop();
}

bool WristbandServer::start(const quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        emit logFetched(errorString(), LogEdit::Error);
        return false;
    }
    m_running = true;
    emit runningChanged(m_running);
    return true;
}

void WristbandServer::stop() {
    if (!m_running) return;
    close();
    m_running = false;
    emit runningChanged(m_running);
    emit requestStop();
}

void WristbandServer::incomingConnection(const qintptr socketDescriptor) {
    if (!m_running) return;

    const auto handler = new WristbandHandler(socketDescriptor);
    QThread* thread = new QThread();
    handler->moveToThread(thread);

    connect(
        handler, &WristbandHandler::errorOccurred,
        this, [=](const QString& id, QAbstractSocket::SocketError, const QString& errorString) {
            emit errorOccurred({{"id", id}, {"msg", errorString}});
            emit logFetched(tr("%1: %2").arg(id, errorString), LogEdit::Error);
        }
    );
    connect(
        handler, &WristbandHandler::clientConnected,
        this, [=](const QString& id) {
            emit clientConnected(id);
            emit logFetched(tr("Client %1 connected.").arg(id), LogEdit::Info);
        }
    );
    connect(
        handler, &WristbandHandler::dataReceived,
        this, [=](const QString& id, const QByteArray& data) {
            emit dataReceived(id, data);
            emit logFetched(tr("Data received from %1").arg(id), LogEdit::Info);
        }
    );
    connect(
        handler, &WristbandHandler::clientDisconnected,
        this, [=](const QString& id) {
            emit clientDisconnected(id);
            emit logFetched(tr("Client %1 disconnected.").arg(id), LogEdit::Info);
        }
    );

    connect(thread, &QThread::started, handler, &WristbandHandler::start);
    connect(handler, &WristbandHandler::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, handler, &WristbandHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(this, &WristbandServer::requestStop, handler, &WristbandHandler::stop);

    thread->start();
}
