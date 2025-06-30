//
// Created by Lenovo on 25-4-30.
//

#ifndef BARCARD_H
#define BARCARD_H

#include <QHBoxLayout>

#include "BaseWidget.h"

class BarCard : public BaseWidget {
    Q_OBJECT
public:
    enum BarPosition{
        Top, Bottom
    };

    BaseWidget* ui_content;
    BaseWidget* ui_bar;
    QHBoxLayout* ui_barLayout;

    QString m_title;
    QString m_iconSource;

    explicit BarCard(const QString& title = "", const QString& iconSource = "",
                     BarPosition barPos = BarPosition::Top, QWidget* parent = nullptr);

private:
    void initUI(BarPosition barPos);
    void initBar();
};



#endif //BARCARD_H
