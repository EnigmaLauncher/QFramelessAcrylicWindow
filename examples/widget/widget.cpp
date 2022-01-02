/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "widget.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdatetime.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include "../../src/utilities.h"
#include "../../src/qtacrylicwidget.h"
#include "../../src/qtacrylicutilities.h"
#include "../../src/framelesswindowsmanager.h"

FRAMELESSHELPER_USE_NAMESPACE

static const QColor systemLightColor = QStringLiteral("#f0f0f0");
static const QColor systemDarkColor = QColor::fromRgb(32, 32, 32);

static constexpr char mainStyleSheet[] = R"(
)";

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();
    setupUi();
    m_globalWidget->setTintOpacity(-1);
    m_titleBarWidget->setTintOpacity(-1);
    startTimer(500);
}

Widget::~Widget() = default;

void Widget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    static bool inited = false;
    if (!inited) {
        inited = true;
        QWindow *win = windowHandle();
        Q_ASSERT(win);
        if (!win) {
            qFatal("Failed to retrieve the window handle.");
            return;
        }
        FramelessWindowsManager::addWindow(win);
        FramelessWindowsManager::setHitTestVisible(win, m_minimizeButton, true);
        FramelessWindowsManager::setHitTestVisible(win, m_maximizeButton, true);
        FramelessWindowsManager::setHitTestVisible(win, m_closeButton, true);
        const int margin = Utilities::getWindowVisibleFrameBorderThickness(winId());
        setContentsMargins(margin, margin, margin, margin);
    }
}

void Widget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (m_clockLabel) {
        m_clockLabel->setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    }

}

void Widget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    bool shouldUpdate = false;
    if (event->type() == QEvent::WindowStateChange) {
        const int margin = ((isMaximized() || isFullScreen()) ? 0 : Utilities::getWindowVisibleFrameBorderThickness(winId()));
        setContentsMargins(margin, margin, margin, margin);
        updateSystemButtonIcons();
        updateTitleBarSize();
        shouldUpdate = true;
    } else if (event->type() == QEvent::ActivationChange) {
        shouldUpdate = true;
    }
    if (shouldUpdate) {
        updateStyleSheet();
    }
}

void Widget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    if (m_titleBarWidget && _qam::Utilities::shouldUseWallpaperBlur()) {
        m_titleBarWidget->update();
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (!isMaximized() && !isFullScreen()) {
        QPainter painter(this);
        painter.save();
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        const auto w = static_cast<qreal>(width());
        const auto h = static_cast<qreal>(height());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        using BorderLines = QList<QLineF>;
#else
        using BorderLines = QVector<QLineF>;
#endif
        const BorderLines lines = {
            {0, 0, w, 0},
            {w, 0, w , h},
            {w, h, 0, h},
            {0, h, 0, 0}
        };
        const ColorizationArea area = Utilities::getColorizationArea();
        const bool colorizedBorder = ((area == ColorizationArea::TitleBar_WindowBorder)
                                      || (area == ColorizationArea::All));
        const QColor borderColor = (isActiveWindow() ? (colorizedBorder ? Utilities::getColorizationColor() : Qt::black) : Qt::darkGray);
        const auto borderThickness = static_cast<qreal>(Utilities::getWindowVisibleFrameBorderThickness(winId()));
        painter.setPen({borderColor, borderThickness});
        painter.drawLines(lines);
        painter.restore();
    }
}

void Widget::setupUi()
{
    setObjectName(QStringLiteral("MainWidget"));
    setWindowTitle(tr("Hello, World!"));
    resize(800, 600);
    m_globalWidget = new QtAcrylicWidget(this);
    const auto mainWidgetLayout = new QVBoxLayout(this);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);
    mainWidgetLayout->setSpacing(0);
    mainWidgetLayout->addWidget(m_globalWidget);
    setLayout(mainWidgetLayout);
    m_titleBarWidget = new QtAcrylicWidget(this);
    m_titleBarWidget->setObjectName(QStringLiteral("TitleBarWidget"));
    m_titleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_windowTitleLabel = new QLabel(m_titleBarWidget);
    m_windowTitleLabel->setObjectName(QStringLiteral("WindowTitleLabel"));
    m_windowTitleLabel->setFrameShape(QFrame::NoFrame);
    QFont titleFont = font();
    titleFont.setPointSize(11);
    m_windowTitleLabel->setFont(titleFont);
    m_windowTitleLabel->setText(windowTitle());
    connect(this, &Widget::windowTitleChanged, m_windowTitleLabel, &QLabel::setText);
    m_minimizeButton = new QPushButton(m_titleBarWidget);
    m_minimizeButton->setObjectName(QStringLiteral("MinimizeButton"));
    connect(m_minimizeButton, &QPushButton::clicked, this, &Widget::showMinimized);
    m_maximizeButton = new QPushButton(m_titleBarWidget);
    m_maximizeButton->setObjectName(QStringLiteral("MaximizeButton"));
    connect(m_maximizeButton, &QPushButton::clicked, this, [this](){
        if (isMaximized() || isFullScreen()) {
            showNormal();
        } else {
            showMaximized();
        }
        updateSystemButtonIcons();
    });
    m_closeButton = new QPushButton(m_titleBarWidget);
    m_closeButton->setObjectName(QStringLiteral("CloseButton"));
    connect(m_closeButton, &QPushButton::clicked, this, &Widget::close);
    updateSystemButtonIcons();
    updateTitleBarSize();
    const auto titleBarLayout = new QHBoxLayout(m_titleBarWidget);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addSpacerItem(new QSpacerItem(10, 10));
    titleBarLayout->addWidget(m_windowTitleLabel);
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(m_minimizeButton);
    titleBarLayout->addWidget(m_maximizeButton);
    titleBarLayout->addWidget(m_closeButton);
    m_titleBarWidget->setLayout(titleBarLayout);
    m_clockLabel = new QLabel(this);
    m_clockLabel->setObjectName(QStringLiteral("ClockLabel"));
    m_clockLabel->setFrameShape(QFrame::NoFrame);
    QFont clockFont = font();
    clockFont.setBold(true);
    clockFont.setPointSize(70);
    m_clockLabel->setFont(clockFont);
    const auto contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_clockLabel);
    contentLayout->addStretch();
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBarWidget);
    mainLayout->addStretch();
    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    m_globalWidget->setLayout(mainLayout);
    updateStyleSheet();
