#include "timeline.h"

#include <QDebug>

timeLine::timeLine(QWidget *parent) :
    QWidget(parent)
{

}

void timeLine::paintEvent(QPaintEvent*event)
{
    Q_UNUSED(event);

    QPen pen;
    pen.setColor(QColor(0,0,0,255));
    QPainter painter(this);
    painter.setPen(pen);
    painter.drawLine(QPoint(0,0),QPoint(9999,0));

    for(int i=0;i<500;i++){
        if(i%5==0){
            painter.drawLine(QPoint(20*i,0),QPoint(20*i,20));
            int tmm,tss;
            tmm=i/60;
            tss=i-tmm*60;
            QString timeLineTime=QString("%1:%2").arg(tmm).arg(tss);
            painter.drawText(20*i-20,23,50,20,Qt::AlignCenter,timeLineTime);
            //qDebug()<<i;
        }
        else{
            painter.drawLine(QPoint(20*i,0),QPoint(20*i,10));
            //qDebug()<<i;
        }
    }
}

timeLine::~timeLine()
{

}
