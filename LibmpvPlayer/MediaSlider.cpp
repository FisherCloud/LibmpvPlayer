#include "MediaSlider.h"
#include <QMouseEvent>

MediaSlider::MediaSlider(QWidget* parent)
    : QSlider(parent)
{
}

MediaSlider::~MediaSlider()
{
}

void MediaSlider::mousePressEvent(QMouseEvent* event)
{
    QSlider::mousePressEvent(event);
    if (Qt::LeftButton == event->button() && rect().contains(event->pos())) {
        const double _wid = event->x();
        const double _ran = maximum() - minimum();
        const double _val = _wid * _ran / width() + 0.5;
        emit sliderClicked(_val);
    }
}
