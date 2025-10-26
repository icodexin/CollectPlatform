#ifndef LOG_H
#define LOG_H

#include <QDateTime>
#include <QObject>

struct LogMessage {
    Q_GADGET

public:
    enum Level { DEBUG, INFO, SUCCESS, WARN, ERROR, FATAL };
    Q_ENUM(Level)

public:
    Level level{};
    QDateTime timestamp;
    QString content;

public:
    LogMessage() = default;
    LogMessage(Level level, const QDateTime& timestamp, const QString& content);
    LogMessage(Level level, const QString& content);
    QString toHtmlText() const;
};

#endif //LOG_H