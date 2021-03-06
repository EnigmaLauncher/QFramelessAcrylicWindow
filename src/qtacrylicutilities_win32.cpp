#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "qtacrylicutilities.h"
#include <QtCore/qsettings.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qt_windows.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <dwmapi.h>
#include <shobjidl_core.h>
#include <wininet.h>
#include <Objbase.h>
#include <shlobj_core.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
#include <QtCore/qscopeguard.h>
#endif

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB
{
  WCA_UNDEFINED = 0,
  WCA_NCRENDERING_ENABLED = 1,
  WCA_NCRENDERING_POLICY = 2,
  WCA_TRANSITIONS_FORCEDISABLED = 3,
  WCA_ALLOW_NCPAINT = 4,
  WCA_CAPTION_BUTTON_BOUNDS = 5,
  WCA_NONCLIENT_RTL_LAYOUT = 6,
  WCA_FORCE_ICONIC_REPRESENTATION = 7,
  WCA_EXTENDED_FRAME_BOUNDS = 8,
  WCA_HAS_ICONIC_BITMAP = 9,
  WCA_THEME_ATTRIBUTES = 10,
  WCA_NCRENDERING_EXILED = 11,
  WCA_NCADORNMENTINFO = 12,
  WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
  WCA_VIDEO_OVERLAY_ACTIVE = 14,
  WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
  WCA_DISALLOW_PEEK = 16,
  WCA_CLOAK = 17,
  WCA_CLOAKED = 18,
  WCA_ACCENT_POLICY = 19,
  WCA_FREEZE_REPRESENTATION = 20,
  WCA_EVER_UNCLOAKED = 21,
  WCA_VISUAL_OWNER = 22,
  WCA_HOLOGRAPHIC = 23,
  WCA_EXCLUDED_FROM_DDA = 24,
  WCA_PASSIVEUPDATEMODE = 25,
  WCA_USEDARKMODECOLORS = 26,
  WCA_LAST = 27
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using ACCENT_STATE = enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6
};

using ACCENT_POLICY = struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    COLORREF GradientColor;
    DWORD AnimationId;
};


typedef enum {
    DWMWCP_DEFAULT,
    DWMWCP_DONOTROUND,
    DWMWCP_ROUND,
    DWMWCP_ROUNDSMALL
} DWM_WINDOW_CORNER_PREFERENCE;


//    typedef enum {
//        DWMWA_NCRENDERING_ENABLED,
//        DWMWA_NCRENDERING_POLICY,
//        DWMWA_TRANSITIONS_FORCEDISABLED,
//        DWMWA_ALLOW_NCPAINT,
//        DWMWA_CAPTION_BUTTON_BOUNDS,
//        DWMWA_NONCLIENT_RTL_LAYOUT,
//        DWMWA_FORCE_ICONIC_REPRESENTATION,
//        DWMWA_FLIP3D_POLICY,
//        DWMWA_EXTENDED_FRAME_BOUNDS,
//        DWMWA_HAS_ICONIC_BITMAP,
//        DWMWA_DISALLOW_PEEK,
//        DWMWA_EXCLUDED_FROM_PEEK,
//        DWMWA_CLOAK,
//        DWMWA_CLOAKED,
//        DWMWA_FREEZE_REPRESENTATION,
//        DWMWA_PASSIVE_UPDATE_MODE,
//        DWMWA_USE_HOSTBACKDROPBRUSH,
//        DWMWA_USE_IMMERSIVE_DARK_MODE,
//        DWMWA_WINDOW_CORNER_PREFERENCE,
//        DWMWA_BORDER_COLOR,
//        DWMWA_CAPTION_COLOR,
//        DWMWA_TEXT_COLOR,
//        DWMWA_VISIBLE_FRAME_BORDER_THICKNESS,
//        DWMWA_SYSTEMBACKDROP_TYPE = 38, // Windows 11 Build 22523+
//        DWMWA_MICA_EFFECT = 1029,
//        DWMWA_LAST

//    } DWMWINDOWATTRIBUTE;


enum DWM_STYLE
{
    DWMSBT_AUTO = 0,
    DWMSBT_DISABLE = 1,         // None
    DWMSBT_MAINWINDOW = 2,      // Mica
    DWMSBT_TRANSIENTWINDOW = 3, // Acrylic
    DWMSBT_TABBEDWINDOW = 4     // Tabbed
};

static const QString g_dwmRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)");
static const QString g_personalizeRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)");
static const QString g_desktopRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Control Panel\Desktop)");

using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);
using ShouldAppsUseDarkModePtr =  BOOL(WINAPI *)();
using ShouldSystemUseDarkModePtr = BOOL(WINAPI *)();

