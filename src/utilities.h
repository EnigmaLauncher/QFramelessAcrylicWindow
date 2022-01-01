#pragma once

#include "framelesshelper_global.h"
#include <QtGui/qwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

namespace Utilities
{

[[nodiscard]] FRAMELESSHELPER_API int getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue = false);
[[nodiscard]] FRAMELESSHELPER_API QWindow *findWindow(const WId winId);
[[nodiscard]] FRAMELESSHELPER_API bool isWindowFixedSize(const QWindow *window);
[[nodiscard]] FRAMELESSHELPER_API bool isHitTestVisible(const QWindow *window);
[[nodiscard]] FRAMELESSHELPER_API QPointF mapOriginPointToWindow(const QObject *object);
[[nodiscard]] FRAMELESSHELPER_API QColor getColorizationColor();
[[nodiscard]] FRAMELESSHELPER_API int getWindowVisibleFrameBorderThickness(const WId winId);
[[nodiscard]] FRAMELESSHELPER_API bool shouldAppsUseDarkMode();
[[nodiscard]] FRAMELESSHELPER_API ColorizationArea getColorizationArea();
[[nodiscard]] FRAMELESSHELPER_API bool isThemeChanged(const void *data);
[[nodiscard]] FRAMELESSHELPER_API bool isSystemMenuRequested(const void *data, QPointF *pos);
[[nodiscard]] FRAMELESSHELPER_API bool showSystemMenu(const WId winId, const QPointF &pos);

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_API bool isWin8OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin8Point1OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin10OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isDwmCompositionAvailable();
FRAMELESSHELPER_API void triggerFrameChange(const WId winId);
FRAMELESSHELPER_API void updateFrameMargins(const WId winId, const bool reset);
FRAMELESSHELPER_API void updateQtFrameMargins(QWindow *window, const bool enable);
[[nodiscard]] FRAMELESSHELPER_API QString getSystemErrorMessage(const QString &function);
#endif

}

FRAMELESSHELPER_END_NAMESPACE
