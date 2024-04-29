#include "timelinelabel.h"

#include <QDebug>
#include <QAction>
#include <QMenu>

timeLineLabel::timeLineLabel(QWidget * parent) : QLabel(parent)
{
}

void timeLineLabel::mousePressEvent(QMouseEvent *ev)
{
    isCho=true;
    //qDebug()<<"true";
    //qDebug()<<"index="<<myIndex;
    emit clicked(ev);
}

