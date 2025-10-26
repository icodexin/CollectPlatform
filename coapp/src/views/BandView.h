#ifndef BANDVIEW_H
#define BANDVIEW_H

#include "DeviceView.h"
#include "services/log.h"

class LogBox;

class BandView final : public DeviceView {
public:
    explicit BandView(QWidget* parent = nullptr);

public slots:
    void log(LogMessage::Level level, const QString& message) const;
    void onConnected() override;
    void onDisconnected() override;
    void onErrorOccurred(const QVariantMap& error) const;
    void onClientConnected(const QString& id) const;
    void onClientDisconnected(const QString& id) const;
    void onDataReceived(const QString& id, const QByteArray& data) const;

private:
    void initUI();

private:
    LogBox* ui_logBox = nullptr;
};


#endif //BANDVIEW_H