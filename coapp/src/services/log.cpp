#include "log.h"

namespace {
    struct Log2StringParma {
        QString color;
        QString backgroundColor;
        QString fontWeight;
        QString levelString;
    };

    const QMap<LogMessage::Level, Log2StringParma> logParams = {
        {LogMessage::DEBUG, {"darkgray", "transparent", "normal", "Debug"}},
        {LogMessage::INFO, {"black", "transparent", "normal", "Info"}},
        {LogMessage::SUCCESS, {"green", "transparent", "normal", "Success"}},
        {LogMessage::WARN, {"orange", "transparent", "normal", "Warn"}},
        {LogMessage::ERROR, {"red", "transparent", "bold", "Error"}},
        {LogMessage::FATAL, {"white", "#8B0000", "bold", "Fatal"}}
    };
}

LogMessage::LogMessage(const Level level, const QDateTime& timestamp, const QString& content)
    : level(level), timestamp(timestamp), content(content) {
}

LogMessage::LogMessage(const Level level, const QString& content)
    : LogMessage(level, QDateTime::currentDateTime(), content) {
}

QString LogMessage::toHtmlText() const {
    return QString("<p>[%1] <span style=\"color:%2; background-color:%3; font-weight:%4;\">[%5]: %6</span></p>")
            .arg(timestamp.toString("yyyy-MM-dd hh:mm:ss"),
                 logParams[level].color,
                 logParams[level].backgroundColor,
                 logParams[level].fontWeight,
                 logParams[level].levelString,
                 content);
}
