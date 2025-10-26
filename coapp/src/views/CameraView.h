#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include "DeviceView.h"

class QVideoFrame;
class ViewFinder;

class CameraView final: public DeviceView {
    Q_OBJECT

public:
    explicit CameraView(QWidget* parent = nullptr);

signals:
    void playingChanged(bool playing);

public slots:
    void setPlaying(bool playing);
    void setFrame(const QVideoFrame& frame);

private:
    void initUI();
    void initConnection();

private:
    ViewFinder* ui_viewFinder;
    bool m_playing = false;
};
#endif //CAMERAVIEW_H
