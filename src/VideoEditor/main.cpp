#include "videoplayer.h"

#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayer w;
    w.showMaximized();
    w.show();

/*-----------------------------测试是否配置好ffmpeg-----------------------------*/

    return a.exec();
}
