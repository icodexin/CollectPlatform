#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>

class BaseWidget: public QWidget {
public:
    explicit BaseWidget(QWidget* parent = nullptr, const QString& objectName = QString());

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif
