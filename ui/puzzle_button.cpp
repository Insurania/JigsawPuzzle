#include "puzzle_button.h"

PuzzleButton::PuzzleButton(QWidget *parent)
    : PuzzleLabel{parent}
    , m_animate(false)
    , m_animationTypes{AnimationType::LIGHTERONENTER, AnimationType::CUSTOMCOLORTEXTONENTER}
    , m_customTextColor(Qt::darkGray)
    , m_pixmapTemp(QPixmap())
{

}

PuzzleButton::PuzzleButton(const QPixmap &background, QWidget *parent, const QString &text, QRect textarea)
    : PuzzleLabel{background, parent, text, textarea}
    , m_animate(false)
    , m_animationTypes{AnimationType::LIGHTERONENTER, AnimationType::CUSTOMCOLORTEXTONENTER}
    , m_customTextColor(Qt::darkGray)
    , m_pixmapTemp(QPixmap())
{

}

PuzzleButton::PuzzleButton(const QSize &size, const QBrush &background, const QPainterPath &jigsawPath, QWidget *parent, const QString &text, QRect textarea)
    : PuzzleLabel{size, background, jigsawPath, parent, text, textarea}
    , m_animate(false)
    , m_animationTypes{AnimationType::LIGHTERONENTER, AnimationType::CUSTOMCOLORTEXTONENTER}
    , m_customTextColor(Qt::darkGray)
    , m_pixmapTemp(QPixmap())
{

}

PuzzleButton::~PuzzleButton()
{

}

void PuzzleButton::animate(bool val)
{
    m_animate = val;
}

void PuzzleButton::setAnimationTypes(const QSet<AnimationType> &newAnimationTypes)
{
    m_animationTypes = newAnimationTypes;
    solveAnimationConflicts();
}

void PuzzleButton::setCustomTextColor(const QColor &newCustomTextColor)
{
    m_customTextColor = newCustomTextColor;
}

void PuzzleButton::setTextColorToBlack()
{
    PuzzleButton::setTextColor(Qt::black);
}

void PuzzleButton::setTextColorToWhite()
{
    PuzzleButton::setTextColor(Qt::white);
}

void PuzzleButton::setTextColor(const QColor &color)
{
    m_textColorTemp = PuzzleLabel::textColor();
    PuzzleLabel::setTextColor(color);
}

void PuzzleButton::setTextColorLighter()
{
    PuzzleButton::setTextColor(PuzzleLabel::textColor().lighter());
}

void PuzzleButton::setTextColorDarker()
{
    PuzzleButton::setTextColor(PuzzleLabel::textColor().darker());
}

void PuzzleButton::resetTextColor()
{
    if (!m_textColorTemp.isValid()) return;
    PuzzleLabel::setTextColor(m_textColorTemp);
}

void PuzzleButton::setPixmapLighter()
{
    m_modeTemp = m_mode;
    QImage image;
    switch (m_mode) {
    case PuzzleLabel::Mode::PIXMAP: {
        if (PuzzleLabel::pixmap().isNull()) return;

        m_pixmapTemp = PuzzleLabel::pixmap();
        image = m_pixmapTemp.toImage();
        break;
    }
    case PuzzleLabel::Mode::BRUSH: {
        if (PuzzleLabel::jigsawPath().isEmpty()) return;
        QPixmap pixmap(QLabel::pixmap().size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(borderPen());
        painter.setBrush(m_brush);
        painter.drawPath(jigsawPath());

        image = pixmap.toImage();
        break;
    }
    }
    for (int i = 0; i < image.height(); ++i) {
        for (int j = 0; j < image.width(); ++j) {
            QColor color = image.pixelColor(j, i);
            if (color == Qt::transparent) continue;
            color = color.lighter();
            image.setPixelColor(j, i, color);
        }
    }
    PuzzleLabel::setPixmap(QPixmap::fromImage(image), true);
}

void PuzzleButton::setPixmapDarker()
{
    m_modeTemp = m_mode;
    QImage image;
    switch (m_mode) {
    case PuzzleLabel::Mode::PIXMAP: {
        if (PuzzleLabel::pixmap().isNull()) return;

        m_pixmapTemp = PuzzleLabel::pixmap();
        image = m_pixmapTemp.toImage();
        break;
    }
    case PuzzleLabel::Mode::BRUSH: {
        if (PuzzleLabel::jigsawPath().isEmpty()) return;
        QPixmap pixmap(QLabel::pixmap().size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(borderPen());
        painter.setBrush(m_brush);
        painter.drawPath(jigsawPath());

        image = pixmap.toImage();
        break;
    }
    }
    for (int i = 0; i < image.height(); ++i) {
        for (int j = 0; j < image.width(); ++j) {
            QColor color = image.pixelColor(j, i);
            if (color == Qt::transparent) continue;
            color = color.darker();
            image.setPixelColor(j, i, color);
        }
    }
    PuzzleLabel::setPixmap(QPixmap::fromImage(image), false);
}

void PuzzleButton::resetPixmap()
{
    switch (m_modeTemp) {
    case (Mode::PIXMAP):
        if (m_pixmapTemp.isNull()) return;
        PuzzleLabel::setPixmap(m_pixmapTemp, false);
        break;
    case(Mode::BRUSH):
        PuzzleLabel::setJigsawPath(jigsawPath());
        break;
    }
}

void PuzzleButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) emit leftClicked();
    if (event->button() == Qt::RightButton) emit rightClicked();
    emit clicked();
}

