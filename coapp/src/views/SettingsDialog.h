#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtWidgets/QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QSpinBox;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onSaveClicked();

private:
    void initUI();
    void setStatusMessage(const QString& message, bool error = false);

private:
    QLineEdit* m_serverAddressEdit = nullptr;
    QSpinBox* m_streamPortSpinBox = nullptr;
    QLabel* m_statusLabel = nullptr;
    QComboBox* m_themeComboBox = nullptr;
    QPushButton* m_saveButton = nullptr;
};

#endif //SETTINGSDIALOG_H
