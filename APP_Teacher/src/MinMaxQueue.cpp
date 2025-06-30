#include "MinMaxQueue.h"

qreal MinMaxQueue::min() const {
    return m_MinDeque.empty() ? 0 : m_MinDeque.front();
}

qreal MinMaxQueue::max() const {
    return m_MaxDeque.empty() ? 0 : m_MaxDeque.front();
}

void MinMaxQueue::append(const qreal value) {
    const qreal oldMin = min();
    const qreal oldMax = max();

    // 维护最小值队列：移除尾部所有比新元素大的元素
    while (!m_MinDeque.empty() && m_MinDeque.back() > value) {
        m_MinDeque.pop_back();
    }
    m_MinDeque.push_back(value);

    // 维护最大值队列：移除尾部所有比新元素小的元素
    while (!m_MaxDeque.empty() && m_MaxDeque.back() < value) {
        m_MaxDeque.pop_back();
    }
    m_MaxDeque.push_back(value);

    if (min() != oldMin)
        emit minChanged();
    if (max() != oldMax)
        emit maxChanged();
    if (min() != oldMin || max() != oldMax)
        emit minMaxChanged();
}

void MinMaxQueue::remove(const qreal value) {
    const qreal oldMin = min();
    const qreal oldMax = max();

    // 更新最大值队列
    if (!m_MaxDeque.empty() && m_MaxDeque.front() == value) {
        m_MaxDeque.pop_front();
        emit maxChanged();
    }

    // 更新最小值队列
    if (!m_MinDeque.empty() && m_MinDeque.front() == value) {
        m_MinDeque.pop_front();
        emit minChanged();
    }

    if (min() != oldMin || max() != oldMax)
        emit minMaxChanged();
}
