#include "WristbandClientHandler.h"

#include <QTcpSocket>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonDocument>
#include "WristbandData.h"

WristbandClientHandler::WristbandClientHandler(const qintptr socketDescriptor, QObject* parent)
    : QObject(parent), m_socketDescriptor(socketDescriptor) {
    // 任务完成后自动删除
    setAutoDelete(true);
}

void WristbandClientHandler::run() {
    m_socket = new QTcpSocket();

    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        m_socket->deleteLater();
        return;
    }

    // 在一开始就获取ID, 以防在连接断开后获取不到
    m_id = m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());

    // connect(this, &WristbandClientHandler::disconnectRequested,
    //         this, &WristbandClientHandler::disconnectClient);

    emit clientConnected(m_id);

    QEventLoop loop;

    // Socket连接断开
    connect(m_socket, &QTcpSocket::disconnected, &loop, &QEventLoop::quit);

    // Socket发生错误
    connect(m_socket, &QTcpSocket::errorOccurred, [&](QAbstractSocket::SocketError) {
        emit errorOccurred(m_id, m_socket->errorString());
        loop.quit();
    });

    // 接收数据逻辑
    connect(m_socket, &QTcpSocket::readyRead, [&]() {
        const QByteArray data = m_socket->readAll();
        this->parseData(data);
    });

    // 主动请求断开连接
    connect(this, &WristbandClientHandler::requestDisconnect, [&]() {
        disconnectClient();
        loop.quit();
    });

    loop.exec();             // 阻塞等待事件
    disconnectClient();      // 断开连接
    m_socket->deleteLater(); // 删除Socket对象
}

void WristbandClientHandler::disconnectClient() {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        emit clientDisconnected(m_id);
    }
}

void WristbandClientHandler::parseData(const QByteArray& data) {
    int startPos = 0, pos;

    while ((pos = data.indexOf('\n', startPos)) != -1) {
        QByteArray line = data.mid(startPos, pos - startPos).trimmed();
        startPos = pos + 1;

        if (line.isEmpty()) continue;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        if (error.error != QJsonParseError::NoError) {
            continue;
        }
        emit dataReceived(m_id, WristbandData::fromJsonObject(doc.object()));
    }
}
