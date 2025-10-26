#ifndef BARCARD_H
#define BARCARD_H

#include "BaseWidget.h"

class QVBoxLayout;
class QLabel;
class QHBoxLayout;

class Bar: public BaseWidget {
public:
    explicit Bar(QWidget* parent = nullptr);

    void addWidgetInLeft(QWidget* widget, int stretch = 0);
    void addWidgetInRight(QWidget* widget, int stretch = 0);
    int insertWidgetInLeft(int index, QWidget* widget, int stretch = 0);
    int insertWidgetInRight(int index, QWidget* widget, int stretch = 0);
    void removeWidget(QWidget* widget, bool deleteWidget = false);
    QPointer<QHBoxLayout> leftLayout() const;
    QPointer<QHBoxLayout> rightLayout() const;

private:
    QHBoxLayout* ui_layout;
    QHBoxLayout* ui_leftLayout;
    QHBoxLayout* ui_rightLayout;
};

class BarCard : public BaseWidget {
    Q_OBJECT

public:
    enum BarPosition {
        Top, Bottom
    };

    explicit BarCard(QWidget* parent = nullptr);
    explicit BarCard(const QString& title = "", const QString& iconSource = "",
                     BarPosition barPos = Bottom, QWidget* parent = nullptr);

    void setContentLayout(QLayout* layout);
    void setTitle(const QString& title);
    void setIconSource(const QString& iconSource);
    void setBarPosition(BarPosition);
    QPointer<Bar> bar() const;

private:
    void initUI(const QString& title, const QString& iconSource, BarPosition barPos);
    void initBar(const QString& title, const QString& iconSource);
    void adjustUI(BarPosition barPos, QWidget* bar, QWidget* content);

protected:
    QVBoxLayout* ui_layout = nullptr;
    BaseWidget* ui_content = nullptr;
    Bar* ui_bar = nullptr;
    QLabel* ui_iconLabel = nullptr;
    QLabel* ui_titleLabel = nullptr;
};


#endif //BARCARD_H
