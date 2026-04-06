#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

class BandServer;
class BandView;
class CameraService;
class CameraView;
class DataPipe;
class EEGRecvService;
class EEGView;
class MqttPublishService;
class MqttPublisher;
class LogBox;
class QDialog;
class QPushButton;
class QSplitter;
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
    void toggleSidebar();
    void updateSidebarToggleButton() const;
    void showPlaceholderDialog(QPointer<QDialog>& dialog, const QString& title);

private:
    QSplitter* ui_mainSplitter = nullptr;
    SettingView* ui_settingView = nullptr;
    EEGView* ui_eegView = nullptr;
    BandView* ui_bandView = nullptr;
    CameraView* ui_cameraView = nullptr;
    LogBox* ui_logBox = nullptr;
    QPushButton* ui_sidebarToggleButton = nullptr;
    EEGRecvService* m_eegRecvService = nullptr;
    BandServer* m_bandServer = nullptr;
    CameraService* m_cameraService = nullptr;
    MqttPublishService* m_mqttPubService = nullptr;
    DataPipe* m_dataPipe = nullptr;
    VideoPushService* m_videoPushService = nullptr;
    QPointer<QDialog> m_settingsDialog;
    QPointer<QDialog> m_userDialog;
    int m_lastSidebarWidth = 320;
    bool m_sidebarCollapsed = false;
};

#endif //MAINWINDOW_H
