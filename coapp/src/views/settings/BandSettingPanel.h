#ifndef BANDSETTINGPANEL_H
#define BANDSETTINGPANEL_H

#include <QGroupBox>

class QSpinBox;
class QPushButton;

class BandSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit BandSettingPanel(QWidget* parent = nullptr);
    int port() const;

signals:
    void requestStartService(int port);
    void requestStopService();

public slots:
    void handleServiceStarted() const;
    void handleServiceStopped() const;
    void handleServiceRunningChanged(bool running) const;

private slots:
    void onListenBtnClicked();

private:
    QSpinBox* m_portSpinBox = nullptr;
    QPushButton* m_listenBtn = nullptr;
};

#endif // BANDSETTINGPANEL_H
