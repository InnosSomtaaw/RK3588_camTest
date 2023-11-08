#include "image_processing.h"

Image_Processing_Class::Image_Processing_Class()
{
    img_input1,img_output1, img_input2, img_output2,img_output3 = NULL;
    hasInited=false;cam1Refreshed=false;cam2Refreshed = false;isDetecting=false;
    img_filenames = new QList<QFileInfo>();

    save_count = 1,max_save_count =100,onceRunTime=0;

    mainwindowIsNext=false;mainwindowIsStopProcess=false;isSavingImage =false;
    onGPU=false;


}

Image_Processing_Class::~Image_Processing_Class()
{

}

void Image_Processing_Class::iniImgProcessor()
{
//    qDebug()<<"imgProc Thread Id: "<<QThread::currentThreadId()<<Qt::endl;
//    sched_setaffinity(0,sizeof(myCPU),&myCPU);
}


void Image_Processing_Class::resetPar()
{
    mainwindowIsNext=false;mainwindowIsStopProcess=false;isSavingImage =false;
    save_count=0;
    hasInited=false;cam1Refreshed=false;cam2Refreshed = false;

    img_filenames->clear();

//    hdrProc->hasInitialized=false;
}

void Image_Processing_Class::generalProcess()
{
    if (img_input1.empty() || !ipcMutex.tryLock())
        return;
    usrtimer.start();

//    current_date_time =QDateTime::currentDateTime();
//    save_count++;
//    qDebug()<<save_count<<" image process start time: "
//           <<current_date_time.toString("hh_mm_ss_zzz")<<Qt::endl;

    if(img_input1.channels()==3)
        cvtColor(img_input1,img_output2,COLOR_RGB2GRAY);
    else
        img_output2 = img_input1.clone();
    ipcMutex.unlock();

    vector<Point> cont;
    if(onGPU)
    {
        UMat umIn,umTemp,umOut;
        img_output2.copyTo(umIn);
        pyrDown(umIn,umTemp);

        bilateralFilter(umTemp,umOut,5,40,75);
        adaptiveThreshold(umOut,umTemp,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,5);

//        Mat morpKern=getStructuringElement(MORPH_RECT,Size(11,11));
//        morphologyEx(umTemp,umOut,MORPH_CLOSE,morpKern);
//        morpKern.release();
//        cout<<"Test Morphology time: "<<usrtimer.elapsed()<<" ms with ocl "<<onGPU<<endl;

        pyrUp(umTemp,umOut);
//        umTemp.release();
//        umIn.release();
        umOut.copyTo(img_output1);
    }
    else
    {
        pyrDown(img_output2,img_output3);

        bilateralFilter(img_output3,img_output2,5,40,75);
        adaptiveThreshold(img_output2,img_output3,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,5);

//        Mat morpKern=getStructuringElement(MORPH_RECT,Size(11,11));
//        morphologyEx(img_output3,img_output2,MORPH_CLOSE,morpKern);
//        morpKern.release();
//        cout<<"Test Morphology time: "<<usrtimer.elapsed()<<" ms with ocl "<<onGPU<<endl;

        pyrUp(img_output3,img_output1);
    }

//        if (save_count % max_save_count == 0)
//        {
//            imwrite("test1.jpg",img_output1);
//            isSavingImage = true;
//        }
//        else
//            isSavingImage = false;

//        save_count++;
//        if (save_count > max_save_count)
//            save_count = 1;

    emit outputImgProcessedRequest();
    onceRunTime = usrtimer.elapsed();

//    cout<<"Current image process thread: "<<QThread::currentThreadId()<<endl;
////    cout<<"Test OCL time: "<<onceRunTime<<" ms with ocl "<<onGPU<<endl;
//    current_date_time =QDateTime::currentDateTime();
//    qDebug()<<save_count<<" image process end time: "
//           <<current_date_time.toString("hh_mm_ss_zzz")<<" used: "<<onceRunTime<<Qt::endl;
}

