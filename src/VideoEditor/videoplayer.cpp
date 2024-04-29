#include "videoplayer.h"
#include "ui_videoplayer.h"
#include "video.h"
#include "timelinelabel.h"

#include <QFileDialog>
#include <QLabel>
#include <iostream>
#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QPainter>
#include <QWidget>
#include <QPen>
#include <QVector>
#include <QFile>
#include <QMessageBox>

VideoPlayer::VideoPlayer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoPlayer)
{
    ui->setupUi(this);

/*----------------------------------------视频播放器----------------------------------------*/
    // 创建播放区域
    myPlayer = new QMediaPlayer;
    playerWidget = new QVideoWidget;
    myPlayer->setVideoOutput(playerWidget);
    ui->vPlayerLayout->addWidget(playerWidget);

    // 载入视频
    connect(ui->ImpV, SIGNAL(triggered()), this, SLOT(setVideoFile()));
    // 按键状态改变,对应的绑定操作写在函数里了
    connect(myPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onStateChanged(QMediaPlayer::State)));
    connect(ui->BtnPlay, SIGNAL(clicked()), myPlayer, SLOT(play()));
    connect(ui->BtnShow,SIGNAL(clicked()),this,SLOT(showVideo()));

    // 总时间获取
    connect(myPlayer,SIGNAL(durationChanged(qint64)),this,SLOT(getAllTime()));
    // 当前时间获取
    connect(ui->slider,SIGNAL(valueChanged(int)),this,SLOT(getNowTime()));

    // 创建进度条
    isReload = true;
    ui->slider->setEnabled(false);
    connect(myPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onSlider(qint64)));
    connect(myPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
    connect(ui->slider, SIGNAL(sigProgress(qint64)), myPlayer, SLOT(setPosition(qint64)));

/*----------------------------------------视频列表----------------------------------------*/
    // 双击菜单重命名or删除or添加到时间轴
    connect(ui->videoList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(videoFileList()));

    // 单击菜单加入播放列表
    connect(ui->videoList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(setMedia()));

/*----------------------------------------素材存放----------------------------------------*/

    // 设置存放界面
    ui->imageList->setViewMode(QListWidget::IconMode); // 显示模式
    ui->imageList->setSpacing(10); // 间距
    ui->imageList->setResizeMode(QListView::Adjust); // 适应布局调整
    ui->imageList->setMovement(QListView::Static); // 不能移动

    // 载入素材
    connect(ui->ImpI, SIGNAL(triggered()), this, SLOT(setImageFile()));

    // 添加文字素材
    connect(ui->BtnWord,SIGNAL(clicked()),this,SLOT(addWordDlg()));


    // 双击菜单重命名or删除or添加
    connect(ui->imageList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(imageFileList()));
    imageListMenu=new QMenu(this);
    renameI=new QAction("重命名");
    deletI=new QAction("删除");
    addI=new QAction("添加到视频");
    imageListMenu->addAction(renameI);
    imageListMenu->addAction(deletI);
    imageListMenu->addAction(addI);
    connect(renameI,SIGNAL(triggered()),this,SLOT(setImageName()));
    connect(deletI,SIGNAL(triggered()),this,SLOT(imageDel()));
    connect(addI,SIGNAL(triggered()),this,SLOT(setImageAdd()));

/*----------------------------------------视频剪辑区----------------------------------------*/
    // 创建剪辑进程
    cutPrc=new QProcess(this);

    // 剪切键关联
    connect(ui->BtnCut,SIGNAL(clicked()),this,SLOT(getCutTime()));

    // 手动设置剪切时间点
    connect(ui->setT, SIGNAL(triggered()), this, SLOT(setCutTime()));

/*----------------------------------------载入字幕----------------------------------------*/
    connect(ui->ImpS, SIGNAL(triggered()), this, SLOT(setSubtitleFile()));

/*----------------------------------------载入字幕----------------------------------------*/
    connect(ui->output, SIGNAL(triggered()), this, SLOT(setOutputName()));

}

VideoPlayer::~VideoPlayer()
{
    delete myPlayer;
    delete playerWidget;
    delete ui;
}

/*----------------------------------------视频播放相关函数----------------------------------------*/
// 选择文件
void VideoPlayer::setVideoFile(void)
{

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec()){
        // 获取视频的路径
        video newVideo(dialog.selectedFiles());
        myVideo.append(newVideo);

        // 添加到播放列表
        ui->videoList->addItem(myVideo[myVideo.size()-1].videoFilePaths[0]);
        ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    }
}

// 播放
void VideoPlayer::setMedia()
{
    indexSelected = ui->videoList->currentRow();
    myPlayer->setMedia(QUrl::fromLocalFile(myVideo[indexSelected].videoFilePaths[0]));
    isReload = true;
    ui->slider->setValue(0);
}

void VideoPlayer::onSlider(qint64 i64Pos)
{
    ui->slider->setProgress(i64Pos);
}

void VideoPlayer::onDurationChanged(qint64 i64Duration)
{
    if(i64Duration > 0 && isReload)
    {
        ui->slider->setRange(0, i64Duration);
        isReload = false;
    }
}

// 播放状态改变
void VideoPlayer::onStateChanged(QMediaPlayer::State enumState)
{
    if(QMediaPlayer::StoppedState == enumState)
    {
        ui->BtnPlay->setText("PLAY");
        ui->slider->setEnabled(false);
        connect(ui->BtnPlay, SIGNAL(clicked()), myPlayer, SLOT(play()));
    }
    else if(QMediaPlayer::PlayingState == enumState)
    {
        ui->BtnPlay->setText("PAUSE");
        ui->slider->setEnabled(true);
        connect(ui->BtnPlay, SIGNAL(clicked()), myPlayer, SLOT(pause()));
    }
    else if(QMediaPlayer::PausedState == enumState)
    {
        ui->BtnPlay->setText("PLAY");
        ui->slider->setEnabled(true);
        connect(ui->BtnPlay, SIGNAL(clicked()), myPlayer, SLOT(play()));
    }
}

// 预览视频
void VideoPlayer::showVideo()
{
    QFile filelist("./file/filelist.txt");
    filelist.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream out(&filelist);//写入
    for(int i=0;i<myTimeLineLabel.size();i++){
        //qDebug()<<"filepath:"<<myTimeLineLabel[i]->myFilePath;
        out<<"file"<<" "<<"'"<<myTimeLineLabel[i]->myFilePath<<"'"<<"\n";
    }
    videoPrc=new QProcess(this);
    QString program="./ffmpeg.exe";
    videoNum++;
    qDebug()<<"videonum="<<videoNum;
    saveName=QString("./myVideo/video%1.mp4").arg(videoNum);
    QStringList argument;
    argument<<"-f"<<"concat"<<"-safe"<<"0"<<"-i"<<"./file/filelist.txt"<<"-codec"<<"copy"<<saveName<<"-y";
    //qDebug()<<"argument"<<argument;
    videoPrc->start(program,argument);
    //qDebug()<<mergePrc->waitForStarted();
    connect(videoPrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doVideoPlay()));
}

