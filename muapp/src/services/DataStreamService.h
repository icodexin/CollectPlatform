#ifndef DATASTREAMSERVICE_H
#define DATASTREAMSERVICE_H

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtQml/QQmlParserStatus>
#include <QtQml/qqmlregistration.h>
#include "model/EEGData.h"
#include "model/WristbandData.h"
#include "./models/EmotionModel.h"
class DataStreamService : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_PROPERTY(QString subStudentId READ subStudentId WRITE setSubStudentId NOTIFY subStudentIdChanged)
    Q_PROPERTY(DataType subDataType READ subDataType WRITE setSubDataType NOTIFY subDataTypeChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(int connectTimes READ connectTimes NOTIFY connectTimesChanged)
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(EmotionModel* emotionModel READ emotionModel CONSTANT)
public:
    enum DataType {
        ALL       = 0,
        EEG       = 1,
        Wristband = 2,
        Emotion = 3,
        Cognition = 4,
    };
    Q_ENUM(DataType)

    enum Status {
        Offline    = 0,
        Online     = 1,
        Connecting = 2
    };
    Q_ENUM(Status)

    explicit DataStreamService(QObject* parent = nullptr);
    ~DataStreamService() override;

    void classBegin() override;
    void componentComplete() override;

    /// 订阅的学生ID
    QString subStudentId() const;
    void setSubStudentId(const QString& studentId);

    /// 订阅的数据类型
    DataType subDataType() const;
    void setSubDataType(DataType type);

    /// 服务状态
    Status status() const;

    /// 尝试连接次数
    int connectTimes() const;

    //读取函数
    EmotionModel* emotionModel() const;
signals:
    void subStudentIdChanged(const QString& studentId);
    void subDataTypeChanged(DataType type);
    void statusChanged(Status status);
    void connectTimesChanged(int times);

    void msgReceived(const QJsonObject& msg);
    void wristbandReceived(const WristbandPacket& data, const QString& studentId);
    void eegReceived(const EEGPacket& data, const QString& studentId);

private:
    void setStatus(Status status);
    void setConnectTimes(int times);
    void start();
    void subscribe(const QString& studentId, DataType type);
    void attachClientSignals(const QString& key);
    void handleTextMessage(const QString& text);
    void handleBinaryMessage(const QByteArray& data);

private:
    QString m_subStudentId;
    DataType m_subDataType = ALL;
    Status m_status = Offline;
    int m_connectTimes = 0;
    EmotionModel *m_emotion_model = NULL;
};

#endif //DATASTREAMSERVICE_H
