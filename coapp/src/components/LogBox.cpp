#include "LogBox.h"
#include <QThread>
#include <QMutexLocker>
#include <QQueue>
#include <QTimer>
#include <QScrollBar>

class LogBuffer {
public:
    void push(const QString& msg) {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(msg);
    }

    QStringList takeAll() {
        QMutexLocker locker(&m_mutex);
        QStringList batch = m_queue;
        m_queue.clear();
        return batch;
    }

private:
    QQueue<QString> m_queue;
    QMutex m_mutex;
};

LoggerWorker::LoggerWorker(QObject* parent)
    : QObject(parent) {
    m_buffer = new LogBuffer();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LoggerWorker::processBuffer);
    m_timer->start(100);
}

void LoggerWorker::appendMessage(const LogMessage& message) const {
    m_buffer->push(message.toHtmlText());
}

void LoggerWorker::processBuffer() {
    const auto messages = m_buffer->takeAll();
    if (!messages.isEmpty()) {
        emit messagesReady(messages);
    }
}

LogBox::LogBox(QWidget* parent, const unsigned int maxLines)
    : QPlainTextEdit(parent), m_maxLines(maxLines) {
    setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    m_thread = new QThread(this);
    m_worker = new LoggerWorker(nullptr);
    m_worker->moveToThread(m_thread);
    connect(this, &LogBox::writeRequest, m_worker, &LoggerWorker::appendMessage, Qt::QueuedConnection);
    connect(m_worker, &LoggerWorker::messagesReady, this, &LogBox::writeMessages);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_thread->start();
}

LogBox::~LogBox() {
    m_thread->quit();
    m_thread->wait();
}

void LogBox::log(const LogMessage::Level level, const QString& message) {
    const LogMessage msg{level, QDateTime::currentDateTime(), message};
    emit writeRequest(msg);
}

void LogBox::writeMessages(const QStringList& messages) {
    this->blockSignals(true);
    for (const auto& msg : messages)
        appendHtml(msg);

    if (const int excess = blockCount() - m_maxLines; excess > 0) {
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, excess);
        cursor.removeSelectedText();
        cursor.deleteChar(); // 删除换行符
    }

    moveCursor(QTextCursor::End);
    ensureCursorVisible();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    this->blockSignals(false);
}