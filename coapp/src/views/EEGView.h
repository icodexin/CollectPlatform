#ifndef EEGVIEW_H
#define EEGVIEW_H

#include <QAbstractSocket>
#include "DeviceView.h"
#include "services/log.h"

struct EEGEventData;
class LogBox;

class EEGView final : public DeviceView {
public:
    explicit EEGView(QWidget* parent = nullptr);

public slots:
    void onDataFetched(const QByteArray& serialized, size_t rawSize, size_t packetCount) const;
    void onEventFetched(const EEGEventData& event) const;
    void onErrorOccurred(QAbstractSocket::SocketError error, const QString& errorString) const;
    void log(LogMessage::Level level, const QString& message) const;

private:
    void initUI();

private:
    LogBox* ui_logBox;
};

#endif //EEGVIEW_H