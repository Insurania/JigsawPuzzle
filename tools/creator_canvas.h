#ifndef CREATOR_CANVAS_H
#define CREATOR_CANVAS_H

#include <QLabel>
#include <QMouseEvent>

class CreatorCanvas : public QLabel
{
    Q_OBJECT
public:
    CreatorCanvas(QWidget* parent = nullptr);
    ~CreatorCanvas();

signals:
    void clicked(const QPoint &position);
    void released();
    void dragged(const QPoint &position);

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // CREATOR_CANVAS_H
