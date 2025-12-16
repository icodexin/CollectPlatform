#include "BandView.h"
#include <QVBoxLayout>
#include "components/LogBox.h"

BandView::BandView(QWidget* parent)
    : DeviceView(tr("Wristband"), ":/res/icons/wristband.svg", Bottom, parent) {
    initUI();
}

void BandView::log(const LogMessage::Level level, const QString& message) const {
    ui_logBox->log(level, message);
}

void BandView::onConnected() {
    log(LogMessage::SUCCESS, tr("Wristband service started successfully."));
    DeviceView::onConnected();
}

void BandView::onDisconnected() {
    log(LogMessage::WARN, tr("Wristband service stopped."));
    DeviceView::onDisconnected();
}

void BandView::onErrorOccurred(const QVariantMap& error) const {
    if (error.contains("client_id")) {
        QString id = error.value("client_id").toString();
        QString msg = error.value("msg").toString();
        log(LogMessage::INFO, tr("Client %1 error: %2").arg(id, msg));
    } else if (error.contains("msg")) {
        const QString msg = error.value("msg").toString();
        log(LogMessage::ERROR, tr("Wristband service error: %1").arg(msg));
    }
}

void BandView::onClientConnected(const QString& id) const {
    log(LogMessage::INFO, tr("Client %1 connected.").arg(id));
}

void BandView::onClientDisconnected(const QString& id) const {
    log(LogMessage::INFO, tr("Client %1 disconnected.").arg(id));
}

void BandView::onDataReceived(const QString& id, const qsizetype length) const {
    log(LogMessage::INFO, tr("Received data from client %1, length: %2.").arg(id).arg(length));
}

void BandView::initUI() {
    setIndicatorHint(tr("Stopped"), tr("Starting"), tr("Running"), tr("Stopping"));
    ui_logBox = new LogBox(this);
    const auto layout = new QVBoxLayout;
    layout->addWidget(ui_logBox);
    setContentLayout(layout);
}
