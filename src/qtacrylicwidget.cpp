#include "qtacrylicwidget.h"
#include <QDebug>
#include <QPainter>
#include "qtacrylicutilities.h"

QtAcrylicWidget::QtAcrylicWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
    setBackgroundRole(QPalette::Base);
    m_acrylicHelper.showPerformanceWarning();
    m_acrylicHelper.updateAcrylicBrush();
}

QtAcrylicWidget::~QtAcrylicWidget() = default;

QColor QtAcrylicWidget::tintColor() const
{
    const QColor color = m_acrylicHelper.getTintColor();
    if (color.isValid() && (color != Qt::transparent)) {
        return color;
    } else {
        return palette().color(backgroundRole());
    }
}

void QtAcrylicWidget::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << "Tint color not valid.";
        return;
    }
    if (m_acrylicHelper.getTintColor() != value) {
        m_acrylicHelper.setTintColor(value);
        QPalette pal = palette();
        pal.setColor(backgroundRole(), m_acrylicHelper.getTintColor());
        setPalette(pal);
        //m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT tintColorChanged();
    }
}

qreal QtAcrylicWidget::tintOpacity() const
{
    return m_acrylicHelper.getTintOpacity();
}

void QtAcrylicWidget::setTintOpacity(const qreal value)
{
    if (m_acrylicHelper.getTintOpacity() != value) {
        m_acrylicHelper.setTintOpacity(value);
        m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT tintOpacityChanged();
    }
}

qreal QtAcrylicWidget::noiseOpacity() const
{
    return m_acrylicHelper.getNoiseOpacity();
}

void QtAcrylicWidget::setNoiseOpacity(const qreal value)
{
    if (m_acrylicHelper.getNoiseOpacity() != value) {
        m_acrylicHelper.setNoiseOpacity(value);
        m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT noiseOpacityChanged();
    }
}

void QtAcrylicWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const QRect rect = {mapToGlobal(QPoint{0, 0}), size()};
    m_acrylicHelper.paintBackground(&painter, rect);
    QWidget::paintEvent(event);
}

void QtAcrylicWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    if (_qam::Utilities::shouldUseWallpaperBlur()) {
        update();
    }
}

void QtAcrylicWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange) {
        m_acrylicHelper.updateAcrylicBrush();
    }
}
