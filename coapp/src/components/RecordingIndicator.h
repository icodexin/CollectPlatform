#ifndef RECORDINGINDICATOR_H
#define RECORDINGINDICATOR_H

#include <QWidget>
#include <QTime>

class QLabel;
class RecordingIconWidget;

class RecordingIndicator final : public QWidget {
    Q_OBJECT

public:
    enum Status: qint8 {
        Inactive, Starting, Recording, Stopping
    };

    explicit RecordingIndicator(QWidget* parent = nullptr, bool recordTime = true);

    void start(bool immediate = false);
    void stop(bool immediate = false);
    void setHint(const QString& inactive, const QString& starting, const QString& recording, const QString& stopping);
    void setInactiveHint(const QString&);
    void setStartingHint(const QString&);
    void setRecordingHint(const QString&);
    void setStoppingHint(const QString&);
    void setRecordTime(bool);

public slots:
    void onStarted(bool success = true);
    void onStopped();

private:
    void initUI();
    void setStatus(Status);
    void handleTimeout() const;

private:
    RecordingIconWidget* ui_iconWidget = nullptr;
    QLabel* ui_timeLabel = nullptr;
    QLabel* ui_hintLabel = nullptr;
    QTimer* m_timer = nullptr;
    QString m_inactiveHint;
    QString m_startingHint;
    QString m_recordingHint;
    QString m_stoppingHint;
    QTime m_startTime;
    bool m_recordTime = false;
    Status m_status = Inactive;
};

#endif //RECORDINGINDICATOR_H