using Win32Data = struct _QAH_UTILITIES_WIN32_DATA
{
    SetWindowCompositionAttributePtr SetWindowCompositionAttributePFN = nullptr;
    ShouldAppsUseDarkModePtr ShouldAppsUseDarkModePFN = nullptr;
    ShouldSystemUseDarkModePtr ShouldSystemUseDarkModePFN = nullptr;

    _QAH_UTILITIES_WIN32_DATA()
    {
        load();
    }

    void load()
    {
        QLibrary User32Dll(QStringLiteral("User32"));
        SetWindowCompositionAttributePFN = reinterpret_cast<SetWindowCompositionAttributePtr>(User32Dll.resolve("SetWindowCompositionAttribute"));

        QLibrary UxThemeDll(QStringLiteral("UxTheme"));
        ShouldAppsUseDarkModePFN = reinterpret_cast<ShouldAppsUseDarkModePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(132)));
        ShouldSystemUseDarkModePFN = reinterpret_cast<ShouldSystemUseDarkModePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(138)));
    }
};

Q_GLOBAL_STATIC(Win32Data, win32Data)

[[nodiscard]] static inline QString __getSystemErrorMessage(const QString &function, const HRESULT hr)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    if (SUCCEEDED(hr)) {
        return {};
    }
    const DWORD dwError = HRESULT_CODE(hr);
    return __getSystemErrorMessage(function, dwError);
}

bool _qam::Utilities::setBlurEffectEnabled(const QWindow *window, const bool enabled, const QColor &gradientColor)
{


    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return false;
    }

    bool result = false;
    // We prefer DwmEnableBlurBehindWindow on Windows 7.
    if (isWin8OrGreater() && win32Data()->SetWindowCompositionAttributePFN) {
        ACCENT_POLICY accentPolicy;
        SecureZeroMemory(&accentPolicy, sizeof(accentPolicy));
        WINDOWCOMPOSITIONATTRIBDATA wcaData;
        SecureZeroMemory(&wcaData, sizeof(wcaData));
        wcaData.Attrib = WCA_ACCENT_POLICY;
        wcaData.pvData = &accentPolicy;
        wcaData.cbData = sizeof(accentPolicy);

        DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
        BOOL enable = TRUE;
        HRESULT hr = S_OK;

        hr = DwmSetWindowAttribute(hwnd, 16, &enable, sizeof(enable)); // DWMWA_USE_HOSTBACKDROPBRUSH
        if (FAILED(hr)) {
            qWarning() << __getSystemErrorMessage(QStringLiteral("DwmSetWindowAttribute_16"), hr);
        }

        hr = DwmSetWindowAttribute(hwnd, 18, &preference, sizeof(preference)); // DWMWA_WINDOW_CORNER_PREFERENCE
        if (FAILED(hr)) {
            qWarning() << __getSystemErrorMessage(QStringLiteral("DwmSetWindowAttribute_18"), hr);
        }

        hr = DwmSetWindowAttribute(hwnd, 20, &enable,sizeof(enable)); // DWMWA_CAPTION_COLOR
        if (FAILED(hr)) {
            qWarning() << __getSystemErrorMessage(QStringLiteral("DwmSetWindowAttribute_20"), hr);
        }

        // FIX: Use the public API for Windows versions 11 (build: 22523) and up
        DWM_STYLE effect_value = DWMSBT_TRANSIENTWINDOW; // Acrylic Effect int(3) value.
        hr = DwmSetWindowAttribute(hwnd, 38, &effect_value, sizeof(enable)); // DWMWA_SYSTEMBACKDROP_TYPE (Windows 11 Build 22523+)
        if (FAILED(hr)) {
            qWarning() << __getSystemErrorMessage(QStringLiteral("DwmSetWindowAttribute_38"), hr);
        }

        if (enabled) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            if (gradientColor.isValid()) {
                accentPolicy.GradientColor = qRgba(gradientColor.blue(), gradientColor.green(), gradientColor.red(), gradientColor.alpha());
            } else {
                const QColor colorizationColor = getColorizationColor();
                accentPolicy.GradientColor =
                    RGB(qRound(colorizationColor.red() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.green() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.blue() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()));
            }
            if (isOfficialMSWin10AcrylicBlurAvailable()) {
                accentPolicy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                if (!gradientColor.isValid()) {
                    accentPolicy.GradientColor = 0x01FFFFFF;
                }
            } else {
                accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
            }
        } else {
            accentPolicy.AccentState = ACCENT_DISABLED;
        }
        result = (win32Data()->SetWindowCompositionAttributePFN(hwnd, &wcaData) != FALSE);
        if (!result) {
            qWarning() << "SetWindowCompositionAttribute failed.";
        }
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = enabled ? TRUE : FALSE;
        result = SUCCEEDED(DwmEnableBlurBehindWindow(hwnd, &dwmBB));
        if (!result) {
            qWarning() << "DwmEnableBlurBehindWindow failed.";
        }
    }
    if (result) {
        const auto win = const_cast<QWindow *>(window);
        win->setProperty(_qam::Global::_qam_blurEnabled_flag, enabled);
        win->setProperty(_qam::Global::_qam_gradientColor_flag, gradientColor);
    }
    return result;
}

