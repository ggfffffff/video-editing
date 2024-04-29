#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class timeLine : public QWidget
{
    Q_OBJECT

public:
    explicit timeLine(QWidget *parent = nullptr);    
    ~timeLine();

    // 绘制时间块
    void paintVideoTime();
    void paintImageTime();
    void paintWordTime();

protected:
     virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    // 时间轴绘制相关
    int timeLineLength; // 以两分钟为步长分段绘制
    int sumTime; // 时间轴中的视频总长
};

#endif // TIMELINE_H