void VideoPlayer::doVideoPlay()
{
    if(myImageLabel.size()==0&&myWordLabel.size()==0){
        video newVideo(saveName);
        myVideo.append(newVideo);

        ui->videoList->addItem(saveName);
        ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->videoList->show();

        myPlayer->setMedia(QUrl::fromLocalFile(saveName));
        isReload = true;
        ui->slider->setValue(0);
    }

    else if(myImageLabel.size()!=0){
        QStringList newArgument;
        newArgument<<"-i"<<saveName;

        // 导入图片
        for(int i=0;i<myImageLabel.size();i++){
            // 无特效or平移进入
            if(myImageLabel[i]->effectIndex==0||myImageLabel[i]->effectIndex==2){
                //-i D:/VE/testImage/1.jpg
                newArgument<<"-i"<<myImageLabel[i]->myFilePath;
            }

            // 淡入淡出or旋转
            else if(myImageLabel[i]->effectIndex==1||myImageLabel[i]->effectIndex==3){
                int sumtime=0;
                for(int i=0;i<myTimeLineLabel.size();i++){
                    sumtime=sumtime+myTimeLineLabel[i]->width();
                }
                sumtime=sumtime/20;
                QString sum=QString("%1").arg(sumtime);
                newArgument<<"-loop"<<"1"<<"-t"<<sum<<"-i"<<myImageLabel[i]->myFilePath;
            }
        }

        newArgument<<"-filter_complex";
        QString argument1;

        // 输入特效
        for(int i=0;i<myImageLabel.size();i++){

            QString pre;
            if(i==0){
                pre=QString("[0:v]");
            }
            else{
                pre=QString("[v1]");
            }

            // 无特效
            if(myImageLabel[i]->effectIndex==0){
                //[0:v][1:v]overlay=x='if(gte(t,0)*lt(t,10),20,NAN)':y=20[v1]
                if(i==myImageLabel.size()-1){
                    argument1=argument1+QString("%6[%1:v]overlay=x='if(gte(t,%2)*lt(t,%3),%4,NAN)':y=%5[v1]").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }
                else{
                    argument1=argument1+QString("%6[%1:v]overlay=x='if(gte(t,%2)*lt(t,%3),%4,NAN)':y=%5[v1],").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }

            }

            // 淡入淡出
            if(myImageLabel[i]->effectIndex==1){
                //[2:v]fade=t=in:st=12:d=5:alpha=1,fade=t=out:st=19:d=5:alpha=1[temp];[v1][temp]overlay=x=20:y=80[v1]
                if(i==myImageLabel.size()-1){
                    argument1=argument1+QString("[%1:v]fade=t=in:st=%2:d=%3:alpha=1,fade=t=out:st=%4:d=%5:alpha=1[temp];%8[temp]overlay=x=%6:y=%7[v1]").arg(i+1).arg(myImageLabel[i]->iStartTime).arg((myImageLabel[i]->iEndTime-myImageLabel[i]->iStartTime)/2).arg(myImageLabel[i]->iEndTime).arg((myImageLabel[i]->iEndTime-myImageLabel[i]->iStartTime)/2).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }
                else{
                    argument1=argument1+QString("[%1:v]fade=t=in:st=%2:d=%3:alpha=1,fade=t=out:st=%4:d=%5:alpha=1[temp];%8[temp]overlay=x=%6:y=%7[v1],").arg(i+1).arg(myImageLabel[i]->iStartTime).arg((myImageLabel[i]->iEndTime-myImageLabel[i]->iStartTime)/2).arg(myImageLabel[i]->iEndTime).arg((myImageLabel[i]->iEndTime-myImageLabel[i]->iStartTime)/2).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }

            }

            //平移
            if(myImageLabel[i]->effectIndex==2){
                //[v1][3:v]overlay=x='if(gte(t,26)*lt(t,35),80+10*(t-26),NAN)':y=20+10*(t-26)[v1]

                if(i==myImageLabel.size()-1){
                    argument1=argument1+QString("%7[%1:v]overlay=x='if(gte(t,%2)*lt(t,%3),%4+20*(t-%5),NAN)':y=%6[v1]").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }
                else{
                    argument1=argument1+QString("%7[%1:v]overlay=x='if(gte(t,%2)*lt(t,%3),%4+20*(t-%5),NAN)':y=%6[v1],").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }

            }

            // 旋转
            if(myImageLabel[i]->effectIndex==3){
                //[4:v]rotate='PI/2*t:ow=hypot(iw,ih):oh=ow:c=0x00000000'[temp];[v1][temp]overlay=x='if(gte(t,40)*lt(t,60),80,NAN)':y=80[v1]

                if(i==myImageLabel.size()-1){
                    argument1=argument1+QString("[%1:v]rotate='PI/2*t:ow=hypot(iw,ih):oh=ow:c=0x00000000'[temp];%6[temp]overlay=x='if(gte(t,%2)*lt(t,%3),%4,NAN)':y=%5[v1]").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }
                else{
                    argument1=argument1+QString("[%1:v]rotate='PI/2*t:ow=hypot(iw,ih):oh=ow:c=0x00000000'[temp];%6[temp]overlay=x='if(gte(t,%2)*lt(t,%3),%4,NAN)':y=%5[v1],").arg(i+1).arg(myImageLabel[i]->iStartTime).arg(myImageLabel[i]->iEndTime).arg(myImageLabel[i]->imageXcoo).arg(myImageLabel[i]->imageYcoo).arg(pre);
                }

            }
        }

        //-map [v1] -threads 5 -preset ultrafast D:/VE/myVideo/video.mp4 -y
        videoNum++;
        saveName=QString("./myVideo/video%1.mp4").arg(videoNum);
        newArgument<<argument1<<"-map"<<"[v1]"<<"-threads"<<"5"<<"-preset"<<"ultrafast"<<saveName<<"-y";
        qDebug()<<"newargument"<<newArgument;

        imagePrc=new QProcess(this);
        QString program="./ffmpeg.exe";
        imagePrc->start(program,newArgument);
        qDebug()<<imagePrc->waitForStarted();
        //qDebug()<<mergePrc->waitForFinished(30000);

        connect(imagePrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doImagePlay()));
    }

    else if(myWordLabel.size()!=0){
        QStringList myArgument;
        myArgument<<"-i"<<saveName<<"-filter_complex";
        QString argument2;
        for(int i=0;i<myWordLabel.size();i++){
            QString pre;
            if(i==0){
                pre=QString("[0:v]");
            }
            else{
                pre=QString("[v1]");
            }
            //[0:v]drawtext=fontcolor=black:fontsize=50:text='111111':x=221:y=221:enable='between(t,1,10)'[v1] -map [v1] 输出 -y

            if(i==myWordLabel.size()-1){
                argument2=argument2+QString("%6drawtext=fontcolor=white:fontsize=50:text='%1':x=%2:y=%3:enable='between(t,%4,%5)'[v1]").arg(myWordLabel[i]->wordText).arg(myWordLabel[i]->imageXcoo).arg(myWordLabel[i]->imageYcoo).arg(myWordLabel[i]->iStartTime).arg(myWordLabel[i]->iEndTime).arg(pre);
            }
            else{
                argument2=argument2+QString("%6drawtext=fontcolor=white:fontsize=50:text='%1':x=%2:y=%3:enable='between(t,%4,%5)'[v1],").arg(myWordLabel[i]->wordText).arg(myWordLabel[i]->imageXcoo).arg(myWordLabel[i]->imageYcoo).arg(myWordLabel[i]->iStartTime).arg(myWordLabel[i]->iEndTime).arg(pre);
            }
        }
        videoNum++;
        saveName=QString("./myVideo/video%1.mp4").arg(videoNum);
        myArgument<<argument2<<"-map"<<"[v1]"<<"-threads"<<"5"<<"-preset"<<"ultrafast"<<saveName<<"-y";
        qDebug()<<"newargument"<<myArgument;

        wordPrc=new QProcess(this);
        QString program="./ffmpeg.exe";
        wordPrc->start(program,myArgument);
        qDebug()<<wordPrc->waitForStarted();
        //qDebug()<<mergePrc->waitForFinished(30000);

        connect(wordPrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doWordPlay()));
    }
}

