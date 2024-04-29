#ifndef PLAYERSLIDER_H
#define PLAYERSLIDER_H

#include <QSlider>
#include <QMouseEvent>

// 重写进度条类

class PlayerSlider : public QSlider
{
    Q_OBJECT
public:
    PlayerSlider(QWidget * parent = 0);
    void setProgress(qint64);
signals:
    void sigProgress(qint64);
private:
    bool isPressed;
protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
};

#endif // PLAYERSLIDER_H
