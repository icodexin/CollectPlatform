#include "BaseWidget.h"
#include <QStyleOption>
#include <QPainter>

BaseWidget::BaseWidget(QWidget* parent, const QString& objectName)
    : QWidget(parent) {
    setObjectName(objectName);
}

void BaseWidget::drawQssStyle(QPainter* painter) const {
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, this);
}

void BaseWidget::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    drawQssStyle(&p);
}
