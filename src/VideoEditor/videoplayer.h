#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "video.h"
#include "timelinelabel.h"

#include <QMainWindow>
#include <QtMultimedia>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QMediaPlayer>
#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QPainter>
#include <QVector>
#include <QMediaPlaylist>
#include <vector>
#include <QFile>
#include <QComboBox>

namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

    bool isReload;
    int sumTime=0; // 时间轴中的视频总长

signals:
    void sentIndex(int &index);

public slots:

    // 播放相关槽
    void setVideoFile(void);
    void onSlider(qint64);
    void onDurationChanged(qint64);
    void onStateChanged(QMediaPlayer::State);
    void showVideo();

    // 时间显示相关槽
    void getAllTime();
    void getNowTime();

    // 视频列表相关槽
    void videoFileList();
    void videoNameChanged();
    void setVideoName();
    void setMedia();
    void videoDel();
    void videoAdd();

    // 素材相关槽
    void setImageFile(void);
    void imageFileList();
    void imageNameChanged();
    void setImageName();
    void imageDel();
    void setImageAdd();
    void imageAdd();
    //void imageTLDel();

    // 视频剪切相关槽
    void getCutTime(); // 获取剪切的起始时间
    void cutVideo(); // 剪切视频
    void setCutTime(); // 手动设置剪切开始时间

    // 时间轴操作相关槽
    void videoTLList();
    void videoBack();
    void videoForward();
    void videoMBack();
    void doVideoMBack();
    void videoMForward();
    void doVideoMForward();
    void videoTLDel();
    void pointCut();
    void doPointCut();

    // 视频预览相关槽
    void doVideoPlay();
    void doImagePlay();
    void doWordPlay();

    // 添加文字相关槽
    void addWordDlg();
    void setWordTLLabel();
    //void wordTLDel();

    // 字幕文件相关槽
    void setSubtitleFile();
    void addSub();

    // 视频导出相关槽
    void setOutputName();
    void videoOutput();

private:

    Ui::VideoPlayer *ui;

    int videoCutNum=0;
    int videoMergeNum=0;
    int videoNum=0;

    QVideoWidget *playerWidget; // 播放视频区
    QMediaPlayer *myPlayer; // 视频播放器

    int playTime; // 视频时长
    int allTime; // 时间轴中视频总时长
    int nowTime; // 当前时刻

    QVector<video> myVideo;
    QVector<timeLineLabel*> myTimeLineLabel;
    QVector<timeLineLabel*> myImageLabel;
    QVector<timeLineLabel*> myWordLabel;


    // 视频列表操作
    QMenu *videoListMenu;
    QAction *renameV;
    QAction *deletV;
    QAction *addToTimeLine;

    // 重命名相关
    int indexSelected; // 需要重命名的序号
    QDialog *renameDlg;
    QLineEdit *newName;
    QLabel *renameLabel;
    QPushButton *ok;

    QStringList imageFilePaths; // 素材路径（不可变）
    QStringList imageFileNames; // 素材名称（可变）

    QStringList subtitleFilePaths;

    // 素材添加设置相关
    QDialog *addImageSetting;
    QLabel *imageStartTime;
    QLabel *imageEndTime;
    QLineEdit *iSTm;
    QLineEdit *iSTs;
    QLineEdit *iETm;
    QLineEdit *iETs;
    QLineEdit *ix;
    QLineEdit *iy;
    QLabel *setImageEffects;
    QComboBox *imageEffects;
    QLabel *setIETime;
    QLineEdit *IETime;


    // 素材列表操作
    QMenu *imageListMenu;
    QAction *renameI;
    QAction *deletI;
    QAction *addI;

    // 视频剪切
    QProcess *cutPrc;
    bool isCut=true;
    QString cutStartTime;
    QString cutEndTime;

    QDialog *setCutTimeDlg;
    QLineEdit *shh;
    QLineEdit *smm;
    QLineEdit *sss;
    QLineEdit *ehh;
    QLineEdit *emm;
    QLineEdit *ess;
    QLabel *cutStartTimeLabel;
    QLabel *cutEndTimeLabel;
    QLabel *colonLabel1;
    QLabel *colonLabel2;
    QLabel *colonLabel3;
    QLabel *colonLabel4;

    bool isSet=false; // 是否手动设置

    // 时间轴操作
    QMenu *videoTLMenu;
    QAction *videoMoveBack;
    QAction *videoMoveForward;
    QAction *videoMergeBack;
    QAction *videoMergeForward;
    QAction *videoDelete;
    QAction *videoCut;

    // 视频切分
    QDialog *cutDlg;
    QLabel *cutLabel;
    QLineEdit *min;
    QLineEdit *sec;
    QLabel *colon;

    // 视频合并
    QProcess *mergePrc;
    QString mergeSaveName;

    // 视频预览
    QString saveName;
    QProcess *videoPrc;
    QProcess *imagePrc;
    QProcess *wordPrc;

    // 文字素材添加设置
    QDialog *addWord;
    QLabel *wordLabel;
    QLineEdit *word;

    // 字幕
    QProcess *subPrc;
    int subNum=0;
    QString subname;

    // 导出
    QDialog *setOutput;
    QLabel *setName;
    QLineEdit *outputName;
    QLabel *outFormatLabel;
    QComboBox *outFormat;
    QLabel *outResolvingLabel;
    QComboBox *outResolving;
    QLabel *outBitRateLabel;
    QComboBox *outBitRate;
    QProcess *outPrc;

};

#endif // VIDEOPLAYER_H
