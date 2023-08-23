#ifndef IMAGE_PROCESSING_CLASS_H
#define IMAGE_PROCESSING_CLASS_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QFileDialog>
#include <QImage>
#include <qdatetime.h>
#include <QThread>

#include <opencv2/opencv.hpp>

#include "cv_stereomatcher.h"
#include "imghdr.h"

QT_BEGIN_NAMESPACE
using namespace cv;
using namespace std;
QT_END_NAMESPACE
enum WorkConditionsEnum
{
    HDRfrom2Img = 0,
    HDRfrom2ImgGPU,
    HDRfrom1Img,
    HDRfrom1ImgGPU,
    ResizeShow,
    InferenceRKNN,
};
void QImage2Mat(QImage img, Mat& imgMat);

//图像处理类
class Image_Processing_Class : public QObject
{
    Q_OBJECT
public:
    explicit  Image_Processing_Class(QObject *parent = 0);
    ~Image_Processing_Class();

signals:
    //刷新主窗体显示后图片信号
    void outputImgProcessedRequest();
    //刷新主窗体多画面显示信号AI测试用
    void outputMulImgAIRequest();
    //请求主窗体按钮状态信号
    void mainwindowStatusRequest();

public slots:
    //开始单次处理槽
    void startProcessOnce();
    //开始图片组处理槽
    void startPicsProcess();
    void startMulCamTemp(QImage recvImg, int i);
    //开始单相机连续处理槽
    void start1CamProcess(QImage receivedImg);
    //开始多相机连续处理槽
    void startMulCamProcess(QImage recvImg, int i);
    void testMulStereoMatcher();
    //修改参数槽
    void changeProcPara(QString qstr,int wc);

public:
    Mat img_input1,img_output1, img_input2, img_output2,img_output3;
    vector<Mat> img_inputs, img_outputs;
    vector<bool> inputFlags;
    bool ai_ini_flg,cam1Refreshed,cam2Refreshed;
    int save_count,max_save_count;
    bool mainwindowIsNext,mainwindowIsStopProcess,isSavingImage;

    WorkConditionsEnum workCond;
    QMutex ipcMutex;

    //选取文件夹内的所有图片地址
    QList<QFileInfo> *img_filenames;

    QQueue<Point> *basePTs;
    QQueue<bool> *islifting;
//    CVStereoMatcher stereoMatcher;
    Point3f pt1, pt2;
    vector<vector<Point> > ptsVec;

    //计时器
    QElapsedTimer ustimer;

    //处理图片组
    bool processFilePic();
    //初始化输入输出图像
    void initImgInOut();
    //重置参数
    void resetPar();

private:
    imgHDR *hdrProc;
    void hdr2Imgs(bool onGPU=false);
    void hdrImg(bool onGPU=false);
    void resizeCompare();

};

#endif // IMAGE_PROCESSING_CLASS_H
