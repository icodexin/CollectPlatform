#include "InfoPanel.h"

#include <QBoxLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QNetworkInterface>
#include <QPushButton>

namespace {
    QList<QString> getLocalIpv4Address() {
        QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
        QList<QString> filtered;
        for (const QHostAddress& address : allAddresses) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback())
                filtered.append(address.toString());
        }
        return filtered;
    }
}

InfoPanel::InfoPanel(QWidget* parent) : QWidget(parent) {
    auto* ipHint = new QLabel(tr("Local IP address:"));
    ui_refreshBtn = new QPushButton;
    ui_refreshBtn->setIcon(QIcon(":/res/icons/refresh.svg"));
    ui_refreshBtn->setToolTip(tr("Refresh"));
    auto* copyrightLabel = new QLabel(tr("©️2025 Intelligent Perception Laboratory"));
    auto* appVersionLabel = new QLabel(qApp->applicationDisplayName() % " v" % qApp->applicationVersion());

    ui_ipListLayout = new QVBoxLayout;

    auto* hLayout = new QHBoxLayout;
    hLayout->addWidget(ipHint);
    hLayout->addLayout(ui_ipListLayout, 1);
    hLayout->addWidget(ui_refreshBtn);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addLayout(hLayout);
    layout->addWidget(copyrightLabel);
    layout->addWidget(appVersionLabel);

    connect(ui_refreshBtn, &QPushButton::clicked, this, &InfoPanel::updateIpAddress);
    updateIpAddress();
}

void InfoPanel::updateIpAddress() {
    QLayoutItem* item;
    while ((item = ui_ipListLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QList<QString> ipAddresses = getLocalIpv4Address();
    if (ipAddresses.isEmpty()) {
        auto* label = new QLabel(tr("No network connection."));
        ui_ipListLayout->addWidget(label);
    } else {
        for (const QString& ip : ipAddresses) {
            auto* label = new QLabel(ip);
            ui_ipListLayout->addWidget(label);
        }
    }
}
