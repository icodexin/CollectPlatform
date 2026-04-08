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
class QLabel;
class QPushButton;
class QSplitter;
class SettingsDialog;
class NavigationView;
class UserInfoDialog;
class VideoPushService;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    void setQuitOnClose(bool enabled);

signals:
    void windowClosed(bool shouldQuitApp);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void tryStartMessagePushService();
    void initUI();
    void initServices();
    void initConnection();
    void toggleSidebar();
    void updateNetworkStatus(bool visible, const QString& text);
    void showSettingsDialog();
    void showUserInfoDialog();

private:
    QSplitter* ui_mainSplitter = nullptr;
    NavigationView* ui_navigationView = nullptr;
    EEGView* ui_eegView = nullptr;
    BandView* ui_bandView = nullptr;
    CameraView* ui_cameraView = nullptr;
    LogBox* ui_logBox = nullptr;
    QLabel* ui_networkStatusLabel = nullptr;
    QPushButton* ui_sidebarToggleButton = nullptr;
    EEGRecvService* m_eegRecvService = nullptr;
    BandServer* m_bandServer = nullptr;
    CameraService* m_cameraService = nullptr;
    MqttPublishService* m_mqttPubService = nullptr;
    DataPipe* m_dataPipe = nullptr;
    VideoPushService* m_videoPushService = nullptr;
    QPointer<SettingsDialog> m_settingsDialog;
    QPointer<UserInfoDialog> m_userDialog;
    int m_lastSidebarWidth = 320;
    bool m_sidebarCollapsed = false;
    bool m_quitOnClose = true;
};

#endif //MAINWINDOW_H
