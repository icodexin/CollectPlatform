#ifndef MINMAXQUEUE_H
#define MINMAXQUEUE_H

#include <deque>
#include <QObject>
#include <qqmlintegration.h>

class MinMaxQueue : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)
    QML_ELEMENT

public:
    explicit MinMaxQueue(QObject* parent = nullptr) : QObject(parent) {}

    ~MinMaxQueue() override = default;

    qreal min() const;

    qreal max() const;

    Q_INVOKABLE void append(qreal value);

    Q_INVOKABLE void remove(qreal value);

signals:
    void minChanged();

    void maxChanged();

    void minMaxChanged();

private:
    std::deque<qreal> m_MinDeque; // 维护最小值（单调递增）
    std::deque<qreal> m_MaxDeque; // 维护最大值（单调递减）
};


#endif //MINMAXQUEUE_H
