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

#pragma once

#include <QtWidgets/qwidget.h>


QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_END_NAMESPACE

class QtAcrylicWidget;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void showEvent(QShowEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif

private:
    void setupUi();
    void updateStyleSheet();
    void updateTitleBarSize();
    void updateSystemButtonIcons();

private:
    QtAcrylicWidget *m_globalWidget = nullptr;
    QtAcrylicWidget *m_titleBarWidget = nullptr;
    QLabel *m_windowTitleLabel = nullptr;
    QPushButton *m_minimizeButton = nullptr;
    QPushButton *m_maximizeButton = nullptr;
    QPushButton *m_closeButton = nullptr;
    QLabel *m_clockLabel = nullptr;
};
