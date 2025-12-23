//
// Created by Lenovo on 2025/12/22.
//
#ifndef EMOTIONMODEL_H
#define EMOTIONMODEL_H

#include <QObject>
#include <QString>
#include <QDateTime>

// 必须继承QObject，才能使用信号槽和QML交互
class EmotionModel : public QObject
{
    Q_OBJECT
    // 声明QML可访问的属性（用于绑定UI），会自动生成对应的getter/setter和信号
    Q_PROPERTY(QString emotionName READ emotionName WRITE setEmotionName NOTIFY emotionNameChanged)
    Q_PROPERTY(int emotionClass READ emotionClass WRITE setEmotionClass NOTIFY emotionClassChanged)
    Q_PROPERTY(QString timestamp READ timestamp NOTIFY timestampChanged)

public:
    explicit EmotionModel(QObject *parent = nullptr);

    // 属性的getter方法
    QString emotionName() const;
    int emotionClass() const;
    QString timestamp() const;

    // 属性的setter方法
    void setEmotionName(const QString &newEmotionName);
    void setEmotionClass(int newEmotionClass);

public slots:
    // 提供一个槽函数，用于接收C++端的情绪数据（也可以直接调用setter）
    void updateEmotionData(int predClass, const QString &predName);

    signals:
        // 属性变化的信号（Q_PROPERTY的NOTIFY必须对应）
        void emotionNameChanged();
    void emotionClassChanged();
    void timestampChanged();

    // 可选：自定义信号，携带完整的情绪数据（QML也可以监听这个信号）
    void emotionUpdated(int predClass, const QString &predName);

private:
    // 私有成员变量
    QString m_emotionName; // 情绪名称（如"开心"、"愤怒"）
    int m_emotionClass;    // 情绪类别（数字标识）
    QString m_timestamp;   // 时间戳（格式化后的字符串）

    // 辅助函数：生成格式化的时间戳
    QString generateTimestamp();
};

#endif // EMOTIONMODEL_H