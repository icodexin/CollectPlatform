#include "EEGView.h"
#include <QVBoxLayout>
#include "components/LogBox.h"
#include "model/EEGData.h"

EEGView::EEGView(QWidget* parent)
    : DeviceView(tr("EEG"), ":/res/icons/head-snowflake-outline.svg", Bottom, parent) {
    initUI();
}

void EEGView::onDataFetched(const size_t rawSize, const size_t packetCount) const {
    log(LogMessage::INFO, tr("Received %1 bytes of data, %2 EEG Data packets").arg(rawSize).arg(packetCount));
}

void EEGView::onEventFetched(const EEGEventData& event) const {
    QString logMsg;
    switch (event.code) {
        case 1:
            logMsg = tr("Version: %1").arg(event.message.value("version").toString());
            break;
        case 9:
            logMsg = tr("Original Sensor Map: %1").arg(event.message.value("sensorMap").toString());
            break;
        case 10:
            logMsg = tr("Mains Frequency: %1 Hz, Sample Frequency: %2 Hz")
                    .arg(event.message.value("mainsFreq").toInt())
                    .arg(event.message.value("sampleFreq").toInt());
            break;
        case 2:
            logMsg = tr("Start receiving EEG from DSI-Streamer.");
            break;
        case 3:
            logMsg = tr("Stop receiving EEG from DSI-Streamer.");
            break;
        default:
            break;
    }
    if (!logMsg.isEmpty())
        log(LogMessage::INFO, logMsg);
}

void EEGView::onErrorOccurred(const QAbstractSocket::SocketError error, const QString& errorString) const {
    QString msg;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            msg = tr("Connection refused. There may be no DSI-Streamer running on the specified IP and port.");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            msg = tr("The remote host closed the connection.");
            break;
        case QAbstractSocket::HostNotFoundError:
            msg = tr("The host was not found. Please check the IP address.");
            break;
        case QAbstractSocket::SocketAccessError:
            msg = tr("The socket operation failed due to insufficient permissions.");
            break;
        case QAbstractSocket::SocketTimeoutError:
            msg = tr("The operation timed out.");
            break;
        case QAbstractSocket::NetworkError:
            msg = tr("A network error occurred accidentally.");
            break;
        default:
            msg = errorString;
            break;
    }
    log(LogMessage::ERROR, msg);
}

void EEGView::log(const LogMessage::Level level, const QString& message) const {
    ui_logBox->log(level, message);
}

void EEGView::initUI() {
    setIndicatorHint(tr("Disconnected"), tr("Connecting"), tr("Connected"), tr("Disconnecting"));
    ui_logBox = new LogBox(this);
    const auto layout = new QVBoxLayout;
    layout->addWidget(ui_logBox);
    setContentLayout(layout);
}
