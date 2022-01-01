#include "framelesswindowsmanager.h"
#include "qtacrylicutilities.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindow.h>
#include <QtCore/qdebug.h>
#include <QtGui/qpainter.h>

#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
#include "framelesshelper.h"
#else
#include <QtGui/qscreen.h>
#include "framelesshelper_win32.h"
#endif
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
Q_GLOBAL_STATIC(FramelessHelper, framelessHelperUnix)
#endif

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (!QCoreApplication::testAttribute(Qt::AA_DontCreateNativeWidgetSiblings)) {
        //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelperUnix()->removeWindowFrame(window);
#else
    FramelessHelperWin::addFramelessWindow(window);
    // Work-around a Win32 multi-monitor bug.
    QObject::connect(window, &QWindow::screenChanged, [window](QScreen *screen){
        Q_UNUSED(screen);
        window->resize(window->size());
    });
#endif
}

void FramelessWindowsManager::setHitTestVisible(QWindow *window, QObject *object, const bool value)
{
    Q_ASSERT(window);
    Q_ASSERT(object);
    if (!window || !object) {
        return;
    }
    if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
        qWarning() << object << "is not a QWidget or QQuickItem.";
        return;
    }
    auto objList = qvariant_cast<QObjectList>(window->property(Constants::kHitTestVisibleFlag));
    if (value) {
        if (objList.isEmpty() || !objList.contains(object)) {
            objList.append(object);
        }
    } else {
        if (!objList.isEmpty() && objList.contains(object)) {
            objList.removeAll(object);
        }
    }
    window->setProperty(Constants::kHitTestVisibleFlag, QVariant::fromValue(objList));
}

int FramelessWindowsManager::getResizeBorderThickness(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return 8;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    const int value = window->property(Constants::kResizeBorderThicknessFlag).toInt();
    return value <= 0 ? 8 : value;
#else
    return Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, false);
#endif
}

void FramelessWindowsManager::setResizeBorderThickness(QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window || (value <= 0)) {
        return;
    }
    window->setProperty(Constants::kResizeBorderThicknessFlag, value);
}

int FramelessWindowsManager::getTitleBarHeight(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return 31;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    const int value = window->property(Constants::kTitleBarHeightFlag).toInt();
    return value <= 0 ? 31 : value;
#else
    return Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, false);
#endif
}

void FramelessWindowsManager::setTitleBarHeight(QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window || (value <= 0)) {
        return;
    }
    window->setProperty(Constants::kTitleBarHeightFlag, value);
}

bool FramelessWindowsManager::getResizable(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    return !window->property(Constants::kWindowFixedSizeFlag).toBool();
#else
    return !Utilities::isWindowFixedSize(window);
#endif
}

void FramelessWindowsManager::setResizable(QWindow *window, const bool value)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    window->setProperty(Constants::kWindowFixedSizeFlag, !value);
#else
    window->setFlag(Qt::MSWindowsFixedSizeDialogHint, !value);
#endif
}

void FramelessWindowsManager::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelperUnix()->bringBackWindowFrame(window);
#else
    FramelessHelperWin::removeFramelessWindow(window);
#endif
}

bool FramelessWindowsManager::isWindowFrameless(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    return window->property(Constants::kFramelessModeFlag).toBool();
}

FRAMELESSHELPER_END_NAMESPACE
