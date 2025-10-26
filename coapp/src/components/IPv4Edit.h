#ifndef IPV4EDIT_H
#define IPV4EDIT_H

#include "BaseWidget.h"

class QLineEdit;

class IPv4Edit final : public BaseWidget {
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

private slots:
    void onSubTextChanged(const QString& text);

private:
    void initUI();
    void initConnection();
    void closeConnection();

private:
    QLineEdit* ui_lineEdit0{nullptr};
    QLineEdit* ui_lineEdit1{nullptr};
    QLineEdit* ui_lineEdit2{nullptr};
    QLineEdit* ui_lineEdit3{nullptr};
    QString m_address;
};


#endif //IPV4EDIT_H
