#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

class InfoPanel final: public QWidget {
public:
    explicit InfoPanel(QWidget* parent = nullptr);

private slots:
    void updateIpAddress();

private:
    QVBoxLayout* ui_ipListLayout;
    QPushButton* ui_refreshBtn;
};

#endif //INFOPANEL_H