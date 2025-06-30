#ifndef LOGEDIT_H
#define LOGEDIT_H

#include <QTextEdit>
#include <QQueue>
#include <QMutex>

class LogEdit: public QTextEdit {
    Q_OBJECT
public:
    enum LogLevel {
        Debug, Info, Success, Warning, Error, Critical
    };
    Q_ENUM(LogLevel)

    explicit LogEdit(int maxLines = 500, QWidget* parent = nullptr);

    void log(const QString& msg, LogLevel level);

private slots:
    void updateEdit();

private:
    int maxLines;
    QTimer* m_timer;
    QQueue<QString> m_logQueue;
    QMutex m_queueMutex;
};



#endif //LOGEDIT_H