void VideoPlayer::doImagePlay()
{
    if(myWordLabel.size()==0){
        video newVideo(saveName);
        myVideo.append(newVideo);

        ui->videoList->addItem(saveName);
        ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
        //ui->videoList->show();

        myPlayer->setMedia(QUrl::fromLocalFile(saveName));
        isReload = true;
        ui->slider->setValue(0);
    }

    else{
        QStringList myArgument;
        myArgument<<"-i"<<saveName<<"-filter_complex";
        QString argument2;
        for(int i=0;i<myWordLabel.size();i++){
            QString pre;
            if(i==0){
                pre=QString("[0:v]");
            }
            else{
                pre=QString("[v1]");
            }
            //[0:v]drawtext=fontcolor=black:fontsize=50:text='111111':x=221:y=221:enable='between(t,1,10)'[v1] -map [v1] 输出 -y

            if(i==myWordLabel.size()-1){
                argument2=argument2+QString("%6drawtext=fontcolor=white:fontsize=50:text='%1':x=%2:y=%3:enable='between(t,%4,%5)'[v1]").arg(myWordLabel[i]->wordText).arg(myWordLabel[i]->imageXcoo).arg(myWordLabel[i]->imageYcoo).arg(myWordLabel[i]->iStartTime).arg(myWordLabel[i]->iEndTime).arg(pre);
            }
            else{
                argument2=argument2+QString("%6drawtext=fontcolor=white:fontsize=50:text='%1':x=%2:y=%3:enable='between(t,%4,%5)'[v1],").arg(myWordLabel[i]->wordText).arg(myWordLabel[i]->imageXcoo).arg(myWordLabel[i]->imageYcoo).arg(myWordLabel[i]->iStartTime).arg(myWordLabel[i]->iEndTime).arg(pre);
            }
        }
        videoNum++;
        saveName=QString("./myVideo/video%1.mp4").arg(videoNum);
        myArgument<<argument2<<"-map"<<"[v1]"<<"-threads"<<"5"<<"-preset"<<"ultrafast"<<saveName<<"-y";
        qDebug()<<"newargument"<<myArgument;

        wordPrc=new QProcess(this);
        QString program="./ffmpeg.exe";
        wordPrc->start(program,myArgument);
        qDebug()<<wordPrc->waitForStarted();
        //qDebug()<<mergePrc->waitForFinished(30000);

        connect(wordPrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doWordPlay()));
    }
}

void VideoPlayer::doWordPlay()
{
    video newVideo(saveName);
    myVideo.append(newVideo);

    ui->videoList->addItem(saveName);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->videoList->show();

    myPlayer->setMedia(QUrl::fromLocalFile(saveName));
    isReload = true;
    ui->slider->setValue(0);
}

/*----------------------------------------进度条相关函数----------------------------------------*/
// 获取总时长
void VideoPlayer::getAllTime()
{
    // 获取视频时长
    playTime=myPlayer->duration();
    playTime=playTime/1000;  //获得的时间是以毫秒为单位的
    //std::cout<<"playTime="<<playTime<<std::endl;

    // 把滑块的相关数据设置好方便得到当前时间
    ui->slider->setMaximum(playTime);
    ui->slider->setMinimum(1);
    ui->slider->setSingleStep(1);
}

// 获取当前时间点
void VideoPlayer::getNowTime()
{
    nowTime=ui->slider->value();
    nowTime=nowTime/1000;
    //std::cout<<"nowTime="<<nowTime<<std::endl;
    int nH,nM,nS;
    nH=nowTime/3600;
    nM=(nowTime-nH*3600)/60;
    nS=nowTime-nH*3600-nM*60;

    int vH,vM,vS;
    vH=playTime/3600;
    vM=(playTime-vH*3600)/60;
    vS=playTime-vH*3600-vM*60;

    ui->videoTime->setText(QString("%1:%2:%3/%4:%5:%6").arg(nH).arg(nM).arg(nS).arg(vH).arg(vM).arg(vS));
}

/*----------------------------------------视频列表相关函数----------------------------------------*/
// 在鼠标位置显示菜单
void VideoPlayer::videoFileList()
{
    videoListMenu=new QMenu(this);
    renameV=new QAction("重命名");
    deletV=new QAction("删除");
    addToTimeLine=new QAction("添加到时间轴");
    videoListMenu->addAction(renameV);
    videoListMenu->addAction(deletV);
    videoListMenu->addAction(addToTimeLine);
    connect(renameV,SIGNAL(triggered()),this,SLOT(setVideoName()));
    connect(deletV,SIGNAL(triggered()),this,SLOT(videoDel()));
    connect(addToTimeLine,SIGNAL(triggered()),this,SLOT(videoAdd()));

    videoListMenu->exec(QCursor::pos());
}

// 弹出重命名窗口
void VideoPlayer::setVideoName()
{
    indexSelected = ui->videoList->currentRow();//用来保存需要修改名称的index
    //std::cout<<"index="<<indexSelected<<std::endl;

    // 弹出对话框
    renameDlg = new QDialog(this);
    newName=new QLineEdit(renameDlg);
    renameLabel=new QLabel(renameDlg);
    ok=new QPushButton(renameDlg);
    renameLabel->setText("文件名：");
    ok->setText("确定");
    QGridLayout *renameLayout=new QGridLayout(renameDlg);
    renameLayout->addWidget(renameLabel,0,0);
    renameLayout->addWidget(newName,0,1);
    renameLayout->addWidget(ok,1,1);

    renameDlg->show();
    connect(ok,SIGNAL(clicked()),this,SLOT(videoNameChanged()));
}

// 重命名
void VideoPlayer::videoNameChanged()
{
    QString setName=newName->text();
    ui->videoList->item(indexSelected)->setText(setName);
    myVideo[indexSelected].videoFileNames[0]=setName;
    renameDlg->close();
}

// 删除
void VideoPlayer::videoDel()
{
    indexSelected = ui->videoList->currentRow();//用来保存需要删除的index
    //std::cout<<"index="<<indexSelected<<std::endl;
    //for(int i=0;i<videoFileNames.size();i++){
        //qDebug()<<"filename:"<<videoFileNames[i];
        //qDebug()<<"filepath:"<<videoFilePaths[i];
    //}
    QVector<video>::iterator iter=myVideo.begin()+indexSelected;
    myVideo.erase(iter);
    QListWidgetItem* itemSelected = ui->videoList->takeItem(indexSelected);
    ui->videoList->removeItemWidget(itemSelected);
    delete itemSelected;
    ui->videoList->show();
}

// 添加到时间轴
void VideoPlayer::videoAdd()
{
    ui->videoWgt->show();
    // 确定绘制的时间块的长度
    playTime=myPlayer->duration();
    playTime=playTime/1000;
    //qDebug()<<"playTime="<<playTime;

    if(myTimeLineLabel.size()==0){
        sumTime=0;
    }
    else{
        for(int i=0;i<myTimeLineLabel.size();i++){
            sumTime=sumTime+myTimeLineLabel[i]->width();
        }
    }
    sumTime=sumTime/20;
    timeLineLabel *myVideoLabel=new timeLineLabel(ui->videoWgt);
    myTimeLineLabel.append(myVideoLabel);

    myVideoLabel->myIndex=myTimeLineLabel.size()-1;
    myVideoLabel->setGeometry(20*sumTime,0,20*playTime,ui->videoWgt->height());
    myVideoLabel->setStyleSheet("QLabel{background-color:rgb(153,172,163);border:3px solid rgb(33,57,45);}");
    myVideoLabel->setText(myVideo[indexSelected].videoFileNames[0]);
    myVideoLabel->show();
    myVideoLabel->myFilePath=myVideo[indexSelected].videoFilePaths[0];

    //qDebug()<<"num="<<myTimeLineLabel.size();

    connect(myVideoLabel,SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));
}