QColor _qam::Utilities::getColorizationColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&color, &opaqueBlend))) {
        return QColor::fromRgba(color);
    }
    qWarning() << "DwmGetColorizationColor failed, reading from the registry instead.";
    bool ok = false;
    const QSettings settings(g_dwmRegistryKey, QSettings::NativeFormat);
    const DWORD value = settings.value(QStringLiteral("ColorizationColor"), 0).toULongLong(&ok);
    return ok ? QColor::fromRgba(value) : Qt::darkGray;
}

bool _qam::Utilities::isDarkThemeEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    // We can't use ShouldAppsUseDarkMode due to the following reason:
    // it's not exported publicly so we can only load it dynamically through its ordinal name,
    // however, its ordinal name has changed in some unknown system versions so we can't find
    // the actual function now. But ShouldSystemUseDarkMode is not affected, we can still
    // use it in the latest version of Windows.
    if (win32Data()->ShouldSystemUseDarkModePFN) {
        return win32Data()->ShouldSystemUseDarkModePFN();
    }
    qDebug() << "ShouldSystemUseDarkMode() not available, reading from the registry instead.";
    bool ok = false;
    const QSettings settings(g_personalizeRegistryKey, QSettings::NativeFormat);
    const bool lightThemeEnabled = settings.value(QStringLiteral("AppsUseLightTheme"), 0).toULongLong(&ok) != 0;
    return (ok && !lightThemeEnabled);
}

QImage _qam::Utilities::getDesktopWallpaperImage(const int screen)
{
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper* pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                UINT monitorCount = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount))) {
                    if (screen > int(monitorCount - 1)) {
                        qWarning() << "Screen number above total screen count.";
                        return {};
                    }
                    const UINT monitorIndex = qMax(screen, 0);
                    LPWSTR monitorId = nullptr;
                    if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathAt(monitorIndex, &monitorId)) && monitorId) {
                        LPWSTR wallpaperPath = nullptr;
                        if (SUCCEEDED(pDesktopWallpaper->GetWallpaper(monitorId, &wallpaperPath)) && wallpaperPath) {
                            CoTaskMemFree(monitorId);
                            const QString _path = QString::fromWCharArray(wallpaperPath);
                            CoTaskMemFree(wallpaperPath);
                            return QImage(_path);
                        } else {
                            CoTaskMemFree(monitorId);
                            qWarning() << "IDesktopWallpaper::GetWallpaper() failed.";
                        }
                    } else {
                        qWarning() << "IDesktopWallpaper::GetMonitorDevicePathAt() failed";
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetMonitorDevicePathCount() failed";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            const auto wallpaperPath = new WCHAR[MAX_PATH];
            // TODO: AD_GETWP_BMP, AD_GETWP_IMAGE, AD_GETWP_LAST_APPLIED. What's the difference?
            if (SUCCEEDED(pActiveDesktop->GetWallpaper(wallpaperPath, MAX_PATH, AD_GETWP_LAST_APPLIED))) {
                const QString _path = QString::fromWCharArray(wallpaperPath);
                delete [] wallpaperPath;
                return QImage(_path);
            } else {
                qWarning() << "IActiveDesktop::GetWallpaper() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Using SystemParametersInfoW instead.";
    const auto wallpaperPath = new WCHAR[MAX_PATH];
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0) != FALSE) {
        const QString _path = QString::fromWCharArray(wallpaperPath);
        delete [] wallpaperPath;
        return QImage(_path);
    }
    qWarning() << "SystemParametersInfoW failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    const QString path = settings.value(QStringLiteral("WallPaper")).toString();
    if (QFileInfo::exists(path)) {
        return QImage(path);
    }
    qWarning() << "Failed to read the registry.";
    return {};
}

QColor _qam::Utilities::getDesktopBackgroundColor(const int screen)
{
    Q_UNUSED(screen); // TODO: make use of it.
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper]() {
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                COLORREF color = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetBackgroundColor(&color))) {
                    return QColor::fromRgba(color);
                } else {
                    qWarning() << "IDesktopWallpaper::GetBackgroundColor() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed.";
    }
    // TODO: Is there any other way to get the background color? Traditional Win32 API? Registry?
    // Is there a Shell API for Win7?
    return Qt::black;
}

