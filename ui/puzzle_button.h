#ifndef PUZZLE_BUTTON_H
#define PUZZLE_BUTTON_H

#include "puzzle_label.h"
#include <QMouseEvent>

/*
 * The JigsawButton class is derived from JigsawLabel and adds some of the functionality of a QPushButton. In addition, there
 * are som simple animations you can use when the cursor enters the JigsawButton's area.
 */

class PuzzleButton : public PuzzleLabel
{
    Q_OBJECT
public:
    enum class AnimationType {
        DARKERONENTER,
        LIGHTERONENTER,
        BLACKTEXTONENTER,
        WHITETEXTONENTER,
        DARKERTEXTONENTER,
        LIGHTERTEXTONENTER,
        CUSTOMCOLORTEXTONENTER
    };

    explicit PuzzleButton(QWidget* parent = nullptr);
    explicit PuzzleButton(const QPixmap &background, QWidget* parent = nullptr, const QString &text = "", QRect textarea = QRect());
    explicit PuzzleButton(const QSize &size, const QBrush &background, const QPainterPath &jigsawPath, QWidget* parent = nullptr, const QString &text = "", QRect textarea = QRect());
    ~ PuzzleButton();

    void animate(bool val = true);
    void setAnimationTypes(const QSet<AnimationType> &newAnimationTypes);
    void setCustomTextColor(const QColor &newCustomTextColor);

public slots:
    void setTextColorToBlack();
    void setTextColorToWhite();
    void setTextColor(const QColor &color);
    void setTextColorLighter();
    void setTextColorDarker();
    void resetTextColor();

    void setPixmapLighter();
    void setPixmapDarker();
    void resetPixmap();

signals:
    void clicked();
    void leftClicked();
    void rightClicked();
    void doubleClicked();
    void released();
    void entered();
    void left();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    QColor m_textColorTemp;
    QColor m_customTextColor;
    QPixmap m_pixmapTemp;
    Mode m_modeTemp;
    bool m_animate;
    QSet<PuzzleButton::AnimationType> m_animationTypes;

    void solveAnimationConflicts();
    void applyAnimation();
};

#endif // PUZZLE_BUTTON_H
