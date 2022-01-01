#include "framelessquickhelper.h"
#include "framelesswindowsmanager.h"
#include <QtQuick/qquickwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent) : QQuickItem(parent)
{
}

qreal FramelessQuickHelper::resizeBorderThickness() const
{
    return FramelessWindowsManager::getResizeBorderThickness(window());
}

void FramelessQuickHelper::setResizeBorderThickness(const qreal val)
{
    FramelessWindowsManager::setResizeBorderThickness(window(), qRound(val));
    Q_EMIT resizeBorderThicknessChanged(val);
}

qreal FramelessQuickHelper::titleBarHeight() const
{
    return FramelessWindowsManager::getTitleBarHeight(window());
}

void FramelessQuickHelper::setTitleBarHeight(const qreal val)
{
    FramelessWindowsManager::setTitleBarHeight(window(), qRound(val));
    Q_EMIT titleBarHeightChanged(val);
}

bool FramelessQuickHelper::resizable() const
{
    return FramelessWindowsManager::getResizable(window());
}

void FramelessQuickHelper::setResizable(const bool val)
{
    FramelessWindowsManager::setResizable(window(), val);
    Q_EMIT resizableChanged(val);
}

void FramelessQuickHelper::removeWindowFrame()
{
    FramelessWindowsManager::addWindow(window());
}

void FramelessQuickHelper::bringBackWindowFrame()
{
    FramelessWindowsManager::removeWindow(window());
}

bool FramelessQuickHelper::isWindowFrameless() const
{
    return FramelessWindowsManager::isWindowFrameless(window());
}

void FramelessQuickHelper::setHitTestVisible(QQuickItem *item, const bool visible)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    FramelessWindowsManager::setHitTestVisible(window(), item, visible);
}

FRAMELESSHELPER_END_NAMESPACE
