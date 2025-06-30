#include "MqttPublishView.h"

#include <QCloseEvent>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>

MqttPublishView::MqttPublishView(const QString& uid, const QString& mqttUserName, const QString& mqttPassword,
                                 const QString& host, const int port, QWidget* parent)
    : BarCard(tr("Data Publisher"), ":/res/icons/rss.svg", Bottom, parent),
      m_mqttPublisher(new MqttPublisher()) {
    initUI(host, port);
    m_mqttPublisher->moveToThread(&m_workerThread);
    QMetaObject::invokeMethod(
        m_mqttPublisher, "initialize", Qt::QueuedConnection,
        Q_ARG(QString, uid), Q_ARG(QString, mqttUserName), Q_ARG(QString, mqttPassword),
        Q_ARG(QString, host), Q_ARG(int, port)
    );
    initConnection();
    m_workerThread.start();
}

MqttPublishView::~MqttPublishView() {
    m_workerThread.quit();
    m_workerThread.wait();
}

void MqttPublishView::log(const QString& msg, const LogEdit::LogLevel level) const {
    ui_logEdit->log(msg, level);
}

void MqttPublishView::publish(const QString& topic, const QByteArray& message, const quint8 qos,
                                  const bool retained) const {
    QMetaObject::invokeMethod(
        m_mqttPublisher, "publish", Qt::QueuedConnection,
        Q_ARG(QString, topic), Q_ARG(QByteArray, message), Q_ARG(quint8, qos), Q_ARG(bool, retained));
}

void MqttPublishView::stopPublishing() const {
    QMetaObject::invokeMethod(m_mqttPublisher, "stopPublishing", Qt::QueuedConnection);
}

void MqttPublishView::setUIConnecting() const {
    ui_publishBtn->setText(tr("Connecting..."));
    ui_publishBtn->setEnabled(false);

    ui_hostEdit->setEnabled(false);
    ui_portEdit->setEnabled(false);

    ui_indicator->startRecording(tr("Connecting..."));
}

void MqttPublishView::setUIConnected() const {
    ui_publishBtn->setText(tr("Stop publish"));
    ui_publishBtn->setEnabled(true);

    ui_indicator->setRunHint(tr("Connected to broker"));
}

void MqttPublishView::setUIDisconnecting() const {
    ui_publishBtn->setText(tr("Disconnecting..."));
    ui_publishBtn->setEnabled(false);

    ui_indicator->setRunHint(tr("Disconnecting..."));
}

void MqttPublishView::setUIDisconnected() const {
    ui_publishBtn->setText(tr("Start publish"));
    ui_publishBtn->setEnabled(true);

    ui_hostEdit->setEnabled(true);
    ui_portEdit->setEnabled(true);

    ui_indicator->stopRecording();
}

void MqttPublishView::onMqttConnected() const {
    log(tr("Connected to MQTT broker."), LogEdit::Success);
    setUIConnected();
}

void MqttPublishView::onMqttDisconnected() const {
    log(tr("Disconnected from MQTT broker."), LogEdit::Warning);
    setUIDisconnected();
}

void MqttPublishView::onMqttErrorOccurred(const QMqttClient::ClientError error) const {
    QString msg;
    switch (error) {
        case QMqttClient::InvalidProtocolVersion:
            msg = "The broker does not accept a connection using the specified protocol version.";
            break;
        case QMqttClient::IdRejected:
            msg = "The broker rejected the client identifier.";
            break;
        case QMqttClient::ServerUnavailable:
            msg = tr("The network connection has been established, but the service is unavailable on the broker side.");
            break;
        case QMqttClient::BadUsernameOrPassword:
            msg = "The data in the username or password is malformed.";
            break;
        case QMqttClient::NotAuthorized:
            msg = tr("The client is not authorized to connect.");
            break;
        case QMqttClient::TransportInvalid:
            msg = "The underlying transport caused an error.";
            break;
        case QMqttClient::ProtocolViolation:
            msg = "The client encountered a protocol violation, and therefore closed the connection.";
            break;
        case QMqttClient::UnknownError:
            msg = "An unknown error occurred.";
            break;
        case QMqttClient::Mqtt5SpecificError:
            msg = "The broker returned an error code that is specific to MQTT 5.0.";
            break;
        default:
            return;
    }
    log(msg, LogEdit::Error);
    setUIDisconnected();
}

void MqttPublishView::initUI(const QString& host, const int port) {
    /******************** 设置Bar区域 ********************/
    ui_indicator = new StatusIndicator("", tr("Not connected"), true, this);
    ui_bar->layout()->addWidget(ui_indicator);

    /******************** 设置内容区域 ********************/
    ui_logEdit = new LogEdit(100, this);
    ui_logEdit->setMinimumSize(300, 100);

    ui_hostEdit = new QLineEdit(this);
    ui_hostEdit->setText(host);
    ui_hostEdit->setAlignment(Qt::AlignCenter);
    ui_hostEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui_portEdit = new QSpinBox(this);
    ui_portEdit->setRange(1024, 65535);
    ui_portEdit->setValue(port);
    ui_portEdit->setAlignment(Qt::AlignCenter);
    ui_portEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui_clearBtn = new QPushButton(tr("Clear log"), this);
    ui_publishBtn = new QPushButton(tr("Start publish"), this);
    ui_publishBtn->setObjectName("primary");

    auto btnLayout = new QHBoxLayout();
    btnLayout->addWidget(ui_clearBtn);
    btnLayout->addWidget(ui_publishBtn, 1);

    auto rightLayout = new QVBoxLayout();
    rightLayout->addStretch(1);
    rightLayout->addWidget(new QLabel(tr("MQTT broker address"), this));
    rightLayout->addWidget(ui_hostEdit);
    rightLayout->addWidget(new QLabel(tr("Port"), this));
    rightLayout->addWidget(ui_portEdit);
    rightLayout->addStretch(1);
    rightLayout->addLayout(btnLayout);

    auto rightWidget = new QWidget(this);
    rightWidget->setLayout(rightLayout);
    rightWidget->setFixedWidth(200);

    auto contentLayout = new QHBoxLayout();
    contentLayout->addWidget(ui_logEdit);
    contentLayout->addWidget(rightWidget);
    ui_content->setLayout(contentLayout);
}

void MqttPublishView::initConnection() {
    connect(ui_publishBtn, &QPushButton::clicked, this, [this]() {
        if (ui_publishBtn->text() == tr("Start publish")) {
            setUIConnecting();
            QMetaObject::invokeMethod(
                m_mqttPublisher, "startPublishing", Qt::QueuedConnection,
                Q_ARG(QString, ui_hostEdit->text()), Q_ARG(int, ui_portEdit->value())
            );
        } else if (ui_publishBtn->text() == tr("Stop publish")) {
            setUIDisconnecting();
            stopPublishing();
        }
    });

    connect(m_mqttPublisher, &MqttPublisher::connected, this, &MqttPublishView::onMqttConnected);
    connect(m_mqttPublisher, &MqttPublisher::disconnected, this, &MqttPublishView::onMqttDisconnected);
    connect(m_mqttPublisher, &MqttPublisher::errorOccurred, this, &MqttPublishView::onMqttErrorOccurred);
    connect(m_mqttPublisher, &MqttPublisher::messageSent, [=] (const qint32 id) {
       log(tr("An qos>0 message sent with id %1.").arg(id), LogEdit::Info);
    });
}
