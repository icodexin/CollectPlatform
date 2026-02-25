#include "BaseWidget.h"
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>

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
