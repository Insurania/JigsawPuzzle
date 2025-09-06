#ifndef IMAGE_EFFECTS_H
#define IMAGE_EFFECTS_H

#include <QImage>
#include <QTimer>
#include <QObject>
#include <QPainter>
#include <QLabel>
#include <random>

/*
 * This class creates the effect for the image you see, when you finish a jigsaw puzzle. It has
 * a slot run() and a signal effectFinished(), so it is possible to run it on a different thread.
 */

class ImageEffects : public QObject
{
    Q_OBJECT

    const int TIMEOUTMSECS = 10;

public:
    enum class TypeOfEffect {
        GROW,
        FADEIN
    };

    explicit ImageEffects(QObject *parent = nullptr);
    explicit ImageEffects(QLabel *target, const QPixmap &source, TypeOfEffect type, QObject *parent = nullptr, double duration = 1000);
    ~ImageEffects();

    QPaintDevice *target() const;
    void setTarget(QLabel *newTarget);
    const QPixmap &source() const;
    void setSource(const QPixmap &newSource);
    TypeOfEffect type() const;
    void setType(TypeOfEffect newType);
    double duration() const;
    void setDuration(double newDuration);

public slots:
    void run();

private:
    QLabel* m_target;
    QPixmap m_source;
    TypeOfEffect m_type;
    double m_duration;
    QTimer* m_timer;

    QSize m_targetSize;
    QSize m_scaledSourceSize;
    QPoint m_startingPoint;
    QPixmap m_unfinishedPixmap;
    int m_counter;
    double m_percentagPerTimeout;

private slots:
    void growTimeout();
    void fadeInTimeout();

signals:
    void effectFinished(bool successful);

};

#endif // IMAGE_EFFECTS_H