_qam::Utilities::DesktopWallpaperAspectStyle _qam::Utilities::getDesktopWallpaperAspectStyle(const int screen)
{
    Q_UNUSED(screen); // TODO: make use of it.
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
                if (SUCCEEDED(pDesktopWallpaper->GetPosition(&position))) {
                    switch (position) {
                    case DWPOS_CENTER:
                        return DesktopWallpaperAspectStyle::Central;
                    case DWPOS_TILE:
                        return DesktopWallpaperAspectStyle::Tiled;
                    case DWPOS_STRETCH:
                        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                    case DWPOS_FIT:
                        return DesktopWallpaperAspectStyle::KeepRatioFit;
                    case DWPOS_FILL:
                        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                    case DWPOS_SPAN:
                        return DesktopWallpaperAspectStyle::Span;
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetPosition() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            WALLPAPEROPT opt;
            SecureZeroMemory(&opt, sizeof(opt));
            opt.dwSize = sizeof(opt);
            if (SUCCEEDED(pActiveDesktop->GetWallpaperOptions(&opt, 0))) {
                switch (opt.dwStyle) {
                case WPSTYLE_CENTER:
                    return DesktopWallpaperAspectStyle::Central;
                case WPSTYLE_TILE:
                    return DesktopWallpaperAspectStyle::Tiled;
                case WPSTYLE_STRETCH:
                    return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                case WPSTYLE_KEEPASPECT:
                    return DesktopWallpaperAspectStyle::KeepRatioFit;
                case WPSTYLE_CROPTOFIT:
                    return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                case WPSTYLE_SPAN:
                    return DesktopWallpaperAspectStyle::Span;
                }
            } else {
                qWarning() << "IActiveDesktop::GetWallpaperOptions() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    bool ok = false;
    const DWORD style = settings.value(QStringLiteral("WallpaperStyle"), 0).toULongLong(&ok);
    if (!ok) {
        qWarning() << "Failed to read the registry.";
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
    }
    switch (style) {
    case 0: {
        bool ok = false;
        if ((settings.value(QStringLiteral("TileWallpaper"), 0).toULongLong(&ok) != 0) && ok) {
            return DesktopWallpaperAspectStyle::Tiled;
        } else {
            return DesktopWallpaperAspectStyle::Central;
        }
    }
    case 2:
        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
    case 6:
        return DesktopWallpaperAspectStyle::KeepRatioFit;
    case 10:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
    case 22:
        return DesktopWallpaperAspectStyle::Span;
    default:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
    }
}

bool _qam::Utilities::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8;
#endif
}

bool _qam::Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

bool _qam::Utilities::isWin10OrGreater(const int subVer)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, subVer);
#else
    Q_UNUSED(ver);
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

bool _qam::Utilities::isWin11OrGreater(const int subVer)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 11, 0, subVer);
#else
    Q_UNUSED(ver);
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS11;
#endif
}

static inline bool forceEnableOfficialMSWin10AcrylicBlur()
{
    return qEnvironmentVariableIsSet(_qam::Global::_qam_forceEnableOfficialMSWin10AcrylicBlur_flag);
}

static inline bool forceDisableOfficialMSWin10AcrylicBlur()
{
    return qEnvironmentVariableIsSet(_qam::Global::_qam_forceDisableOfficialMSWin10AcrylicBlur_flag);
}

static inline bool shouldUseOfficialMSWin10AcrylicBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    const QOperatingSystemVersion currentVersion = QOperatingSystemVersion::current();
    if (currentVersion > QOperatingSystemVersion::Windows10) {
        return true;
    }
    // For these versions, the undocumented API is known to be working well.
    return (((currentVersion.microVersion() >= 16190) && (currentVersion.microVersion() < 17134)) || (currentVersion.microVersion() >= 21343));
#else
    // TODO
    return false;
#endif
}

bool _qam::Utilities::isOfficialMSWin10AcrylicBlurAvailable()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    if (forceDisableTraditionalBlur() || forceEnableWallpaperBlur()) {
        // We can't enable the official Acrylic blur in wallpaper blur mode.
        return false;
    }
    if (forceEnableOfficialMSWin10AcrylicBlur()) {
        return true;
    }
    if (forceDisableOfficialMSWin10AcrylicBlur()) {
        return false;
    }
    return shouldUseOfficialMSWin10AcrylicBlur();
}

static inline bool shouldUseOriginalDwmBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return _qam::Utilities::isWin10OrGreater() || (QOperatingSystemVersion::current() >= QOperatingSystemVersion::OSXYosemite);
#else
    // TODO
    return false;
#endif
}

bool _qam::Utilities::shouldUseTraditionalBlur()
{
    const bool userAllowed = !(forceDisableTraditionalBlur() || forceEnableWallpaperBlur());
    const bool osAllowed = shouldUseOriginalDwmBlur();
    const bool result = (userAllowed && osAllowed);
    return result;
}
