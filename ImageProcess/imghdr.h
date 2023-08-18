#ifndef IMGHDR_H
#define IMGHDR_H

#include <QObject>
#include <QElapsedTimer>

#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
using namespace cv;
using namespace std;
QT_END_NAMESPACE

class imgHDR : public QObject
{
    Q_OBJECT
public:
    explicit imgHDR(QObject *parent = nullptr);
    ~imgHDR();

signals:

public:
    bool hasInitialized;
    Mat coffMat,coffMatC2,mat4derivate_r3c1,mat4derivate_r1c3;
    cuda::GpuMat coffMat_cu,coffMatC2_cu;
    float idConst;

    //计时器
    QElapsedTimer hdrTimer;

    void create_coff_mat();
    //双灰度图片HDR合成
    Mat hdr2GrayImgs(Mat img_src_bright, Mat img_src_dark);
    //单灰度图片HDR合成
    Mat hdr1GrayImgs(Mat img);

private:
    //获取最大梯度图像
    void get_max_derivation(Mat bright_img, Mat dark_img,
                            Mat &max_derivativeX, Mat &max_derivativeY);
    //增强梯度图像
    void enhanceDerivation(Mat img,Mat &max_derivativeX, Mat &max_derivativeY);
    //傅里叶变换数值求解泊松方程
    Mat second_method_solving_Possion(Mat derivativeX, Mat derivativeY);

};

#endif // IMGHDR_H
