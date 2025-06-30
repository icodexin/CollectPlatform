#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
// #include "views/SettingView.h"

class CameraView;
class WristbandServer;
class EEGDataReceiver;
class SettingView;
class LogView;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    SettingView* ui_settingView;
    LogView* ui_eegView;
    LogView* ui_bandView;
    CameraView* ui_cameraView;

    EEGDataReceiver* m_eegReceiver;
    QThread m_eegThread;

    WristbandServer* m_bandServer;

    void initUI();
    void initCamera();
    void initConnection();
};


#endif //MAINWINDOW_H