/*----------------------------------------时间轴操作相关函数----------------------------------------*/
// 单击菜单
void VideoPlayer::videoTLList()
{
    videoTLMenu=new QMenu(this);
    videoDelete=new QAction("删除");
    videoCut=new QAction("切分");
    videoMoveBack=new QAction("向后移");
    videoMoveForward=new QAction("向前移");
    videoMergeBack=new QAction("向后合并");
    videoMergeForward=new QAction("向前合并");
    videoTLMenu->addAction(videoDelete);
    videoTLMenu->addAction(videoCut);
    videoTLMenu->addAction(videoMoveBack);
    videoTLMenu->addAction(videoMoveForward);
    videoTLMenu->addAction(videoMergeBack);
    videoTLMenu->addAction(videoMergeForward);
    connect(videoCut,SIGNAL(triggered()),this,SLOT(pointCut()));
    connect(videoDelete,SIGNAL(triggered()),this,SLOT(videoTLDel()));
    connect(videoMoveBack,SIGNAL(triggered()),this,SLOT(videoBack()));
    connect(videoMoveForward,SIGNAL(triggered()),this,SLOT(videoForward()));
    connect(videoMergeBack,SIGNAL(triggered()),this,SLOT(videoMBack()));
    connect(videoMergeForward,SIGNAL(triggered()),this,SLOT(videoMForward()));

    videoTLMenu->exec(QCursor::pos());
}

// 删除
void VideoPlayer::videoTLDel()
{
    int nowIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
        }
    }


    for(int i=nowIndex;i<myTimeLineLabel.size()-1;i++){
        myTimeLineLabel[i]->setText(myTimeLineLabel[i+1]->text());
        myTimeLineLabel[i]->setGeometry(myTimeLineLabel[i]->x(),0,myTimeLineLabel[i+1]->width(),ui->videoWgt->height());
        myTimeLineLabel[i]->myFilePath=myTimeLineLabel[i+1]->myFilePath;
        myTimeLineLabel[i]->show();
    }

    delete myTimeLineLabel[myTimeLineLabel.size()-1];
    myTimeLineLabel.pop_back();

    if(nowIndex<myTimeLineLabel.size()){
        myTimeLineLabel[nowIndex]->isCho=false;
    }
}

// 切分
void VideoPlayer::pointCut()
{
    cutDlg=new QDialog(this);
    cutLabel=new QLabel(cutDlg);
    cutLabel->setText("切分时间点：");

    min=new QLineEdit(cutDlg);
    sec=new QLineEdit(cutDlg);
    colon=new QLabel(cutDlg);
    colon->setText(":");

    QPushButton *ok;
    ok=new QPushButton(cutDlg);
    ok->setText("确定");

    QGridLayout *cutLayout=new QGridLayout(cutDlg);
    cutLayout->addWidget(cutLabel,0,0);
    cutLayout->addWidget(min,0,1);
    cutLayout->addWidget(colon,0,2);
    cutLayout->addWidget(sec,0,3);
    cutLayout->addWidget(ok,1,3);

    cutDlg->show();

    connect(ok,SIGNAL(clicked()),this,SLOT(doPointCut()));
}

void VideoPlayer::doPointCut()
{
    cutDlg->close();
    int nowIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
        }
    }
    QString cutTime=QString("0:%1:%2").arg(min->text()).arg(sec->text());
    int videoTime=myTimeLineLabel[nowIndex]->width()/20;
    int videoMin=videoTime/60;
    int videoSec=videoTime-videoMin*60;
    QString endTime=QString("0:%1:%2").arg(videoMin).arg(videoSec);

    QString program="./ffmpeg.exe";

    QString cutSaveName1;
    videoCutNum++;
    cutSaveName1=QString("./cutVideo/cut%1.mp4").arg(videoCutNum);
    QStringList argument1;
    argument1<<"-i"<<myVideo[indexSelected].videoFilePaths[0]<<"-vcodec"<<"copy"<<"-acodec"<<"copy"<<"-ss"<<"0:0:0"<<"-to"<<cutTime<<cutSaveName1<<"-y";
    qDebug()<<argument1;
    cutPrc->start(program,argument1);
    qDebug()<<cutPrc->waitForFinished();

    video video1(cutSaveName1);
    myVideo.append(video1);
    ui->videoList->addItem(cutSaveName1);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();

    QString cutSaveName2;
    videoCutNum++;
    cutSaveName2=QString("./cutVideo/cut%1.mp4").arg(videoCutNum);
    QStringList argument2;
    argument2<<"-i"<<myVideo[indexSelected].videoFilePaths[0]<<"-vcodec"<<"copy"<<"-acodec"<<"copy"<<"-ss"<<cutTime<<"-to"<<endTime<<cutSaveName2<<"-y";
    qDebug()<<argument2;
    cutPrc->start(program,argument2);
    qDebug()<<cutPrc->waitForFinished();

    video video2(cutSaveName2);
    myVideo.append(video2);
    ui->videoList->addItem(cutSaveName2);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();

    int videoCut=60*min->text().toInt()+sec->text().toInt();
    videoCut=videoCut*20;
    //qDebug()<<"cuttime"<<videoCut;

    timeLineLabel *cutVideo1=new timeLineLabel(ui->videoWgt);
    timeLineLabel *cutVideo2=new timeLineLabel(ui->videoWgt);

    //qDebug()<<"nowindex="<<nowIndex;
    //qDebug()<<"x1="<<myTimeLineLabel[nowIndex]->x();
    //qDebug()<<"width1="<<videoCut-myTimeLineLabel[nowIndex]->x();

    cutVideo1->myIndex=nowIndex;
    cutVideo1->setGeometry(myTimeLineLabel[nowIndex]->x(),0,videoCut-myTimeLineLabel[nowIndex]->x(),ui->videoWgt->height());
    cutVideo1->setStyleSheet("QLabel{background-color:rgb(153,172,163);border:3px solid rgb(33,57,45);}");
    cutVideo1->setText(cutSaveName1);
    cutVideo1->show();
    cutVideo1->myFilePath=cutSaveName1;


    //qDebug()<<"nowindex="<<nowIndex;
    //qDebug()<<"x2="<<videoCut;
    //qDebug()<<"width2="<<myTimeLineLabel[nowIndex]->width()-videoCut+myTimeLineLabel[nowIndex]->x();

    cutVideo2->myIndex=nowIndex+1;
    cutVideo2->setGeometry(videoCut,0,myTimeLineLabel[nowIndex]->width()-videoCut+myTimeLineLabel[nowIndex]->x(),ui->videoWgt->height());
    cutVideo2->setStyleSheet("QLabel{background-color:rgb(153,172,163);border:3px solid rgb(33,57,45);}");
    cutVideo2->setText(cutSaveName2);
    cutVideo2->show();
    cutVideo2->myFilePath=cutSaveName2;

    myTimeLineLabel.append(cutVideo1);
    myTimeLineLabel.append(cutVideo2);

    delete myTimeLineLabel[nowIndex];
    myTimeLineLabel.erase(myTimeLineLabel.begin()+nowIndex);

    connect(cutVideo1,SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));
    connect(cutVideo2,SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));

    cutVideo1->show();
    cutVideo2->show();
}