void PuzzleButton::mouseReleaseEvent(QMouseEvent *event)
{
    emit released();
}

void PuzzleButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClicked();
}

void PuzzleButton::enterEvent(QEnterEvent *event)
{
    emit entered();
    if (!m_animate) return;
    applyAnimation();
}

void PuzzleButton::leaveEvent(QEvent *event)
{
    emit left();
    if (!m_animate) return;
    resetTextColor();
    resetPixmap();
}

/*
 * Some animations can't be used at the same time, so every time the types are set, this function is called to eliminate
 * the conflicts. It should be obvious, which animations can't be paired, so there is no complicated logic, which animations
 * are deleted from the set and which are not.
 */

void PuzzleButton::solveAnimationConflicts()
{
    if (m_animationTypes.contains(AnimationType::LIGHTERONENTER)) {
        m_animationTypes.remove(AnimationType::DARKERONENTER);
    }
    if (m_animationTypes.contains(AnimationType::DARKERONENTER)) {
        m_animationTypes.remove(AnimationType::LIGHTERONENTER);
    }
    if (m_animationTypes.contains(AnimationType::BLACKTEXTONENTER)) {
        m_animationTypes.remove(AnimationType::WHITETEXTONENTER);
        m_animationTypes.remove(AnimationType::LIGHTERTEXTONENTER);
        m_animationTypes.remove(AnimationType::DARKERTEXTONENTER);
        m_animationTypes.remove(AnimationType::CUSTOMCOLORTEXTONENTER);
    }
    if (m_animationTypes.contains(AnimationType::WHITETEXTONENTER)) {
        m_animationTypes.remove(AnimationType::BLACKTEXTONENTER);
        m_animationTypes.remove(AnimationType::LIGHTERTEXTONENTER);
        m_animationTypes.remove(AnimationType::DARKERTEXTONENTER);
        m_animationTypes.remove(AnimationType::CUSTOMCOLORTEXTONENTER);
    }
    if (m_animationTypes.contains(AnimationType::LIGHTERTEXTONENTER)) {
        m_animationTypes.remove(AnimationType::WHITETEXTONENTER);
        m_animationTypes.remove(AnimationType::BLACKTEXTONENTER);
        m_animationTypes.remove(AnimationType::DARKERTEXTONENTER);
        m_animationTypes.remove(AnimationType::CUSTOMCOLORTEXTONENTER);
    }
    if (m_animationTypes.contains(AnimationType::DARKERTEXTONENTER)) {
        m_animationTypes.remove(AnimationType::WHITETEXTONENTER);
        m_animationTypes.remove(AnimationType::LIGHTERTEXTONENTER);
        m_animationTypes.remove(AnimationType::BLACKTEXTONENTER);
        m_animationTypes.remove(AnimationType::CUSTOMCOLORTEXTONENTER);
    }
    if (m_animationTypes.contains(AnimationType::CUSTOMCOLORTEXTONENTER)) {
        m_animationTypes.remove(AnimationType::WHITETEXTONENTER);
        m_animationTypes.remove(AnimationType::LIGHTERTEXTONENTER);
        m_animationTypes.remove(AnimationType::DARKERTEXTONENTER);
        m_animationTypes.remove(AnimationType::BLACKTEXTONENTER);
    }
}

void PuzzleButton::applyAnimation()
{
    if (m_animationTypes.contains(AnimationType::LIGHTERONENTER)) setPixmapLighter();
    if (m_animationTypes.contains(AnimationType::DARKERONENTER)) setPixmapDarker();
    if (m_animationTypes.contains(AnimationType::BLACKTEXTONENTER)) setTextColorToBlack();
    if (m_animationTypes.contains(AnimationType::WHITETEXTONENTER)) setTextColorToWhite();
    if (m_animationTypes.contains(AnimationType::LIGHTERTEXTONENTER)) setTextColorLighter();
    if (m_animationTypes.contains(AnimationType::DARKERTEXTONENTER)) setTextColorDarker();
    if (m_animationTypes.contains(AnimationType::CUSTOMCOLORTEXTONENTER)) PuzzleButton::setTextColor(m_customTextColor);
}
