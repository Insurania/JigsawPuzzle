#include "puzzle_slider.h"

PuzzleSlider::PuzzleSlider(QWidget *parent)
    : PuzzleButton{parent}
    , m_backgroundLabel(new PuzzleLabel(parent))
    , m_sliderBrush(Qt::gray)
    , m_overallRect(QRect(0, 0, PuzzleLabel::pixmap().width() / SIZEPROPORTIONBUTTON, PuzzleLabel::pixmap().height()))
    , m_val(0)
    , m_minVal(0)
    , m_maxVal(100)
    , m_orientation(PuzzleSlider::Orientation::HORIZONTAL)
    , m_cursorOffset(QPoint(0, 0))
{
    paintBackgroundLabel();
    setGeometry(m_overallRect);
}

PuzzleSlider::PuzzleSlider(const QPixmap &backgroundButton, const QBrush &backgroundSlider, QWidget *parent, int val, int minVal, int maxVal, Orientation orientation)
    : PuzzleButton{backgroundButton, parent, QString::number(val)}
    , m_backgroundLabel(new PuzzleLabel(parent))
    , m_sliderBrush(backgroundSlider)
    , m_overallRect(orientation == PuzzleSlider::Orientation::HORIZONTAL
                    ? QRect(0, 0, backgroundButton.width() / SIZEPROPORTIONBUTTON, backgroundButton.height())
                    : QRect(0, 0, backgroundButton.width(), backgroundButton.height() / SIZEPROPORTIONBUTTON))
    , m_val(val)
    , m_minVal(minVal)
    , m_maxVal(maxVal)
    , m_orientation(orientation)
    , m_cursorOffset(QPoint(0, 0))
{
    paintBackgroundLabel();
    setGeometry(m_overallRect);
}

PuzzleSlider::PuzzleSlider(const QSize &sizeButton, const QBrush &backgroundButton, const QPainterPath &jigsawPathButton, const QBrush &backgroundSlider, QWidget *parent, int val, int minVal, int maxVal, Orientation orientation)
    : PuzzleButton{sizeButton, backgroundButton, jigsawPathButton, parent, QString::number(val)}
    , m_backgroundLabel(new PuzzleLabel(parent))
    , m_sliderBrush(backgroundSlider)
    , m_overallRect(orientation == PuzzleSlider::Orientation::HORIZONTAL
                    ? QRect(0, 0, sizeButton.width() / SIZEPROPORTIONBUTTON, sizeButton.height())
                    : QRect(0, 0, sizeButton.width(), sizeButton.height() / SIZEPROPORTIONBUTTON))
    , m_val(val)
    , m_minVal(minVal)
    , m_maxVal(maxVal)
    , m_orientation(orientation)
    , m_cursorOffset(QPoint(0, 0))
{
    paintBackgroundLabel();
    setGeometry(m_overallRect);
}

PuzzleSlider::~PuzzleSlider()
{

}


void PuzzleSlider::setGeometry(int x, int y, int w, int h)
{
    QRect rect(x, y, w, h);
    PuzzleSlider::setGeometry(rect);
}

void PuzzleSlider::setGeometry(const QRect &rect)
{
    int width = m_orientation == PuzzleSlider::Orientation::HORIZONTAL ? rect.width() : QLabel::width();
    int height = m_orientation == PuzzleSlider::Orientation::VERTICAL ? rect.height() : QLabel::height();

    m_overallRect = QRect(rect.topLeft(), QSize(width, height));

    paintBackgroundLabel();
    move(m_overallRect.topLeft());
}

void PuzzleSlider::move(int x, int y)
{
    QPoint point(x, y);
    PuzzleSlider::move(point);
}

void PuzzleSlider::move(const QPoint &point)
{
    m_overallRect = QRect(point, m_overallRect.size());
    m_backgroundLabel->QLabel::move(point);
    QPoint buttonPoint(point + calculateButtonPositionOffset(PuzzleLabel::pixmap().size()).toPoint());
    QLabel::move(buttonPoint);
    m_originalPosition = buttonPoint;
}