// 向后移
void VideoPlayer::videoBack()
{
    int nowIndex=0;
    int nextIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
            nextIndex=nowIndex+1;
        }
    }
    if(nextIndex<=myTimeLineLabel.size()){

        myTimeLineLabel[nextIndex]->isCho=false;

        timeLineLabel *temp=new timeLineLabel(ui->videoWgt);
        temp->setText(myTimeLineLabel[nowIndex]->text());
        temp->setGeometry(myTimeLineLabel[nowIndex]->x(),0,myTimeLineLabel[nowIndex]->width(),ui->videoWgt->height());
        temp->myFilePath=myTimeLineLabel[nowIndex]->myFilePath;

        myTimeLineLabel[nowIndex]->setText(myTimeLineLabel[nextIndex]->text());
        myTimeLineLabel[nowIndex]->setGeometry(myTimeLineLabel[nowIndex]->x(),0,myTimeLineLabel[nextIndex]->width(),ui->videoWgt->height());
        myTimeLineLabel[nowIndex]->myFilePath=myTimeLineLabel[nextIndex]->myFilePath;

        myTimeLineLabel[nextIndex]->setText(temp->text());
        myTimeLineLabel[nextIndex]->setGeometry(temp->x()+myTimeLineLabel[nextIndex]->width(),0,temp->width(),ui->videoWgt->height());
        myTimeLineLabel[nextIndex]->myFilePath=temp->myFilePath;

        myTimeLineLabel[nowIndex]->show();
        myTimeLineLabel[nextIndex]->show();

        connect(myTimeLineLabel[nowIndex],SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));
        connect(myTimeLineLabel[nextIndex],SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));

        delete temp;
    }

}

// 向前移
void VideoPlayer::videoForward()
{
    int nowIndex=0;
    int frontIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
            frontIndex=nowIndex-1;
        }
    }
    if(frontIndex>=0){

        myTimeLineLabel[frontIndex]->isCho=false;

        timeLineLabel *temp=new timeLineLabel(ui->videoWgt);
        temp->setText(myTimeLineLabel[frontIndex]->text());
        temp->setGeometry(myTimeLineLabel[frontIndex]->x(),0,myTimeLineLabel[frontIndex]->width(),ui->videoWgt->height());
        temp->myFilePath=myTimeLineLabel[frontIndex]->myFilePath;

        myTimeLineLabel[frontIndex]->setText(myTimeLineLabel[nowIndex]->text());
        myTimeLineLabel[frontIndex]->setGeometry(myTimeLineLabel[frontIndex]->x(),0,myTimeLineLabel[nowIndex]->width(),ui->videoWgt->height());
        myTimeLineLabel[frontIndex]->myFilePath=myTimeLineLabel[nowIndex]->myFilePath;

        myTimeLineLabel[nowIndex]->setText(temp->text());
        myTimeLineLabel[nowIndex]->setGeometry(temp->x()+myTimeLineLabel[nowTime]->width(),0,temp->width(),ui->videoWgt->height());
        myTimeLineLabel[nowIndex]->myFilePath=temp->myFilePath;

        myTimeLineLabel[nowIndex]->show();
        myTimeLineLabel[frontIndex]->show();

        connect(myTimeLineLabel[nowIndex],SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));
        connect(myTimeLineLabel[frontIndex],SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));


        delete temp;
    }
}

// 向后合并
void VideoPlayer::videoMBack()
{
    int nowIndex=0;
    int nextIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
            nextIndex=nowIndex+1;
        }
    }
    QFile filelist("./file/filelist.txt");
    filelist.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream out(&filelist);//写入
    out<<"file"<<" "<<"'"<<myTimeLineLabel[nowIndex]->myFilePath<<"'"<<"\n"<<"file"<<" "<<"'"<<myTimeLineLabel[nextIndex]->myFilePath<<"'";

    //ffmpeg -f concat -safe 0 -i filelist.txt -codec copy 输出路径 -y

    mergePrc=new QProcess(this);
    QString program="./ffmpeg.exe";
    videoMergeNum++;
    qDebug()<<"videoMergeNum="<<videoMergeNum;
    mergeSaveName=QString("./mergeVideo/merge%1.mp4").arg(videoMergeNum);
    QStringList argument;
    argument<<"-f"<<"concat"<<"-safe"<<"0"<<"-i"<<"./file/filelist.txt"<<"-c"<<"copy"<<mergeSaveName<<"-y";
    qDebug()<<argument;
    mergePrc->start(program,argument);
    qDebug()<<mergePrc->waitForStarted();
    //mergePrc-> waitForFinished();
    connect(mergePrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doVideoMBack()));

    timeLineLabel *myVideoLabel=new timeLineLabel(ui->videoWgt);

    myTimeLineLabel.append(myVideoLabel);

    myVideoLabel->myIndex=myTimeLineLabel.size()-1;
    myVideoLabel->setGeometry(myTimeLineLabel[nowIndex]->x(),0,myTimeLineLabel[nowIndex]->width()+myTimeLineLabel[nextIndex]->width(),ui->videoWgt->height());
    myVideoLabel->setStyleSheet("QLabel{background-color:rgb(153,172,163);border:3px solid rgb(33,57,45);}");
    myVideoLabel->setText(mergeSaveName);
    myVideoLabel->show();
    myVideoLabel->myFilePath=mergeSaveName;

    connect(myVideoLabel,SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));

    delete myTimeLineLabel[nowIndex];
    delete myTimeLineLabel[nextIndex];
    myTimeLineLabel.erase(myTimeLineLabel.begin()+nowIndex);
    myTimeLineLabel.erase(myTimeLineLabel.begin()+nowIndex);
}

void VideoPlayer::doVideoMBack()
{
    video newVideo(mergeSaveName);
    myVideo.append(newVideo);

    ui->videoList->addItem(mergeSaveName);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();
}

// 向前合并
void VideoPlayer::videoMForward()
{
    int nowIndex=0;
    int frontIndex=0;
    for(int i=0;i<myTimeLineLabel.size();i++){
        if(myTimeLineLabel[i]->isCho==true){
            //qDebug()<<"i="<<i;
            nowIndex=i;
            frontIndex=nowIndex-1;
        }
    }
    QFile filelist("./file/filelist.txt");
    filelist.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream out(&filelist);//写入
    out<<"file"<<" "<<"'"<<myTimeLineLabel[frontIndex]->myFilePath<<"'"<<"\n"<<"file"<<" "<<"'"<<myTimeLineLabel[nowIndex]->myFilePath<<"'";

    //ffmpeg -f concat -i filelist.txt -codec copy 输出路径
    //ffmpeg-f concat -safe 0 -i ./fileToMerge.txt -c copy -y ./out.mp4

    mergePrc=new QProcess(this);
    QString program="./ffmpeg.exe";
    QString mergeSaveName;
    videoMergeNum++;
    mergeSaveName=QString("./mergeVideo/merge%1.mp4").arg(videoMergeNum);
    QStringList argument;
    argument<<"-f"<<"concat"<<"-safe"<<"0"<<"-i"<<"./file/filelist.txt"<<"-codec"<<"copy"<<mergeSaveName<<"-y";
    mergePrc->start(program,argument);
    connect(mergePrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(doVideoMForward()));
    mergePrc->waitForFinished();

    timeLineLabel *myVideoLabel=new timeLineLabel(ui->videoWgt);

    myTimeLineLabel.append(myVideoLabel);

    myVideoLabel->myIndex=myTimeLineLabel.size()-1;

    myVideoLabel->setGeometry(myTimeLineLabel[frontIndex]->x(),0,myTimeLineLabel[nowIndex]->width()+myTimeLineLabel[frontIndex]->width(),ui->videoWgt->height());
    myVideoLabel->setStyleSheet("QLabel{background-color:rgb(153,172,163);border:3px solid rgb(33,57,45);}");
    myVideoLabel->setText(mergeSaveName);
    myVideoLabel->show();

    myVideoLabel->myFilePath=mergeSaveName;

    connect(myVideoLabel,SIGNAL(clicked(QMouseEvent*)),this,SLOT(videoTLList()));


    delete myTimeLineLabel[nowIndex];
    delete myTimeLineLabel[frontIndex];
    myTimeLineLabel.erase(myTimeLineLabel.begin()+frontIndex);
    myTimeLineLabel.erase(myTimeLineLabel.begin()+frontIndex);

}

