#include "qtacryliceffecthelper_win32.h"
#include "qtacrylicutilities.h"
#include <QtCore/qt_windows.h>
#include <QtCore/qcoreapplication.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
// Only available since Windows Vista
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
// Only available since Windows Vista
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif

#ifndef WM_DPICHANGED
// Only available since Windows 8.1
#define WM_DPICHANGED 0x02E0
#endif

const int QtAcrylicWinUpdateEvent::QtAcrylicEffectChangeEventId = QEvent::registerEventType();

QtAcrylicWinUpdateEvent::QtAcrylicWinUpdateEvent(const bool clearWallpaper) : QEvent(static_cast<QEvent::Type>(QtAcrylicEffectChangeEventId))
{
    m_shouldClearPreviousWallpaper = clearWallpaper;
}

QtAcrylicWinUpdateEvent::~QtAcrylicWinUpdateEvent() = default;

static QScopedPointer<QtAcrylicWinEventFilter> g_instance;

QtAcrylicWinEventFilter::QtAcrylicWinEventFilter() = default;

QtAcrylicWinEventFilter::~QtAcrylicWinEventFilter()
{
    // FIXME
    //unsetup();
}

void QtAcrylicWinEventFilter::setup()
{
    if (g_instance.isNull()) {
        g_instance.reset(new QtAcrylicWinEventFilter);
        qApp->installNativeEventFilter(g_instance.data());
    }
}

void QtAcrylicWinEventFilter::unsetup()
{
    if (!g_instance.isNull()) {
        qApp->removeNativeEventFilter(g_instance.data());
        g_instance.reset();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool QtAcrylicWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool QtAcrylicWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_ASSERT(eventType == "windows_generic_MSG");
    Q_ASSERT(message);
    Q_UNUSED(result); // We don't need this parameter.
    if ((eventType != "windows_generic_MSG") || !message) {
        return false;
    }
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    const auto msg = *reinterpret_cast<MSG **>(message);
#else
    const auto msg = static_cast<LPMSG>(message);
#endif
    bool shouldUpdate = false;
    bool shouldClearWallpaper = false;
    switch (msg->message) {
    case WM_SETTINGCHANGE: {
        if (msg->wParam == SPI_SETDESKWALLPAPER) {
            shouldClearWallpaper = true;
            shouldUpdate = true;
        }
        if ((msg->wParam == 0) && (QString::fromWCharArray(reinterpret_cast<LPCWSTR>(msg->lParam)) == QStringLiteral("ImmersiveColorSet"))) {
            shouldUpdate = true;
        }
    } break;
    case WM_DPICHANGED: {
        shouldClearWallpaper = true;
        shouldUpdate = true;
    } break;
    case WM_THEMECHANGED:
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        shouldUpdate = true;
        break;
    default :
        break;
    }
    if (shouldUpdate) {
        const QWindow *window = _qam::Utilities::findWindow(reinterpret_cast<WId>(msg->hwnd));
        if (window) {
            QtAcrylicWinUpdateEvent event(shouldClearWallpaper);
            QCoreApplication::sendEvent(const_cast<QWindow *>(window), &event);
        }
    }
    return false;
}
