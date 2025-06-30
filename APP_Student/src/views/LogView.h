//
// Created by Lenovo on 25-6-25.
//

#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "components/BarCard.h"
#include "components/LogEdit.h"

class LogView final : public BarCard {
public:
    LogView(const QString& title, const QString& iconSource, QWidget* parent = nullptr);

public slots:
    void log(const QString& msg, LogEdit::LogLevel level = LogEdit::Info) const;

    void setUIConnecting(const QString& hint = "") const;
    void setUIConnected(const QString& hint = "") const;
    void setUIDisconnecting(const QString& hint = "") const;
    void setUIDisconnected() const;

private:
    class StatusIndicator* ui_indicator;
    LogEdit* ui_logEdit;
    class QPushButton* ui_clearBtn;

    void initUI(const QString& title, const QString& iconSource);
    void initConnection();
};


#endif //LOGVIEW_H