void VideoPlayer::doVideoMForward()
{
    video newVideo(mergeSaveName);
    myVideo.append(newVideo);

    ui->videoList->addItem(mergeSaveName);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();
}

/*----------------------------------------图片素材操作相关函数----------------------------------------*/
// 导入素材
void VideoPlayer::setImageFile(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec()){
        imageFilePaths.append(dialog.selectedFiles());
        imageFileNames.append(dialog.selectedFiles());
    }

    if(!imageFilePaths[imageFilePaths.size()-1].isEmpty())
    {
        QListWidgetItem *imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon(imageFilePaths[imageFilePaths.size()-1]));
        imageItem->setText(imageFilePaths[imageFilePaths.size()-1]);
        ui->imageList->addItem(imageItem);
    }
}

// 在鼠标位置显示菜单
void VideoPlayer::imageFileList()
{
    imageListMenu->exec(QCursor::pos());
}

// 弹出重命名窗口
void VideoPlayer::setImageName()
{
    indexSelected = ui->imageList->currentRow(); // 用来保存需要修改名称的index

    // 弹出对话框
    renameDlg = new QDialog(this);
    newName=new QLineEdit(renameDlg);
    renameLabel=new QLabel(renameDlg);
    ok=new QPushButton(renameDlg);
    renameLabel->setText("文件名：");
    ok->setText("确定");
    QGridLayout *renameLayout=new QGridLayout(renameDlg);
    renameLayout->addWidget(renameLabel,0,0);
    renameLayout->addWidget(newName,0,1);
    renameLayout->addWidget(ok,1,1);

    renameDlg->show();
    connect(ok,SIGNAL(clicked()),this,SLOT(imageNameChanged()));
}

// 重命名
void VideoPlayer::imageNameChanged()
{
    indexSelected = ui->imageList->currentRow();
    QString setName=newName->text();
    ui->imageList->item(indexSelected)->setText(setName);
    imageFileNames[indexSelected]=setName;
    renameDlg->close();
}

// 删除
void VideoPlayer::imageDel()
{
    indexSelected = ui->imageList->currentRow(); // 用来保存需要删除的index
    //std::cout<<"index="<<indexSelected<<std::endl;
    //for(int i=0;i<videoFileNames.size();i++){
        //qDebug()<<"filepath:"<<imageFilePaths[i];
    //}
    imageFilePaths.removeAt(indexSelected);

    QListWidgetItem* itemSelected = ui->imageList->takeItem(indexSelected);
    ui->imageList->removeItemWidget(itemSelected);
    delete itemSelected;
    ui->imageList->show();
}

// 弹出设置素材窗口
void VideoPlayer::setImageAdd()
{
    indexSelected = ui->imageList->currentRow(); // 用来保存需要修改名称的index

    // 弹出对话框
    addImageSetting = new QDialog(this);
    imageStartTime=new QLabel(addImageSetting);
    imageEndTime=new QLabel(addImageSetting);
    imageStartTime->setText("开始时间：");
    imageEndTime->setText("结束时间");

    iSTm=new QLineEdit(addImageSetting);
    iSTs=new QLineEdit(addImageSetting);
    iETm=new QLineEdit(addImageSetting);
    iETs=new QLineEdit(addImageSetting);

    QLabel *imageX=new QLabel(addImageSetting);
    QLabel *imageY=new QLabel(addImageSetting);
    imageX->setText("x坐标");
    imageY->setText("y坐标");

    ix=new QLineEdit(addImageSetting);
    iy=new QLineEdit(addImageSetting);

    colonLabel1=new QLabel(addImageSetting);
    colonLabel1->setText(":");
    colonLabel2=new QLabel(addImageSetting);
    colonLabel2->setText(":");
    colonLabel3=new QLabel(addImageSetting);
    colonLabel3->setText("秒");

    setImageEffects=new QLabel(addImageSetting);
    setImageEffects->setText("设置特效：");
    imageEffects=new QComboBox(addImageSetting);
    imageEffects->addItem("无");
    imageEffects->addItem("淡入淡出");
    imageEffects->addItem("平移进入");
    imageEffects->addItem("旋转");
    setIETime=new QLabel(addImageSetting);
    setIETime->setText("特效持续时间：");
    IETime=new QLineEdit(addImageSetting);

    ok=new QPushButton(addImageSetting);
    ok->setText("确定");
    QGridLayout *imageSettingLayout=new QGridLayout(addImageSetting);
    imageSettingLayout->addWidget(imageStartTime,0,0);
    imageSettingLayout->addWidget(iSTm,0,1);
    imageSettingLayout->addWidget(colonLabel1,0,2);
    imageSettingLayout->addWidget(iSTs,0,3);

    imageSettingLayout->addWidget(imageEndTime,1,0);
    imageSettingLayout->addWidget(iETm,1,1);
    imageSettingLayout->addWidget(colonLabel2,1,2);
    imageSettingLayout->addWidget(iETs,1,3);

    imageSettingLayout->addWidget(imageX,2,0);
    imageSettingLayout->addWidget(ix,2,1);
    imageSettingLayout->addWidget(imageY,3,0);
    imageSettingLayout->addWidget(iy,3,1);

    imageSettingLayout->addWidget(setImageEffects,4,0);
    imageSettingLayout->addWidget(imageEffects,4,1);
    imageSettingLayout->addWidget(setIETime,5,0);
    imageSettingLayout->addWidget(IETime,5,1);
    imageSettingLayout->addWidget(colonLabel3,5,2);

    imageSettingLayout->addWidget(ok,6,3);

    addImageSetting->show();

    connect(ok,SIGNAL(clicked()),this,SLOT(imageAdd()));
}