QPoint PuzzleSlider::pos() const
{
    return m_overallRect.topLeft();
}

void PuzzleSlider::raise()
{
    QWidget::raise();
    m_backgroundLabel->QWidget::stackUnder(this);
}

void PuzzleSlider::lower()
{
    QWidget::lower();
    m_backgroundLabel->QWidget::stackUnder(this);
}

void PuzzleSlider::stackUnder(QWidget *w)
{
    QWidget::stackUnder(w);
    m_backgroundLabel->QWidget::stackUnder(this);
}

void PuzzleSlider::setPixmap(const QPixmap &newPixmap)
{
    switch (m_orientation) {
    case PuzzleSlider::Orientation::HORIZONTAL:
        m_overallRect.setBottom(m_overallRect.top() + newPixmap.height());
        break;
    case PuzzleSlider::Orientation::VERTICAL:
        m_overallRect.setRight(m_overallRect.left() + newPixmap.width());
        break;
    }

    PuzzleLabel::setPixmap(newPixmap, true);
    setGeometry(m_overallRect);
}

void PuzzleSlider::setJigsawPath(const QPainterPath &newJigsawPath, const QSize &newSize, const QBrush &newBrush)
{
    m_brush = newBrush;
    setJigsawPath(newJigsawPath, newSize);
}

void PuzzleSlider::setJigsawPath(const QPainterPath &newJigsawPath, const QSize &newSize)
{
    switch (m_orientation) {
    case PuzzleSlider::Orientation::HORIZONTAL:
        m_overallRect.setBottom(m_overallRect.top() + newSize.height());
        break;
    case PuzzleSlider::Orientation::VERTICAL:
        m_overallRect.setRight(m_overallRect.left() + newSize.width());
        break;
    }
    PuzzleLabel::setJigsawPath(newJigsawPath, newSize);
    setGeometry(m_overallRect);
}

void PuzzleSlider::setJigsawPath(const QPainterPath &newJigsawPath)
{
    PuzzleLabel::setJigsawPath(newJigsawPath);
    setGeometry(m_overallRect);
}

int PuzzleSlider::val() const
{
    return m_val;
}

void PuzzleSlider::setVal(int newVal)
{
    if (newVal < m_minVal) newVal = m_minVal;
    if (newVal > m_maxVal) newVal = m_maxVal;
    m_val = newVal;
    setGeometry(m_overallRect);
    setText(QString::number(newVal));
}

int PuzzleSlider::minVal() const
{
    return m_minVal;
}

void PuzzleSlider::setMinVal(int newMinVal)
{
    if (m_val < newMinVal) m_val = newMinVal;
    m_minVal = newMinVal;
    setGeometry(m_overallRect);
}

int PuzzleSlider::maxVal() const
{
    return m_maxVal;
}

void PuzzleSlider::setMaxVal(int newMaxVal)
{
    if (m_val > newMaxVal) m_val = newMaxVal;
    m_maxVal = newMaxVal;
    setGeometry(m_overallRect);
}

const QBrush &PuzzleSlider::sliderBrush() const
{
    return m_sliderBrush;
}

void PuzzleSlider::setSliderBrush(const QBrush &newSliderBrush)
{
    m_sliderBrush = newSliderBrush;
    paintBackgroundLabel();
}

PuzzleSlider::Orientation PuzzleSlider::orientation() const
{
    return m_orientation;
}

void PuzzleSlider::setOrientation(Orientation newOrientation)
{
    m_orientation = newOrientation;
    QRect newRect(m_overallRect.topLeft(), QSize(m_overallRect.height(), m_overallRect.width()));
    setGeometry(newRect);
}

