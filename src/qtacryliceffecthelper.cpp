#include "qtacryliceffecthelper.h"
#include "qtacrylicutilities.h"
#include <QtGui/qpainter.h>
#include <QtCore/qdebug.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>

// We only need one copy of the blured wallpaper and the noise texture for the whole application.
// But making them become static variables is not allowed because QPixmap can't be constructed
// before QGuiApplication, so we use "Q_GLOBAL_STATIC" instead, it will be initialized when we
// first use it.

struct QtAcrylicHelperData {
    QPixmap bluredWallpaper = {};
    QImage noiseTexture = {};
};

Q_GLOBAL_STATIC(QtAcrylicHelperData, acrylicData)

QtAcrylicEffectHelper::QtAcrylicEffectHelper()
{
    //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#ifdef Q_OS_MACOS
    if (Utilities::shouldUseTraditionalBlur()) {
        m_tintOpacity = 0.6;
    }
#endif
}

QtAcrylicEffectHelper::~QtAcrylicEffectHelper() = default;

void QtAcrylicEffectHelper::showPerformanceWarning() const
{
    qDebug() << "The Acrylic blur effect has been enabled. Rendering acrylic material "
                "surfaces is highly GPU-intensive, which can slow down the application, "
                "increase the power consumption on the devices on which the application "
                "is running.";
}

void QtAcrylicEffectHelper::regenerateWallpaper()
{
    if (!acrylicData()->bluredWallpaper.isNull()) {
        acrylicData()->bluredWallpaper = {};
    }
    generateBluredWallpaper();
}

const QBrush &QtAcrylicEffectHelper::getAcrylicBrush() const
{
    return m_acrylicBrush;
}

const QColor &QtAcrylicEffectHelper::getTintColor() const
{
    return m_tintColor;
}

qreal QtAcrylicEffectHelper::getTintOpacity() const
{
    return m_tintOpacity;
}

qreal QtAcrylicEffectHelper::getNoiseOpacity() const
{
    return m_noiseOpacity;
}

const QPixmap &QtAcrylicEffectHelper::getBluredWallpaper() const
{
    return acrylicData()->bluredWallpaper;
}

void QtAcrylicEffectHelper::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << value << "is not a valid color.";
        return;
    }
    if (m_tintColor != value) {
        m_tintColor = value;
    }
}

void QtAcrylicEffectHelper::setTintOpacity(const qreal value)
{
    if (m_tintOpacity != value) {
        m_tintOpacity = value;
    }
}

void QtAcrylicEffectHelper::setNoiseOpacity(const qreal value)
{
    if (m_noiseOpacity != value) {
        m_noiseOpacity = value;
    }
}

void QtAcrylicEffectHelper::paintBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid()) {
        return;
    }
    // TODO: should we limit it to Win32 only? Or should we do something about the
    // acrylic brush instead?
    if (_qam::Utilities::disableExtraProcessingForBlur()) {
        return;
    }
    painter->save();
    const QRect maskRect = {QPoint{0, 0}, rect.size()};
    if (_qam::Utilities::shouldUseTraditionalBlur()) {
        const QPainter::CompositionMode mode = painter->compositionMode();
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(maskRect, defaultMaskColor());
        painter->setCompositionMode(mode);
    } else {
        // Emulate blur behind window by blurring the desktop wallpaper.
        if (acrylicData()->bluredWallpaper.isNull()) {
            generateBluredWallpaper();
        }
        painter->drawPixmap(QPoint{0, 0}, acrylicData()->bluredWallpaper, rect);
    }
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(1);
    painter->fillRect(maskRect, m_acrylicBrush);
    painter->restore();
}

void QtAcrylicEffectHelper::updateAcrylicBrush(const QColor &alternativeTintColor)
{
    if (acrylicData()->noiseTexture.isNull()) {
        Q_INIT_RESOURCE(qtacrylichelper);
        acrylicData()->noiseTexture = QImage{QStringLiteral(":/QtAcrylicHelper/Noise.png")};
    }
    QImage acrylicTexture({64, 64}, QImage::Format_ARGB32_Premultiplied);
    QColor fillColor = Qt::transparent;
#ifdef Q_OS_WINDOWS
    if (!_qam::Utilities::isOfficialMSWin10AcrylicBlurAvailable()) {
        // Add a soft light layer for the background.
        fillColor = defaultMaskColor();
        fillColor.setAlpha(150);
    }
#endif
    acrylicTexture.fill(fillColor);
    QPainter painter(&acrylicTexture);
    painter.setOpacity(m_tintOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, getAppropriateTintColor(alternativeTintColor));
    painter.setOpacity(m_noiseOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, acrylicData()->noiseTexture);
    m_acrylicBrush = acrylicTexture;
}

void QtAcrylicEffectHelper::generateBluredWallpaper()
{
    if (!acrylicData()->bluredWallpaper.isNull()) {
        return;
    }
    const QSize size = QGuiApplication::primaryScreen()->size();
    acrylicData()->bluredWallpaper = QPixmap(size);
    acrylicData()->bluredWallpaper.fill(Qt::transparent);
    QImage image = _qam::Utilities::getDesktopWallpaperImage();
    // On some platforms we may not be able to get the desktop wallpaper, such as Linux and WebAssembly.
    if (image.isNull()) {
        return;
    }
    const _qam::Utilities::DesktopWallpaperAspectStyle aspectStyle = _qam::Utilities::getDesktopWallpaperAspectStyle();
    QImage buffer(size, QImage::Format_ARGB32_Premultiplied);
#ifdef Q_OS_WINDOWS
    if ((aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::Central) ||
            (aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::KeepRatioFit)) {
        buffer.fill(_qam::Utilities::getDesktopBackgroundColor());
    }
#endif
    if (aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit ||
            aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::KeepRatioFit ||
            aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::KeepRatioByExpanding) {
        Qt::AspectRatioMode mode;
        if (aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::KeepRatioFit) {
            mode = Qt::KeepAspectRatio;
        } else {
            mode = Qt::KeepAspectRatioByExpanding;
        }
        QSize newSize = image.size();
        newSize.scale(size, mode);
        image = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    if (aspectStyle == _qam::Utilities::DesktopWallpaperAspectStyle::Tiled) {
        QPainter painterBuffer(&buffer);
        painterBuffer.fillRect(QRect{{0, 0}, size}, image);
    } else {
        QPainter painterBuffer(&buffer);
        const QRect rect = _qam::Utilities::alignedRect(Qt::LeftToRight, Qt::AlignCenter, image.size(), {{0, 0}, size});
        painterBuffer.drawImage(rect.topLeft(), image);
    }
    QPainter painter(&acrylicData()->bluredWallpaper);
#if 1
    _qam::Utilities::blurImage(&painter, buffer, 128, false, false);
#else
    painter.drawImage(QPoint{0, 0}, buffer);
#endif
}

const QColor &QtAcrylicEffectHelper::defaultMaskColor() const
{
    static const QColor color = _qam::Utilities::isDarkThemeEnabled() ? Qt::darkGray : Qt::white;
    return color;
}

const QColor &QtAcrylicEffectHelper::getAppropriateTintColor(const QColor &alternativeTintColor) const
{
    if (alternativeTintColor.isValid() && (alternativeTintColor != Qt::transparent)) {
        return alternativeTintColor;
    }
    if (m_tintColor.isValid() && (m_tintColor != Qt::transparent)) {
        return m_tintColor;
    }
    return defaultMaskColor();
}