// 添加图片素材到时间轴
void VideoPlayer::imageAdd()
{
    int effectIndex=imageEffects->currentIndex(); // 特效序号
    indexSelected = ui->imageList->currentRow(); // 素材序号
    //qDebug()<<"index"<<indexSelected;
    addImageSetting->close();

    // 添加到时间轴
    timeLineLabel *imageTLLabel=new timeLineLabel(ui->imageWgt);
    myImageLabel.append(imageTLLabel);
    int startTime=iSTm->text().toInt()*60+iSTs->text().toInt();
    int endTime=iETm->text().toInt()*60+iETs->text().toInt();
    //qDebug()<<"start="<<startTime;
    //qDebug()<<"end="<<endTime;
    imageTLLabel->setGeometry(20*startTime,0,20*(endTime-startTime),ui->imageWgt->height());
    imageTLLabel->setStyleSheet("QLabel{background-color:rgb(163,128,131);border:3px solid rgb(107,29,3);}");
    imageTLLabel->setText(imageFileNames[indexSelected]);

    imageTLLabel->effectIndex=effectIndex;
    imageTLLabel->myFilePath=imageFilePaths[indexSelected];
    imageTLLabel->effectTime=IETime->text().toInt();
    imageTLLabel->imageXcoo=ix->text().toInt();
    imageTLLabel->imageYcoo=iy->text().toInt();
    imageTLLabel->iStartTime=startTime;
    imageTLLabel->iEndTime=endTime;

    //qDebug()<<imageTLLabel->text();
    imageTLLabel->show();
}

/*----------------------------------------视频剪切相关函数----------------------------------------*/
// 自动获取剪切时间
void VideoPlayer::getCutTime()
{
    if(isCut==true){
        ui->BtnCut->setText("开始剪切");
        nowTime=ui->slider->value();
        int tempCutStartTime=nowTime/1000;
        int cH,cM,cS;
        cH=tempCutStartTime/3600;
        cM=(tempCutStartTime-cH*3600)/60;
        cS=tempCutStartTime-cH*3600-cM*60;
        cutStartTime=QString("%1:%2:%3").arg(cH).arg(cM).arg(cS);
        isCut=false;
        ui->BtnCut->setText("结束剪切");
        //qDebug()<<"start="<<cutStartTime;
    }
    else if(isCut==false){
        ui->BtnCut->setText("结束剪切");
        nowTime=ui->slider->value();
        int tempCutEndTime=nowTime/1000;
        int eH,eM,eS;
        eH=tempCutEndTime/3600;
        eM=(tempCutEndTime-eH*3600)/60;
        eS=tempCutEndTime-eH*3600-eM*60;
        cutEndTime=QString("%1:%2:%3").arg(eH).arg(eM).arg(eS);
        isCut=true;
        ui->BtnCut->setText("开始剪切");
        //qDebug()<<"end="<<cutEndTime;

        cutVideo();
    }
}

// 手动获取剪切起止时间，如果超出原视频范围则自动剪出可选范围内的视频
void VideoPlayer::setCutTime()
{
    isSet=true;

    // 弹出对话框
    setCutTimeDlg = new QDialog(this);
    shh=new QLineEdit(setCutTimeDlg);
    smm=new QLineEdit(setCutTimeDlg);
    sss=new QLineEdit(setCutTimeDlg);
    ehh=new QLineEdit(setCutTimeDlg);
    emm=new QLineEdit(setCutTimeDlg);
    ess=new QLineEdit(setCutTimeDlg);
    cutStartTimeLabel=new QLabel(setCutTimeDlg);
    cutStartTimeLabel->setText("剪切开始时间：");
    cutEndTimeLabel=new QLabel(setCutTimeDlg);
    cutEndTimeLabel->setText("剪切结束时间：");
    colonLabel1=new QLabel(setCutTimeDlg);
    colonLabel1->setText(":");
    colonLabel2=new QLabel(setCutTimeDlg);
    colonLabel2->setText(":");
    colonLabel3=new QLabel(setCutTimeDlg);
    colonLabel3->setText(":");
    colonLabel4=new QLabel(setCutTimeDlg);
    colonLabel4->setText(":");
    ok=new QPushButton(setCutTimeDlg);
    ok->setText("确定");

    QGridLayout *setCutTimeLayout=new QGridLayout(setCutTimeDlg);
    setCutTimeLayout->addWidget(cutStartTimeLabel,0,0);
    setCutTimeLayout->addWidget(cutEndTimeLabel,1,0);
    setCutTimeLayout->addWidget(shh,0,1);
    setCutTimeLayout->addWidget(smm,0,3);
    setCutTimeLayout->addWidget(sss,0,5);
    setCutTimeLayout->addWidget(ehh,1,1);
    setCutTimeLayout->addWidget(emm,1,3);
    setCutTimeLayout->addWidget(ess,1,5);
    setCutTimeLayout->addWidget(colonLabel1,0,2);
    setCutTimeLayout->addWidget(colonLabel2,0,4);
    setCutTimeLayout->addWidget(colonLabel3,1,2);
    setCutTimeLayout->addWidget(colonLabel4,1,4);
    setCutTimeLayout->addWidget(ok,3,5);

    setCutTimeDlg->show();
    //qDebug()<<cutStartTime;
    //qDebug()<<cutEndTime;
    connect(ok,SIGNAL(clicked()),this,SLOT(cutVideo()));
}

// 剪切视频
void VideoPlayer::cutVideo()
{

    if(isSet==true){
        setCutTimeDlg->close();
        cutStartTime=QString("%1:%2:%3").arg(shh->text()).arg(smm->text()).arg(sss->text());
        cutEndTime=QString("%1:%2:%3").arg(ehh->text()).arg(emm->text()).arg(ess->text());
    }

    //ffmpeg -i 源文件名 -vcodec copy -acodec copy -ss 开始时间 -to 结束时间 目标文件名 -y

    indexSelected = ui->videoList->currentRow();
    QString program="./ffmpeg.exe";
    QString cutSaveName;
    videoCutNum++;
    cutSaveName=QString("./cutVideo/cut%1.mp4").arg(videoCutNum);
    //qDebug()<<videoFilePaths.size();
    QStringList argument;
    argument<<"-i"<<myVideo[indexSelected].videoFilePaths[0]<<"-vcodec"<<"copy"<<"-acodec"<<"copy"<<"-ss"<<cutStartTime<<"-to"<<cutEndTime<<cutSaveName<<"-y";
    qDebug()<<argument;
    cutPrc->start(program,argument);
    qDebug()<<cutPrc-> waitForStarted();

    video newVideo(cutSaveName);
    myVideo.append(newVideo);

    ui->videoList->addItem(cutSaveName);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();
}

/*----------------------------------------文字素材操作相关函数----------------------------------------*/
// 弹出文字素材设置窗口
void VideoPlayer::addWordDlg()
{
    addWord=new QDialog(this);

    wordLabel=new QLabel(addWord);
    imageStartTime=new QLabel(addWord);
    imageEndTime=new QLabel(addWord);
    wordLabel->setText("文字内容：");
    imageStartTime->setText("开始时间：");
    imageEndTime->setText("结束时间");

    word=new QLineEdit(addWord);
    iSTm=new QLineEdit(addWord);
    iSTs=new QLineEdit(addWord);
    iETm=new QLineEdit(addWord);
    iETs=new QLineEdit(addWord);

    colonLabel1=new QLabel(addWord);
    colonLabel1->setText(":");
    colonLabel2=new QLabel(addWord);
    colonLabel2->setText(":");

    QLabel *imageX=new QLabel(addWord);
    QLabel *imageY=new QLabel(addWord);
    imageX->setText("x坐标");
    imageY->setText("y坐标");

    ix=new QLineEdit(addWord);
    iy=new QLineEdit(addWord);

    ok=new QPushButton(addWord);
    ok->setText("确定");

    QGridLayout *wordLayout=new QGridLayout(addWord);
    wordLayout->addWidget(wordLabel,0,0);
    wordLayout->addWidget(word,0,1);
    wordLayout->addWidget(imageStartTime,1,0);
    wordLayout->addWidget(iSTm,1,1);
    wordLayout->addWidget(colonLabel1,1,2);
    wordLayout->addWidget(iSTs,1,3);
    wordLayout->addWidget(imageEndTime,2,0);
    wordLayout->addWidget(iETm,2,1);
    wordLayout->addWidget(colonLabel2,2,2);
    wordLayout->addWidget(iETs,2,3);
    wordLayout->addWidget(imageX,3,0);
    wordLayout->addWidget(ix,3,1);
    wordLayout->addWidget(imageY,4,0);
    wordLayout->addWidget(iy,4,1);
    wordLayout->addWidget(ok,5,3);

    addWord->show();
    connect(ok,SIGNAL(clicked()),this,SLOT(setWordTLLabel()));
}

