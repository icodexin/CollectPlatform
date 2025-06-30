//
// Created by Lenovo on 25-5-2.
//

#ifndef STATUSINDICATOR_H
#define STATUSINDICATOR_H

#include <QWidget>
#include <QLabel>
#include <QTime>

class StatusIndicator : public QWidget {
    Q_OBJECT
public:
    StatusIndicator(const QString& runHint, const QString& haltHint, bool recordTime = false, QWidget* parent = nullptr);

    void startRecording(const QString& hint_text = QString());

    void stopRecording();

    void setRunHint(const QString& text);

private slots:
    void onTimerTimeout();

private:
    QLabel* ui_iconLabel;
    QLabel* ui_hintLabel;
    QString m_runHint;
    QString m_haltHint;
    bool m_needRecordTime;
    QTimer* m_timer;
    QTime m_time;
    int m_counter;
    QPixmap m_recordingIcon;
    QPixmap m_lightRecordingIcon;
    QPixmap m_unrecordingIcon;
    void initUI();
};



#endif //STATUSINDICATOR_H