void PuzzleSlider::paintBackgroundLabel()
{
    QPixmap background(m_overallRect.size());
    background.fill(Qt::transparent);
    QPainter painter(&background);
    painter.setPen(QPen(Qt::black, SLIDERPENWIDTH));
    painter.setBrush(m_sliderBrush);
    QRect paintArea = m_orientation == PuzzleSlider::Orientation::HORIZONTAL
                    ? QRect(QPoint(SLIDERPENWIDTH, m_overallRect.height() / 2 - SLIDERWIDTH / 2), QSize(m_overallRect.width() - SLIDERPENWIDTH * 2, SLIDERWIDTH))
                    : QRect(QPoint(m_overallRect.width() / 2 - SLIDERWIDTH / 2, SLIDERPENWIDTH), QSize(SLIDERWIDTH, m_overallRect.height() - SLIDERPENWIDTH * 2));
    painter.drawRoundedRect(paintArea, SLIDERRADIUS, SLIDERRADIUS);
    m_backgroundLabel->setPixmap(background, true);
    m_backgroundLabel->setMask((background.mask()));
    m_backgroundLabel->QWidget::stackUnder(this);
}

QPointF PuzzleSlider::calculateButtonPositionOffset(const QSize &buttonSize) const
{
    int distance = m_orientation == PuzzleSlider::Orientation::HORIZONTAL
                 ? m_overallRect.width() - buttonSize.width()
                 : m_overallRect.height() - buttonSize.height();
    double positionOffsetF = 1.0 * distance / (m_maxVal - m_minVal) * m_val;
    QPointF positionOffset(m_orientation == PuzzleSlider::Orientation::HORIZONTAL ? positionOffsetF : 0.0,
                           m_orientation == PuzzleSlider::Orientation::VERTICAL ? positionOffsetF : 0.0);
    return positionOffset;
}

void PuzzleSlider::mousePressEvent(QMouseEvent *event)
{
    QPoint cursorPosition = QCursor::pos();
    m_cursorOffset = QLabel::pos() - cursorPosition;
}

void PuzzleSlider::mouseMoveEvent(QMouseEvent *event)
{
    int sliderButtonMinPosition;
    int sliderButtonMaxPosition;
    int distance;
    int distanceValue = m_maxVal - m_minVal;

    QPoint cursorPosition = QCursor::pos();
    int newXPosition = cursorPosition.x() + m_cursorOffset.x();
    int newYPosition = cursorPosition.y() + m_cursorOffset.y();
    QPointF newPosition;

    double newValue;
    int newIntValue;

    switch (m_orientation) {
    case PuzzleSlider::Orientation::HORIZONTAL:
        sliderButtonMinPosition = m_overallRect.left();
        sliderButtonMaxPosition = m_overallRect.left() + m_overallRect.width() - width();
        distance = sliderButtonMaxPosition - sliderButtonMinPosition;

        newXPosition = newXPosition < sliderButtonMinPosition ? sliderButtonMinPosition : newXPosition > sliderButtonMaxPosition ? sliderButtonMaxPosition : newXPosition;
        newPosition = QPointF(newXPosition, QLabel::y());

        newValue =  1.0 * distanceValue / distance * (newXPosition - sliderButtonMinPosition) + m_minVal;
        newIntValue = round(newValue);
        break;
    case PuzzleSlider::Orientation::VERTICAL:
        sliderButtonMinPosition = m_overallRect.top();
        sliderButtonMaxPosition = m_overallRect.top() + m_overallRect.height() - height();
        distance = sliderButtonMaxPosition - sliderButtonMinPosition;

        newYPosition = newYPosition < sliderButtonMinPosition ? sliderButtonMinPosition : newYPosition > sliderButtonMaxPosition ? sliderButtonMaxPosition : newYPosition;
        newPosition = QPointF(QLabel::x(), newYPosition);

        newValue =  1.0 * distanceValue / distance * (newYPosition - sliderButtonMinPosition) + m_minVal;
        newIntValue = round(newValue);
        break;
    }

    PuzzleLabel::move(newPosition);
    setText(QString::number(newIntValue));
    m_val = newIntValue;
    emit valueChanged(newIntValue);
}
