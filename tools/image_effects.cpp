#include "image_effects.h"

ImageEffects::ImageEffects(QObject *parent)
    : QObject{parent}
    , m_target(nullptr)
    , m_source(QPixmap())
    , m_type(TypeOfEffect::FADEIN)
    , m_duration(1000)
    , m_timer(new QTimer(this))
{
    QObject::connect(m_timer, &QTimer::timeout, this, &ImageEffects::fadeInTimeout);
}

ImageEffects::ImageEffects(QLabel* target, const QPixmap &source, TypeOfEffect type, QObject *parent, double duration)
    : QObject{parent}
    , m_target(target)
    , m_source(source)
    , m_type(type)
    , m_duration(duration)
    , m_timer(new QTimer(this))
{
    setType(type);
}

ImageEffects::~ImageEffects()
{

}

void ImageEffects::run()
{
    if (m_target == nullptr || m_source.isNull() || qFuzzyIsNull(m_duration)) {
        emit effectFinished(false);
        return;
    }

    m_targetSize = m_target->size();
    m_source = m_source.scaled(m_targetSize, Qt::KeepAspectRatio);
    m_scaledSourceSize = m_source.size();
    m_startingPoint = QPoint((m_targetSize.width() - m_scaledSourceSize.width()) / 2, (m_targetSize.height() - m_scaledSourceSize.height()) / 2);
    m_counter = 0;
    m_percentagPerTimeout = 1.0 * TIMEOUTMSECS / m_duration;

    if (m_target->pixmap().isNull()) {
        m_unfinishedPixmap = QPixmap(m_targetSize);
        m_unfinishedPixmap.fill(Qt::transparent);
    }
    else {
        m_unfinishedPixmap = m_target->pixmap();
    }

    m_timer->start(TIMEOUTMSECS);
}

QPaintDevice *ImageEffects::target() const
{
    return m_target;
}

void ImageEffects::setTarget(QLabel *newTarget)
{
    m_target = newTarget;
}

const QPixmap &ImageEffects::source() const
{
    return m_source;
}

void ImageEffects::setSource(const QPixmap &newSource)
{
    m_source = newSource;
}

ImageEffects::TypeOfEffect ImageEffects::type() const
{
    return m_type;
}

void ImageEffects::setType(TypeOfEffect newType)
{
    switch (m_type) {
    case ImageEffects::TypeOfEffect::GROW:
        QObject::disconnect(m_timer, &QTimer::timeout, this, &ImageEffects::growTimeout);
        break;
    case ImageEffects::TypeOfEffect::FADEIN:
        QObject::disconnect(m_timer, &QTimer::timeout, this, &ImageEffects::fadeInTimeout);
        break;
    default:
        break;
    }
    switch (newType) {
    case ImageEffects::TypeOfEffect::GROW:
        QObject::connect(m_timer, &QTimer::timeout, this, &ImageEffects::growTimeout);
        break;
    case ImageEffects::TypeOfEffect::FADEIN:
        QObject::connect(m_timer, &QTimer::timeout, this, &ImageEffects::fadeInTimeout);
        break;
    }
    m_type = newType;
}

double ImageEffects::duration() const
{
    return m_duration;
}

void ImageEffects::setDuration(double newDuration)
{
    m_duration = newDuration;
}
void ImageEffects::growTimeout()
{
    if (m_percentagPerTimeout * m_counter >= 1.0) {
        m_timer->stop();
        emit effectFinished(true);
        return;
    }
    QPainter painter(&m_unfinishedPixmap);
    QPixmap scaledPixmap = m_source.scaled(m_scaledSourceSize * m_percentagPerTimeout * m_counter, Qt::KeepAspectRatio);
    QPoint offset((m_scaledSourceSize.width() - scaledPixmap.width()) / 2, (m_scaledSourceSize.height() - scaledPixmap.height()) / 2);
    painter.drawPixmap(QRect(m_startingPoint + offset, scaledPixmap.size()), scaledPixmap, scaledPixmap.rect());
    m_target->setPixmap(m_unfinishedPixmap);
    ++m_counter;
}

void ImageEffects::fadeInTimeout()
{
    if (m_percentagPerTimeout * m_counter >= 1.0) {
        m_timer->stop();
        emit effectFinished(true);
        return;
    }
    QPainter painter(&m_unfinishedPixmap);

    //No idea why, but the pixmap is fully opaque at a value of 0.1, and not 1.0 as stated in the documentation

    painter.setOpacity(m_percentagPerTimeout * m_counter / 10);
    painter.drawPixmap(QRect(m_startingPoint, m_source.size()), m_source, m_source.rect());
    m_target->setPixmap(m_unfinishedPixmap);
    ++m_counter;
}
