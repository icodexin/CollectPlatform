#include "SvgIconHelper.h"
#include <QApplication>
#include <QPainter>
#include <QSvgRenderer>
#include <QFile>

SvgIconHelper& SvgIconHelper::instance() {
    static SvgIconHelper instance;
    return instance;
}

QPixmap SvgIconHelper::pixmap(const QString& fileName, const QSize& size, const QColor& color, const bool cache) {
    return instance().getPixmap(fileName, size, color, cache);
}

QPixmap SvgIconHelper::pixmap(const QString& fileName, const int width, const int height, const QColor& color,
                              const bool cache) {
    return pixmap(fileName, {width, height}, color, cache);
}

QString SvgIconHelper::cacheKey(const QString& fileName, const QSize& size, const QColor& color, const qreal dpr) {
    return QString("%1_%2x%3_%4_%5")
            .arg(fileName)
            .arg(size.width()).arg(size.height())
            .arg(color.name())
            .arg(dpr);
}

QPixmap SvgIconHelper::getPixmap(const QString& fileName, const QSize& size, const QColor& color, const bool cache) {
    const qreal dpr = qApp->devicePixelRatio();
    QString key;
    if (cache) {
        key = cacheKey(fileName, size, color, dpr);
        if (m_cache.contains(key))
            return m_cache.value(key);
    }

    QPixmap pm = createPixmap(fileName, size, dpr, color);
    if (cache && !pm.isNull()) {
        m_cache.insert(key, pm);
    }
    return pm;
}

QPixmap SvgIconHelper::createPixmap(const QString& fileName, const QSize& size, const qreal dpr, const QColor& color) {
    Q_ASSERT(dpr > 0);
    if (!QFile::exists(fileName) || size.isEmpty()) {
        return {};
    }

    QPixmap pm(size * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter painter(&pm);
    QSvgRenderer renderer(fileName);
    renderer.render(&painter, QRectF({0, 0}, size));
    if (color.isValid()) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pm.rect(), color);
    }
    return pm;
}
