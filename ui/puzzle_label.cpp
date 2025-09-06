#include "puzzle_label.h"

const QString &PuzzleLabel::text() const
{
    return m_text;
}

void PuzzleLabel::setText(const QString &newText)
{
    m_text = newText;
    redraw();
}

const QPixmap &PuzzleLabel::pixmap() const
{
    return m_pixmap;
}

void PuzzleLabel::setPixmap(const QPixmap &newPixmap, bool resize, QRect newTextArea)
{
    m_pixmap = newPixmap;
    m_mode = PuzzleLabel::Mode::PIXMAP;
    if (resize) {
        m_originalSize = newPixmap.size();
        setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());
    }
    if (!newTextArea.isNull()) {
        m_textArea = newTextArea;
    }
    else if (resize) {
        m_textArea = QRectF(QPoint(0, 0), m_originalSize).toRect();
    }
    redraw();
}

QPainterPath PuzzleLabel::jigsawPath() const
{
    return m_jigsawPath;
}

void PuzzleLabel::setJigsawPath(const QPainterPath &newJigsawPath, const QSize &newSize, const QBrush &newBrush)
{
    m_brush = newBrush;
    setJigsawPath(newJigsawPath, newSize);
}

void PuzzleLabel::setJigsawPath(const QPainterPath &newJigsawPath, const QSize &newSize)
{
    m_originalSize = newSize;
    setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());
    setJigsawPath(newJigsawPath);
}

void PuzzleLabel::setJigsawPath(const QPainterPath &newJigsawPath)
{
    m_jigsawPath = newJigsawPath;
    m_mode = PuzzleLabel::Mode::BRUSH;
    redraw();
}

const QRect &PuzzleLabel::textArea() const
{
    return m_textArea;
}

void PuzzleLabel::setTextArea(const QRect &newTextArea)
{
    m_textArea = newTextArea;
    redraw();
}

const QPen &PuzzleLabel::borderPen() const
{
    return m_borderPen;
}

void PuzzleLabel::setBorderPen(const QPen &newBorderPen)
{
    m_borderPen = newBorderPen;
    redraw();
}

const QFont &PuzzleLabel::font() const
{
    return m_font;
}

void PuzzleLabel::setFont(const QFont &newFont)
{
    m_font = newFont;
    redraw();
}

const QColor &PuzzleLabel::textColor() const
{
    return m_textColor;
}

void PuzzleLabel::setTextColor(const QColor &newTextColor)
{
    m_textColor = newTextColor;
    redraw();
}

const QFlags<Qt::AlignmentFlag> &PuzzleLabel::alignment() const
{
    return m_alignment;
}

void PuzzleLabel::setAlignment(const QFlags<Qt::AlignmentFlag> &newAlignment)
{
    m_alignment = newAlignment;
    redraw();
}

void PuzzleLabel::move(const QPointF &pos)
{
    QLabel::move(pos.toPoint());
    m_originalPosition = pos;
}

void PuzzleLabel::move(double x, double y)
{
    QLabel::move(x, y);
    m_originalPosition = QPointF(x, y);
}

QPointF PuzzleLabel::originalPosition() const
{
    return m_originalPosition;
}

void PuzzleLabel::redraw()
{
    setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());

    QPixmap pixmap(m_originalSize.toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter;

    switch (m_mode) {
    case PuzzleLabel::Mode::PIXMAP:
        pixmap = m_pixmap;
        painter.begin(&pixmap);
        break;
    case PuzzleLabel::Mode::BRUSH:
        painter.begin(&pixmap);
        painter.setPen(m_borderPen);
        painter.setBrush(m_brush);
        painter.drawPath(m_jigsawPath);
        break;
    default:
        return;
    }

    painter.setPen(m_textColor);
    painter.setFont(m_font);
    painter.drawText(m_textArea, m_alignment, m_text);

    QLabel::setPixmap(pixmap);
    setMask(pixmap.mask());
}

PuzzleLabel::PuzzleLabel(QWidget *parent)
    : QLabel{parent}
{
    m_originalPosition = QPointF(0.0, 0.0);
    m_originalSize = QSizeF(DEFAULTWIDTH, DEFAULTHEIGHT);
    setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());
    m_text = "";
    m_pixmap = QPixmap(QSize(DEFAULTWIDTH, DEFAULTHEIGHT));
    m_pixmap.fill(Qt::transparent);
    m_brush = QBrush();
    m_jigsawPath = QPainterPath();
    m_mode = PuzzleLabel::Mode::PIXMAP;
    m_textArea = QRect(QPoint(0, 0), m_pixmap.size());
    m_borderPen = QPen();
    m_font = QFont();
    m_textColor = Qt::black;
    m_alignment = Qt::AlignCenter;
    redraw();
}

PuzzleLabel::PuzzleLabel(const QPixmap &background, QWidget *parent, const QString &text, QRect textarea)
    : QLabel{parent}
{
    m_originalPosition = QPointF(0.0, 0.0);
    m_originalSize = QSizeF(background.size());
    setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());
    m_text = text;
    m_pixmap = background;
    m_brush = QBrush();
    m_jigsawPath = QPainterPath();
    m_mode = PuzzleLabel::Mode::PIXMAP;
    m_textArea = textarea == QRect() ? QRect(QPoint(0, 0), m_pixmap.size()) : textarea;
    m_borderPen = QPen();
    m_font = QFont();
    m_textColor = Qt::black;
    m_alignment = Qt::AlignCenter;
    redraw();
}

PuzzleLabel::PuzzleLabel(const QSize &size, const QBrush &background, const QPainterPath &jigsawPath, QWidget *parent, const QString &text, QRect textarea)
    : QLabel{parent}
{
    m_originalPosition = QPointF(0.0, 0.0);
    m_originalSize = size;
    setGeometry(QRectF(m_originalPosition, m_originalSize).toRect());
    m_text = text;
    m_pixmap = QPixmap(size);
    m_brush = background;
    m_jigsawPath = jigsawPath;
    m_mode = PuzzleLabel::Mode::BRUSH;
    m_textArea = textarea == QRect() ? QRect(QPoint(0, 0), m_pixmap.size()) : textarea;
    m_borderPen = QPen();
    m_font = QFont();
    m_textColor = Qt::black;
    m_alignment = Qt::AlignCenter;
    redraw();
}

PuzzleLabel::~PuzzleLabel()
{

}
