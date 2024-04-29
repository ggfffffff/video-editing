#ifndef VIDEO_H
#define VIDEO_H

#include "timelinelabel.h"

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QPushButton>

class video
{
public:
    video();
    video(QStringList filepath);
    video(QString filepath);

    QStringList videoFileNames; // 视频名称（可变）
    QStringList videoFilePaths; // 视频路径（不可变）

    //timeLineLabel *myVideoTimeLine; // 视频时间轴

    int videoPlayTime; // 视频时长
};

#endif // VIDEO_H
