#include "playerslider.h"

PlayerSlider::PlayerSlider(QWidget * parent) : QSlider(parent)
{
    isPressed = false;
}

void PlayerSlider::mousePressEvent(QMouseEvent *e)
{
    isPressed = true;
    QSlider::mousePressEvent(e);
}

void PlayerSlider::mouseMoveEvent(QMouseEvent *e)
{
    QSlider::mouseMoveEvent(e);
}

void PlayerSlider::mouseReleaseEvent(QMouseEvent *e)
{
    isPressed = false;
    qint64 i64Pos = value();
    emit sigProgress(i64Pos);

    QSlider::mouseReleaseEvent(e);
}

void PlayerSlider::setProgress(qint64 i64Progress)
{
    if(!isPressed){
        setValue(i64Progress);
    }
}