//    m_globalWidget = new QtAcrylicWidget(this);
//    m_windowTitleLabel = new QLabel(this);
//    QFont f = font();
//    f.setBold(true);
//    f.setPointSize(70);
//    m_windowTitleLabel->setFont(f);
//    const auto mainLayout = new QVBoxLayout(this);
//    mainLayout->setContentsMargins(0, 0, 0, 0);
//    mainLayout->setSpacing(0);
//    mainLayout->addWidget(m_globalWidget);
//    setLayout(mainLayout);
//    const auto labelLayout2 = new QHBoxLayout;
//    labelLayout2->setContentsMargins(0, 0, 0, 0);
//    labelLayout2->setSpacing(0);
//    labelLayout2->addStretch();
//    labelLayout2->addWidget(m_windowTitleLabel);
//    labelLayout2->addStretch();
//    const auto labelLayout1 = new QVBoxLayout(m_globalWidget);
//    labelLayout1->setContentsMargins(0, 0, 0, 0);
//    labelLayout1->setSpacing(0);
//    labelLayout1->addStretch();
//    labelLayout1->addLayout(labelLayout2);
//    labelLayout1->addStretch();
//    m_globalWidget->setLayout(labelLayout1);
}

void Widget::updateStyleSheet()
{
    const bool active = isActiveWindow();
    const bool dark = Utilities::shouldAppsUseDarkMode();
    const ColorizationArea area = Utilities::getColorizationArea();
    const bool colorizedTitleBar = ((area == ColorizationArea::TitleBar_WindowBorder)
                                    || (area == ColorizationArea::All));
    const QColor colorizationColor = Utilities::getColorizationColor();
    const QColor mainWidgetBackgroundColor = (dark ? systemDarkColor : systemLightColor);
    const QColor titleBarWidgetBackgroundColor = [active, colorizedTitleBar, &colorizationColor, dark]{
        if (active) {
            if (colorizedTitleBar) {
                return colorizationColor;
            } else {
                if (dark) {
                    return QColor(Qt::black);
                } else {
                    return QColor(Qt::white);
                }
            }
        } else {
            if (dark) {
                return systemDarkColor;
            } else {
                return QColor(Qt::white);
            }
        }
    }();
    const QColor windowTitleLabelTextColor = (active ? (dark ? Qt::white : Qt::black) : Qt::darkGray);
    const QColor clockLabelTextColor = (dark ? Qt::white : Qt::black);
    setStyleSheet(QString::fromUtf8(mainStyleSheet)
                  .arg(mainWidgetBackgroundColor.name(),
                       titleBarWidgetBackgroundColor.name(),
                       windowTitleLabelTextColor.name(),
                       clockLabelTextColor.name()));
    update();
}

void Widget::updateTitleBarSize()
{
    const QWindow *win = windowHandle();
    Q_ASSERT(win);
    if (!win) {
        return;
    }
    const int titleBarHeight = Utilities::getSystemMetric(win, SystemMetric::TitleBarHeight, false);
    const QSize systemButtonSize = {qRound(static_cast<qreal>(titleBarHeight) * 1.5), titleBarHeight};
    m_minimizeButton->setFixedSize(systemButtonSize);
    m_minimizeButton->setIconSize(systemButtonSize);
    m_maximizeButton->setFixedSize(systemButtonSize);
    m_maximizeButton->setIconSize(systemButtonSize);
    m_closeButton->setFixedSize(systemButtonSize);
    m_closeButton->setIconSize(systemButtonSize);
    m_titleBarWidget->setFixedHeight(titleBarHeight);
}

void Widget::updateSystemButtonIcons()
{
    Q_ASSERT(m_minimizeButton);
    Q_ASSERT(m_maximizeButton);
    Q_ASSERT(m_closeButton);
    if (!m_minimizeButton || !m_maximizeButton || !m_closeButton) {
        return;
    }
    const QString suffix = (Utilities::shouldAppsUseDarkMode() ? QStringLiteral("white") : QStringLiteral("black"));
    m_minimizeButton->setIcon(QIcon(QStringLiteral(":/images/button_minimize_%1.svg").arg(suffix)));
    if (isMaximized() || isFullScreen()) {
        m_maximizeButton->setIcon(QIcon(QStringLiteral(":/images/button_restore_%1.svg").arg(suffix)));
    } else {
        m_maximizeButton->setIcon(QIcon(QStringLiteral(":/images/button_maximize_%1.svg").arg(suffix)));
    }
    m_closeButton->setIcon(QIcon(QStringLiteral(":/images/button_close_%1.svg").arg(suffix)));
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool Widget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    if (message) {
        if (Utilities::isThemeChanged(message)) {
            updateStyleSheet();
            updateSystemButtonIcons();
            return true;
        }
        QPointF pos = {};
        if (Utilities::isSystemMenuRequested(message, &pos)) {
            if (Utilities::showSystemMenu(winId(), pos)) {
                return true;
            } else {
                qWarning() << "Failed to display the system menu.";
            }
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}
