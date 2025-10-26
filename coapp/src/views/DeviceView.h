#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include "components/BarCard.h"

class RecordingIndicator;

class DeviceView : public BarCard {
public:
    explicit DeviceView(QWidget* parent);
    explicit DeviceView(const QString& title = "", const QString& iconSource = "",
                         BarPosition barPos = Bottom, QWidget* parent = nullptr);

    void setIndicatorHint(const QString& inactive, const QString& starting, const QString& running,
                          const QString& stopping);
    void addBarWidget(QWidget* widget);

public slots:
    virtual void onConnecting();
    virtual void onConnected();
    virtual void onDisconnecting();
    virtual void onDisconnected();
    virtual void onConnectionStatusChanged(bool connected);

protected:
    RecordingIndicator* ui_indicator;
};


#endif //DEVICEVIEW_H
