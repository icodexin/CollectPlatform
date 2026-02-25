#ifndef STREAMADVANCEDSETTINGDIALOG_H
#define STREAMADVANCEDSETTINGDIALOG_H

#include <QtWidgets/QDialog>

#include "services/FFmpegHelper.h"
#include "services/VideoPushService.h"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QSpinBox;
class QStackedWidget;

class StreamAdvancedSettingDialog final : public QDialog {
    Q_OBJECT

public:
    explicit StreamAdvancedSettingDialog(QWidget* parent = nullptr);

private slots:
    void onEncoderIndexChanged(int index);

private:
    void initUI();
    void initConnections();
    void populateEncoders();

private:
    // ---- 通用设置控件 ----
    QSpinBox*       m_bitrateSpinBox    = nullptr;
    QComboBox*      m_profileCombo      = nullptr;
    QSpinBox*       m_gopSpinBox        = nullptr;
    QSpinBox*       m_bFramesSpinBox    = nullptr;
    QComboBox*      m_encoderCombo      = nullptr;

    // ---- 编码器专有选项（堆叠页） ----
    QStackedWidget* m_optStack          = nullptr;

    // libx264 页
    QComboBox*      m_x264PresetCombo     = nullptr;
    QComboBox*      m_x264TuneCombo       = nullptr;

    // openh264 页
    QLabel*         m_o264InfoLabel    = nullptr;

    // VideoToolbox 页
    QCheckBox*      m_vtRealtimeCheck       = nullptr;
    QCheckBox*      m_vtFallbackCheck       = nullptr;
    QCheckBox*      m_vtPriorSpeedCheck     = nullptr;
    QCheckBox*      m_vtSpatialAQCheck      = nullptr;

    // QSV 页
    QComboBox*      m_qsvPresetCombo        = nullptr;
    QSpinBox*       m_qsvAsyncDepthSpinBox  = nullptr;

    // NVENC 页
    QComboBox*      m_nvPresetCombo         = nullptr;
    QComboBox*      m_nvTuneCombo           = nullptr;
    QSpinBox*       m_nvLookaheadSpinBox    = nullptr;

    QDialogButtonBox* m_buttonBox           = nullptr;

    QList<FFEncoderInfo> m_encoders;
};

#endif // STREAMADVANCEDSETTINGDIALOG_H