// 添加文字素材到时间轴
void VideoPlayer::setWordTLLabel()
{
    addWord->close();
    timeLineLabel *wordTLLabel=new timeLineLabel(ui->wordWgt);
    myWordLabel.append(wordTLLabel);
    int startTime=iSTm->text().toInt()*60+iSTs->text().toInt();
    int endTime=iETm->text().toInt()*60+iETs->text().toInt();
    //qDebug()<<"start="<<startTime;
    //qDebug()<<"end="<<endTime;
    wordTLLabel->setGeometry(20*startTime,0,20*(endTime-startTime),ui->imageWgt->height());
    wordTLLabel->setStyleSheet("QLabel{background-color:rgb(139,164,198);border:3px solid rgb(6,38,68);}");
    wordTLLabel->setText(word->text());

    wordTLLabel->wordText=QString("%1").arg(word->text());
    wordTLLabel->imageXcoo=ix->text().toInt();
    wordTLLabel->imageYcoo=iy->text().toInt();
    wordTLLabel->iStartTime=startTime;
    wordTLLabel->iEndTime=endTime;

    wordTLLabel->show();
}

/*----------------------------------------字幕操作相关函数----------------------------------------*/
// 导入字幕文件
void VideoPlayer::setSubtitleFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec()){
        subtitleFilePaths.append(dialog.selectedFiles());
    }
    //D:\QtProjects\VideoEditor2\ffmpeg.exe -i D:\ANY.mp4 -vf subtitles=\'D:/test.srt\' D:\VideoEditor\videoRes\output4.mp4 -y
    QString program;
    program="./ffmpeg.exe";
    QStringList argument;
    subPrc=new QProcess(this);

    subNum++;
    subname=QString("./subVideo/subVideo%1.mp4").arg(subNum);
    QString argument3=QString("subtitles=\\'%1\\'").arg(subtitleFilePaths[0]);
    argument<<"-i"<<myTimeLineLabel[0]->myFilePath<<"-vf"<<argument3<<subname<<"-y";

    qDebug()<<"subargument"<<argument;

    subPrc->start(program,argument);
    qDebug()<<subPrc->waitForStarted();
    connect(subPrc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(addSub()));
}

void VideoPlayer::addSub()
{
    video newVideo(subname);
    myVideo.append(newVideo);

    ui->videoList->addItem(subname);
    ui->videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->videoList->show();
}

/*----------------------------------------导出相关函数----------------------------------------*/
// 弹出导出文件重命名窗口
void VideoPlayer::setOutputName()
{
    setOutput=new QDialog(this);
    setName=new QLabel(setOutput);
    setName->setText("文件名:");
    outputName=new QLineEdit(setOutput);

    outFormatLabel=new QLabel(setOutput);
    outFormatLabel->setText("导出格式：");
    outFormat=new QComboBox(setOutput);
    outFormat->addItem("mp4");
    outFormat->addItem("avi");

    outResolvingLabel=new QLabel(setOutput);
    outResolvingLabel->setText("分辨率：");
    outResolving=new QComboBox(setOutput);
    outResolving->addItem("1080P 1920*1080");
    outResolving->addItem("720P 1280*720");
    outResolving->addItem("480P 640*480");

    outBitRateLabel=new QLabel(setOutput);
    outBitRateLabel->setText("码率：");
    outBitRate=new QComboBox(setOutput);
    outBitRate->addItem("500kbps");
    outBitRate->addItem("2Mbps");
    outBitRate->addItem("5Mbps");

    ok=new QPushButton(setOutput);
    ok->setText("确定");

    QGridLayout *outLayout=new QGridLayout(setOutput);
    outLayout->addWidget(setName,0,0);
    outLayout->addWidget(outputName,0,1);
    outLayout->addWidget(outFormatLabel,1,0);
    outLayout->addWidget(outFormat,1,1);
    outLayout->addWidget(outResolvingLabel,2,0);
    outLayout->addWidget(outResolving,2,1);
    outLayout->addWidget(outBitRateLabel,3,0);
    outLayout->addWidget(outBitRate,3,1);
    outLayout->addWidget(ok,4,1);

    connect(ok,SIGNAL(clicked()),this,SLOT(videoOutput()));

    setOutput->show();
}

void VideoPlayer::videoOutput()
{
    setOutput->close();

    int format=outFormat->currentIndex();
    int resolving=outResolving->currentIndex();
    int bitrate=outBitRate->currentIndex();

    outPrc=new QProcess(this);
    QString program="./ffmpeg.exe";

    if(resolving==0){
        //ffmpeg -i video_1920.mp4 -vf scale=640:360 video_640.mp4 -hide_banner
        QStringList argument;
        argument<<"-i"<<saveName<<"-vf"<<"scale=1920:1080"<<"./myVideo/test.mp4"<<"-y"<<"-hide_banner";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }
    else if(resolving==1){
        QStringList argument;
        argument<<"-i"<<saveName<<"-vf"<<"scale=1280:720"<<"./myVideo/test.mp4"<<"-y"<<"-hide_banner";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }
    else if(resolving==2){
        QStringList argument;
        argument<<"-i"<<saveName<<"-vf"<<"scale=640:480"<<"./myVideo/test.mp4"<<"-y"<<"-hide_banner";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }

    if(bitrate==0){
        //ffmpeg -i input.mp4 -b:v 2000k -bufsize 2000k output.mp4
        QStringList argument;
        argument<<"-i"<<"./myVideo/test.mp4"<<"-b:v"<<"500k"<<"-bufsize"<<"500k"<<"./myVideo/test.mp4"<<"-y";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }
    else if(bitrate==1){
        //ffmpeg -i input.mp4 -b:v 2000k -bufsize 2000k output.mp4
        QStringList argument;
        argument<<"-i"<<"./myVideo/test.mp4"<<"-b:v"<<"2000k"<<"-bufsize"<<"2000k"<<"./myVideo/test.mp4"<<"-y";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }
    else if(bitrate==2){
        //ffmpeg -i input.mp4 -b:v 2000k -bufsize 2000k output.mp4
        QStringList argument;
        argument<<"./myVideo/test.mp4"<<"./myVideo/test.mp4"<<"-b:v"<<"5000k"<<"-bufsize"<<"5000k"<<"./myVideo/test.mp4"<<"-y";
        outPrc->start(program,argument);
        outPrc->waitForFinished();
    }

    QString orgFilePath="./myVideo/test.mp4";

    if(format==0){
        QString newFilePath=QString("./outPutVideo/%1.mp4").arg(outputName->text());
        QFile::copy(orgFilePath,newFilePath);
    }

    else if(format==1){
        QString newFilePath=QString("./outPutVideo/%1.avi").arg(outputName->text());
        QFile::copy(orgFilePath,newFilePath);
    }

}
