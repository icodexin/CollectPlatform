//
// Created by Lenovo on 25-4-30.
//

#include "BarCard.h"
#include <QVBoxLayout>
#include <QLabel>

BarCard::BarCard(const QString& title, const QString& iconSource, const BarPosition barPos, QWidget* parent)
    : BaseWidget(parent), m_title(title), m_iconSource(iconSource) {
    initUI(barPos);
}

void BarCard::initUI(const BarPosition barPos) {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    ui_content = new BaseWidget(this, "content");
    ui_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initBar();

    if (barPos == BarPosition::Bottom) {
        setProperty("bar-position", "bottom");
        layout->addWidget(ui_content);
        layout->addWidget(ui_bar);
    } else {
        setProperty("bar-position", "top");
        layout->addWidget(ui_bar);
        layout->addWidget(ui_content);
    }
}

void BarCard::initBar() {
    ui_bar = new BaseWidget(this, "bar");
    ui_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    ui_barLayout = new QHBoxLayout();
    ui_bar->setLayout(ui_barLayout);

    auto iconLabel = new QLabel(this);
    auto iconSize = fontMetrics().height();
    iconLabel->setPixmap(QIcon(m_iconSource).pixmap(iconSize, iconSize));

    auto titleLabel = new QLabel(this);
    titleLabel->setText(m_title);
    titleLabel->setStyleSheet("font-weight: bold");

    ui_barLayout->addWidget(iconLabel);
    ui_barLayout->addWidget(titleLabel);
    ui_barLayout->addStretch(1);
}
