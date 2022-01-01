#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtGui/qguiapplication.h>

#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

QWindow *Utilities::findWindow(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == winId) {
                return window;
            }
        }
    }
    return nullptr;
}

bool Utilities::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef Q_OS_WINDOWS
    if (window->flags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
#endif
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    return false;
}

bool Utilities::isHitTestVisible(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const auto objs = qvariant_cast<QObjectList>(window->property(Constants::kHitTestVisibleFlag));
    if (objs.isEmpty()) {
        return false;
    }
    for (auto &&obj : qAsConst(objs)) {
        if (!obj || !(obj->isWidgetType() || obj->inherits("QQuickItem"))) {
            continue;
        }
        if (!obj->property("visible").toBool()) {
            continue;
        }
        const QPointF originPoint = mapOriginPointToWindow(obj);
        const qreal width = obj->property("width").toReal();
        const qreal height = obj->property("height").toReal();
        const QRectF rect = {originPoint.x(), originPoint.y(), width, height};
        if (rect.contains(QCursor::pos(window->screen()))) {
            return true;
        }
    }
    return false;
}

QPointF Utilities::mapOriginPointToWindow(const QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return {};
    }
    if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
        qWarning() << object << "is not a QWidget or a QQuickItem.";
        return {};
    }
    QPointF point = {object->property("x").toReal(), object->property("y").toReal()};
    for (QObject *parent = object->parent(); parent; parent = parent->parent()) {
        point += {parent->property("x").toReal(), parent->property("y").toReal()};
        if (parent->isWindowType()) {
            break;
        }
    }
    return point;
}

FRAMELESSHELPER_END_NAMESPACE
