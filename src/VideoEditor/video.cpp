#include "video.h"

video::video()
{

}

video::video(QStringList filepath)
{
    videoFileNames=filepath;
    videoFilePaths=filepath;
}

video::video(QString filepath)
{
    videoFileNames.append(filepath);
    videoFilePaths.append(filepath);
}


