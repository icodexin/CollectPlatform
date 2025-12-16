#ifndef CQUEUE_H
#define CQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <utility>

template <typename T>
class CQueue {
public:
    CQueue() = default;

    ~CQueue() {
        close();
    }

    CQueue(const CQueue&) = delete;
    CQueue& operator=(const CQueue&) = delete;

    bool push(T value) {
        {
            std::lock_guard lock(m_mutex);
            if (m_closed)
                return false;
            m_queue.push(std::move(value));
        }
        m_notEmpty.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock lock(m_mutex);
        m_notEmpty.wait(lock, [&] {
            return !m_queue.empty() || m_closed;
        });
        if (m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    bool tryPop(T& out) {
        std::lock_guard lock(m_mutex);
        if (m_queue.empty())
            return false;
        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    std::size_t size() const {
        std::lock_guard lock(m_mutex);
        return m_queue.size();
    }

    bool empty() const {
        std::lock_guard lock(m_mutex);
        return m_queue.empty();
    }

    void clear() {
        std::lock_guard lock(m_mutex);
        while (!m_queue.empty())
            m_queue.pop();
    }

    void close() {
        {
            std::lock_guard lock(m_mutex);
            if (m_closed)
                return;
            m_closed = true;
        }
        m_notEmpty.notify_all();
    }

    bool isClosed() const {
        std::lock_guard lock(m_mutex);
        return m_closed;
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_notEmpty;
    std::queue<T> m_queue;
    bool m_closed = false;
};

#endif //CQUEUE_H
