#include "DeviceView.h"
#include <QPointer>
#include <QHBoxLayout>
#include "components/RecordingIndicator.h"

DeviceView::DeviceView(QWidget* parent)
    : DeviceView("", "", Bottom, parent) {
}

DeviceView::DeviceView(const QString& title, const QString& iconSource, const BarPosition barPos, QWidget* parent)
    : BarCard(title, iconSource, barPos, parent) {
    ui_indicator = new RecordingIndicator(this, false);
    ui_indicator->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    bar()->addWidgetInRight(ui_indicator);
}

void DeviceView::setIndicatorHint(const QString& inactive, const QString& starting, const QString& running,
    const QString& stopping) {
    ui_indicator->setHint(inactive, starting, running, stopping);
}

void DeviceView::addBarWidget(QWidget* widget) {
    bar()->insertWidgetInRight(bar()->rightLayout()->count() - 1, widget);
}

void DeviceView::onConnecting() {
    ui_indicator->start();
}

void DeviceView::onConnected() {
    ui_indicator->onStarted();
}

void DeviceView::onDisconnecting() {
    ui_indicator->stop();
}

void DeviceView::onDisconnected() {
    ui_indicator->onStopped();
}

void DeviceView::onConnectionStatusChanged(const bool connected) {
    if (connected) {
        onConnected();
    } else {
        onDisconnected();
    }
}
