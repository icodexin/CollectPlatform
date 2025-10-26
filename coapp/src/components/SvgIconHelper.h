#ifndef SVGICONHELPER_H
#define SVGICONHELPER_H

#include <QPixmap>
#include <QHash>

class SvgIconHelper {
public:
    SvgIconHelper(const SvgIconHelper&) = delete;
    SvgIconHelper& operator=(const SvgIconHelper&) = delete;

    static SvgIconHelper& instance();
    static QPixmap pixmap(const QString& fileName, const QSize& size, const QColor& color = QColor{}, bool cache = true);
    static QPixmap pixmap(const QString& fileName, int width, int height, const QColor& color = QColor{}, bool cache = true);

private:
    SvgIconHelper() = default;
    ~SvgIconHelper() = default;

    static QString cacheKey(const QString& fileName, const QSize& size, const QColor& color, qreal dpr);

    QPixmap getPixmap(const QString& fileName, const QSize& size, const QColor& color, bool cache);
    static QPixmap createPixmap(const QString& fileName, const QSize& size, qreal dpr, const QColor& color);

private:
    QHash<QString, QPixmap> m_cache = {};
};



#endif //SVGICONHELPER_H
