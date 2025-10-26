#include "BarCard.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPointer>
#include <qstyle.h>

Bar::Bar(QWidget* parent) : BaseWidget(parent, "bar") {
    ui_layout = new QHBoxLayout(this);
    ui_layout->setSpacing(8);
    ui_layout->setContentsMargins(8, 8, 8, 8);

    ui_leftLayout = new QHBoxLayout();
    ui_leftLayout->setSpacing(4);
    ui_leftLayout->setContentsMargins(0, 0, 0, 0);
    ui_layout->addLayout(ui_leftLayout);

    ui_layout->addStretch(1);

    ui_rightLayout = new QHBoxLayout();
    ui_rightLayout->setSpacing(4);
    ui_rightLayout->setContentsMargins(0, 0, 0, 0);
    ui_layout->addLayout(ui_rightLayout);
}

void Bar::addWidgetInLeft(QWidget* widget, const int stretch) {
    ui_leftLayout->addWidget(widget, stretch);
}

void Bar::addWidgetInRight(QWidget* widget, const int stretch) {
    ui_rightLayout->addWidget(widget, stretch);
}

int Bar::insertWidgetInLeft(const int index, QWidget* widget, const int stretch) {
    const int insertIndex = qBound(0, index, ui_leftLayout->count());
    ui_leftLayout->insertWidget(insertIndex, widget, stretch);
    return insertIndex;
}

int Bar::insertWidgetInRight(const int index, QWidget* widget, const int stretch) {
    const int insertIndex = qBound(0, index, ui_rightLayout->count());
    ui_rightLayout->insertWidget(insertIndex, widget, stretch);
    return insertIndex;
}

void Bar::removeWidget(QWidget* widget, const bool deleteWidget) {
    if (!widget) return;

    if (ui_leftLayout->indexOf(widget) != -1) {
        ui_leftLayout->removeWidget(widget);
    } else if (ui_rightLayout->indexOf(widget) != -1) {
        ui_rightLayout->removeWidget(widget);
    }

    if (deleteWidget) widget->deleteLater();
}

QPointer<QHBoxLayout> Bar::leftLayout() const {
    return ui_leftLayout;
}

QPointer<QHBoxLayout> Bar::rightLayout() const {
    return ui_rightLayout;
}

BarCard::BarCard(QWidget* parent) : BaseWidget(parent) {
    initUI("", "", Top);
}

BarCard::BarCard(const QString& title, const QString& iconSource, const BarPosition barPos, QWidget* parent)
    : BaseWidget(parent) {
    initUI(title, iconSource, barPos);
}

void BarCard::setContentLayout(QLayout* layout) {
    ui_content->setLayout(layout);
}

void BarCard::setTitle(const QString& title) {
    ui_titleLabel->setText(title);
}

void BarCard::setIconSource(const QString& iconSource) {
    const auto iconSize = fontMetrics().height();
    ui_iconLabel->setPixmap(QIcon(iconSource).pixmap(iconSize, iconSize));
}

void BarCard::setBarPosition(const BarPosition pos) {
    adjustUI(pos, ui_bar,  ui_content);
    ui_content->style()->polish(ui_content);
    ui_bar->style()->polish(ui_bar);
}

QPointer<Bar> BarCard::bar() const {
    return ui_bar;
}

void BarCard::initUI(const QString& title, const QString& iconSource, const BarPosition barPos) {
    ui_layout = new QVBoxLayout(this);
    ui_layout->setSpacing(0);
    ui_layout->setContentsMargins(0, 0, 0, 0);

    ui_content = new BaseWidget(this, "content");
    ui_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initBar(title, iconSource);
    adjustUI(barPos, ui_bar, ui_content);
}

void BarCard::initBar(const QString& title, const QString& iconSource) {
    ui_iconLabel = new QLabel(this);
    this->setIconSource(iconSource);

    ui_titleLabel = new QLabel(this);
    ui_titleLabel->setText(title);
    ui_titleLabel->setStyleSheet("font-weight: bold");

    ui_bar = new Bar(this);
    ui_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_bar->addWidgetInLeft(ui_iconLabel);
    ui_bar->addWidgetInLeft(ui_titleLabel);
}

void BarCard::adjustUI(const BarPosition barPos, QWidget* bar, QWidget* content) {
    if (ui_layout->indexOf(bar) != -1)
        ui_layout->removeWidget(bar);
    if (ui_layout->indexOf(content) != -1)
        ui_layout->removeWidget(content);

    if (barPos == Bottom) {
        setProperty("bar-position", "bottom");
        ui_layout->addWidget(content);
        ui_layout->addWidget(bar);
    } else {
        setProperty("bar-position", "top");
        ui_layout->addWidget(bar);
        ui_layout->addWidget(content);
    }
}
