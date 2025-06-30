#include "LogEdit.h"
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>
#include <QMutexLocker>

struct LogOutputParam {
    QString color;
    QString background;
    QString prefix;
    QString fontWeight;
};

QMap<LogEdit::LogLevel, LogOutputParam> logParams = {
    {LogEdit::Debug, {"gray", "transparent", QObject::tr("DEBUG"), "normal"}},
    {LogEdit::Info, {"black", "transparent", QObject::tr("INFO"), "normal"}},
    {LogEdit::Success, {"green", "transparent", QObject::tr("SUCCESS"), "normal"}},
    {LogEdit::Warning, {"orange", "transparent", QObject::tr("WARNING"), "normal"}},
    {LogEdit::Error, {"red", "transparent", QObject::tr("ERROR"), "bold"}},
    {LogEdit::Critical, {"white", "#8B0000", QObject::tr("CRITICAL"), "bold"}}
};

LogEdit::LogEdit(const int maxLines, QWidget* parent)
    : QTextEdit(parent), maxLines(maxLines), m_timer(new QTimer(this)) {
    setReadOnly(true);
    setPlaceholderText(tr("Logging Output Area"));
    setContextMenuPolicy(Qt::NoContextMenu);
    m_timer->setInterval(200);
    m_timer->start();
    connect(m_timer, &QTimer::timeout, this, &LogEdit::updateEdit);
}

void LogEdit::log(const QString& msg, const LogLevel level) {
    const QString formattedMsg =
            QString("<p>[%1] <span style=\"color:%2; background-color:%3; font-weight:%4;\">%5: %6</span></p>")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 logParams[level].color, logParams[level].background, logParams[level].fontWeight,
                 logParams[level].prefix, msg.toHtmlEscaped());

    QMutexLocker locker(&m_queueMutex);
    m_logQueue.enqueue(formattedMsg);
}

void LogEdit::updateEdit() {
    QMutexLocker locker(&m_queueMutex);
    if (m_logQueue.isEmpty())
        return;

    while (!m_logQueue.isEmpty()) {
        append(m_logQueue.dequeue());
    }
    // append("");
    locker.unlock();

    if (document()->blockCount() > maxLines) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, document()->blockCount() - maxLines);
        cursor.removeSelectedText();
        setTextCursor(cursor);
    }

    moveCursor(QTextCursor::End);
    ensureCursorVisible();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
