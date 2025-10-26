#ifndef LOGBOX_H
#define LOGBOX_H

#include <QPlainTextEdit>
#include "services/log.h"

class QThread;
class LogBuffer;

class LoggerWorker final : public QObject {
    Q_OBJECT
public:
    explicit LoggerWorker(QObject* parent = nullptr);

signals:
    void messagesReady(const QStringList& messages);

public slots:
    void appendMessage(const LogMessage& message) const;

private slots:
    void processBuffer();

private:
    LogBuffer* m_buffer;
    QTimer* m_timer;
};

class LogBox final: public QPlainTextEdit {
    Q_OBJECT

public:
    explicit LogBox(QWidget* parent = nullptr, unsigned int maxLines = 500);
    ~LogBox() override;
    void log(LogMessage::Level level, const QString& message);

signals:
    void writeRequest(const LogMessage&);

public slots:
    void writeMessages(const QStringList& messages);

private:
    unsigned int m_maxLines;
    QThread* m_thread;
    LoggerWorker* m_worker;
};


#endif //LOGBOX_H