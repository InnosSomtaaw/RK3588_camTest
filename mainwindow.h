#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtWidgets>

#include "Camera/videoplayer.h"
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
    //数字相机类（mpp读取）
    MPP_PLAYER *cam1;
    //数字相机类（HIKVISION相机）
    CMvCamera *cam2;
//    //数字相机类（ffmpeg读取）
//    VideoPlayer *cam2;
    //图像处理类
    Image_Processing_Class *imgProcessor1;
    Image_Processing_Class *imgProcessor2;
    RKNN_INFERENCER *imgProcRKNN;
    IMG_HDR *imgProcHDR;
    //图像处理线程
    QThread *imgProThread1,*imgProThread2;

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

    //前方摄像头图像鼠标事件响应函数
    void onMouseFront(int event, int x, int y, int flags);
    //后方摄像头图像鼠标事件响应函数
    void onMouseBack(int event, int x, int y, int flags);
    //前方摄像头图像鼠标事件友元函数
    friend void on_mouse_front(int event, int x, int y, int flags, void* params);
    //后方摄像头图像鼠标事件友元函数
    friend void on_mouse_back(int event, int x, int y, int flags, void* params);
    //设置基准
    void setBases(int index);

private slots:
    //窗体1打开图片
    void on_imagebox1_OpenImage();
    //窗体2打开图片
    void on_imagebox2_OpenImage();
    //选择AI工程和节点按钮槽
    void on_buttonOpenAIProject_clicked();
    //重置按钮槽
     void on_buttonReset_clicked();
    //实时处理按钮槽
    void on_buttonProcess_clicked();
    //确认基准按钮槽
    void on_submitBtn_clicked();
    //可编辑状态改变复选框槽
    void on_editCheckBox_stateChanged(int arg1);
    //图片1刷新槽
    void on_imagebox1_refresh();
    //图片2刷新槽
    void on_imagebox2_refresh();
    //多图片刷新槽
    void on_imageboxes_refresh();
    //刷新图片保存参数文本框槽
    void on_textSavingCount_textChanged();
    //开始连续采集按钮槽
    void on_buttonStartCapture_clicked();
    //视频流1获取槽
    void slotGetOneFrame1(QImage img);
    //视频流2获取槽
    void slotGetOneFrame2(QImage img);
    //打开图片序列按钮槽
    void on_buttonOpenImgList_clicked();
    //切换处理算法槽
    void on_condComboBox_activated(int index);

signals:
    //开始单次处理信号
    void startCam1Request();
    void startCam2Request();
    //开始图片组处理信号
    void startPicsProcessRequest();
    //处理参数改变信号
    void changeProcParasRequest(QString,int);
};

void on_mouse_front(int event, int x, int y, int flags, void* params);
void on_mouse_back(int event, int x, int y, int flags, void* params);
#endif // MAINWINDOW_H
