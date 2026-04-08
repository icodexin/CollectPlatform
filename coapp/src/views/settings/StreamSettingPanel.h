#ifndef STREAMSETTINGPANEL_H
#define STREAMSETTINGPANEL_H

#include <QtWidgets/QGroupBox>
#include "services/VideoPushService.h"

class QComboBox;
class QLabel;
class QPushButton;

class StreamSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit StreamSettingPanel(QWidget* parent = nullptr);

    /** 根据当前 UI 状态构建完整推流配置 */
    PushConfig buildConfig() const;

signals:
    void requestStart(const PushConfig& config);
    void requestStop();

public slots:
    void handleStarted() const;
    void handleStopped() const;
    void handleStateChanged(PushWorkerState state) const;

private slots:
    void onStartBtnClicked();

private:
    void initUI();
    void initConnections();
    void updatePushUrlLabel();

private:
    QComboBox* m_resolutionCombo = nullptr;
    QComboBox* m_fpsCombo = nullptr;

    QPushButton* m_moreSettingsBtn = nullptr;
    QPushButton* m_startBtn = nullptr;
    QLabel* m_pushUrlLabel = nullptr;
};

#endif //STREAMSETTINGPANEL_H