void Image_Processing_Class::startProcessOnce()
{
//    sched_setaffinity(0,sizeof(myCPU),&myCPU);
    switch (workCond) {
    case GeneralProcess:
    default:
        generalProcess();
        break;
    }
}

void Image_Processing_Class::startPicsProcess()
{
//    emit mainwindowStatusRequest();
    do
    {
        bool flg = processFilePic();
        if(mainwindowIsStopProcess || !flg)
            break;
        QThread::sleep(10);
//        emit mainwindowStatusRequest();
    }while(mainwindowIsNext);
}

void Image_Processing_Class::startMulCamTemp(QImage recvImg, int i)
{
    QImage2Mat(recvImg, img_inputs[i]);
}

void Image_Processing_Class::start1CamProcess(QImage receivedImg)
{
    if(receivedImg.isNull())
        return;
    Mat temp_img(receivedImg.height(),receivedImg.width(),
                CV_8UC4,receivedImg.bits(),
                size_t(receivedImg.bytesPerLine()));
    cvtColor(temp_img,img_input1,COLOR_BGRA2BGR);
//    cvtColor(temp_img,img_input1,CV_BGRA2BGR);
    temp_img.release();
}


void QImage2Mat(QImage img, Mat& imgMat)
{
//    printf("QImage height: %d, width: %d", img.height(), img.width());

//    Mat temp_img(img.height(),img.width(),
//                CV_8UC4,img.bits(),
//                img.bytesPerLine());
//    printf("cvtColor into imgMat");
//    cvtColor(temp_img,imgMat,COLOR_BGRA2BGR);
    Mat temp_img(img.height(), img.width(), CV_8UC3, img.bits(), img.bytesPerLine());
    cvtColor(temp_img, imgMat, COLOR_RGB2BGR);
    temp_img.release();
//    printf("QImage2Mat ended.");
}


void Image_Processing_Class::startMulCamProcess(QImage recvImg, int i)
{
//    if(inputFlags[i])
//    {
//        cout<<i<<" unsync happend."<<endl;
//        return;
//    }
    QImage2Mat(recvImg, img_inputs[i]);
    inputFlags[i] = true;
    for(size_t j = 0; j<4; j++)
    {
        if(!inputFlags[j])
            return;
    }

}

void Image_Processing_Class::changeProcPara(QString qstr, int wc)
{
//    switch (wc) {
//    case WorkConditionsEnum::HDRfrom1Img:
//    case WorkConditionsEnum::HDRfrom1ImgGPU:
//        hdrProc->idConst=qstr.toFloat();
//        break;
//    default:
//        break;
//    }

}

bool Image_Processing_Class::processFilePic()
{
    if (img_filenames->count() > 0)
    {
        img_input1 = imread(img_filenames->at(0).filePath().toLocal8Bit().toStdString());

        if(img_input1.empty())
            return false;

        startProcessOnce();

        img_filenames->removeAt(0);
        return true;
    }
    else
    {
        return false;
    }
}

void ImageWriter::run()
{
    usrtimer.start();

    //按需保存图片：
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy_MM_dd");
    QString current_time =current_date_time.toString("hh_mm_ss_zzz");
    QString dir_str = "./SaveFile/"+current_date+"/";
    QDir dir;
    if (!dir.exists(dir_str))
        dir.mkpath(dir_str);
    QString fn;
    switch (method) {
    case 1:
        fn=dir_str+current_time+"_mannulsaved.bmp";
        break;
    case 2:
        fn=dir_str+current_time+"_mannulsaved.jpg";
        break;
    default:
        fn=dir_str+current_time+"_mannulsaved.png";
        break;
    }

    qimg.save(fn);
    cout<<"Save time(width:"<<qimg.width()<<",height:"<<qimg.height()<<
          "): "<<usrtimer.elapsed()<<" ms."<<endl;
}
