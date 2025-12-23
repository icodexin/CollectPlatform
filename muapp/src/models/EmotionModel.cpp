//
// Created by Lenovo on 2025/12/22.
//

#include "EmotionModel.h"
#include <QDateTime>

EmotionModel::EmotionModel(QObject *parent)
    : QObject(parent)
    , m_emotionClass(-1) // 初始值表示无数据
{
}

// 获取情绪名称
QString EmotionModel::emotionName() const
{
    return m_emotionName;
}

// 获取情绪类别
int EmotionModel::emotionClass() const
{
    return m_emotionClass;
}

// 获取时间戳
QString EmotionModel::timestamp() const
{
    return m_timestamp;
}

// 设置情绪名称
void EmotionModel::setEmotionName(const QString &newEmotionName)
{
    if (m_emotionName == newEmotionName)
        return; // 数据未变化，不触发信号
    m_emotionName = newEmotionName;
    emit emotionNameChanged(); // 触发信号，通知QML更新
}

// 设置情绪类别
void EmotionModel::setEmotionClass(int newEmotionClass)
{
    if (m_emotionClass == newEmotionClass)
        return;
    m_emotionClass = newEmotionClass;
    emit emotionClassChanged();
}

// 槽函数：更新情绪数据（核心）
void EmotionModel::updateEmotionData(int predClass, const QString &predName)
{
    // 更新属性
    setEmotionClass(predClass);
    setEmotionName(predName);
    // 更新时间戳
    m_timestamp = generateTimestamp();
    emit timestampChanged();
    // 发射自定义信号（可选）
    emit emotionUpdated(predClass, predName);
}

// 生成格式化的时间戳（如"2025-12-22 15:30:25"）
QString EmotionModel::generateTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}