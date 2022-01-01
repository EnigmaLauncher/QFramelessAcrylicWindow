#include "utilities.h"

#include <QtCore/qvariant.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

static constexpr int kDefaultResizeBorderThickness = 8;
static constexpr int kDefaultCaptionHeight = 23;

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    const qreal devicePixelRatio = window->devicePixelRatio();
    const qreal scaleFactor = (dpiScale ? devicePixelRatio : 1.0);
    switch (metric) {
    case SystemMetric::ResizeBorderThickness: {
        const int resizeBorderThickness = window->property(Constants::kResizeBorderThicknessFlag).toInt();
        if ((resizeBorderThickness > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(resizeBorderThickness) * scaleFactor);
        } else {
            // ### TO BE IMPLEMENTED: Retrieve system value through official API
            if (dpiScale) {
                return qRound(static_cast<qreal>(kDefaultResizeBorderThickness) * devicePixelRatio);
            } else {
                return kDefaultResizeBorderThickness;
            }
        }
    }
    case SystemMetric::CaptionHeight: {
        const int captionHeight = window->property(Constants::kCaptionHeightFlag).toInt();
        if ((captionHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(captionHeight) * scaleFactor);
        } else {
            // ### TO BE IMPLEMENTED: Retrieve system value through official API
            if (dpiScale) {
                return qRound(static_cast<qreal>(kDefaultCaptionHeight) * devicePixelRatio);
            } else {
                return kDefaultCaptionHeight;
            }
        }
    }
    case SystemMetric::TitleBarHeight: {
        const int titleBarHeight = window->property(Constants::kTitleBarHeightFlag).toInt();
        if ((titleBarHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(titleBarHeight) * scaleFactor);
        } else {
            const int captionHeight = getSystemMetric(window,SystemMetric::CaptionHeight,
                                                      dpiScale, forceSystemValue);
            const int resizeBorderThickness = getSystemMetric(window, SystemMetric::ResizeBorderThickness,
                                                              dpiScale, forceSystemValue);
            return (((window->windowState() == Qt::WindowMaximized)
                     || (window->windowState() == Qt::WindowFullScreen))
                    ? captionHeight : (captionHeight + resizeBorderThickness));
        }
    }
    }
    return 0;
}

QColor Utilities::getColorizationColor()
{
    // ### TO BE IMPLEMENTED
    return Qt::darkGray;
}

int Utilities::getWindowVisibleFrameBorderThickness(const WId winId)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    return 1;
}

bool Utilities::shouldAppsUseDarkMode()
{
    // ### TO BE IMPLEMENTED
    return false;
}

ColorizationArea Utilities::getColorizationArea()
{
    // ### TO BE IMPLEMENTED
    return ColorizationArea::None;
}

bool Utilities::isThemeChanged(const void *data)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    return false;
}

bool Utilities::isSystemMenuRequested(const void *data, QPointF *pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    Q_UNUSED(pos);
    return false;
}

bool Utilities::showSystemMenu(const WId winId, const QPointF &pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    Q_UNUSED(pos);
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
