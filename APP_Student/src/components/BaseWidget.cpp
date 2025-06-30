//
// Created by Lenovo on 25-4-28.
//

#include "BaseWidget.h"
#include <QStyleOption>
#include <QPainter>

BaseWidget::BaseWidget(QWidget* parent, const QString& objectName)
    : QWidget(parent) {
    setObjectName(objectName);
}

void BaseWidget::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
