#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtWidgets>

#include "Camera/videoplayer.h"
#include "Camera/mpp_compressor.h"
#include "Camera/mpp_player.h"
#include "Camera/MvCamera.h"
#include "ImageProcess/image_processing.h"
#include "ImageProcess/imghdr.h"
#include "RKNN/rknn_inferencer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
using namespace cv;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isDetecting,detecOnly1st,detecOnly2nd;
    QElapsedTimer time1,time2;

private:
    Ui::MainWindow *ui;
    Mat img_input1,img_output1,img_input2,img_output2,img_output3;

    //网络相机参数
    vector<QString> urls;
    MV_CC_DEVICE_INFO_LIST  m_stDevList; 
    //AI工程相关文件路径
    QString proj_path;

    vector<Point2i> basePoints;
    WorkConditionsEnum workCond;
    QSettings *iniRW;
//    //数字相机类（ffmpeg读取）
//    VideoPlayer *cam1;
    //数字相机类（mpp读取）
    MPP_PLAYER *cam1;
    //数字相机类（HIKVISION相机）
    CMvCamera *cam2;
    //图像保存类
    MPP_COMPRESSOR *cmp1,*cmp2;
    //图像处理类
    Image_Processing_Class *imgProcessor1;
    Image_Processing_Class *imgProcessor2;
    RKNN_INFERENCER *imgProcRKNN;
    IMG_HDR *imgProcHDR;
    //图像处理线程
    QThread *imgProThread1,*imgProThread2;
//    //图像保存线程
//    QThread *imgSavThread1,*imgSavThread2;

    QThreadPool mainTP;

    QGraphicsScene scene1, scene2, scene3;
    QGraphicsPixmapItem pixmapShow1, pixmapShow2, pixmapShow3;
    QString strOutput1,strOutput2;
    //显示窗体初始化
    void initImageBoxes();
    //初始化程序工况
    void initWorkCondition();
    //文本输出初始化
    void initTextBrowsers();
    //初始化参数设置
    void initBaseSettings();
    //图像处理类初始化
    void initImageProcess();
    //初始化网络相机设置
    void initCamSettings();
    //数字相机类初始化
    void initVideoPlayers();

private slots:
    //窗体打开图片
    void on_imageboxOpenImage();
    //窗体保存图片
    void on_imageboxSaveImage();
    //选择AI工程和节点按钮槽
    void on_buttonOpenAIProject_clicked();
    //重置按钮槽
     void on_buttonReset_clicked();
    //实时处理按钮槽
    void on_buttonProcess_clicked();
    //开始连续采集按钮槽
    void on_buttonStartCapture_clicked();
    //打开图片序列按钮槽
    void on_buttonOpenImgList_clicked();
    //切换处理算法槽
    void on_condComboBox_activated(int index);

    //图片1刷新槽
    void slotimagebox1Refresh();
    //图片2刷新槽
    void slotimagebox2Refresh();
    //多图片刷新槽
    void slotimageboxesRefresh();
    //视频流1获取槽
    void slotGetOneFrame1(QImage img);
    //视频流2获取槽
    void slotGetOneFrame2(QImage img);

signals:
    //开始单次处理信号
    void startCam1Request();
    void startCam2Request();
    //开始单次保存信号
    void saveCam1Request();
    void saveCam2Request();
    //开始图片组处理信号
    void startPicsProcessRequest();
};
#endif // MAINWINDOW_H
