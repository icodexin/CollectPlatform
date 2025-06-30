//
// Created by Lenovo on 25-6-25.
//

#include "LogView.h"

#include <QPushButton>

#include "components/LogEdit.h"
#include "components/StatusIndicator.h"

LogView::LogView(const QString& title, const QString& iconSource, QWidget* parent)
    : BarCard(title, iconSource, Bottom, parent) {
    initUI(title, iconSource);
    initConnection();
}

void LogView::log(const QString& msg, const LogEdit::LogLevel level) const {
    ui_logEdit->log(msg, level);
}

void LogView::setUIConnecting(const QString& hint) const {
    ui_indicator->startRecording(hint.isEmpty() ? tr("Connecting..."): hint);
}

void LogView::setUIConnected(const QString& hint) const {
    ui_indicator->setRunHint(hint.isEmpty() ? tr("Connected") : hint);
}

void LogView::setUIDisconnecting(const QString& hint) const {
    ui_indicator->setRunHint(hint.isEmpty() ? tr("Disconnecting...") : hint);
}

void LogView::setUIDisconnected() const {
    ui_indicator->stopRecording();
}

void LogView::initUI(const QString& title, const QString& iconSource) {
    /******************** 设置Bar区域 ********************/
    ui_indicator = new StatusIndicator("", tr("Stopped"), false, this);
    ui_bar->layout()->addWidget(ui_indicator);

    /******************** 设置内容区域 ********************/
    ui_logEdit = new LogEdit(20);
    ui_logEdit->setMinimumSize(300, 100);
    ui_clearBtn = new QPushButton(tr("Clear log"));

    const auto layout = new QVBoxLayout;
    layout->addWidget(ui_logEdit);
    layout->addWidget(ui_clearBtn, 0, Qt::AlignRight);
    layout->setSpacing(5);
    ui_content->setLayout(layout);
}

void LogView::initConnection() {
    connect(ui_clearBtn, &QPushButton::clicked, ui_logEdit, &LogEdit::clear);
}
