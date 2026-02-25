#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BandServer;
class BandView;
class CameraService;
class CameraView;
class DataPipe;
class EEGReceiver;
class EEGView;
class MqttPublishService;
class MqttPublisher;
class LogBox;
class SettingView;
class VideoPushService;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void initUI();
    void initServices();
    void initConnection();

private:
    SettingView* ui_settingView = nullptr;
    EEGView* ui_eegView = nullptr;
    BandView* ui_bandView = nullptr;
    CameraView* ui_cameraView = nullptr;
    LogBox* ui_logBox = nullptr;
    EEGReceiver* m_eegReceiver = nullptr;
    QThread* m_eegThread = nullptr;
    BandServer* m_bandServer = nullptr;
    CameraService* m_cameraService = nullptr;
    MqttPublishService* m_mqttPubService = nullptr;
    DataPipe* m_dataPipe = nullptr;
    VideoPushService* m_videoPushService = nullptr;
};

#endif //MAINWINDOW_H
