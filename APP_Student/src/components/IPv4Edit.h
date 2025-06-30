#ifndef IPV4EDIT_H
#define IPV4EDIT_H

#include "BaseWidget.h"
#include <QLineEdit>
#include <QHostAddress>
#include <QStyleOption>
#include <QPainter>

class IPv4Edit : public BaseWidget {
    Q_OBJECT
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)

public:
    explicit IPv4Edit(QWidget* parent = nullptr);

    void clear() const;

    QString address() const;

    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void addressChanged(const QString& address);

public slots:
    void setAddress(const QString& address);
    void onTextChanged(const QString& text);

private:
    QLineEdit* ui_lineEdit0;
    QLineEdit* ui_lineEdit1;
    QLineEdit* ui_lineEdit2;
    QLineEdit* ui_lineEdit3;

    QString m_address;

    void initUI();

    void initConnection();

    void closeConnection();
};


#endif //IPV4EDIT_H
