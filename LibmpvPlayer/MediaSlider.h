#pragma once

#include <QSlider>

class MediaSlider : public QSlider {
    Q_OBJECT

public:
    MediaSlider(QWidget* parent);
    ~MediaSlider();

signals:
    void sliderClicked(int);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
};
