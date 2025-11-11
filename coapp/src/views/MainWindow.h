#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DataPipe;
class MqttPublishService;
class MqttPublisher;
class LogBox;
class EEGReceiver;
class BandServer;
class CameraService;
class VideoWidget;
class BandView;
class EEGView;
class SettingView;

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
    VideoWidget* ui_cameraView = nullptr;
    LogBox* ui_logBox = nullptr;
    EEGReceiver* m_eegReceiver = nullptr;
    QThread* m_eegThread = nullptr;
    BandServer* m_bandServer = nullptr;
    //之前QT的相机
    // CameraService* m_cameraService = nullptr;
    MqttPublishService* m_mqttPubService = nullptr;
    DataPipe* m_dataPipe = nullptr;
};

#endif //MAINWINDOW_H
