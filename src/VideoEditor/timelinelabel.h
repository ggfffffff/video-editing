#ifndef TIMELINELABEL_H
#define TIMELINELABEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>

class timeLineLabel:public QLabel
{
    Q_OBJECT

public:
    timeLineLabel(QWidget *parent=nullptr);
    QString myFilePath;
    int myIndex=0; // 视频在时间轴上的序号
    bool isCho=false;

    int effectIndex=0; // 素材的特效序号
    int imageXcoo=0; //素材x坐标
    int imageYcoo=0; //素材y坐标
    int effectTime=0; //特效持续时间
    int iStartTime=0; //素材开始时间
    int iEndTime=0; //素材结束时间

    QString wordText; // 文字素材内容

signals:
    void clicked(QMouseEvent *ev);

protected:
    void mousePressEvent(QMouseEvent *ev);

public slots:

private:

};

#endif // TIMELINELABEL_H
