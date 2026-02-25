#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QtMultimedia/QVideoFrame>
#include "DeviceView.h"

class QLabel;

class ViewFinder final : public QWidget {
    Q_OBJECT

public:
    explicit ViewFinder(QWidget* parent = nullptr);
    void setPlaying(bool playing);
    void setCurrentFrame(const QVideoFrame& frame);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* ui_hint = nullptr;
    QVideoFrame::PaintOptions m_framePaintOptions{};
    QVideoFrame m_currentFrame{};
    bool m_playing = false;
};

class CameraView final : public DeviceView {
    Q_OBJECT

public:
    explicit CameraView(QWidget* parent = nullptr);

signals:
    void playingChanged(bool playing);

public slots:
    void setPlaying(bool playing);
    void setFrame(const QVideoFrame& frame);
    void setStreamStats(double fps, double kbps);
    void clearStreamStats();

private:
    void initUI();
    void initConnection();

private:
    ViewFinder* ui_viewFinder;
    QLabel* ui_fpsLabel = nullptr;
    bool m_playing = false;
};
#endif //CAMERAVIEW_H
