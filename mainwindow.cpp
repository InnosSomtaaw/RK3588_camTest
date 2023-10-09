#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtXml/QDomDocument>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //显示窗体初始化
    initImageBoxes();
    //初始化程序工况
    initWorkCondition();
    //文本输出初始化
    initTextBrowsers();
    //初始化参数设置
    initBaseSettings();
    //图像处理类初始化
    initImageProcess();
    //初始化网络相机设置
    initCamSettings();
    //数字相机类初始化
    initVideoPlayers();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCamSettings()
{
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/sub/av_stream");
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/main/av_stream");

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        QString strMsg;
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]GigE:   %s  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName, nIp1, nIp2, nIp3, nIp4);
            }
            else
            {
                strMsg.sprintf("[%d]GigE:   %s %s (%s)  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                               pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
            }
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]UsbV3:  %s", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
            }
            else
            {
                strMsg.sprintf("[%d]UsbV3:  %s %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
                               pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        else
        {
            ShowErrorMsg("Unknown device enumerated", 0);
        }
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }
}

void MainWindow::initImageBoxes()
{
    ui->imagebox1->setScene(&scene1);
    scene1.addItem(&pixmapShow1);

    ui->imagebox2->setScene(&scene2);
    scene2.addItem(&pixmapShow2);

    ui->imagebox3->setScene(&scene3);
    scene3.addItem(&pixmapShow3);

    connect(this->ui->imagebox1,&PictureView::loadImgRequest,
            this,&MainWindow::on_imageboxOpenImage);
    connect(this->ui->imagebox2,&PictureView::loadImgRequest,
            this,&MainWindow::on_imageboxOpenImage);
    connect(this->ui->imagebox1,&PictureView::saveImgRequest,
            this,&MainWindow::on_imageboxSaveImage);
    connect(this->ui->imagebox2,&PictureView::saveImgRequest,
            this,&MainWindow::on_imageboxSaveImage);
}

void MainWindow::initImageProcess()
{
    imgProcHDR=new IMG_HDR;
    imgProcHDR->workCond=workCond;
    //RKNN类初始化
    imgProcRKNN=new RKNN_INFERENCER;
    imgProcRKNN->workCond=workCond;
    switch (workCond) {
    case HDRfrom2Img:
    case HDRfrom2ImgGPU:
        connect(imgProcHDR,&IMG_HDR::outputMulImgAIRequest,
                this,&MainWindow::slotimageboxesRefresh);
        imgProcessor1=imgProcHDR;
        break;
    case HDRfrom1Img:
    {
        connect(this,&MainWindow::startCam1Request,
                imgProcHDR,&IMG_HDR::startProcessOnce);
        connect(imgProcHDR,&IMG_HDR::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        imgProcessor1=imgProcHDR;
        break;
    }
    case InferenceRKNN:
    {
        connect(this,&MainWindow::startCam1Request,
                imgProcRKNN,&RKNN_INFERENCER::startProcessOnce);
        connect(imgProcRKNN,&RKNN_INFERENCER::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        QByteArray qba = proj_path.toLocal8Bit();
        imgProcRKNN->model_name=qba.data();
        imgProcRKNN->iniImgProcessor();
        if(imgProcRKNN->hasInited)
            ui->buttonOpenAIProject->setEnabled(false);
        imgProcessor1=imgProcRKNN;
        break;
    }
    case GeneralProcess:
    default:
    {
        imgProcessor1=new Image_Processing_Class;
        imgProcessor1->workCond=workCond;
        connect(this,&MainWindow::startCam1Request,
                imgProcessor1,&Image_Processing_Class::startProcessOnce);
        connect(imgProcessor1,&Image_Processing_Class::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        break;
    }
    }
    imgProThread1 = new QThread();
    imgProcessor1->moveToThread(imgProThread1);
    imgProThread1->start();

    imgProcessor2=new Image_Processing_Class;
    imgProcessor2->workCond=workCond;
    imgProcessor2->onGPU=true;
    ocl::setUseOpenCL(imgProcessor2->onGPU);
    connect(this,&MainWindow::startCam2Request,
            imgProcessor2,&Image_Processing_Class::startProcessOnce);
    connect(imgProcessor2,&Image_Processing_Class::outputImgProcessedRequest,
            this,&MainWindow::slotimagebox2Refresh);
    imgProThread2 = new QThread();
    imgProcessor2->moveToThread(imgProThread2);
    imgProThread2->start();
}

void MainWindow::initVideoPlayers()
{
//    //数字相机类初始化（ffmpeg读取）
//    cam1=new VideoPlayer;
//    cam1->camURL=urls[1];
//    connect(cam1,&VideoPlayer::sigGetOneFrame,
//            this,&MainWindow::slotGetOneFrame1);

    //数字相机类初始化（mpp读取）
    cam1=new MPP_PLAYER;
    cam1->camURL = urls[1];
    connect(cam1,&MPP_PLAYER::sigGetOneFrame,
            this,&MainWindow::slotGetOneFrame1);

    //数字相机类初始化（HIKVISION读取）
    cam2=new CMvCamera;
    if(m_stDevList.nDeviceNum<1)
        return;
    cam2->m_stDevList=m_stDevList;
    cam2->nIndex=0;
    cam2->froceRaw=false;
    cam2->startCamera();
    connect(cam2,&CMvCamera::sigGetOneFrame,
            this,&MainWindow::slotGetOneFrame2);

    //图像保存类初始化(mpp)
    cmp1=new MPP_COMPRESSOR;
    connect(this,&MainWindow::saveCam1Request,
            cmp1,&MPP_COMPRESSOR::getOneFrame);
//    imgSavThread1=new QThread();
//    cmp1->moveToThread(imgSavThread1);
//    imgSavThread1->start();

    cmp2=new MPP_COMPRESSOR;
    MVCC_INTVALUE_EX stIntValue;
    memset(&stIntValue, 0, sizeof(MVCC_INTVALUE_EX));
    cam2->GetIntValue("Width",&stIntValue);
    cmp2->width=stIntValue.nCurValue;
    cam2->GetIntValue("Height",&stIntValue);
    cmp2->height=stIntValue.nCurValue;
    cmp2->startCamera();
    connect(this,&MainWindow::saveCam2Request,
            cmp2,&MPP_COMPRESSOR::getOneFrame);
//    imgSavThread2=new QThread();
//    cmp2->moveToThread(imgSavThread2);
//    imgSavThread2->start();
}

void MainWindow::initBaseSettings()
{
    isDetecting=false;
    detecOnly1st=false;detecOnly2nd=false;

    mainTP.setMaxThreadCount(8);
}

void MainWindow::initTextBrowsers()
{
    ui->textBrowser1->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->textBrowser2->setStyleSheet("background:transparent;border-width:0;border-style:outset");

    strOutput1="place holder 1...\n";
    strOutput2="place holder 2...\n";
    ui->textBrowser1->setText(strOutput1);
    ui->textBrowser2->setText(strOutput2);
}

void MainWindow::initWorkCondition()
{
    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    workCond=WorkConditionsEnum(iniRW->value("WorkCondition/WorkCondition").toInt());
    proj_path=iniRW->value("InferenceRKNN/ModelPath").toString();
    ui->condComboBox->setCurrentIndex(workCond);
}

void MainWindow::on_buttonOpenAIProject_clicked()
{
    proj_path = QFileDialog::getOpenFileName(this,tr("Open RKNN Model: "),"./model/",
                                             tr("Model File(*.rknn)"));
    if (proj_path.isEmpty())
        return;
//    proj_path = proj_path.replace("/","\");
}


void MainWindow::slotimageboxesRefresh()
{
//    if(!imgProcessor1->ipcMutex.tryLock())
//        return;

    QImage disImage;

    img_input1=imgProcessor1->img_input1.clone();
    //窗体1显示
    if(img_input1.channels()==1)
        disImage = QImage(img_input1.data,img_input1.cols,img_input1.rows,
                          img_input1.cols*img_input1.channels(),QImage::Format_Grayscale8);
    else
    {
//        cvtColor(img_input1, img_input1, COLOR_BGR2RGB);//图像格式转换
        disImage = QImage(img_input1.data,img_input1.cols,img_input1.rows,
                          img_input1.cols*img_input1.channels(),QImage::Format_BGR888);
    }
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));

    switch (workCond){
    case GeneralProcess:
        if(!isDetecting)
            break;
        img_output1 = imgProcessor1->img_output1.clone();
        //窗体2显示
        if(img_output1.channels()==1)
            disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                              img_output1.cols*img_output1.channels(),QImage::Format_Grayscale8);
        else
        {
//            cvtColor(img_output1, img_output1, COLOR_BGR2RGB);//图像格式转换
            disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                              img_output1.cols*img_output1.channels(),QImage::Format_BGR888);
        }
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        break;
    case HDRfrom1ImgGPU:
    case HDRfrom1Img:
        //窗体2显示
        if(imgProcessor1->img_output2.channels()==1)
            disImage = QImage(imgProcessor1->img_output2.data,
                              imgProcessor1->img_output2.cols,
                              imgProcessor1->img_output2.rows,
                              imgProcessor1->img_output2.cols*imgProcessor1->img_output3.channels(),
                              QImage::Format_Grayscale8);
        else
            disImage = QImage(imgProcessor1->img_output2.data,
                              imgProcessor1->img_output2.cols,
                              imgProcessor1->img_output2.rows,
                              imgProcessor1->img_output2.cols*imgProcessor1->img_output3.channels(),
                              QImage::Format_RGB888);
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        break;
    case HDRfrom2ImgGPU:
    case HDRfrom2Img:
        //窗体2显示
        if(img_input2.channels()==1)
            disImage = QImage(img_input2.data,img_input2.cols,img_input2.rows,
                              img_input2.cols*img_input2.channels(),QImage::Format_Grayscale8);
        else
        {
    //        cvtColor(img_input1, img_input1, COLOR_BGR2RGB);//图像格式转换
            disImage = QImage(img_input2.data,img_input2.cols,img_input2.rows,
                              img_input2.cols*img_input2.channels(),QImage::Format_BGR888);
        }
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        //窗体3显示
        if(imgProcessor1->img_output3.channels()==1)
            disImage = QImage(imgProcessor1->img_output3.data,
                              imgProcessor1->img_output3.cols,
                              imgProcessor1->img_output3.rows,
                              imgProcessor1->img_output3.cols*imgProcessor1->img_output3.channels(),
                              QImage::Format_Grayscale8);
        else
            disImage = QImage(imgProcessor1->img_output3.data,
                              imgProcessor1->img_output3.cols,
                              imgProcessor1->img_output3.rows,
                              imgProcessor1->img_output3.cols*imgProcessor1->img_output3.channels(),
                              QImage::Format_RGB888);

        pixmapShow3.setPixmap(QPixmap::fromImage(disImage));
        ui->imagebox3->Adapte();
    default:
        break;
    }

//    imgProcessor1->ipcMutex.unlock();

    //界面状态显示
}

void MainWindow::on_imageboxOpenImage()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,tr("Open Image"),".",tr("Image File(*.png *.jpg *.jpeg *.bmp)"));

    if (fileName.isEmpty())
        return;

    QImageReader qir(fileName);
    QImage img=qir.read();

    Mat srcImage;
    if(img.format()==QImage::Format_RGB888)
        srcImage=Mat(img.height(), img.width(), CV_8UC3,
                       img.bits(), img.bytesPerLine());
    else if(img.format()==QImage::Format_RGB32 ||
            img.format()==QImage::Format_RGBA8888)
        srcImage=Mat(img.height(), img.width(), CV_8UC4,
                       img.bits(), img.bytesPerLine());
    else
        srcImage=Mat(img.height(), img.width(), CV_8UC1,
                       img.bits(), img.bytesPerLine());

    QObject *ptrSender=sender();
    if(ptrSender==ui->imagebox1)
    {
        img_input1 = srcImage.clone();
        imgProcessor1->img_input1=img_input1;
        //窗体1显示
        pixmapShow1.setPixmap(QPixmap::fromImage(img));
        ui->imagebox1->Adapte();
    }
    else if(ptrSender==ui->imagebox2)
    {
        img_input2 = srcImage.clone();
        imgProcessor1->img_input2=img_input2;
        //窗体2显示
        pixmapShow2.setPixmap(QPixmap::fromImage(img));
        ui->imagebox2->Adapte();
    }

    srcImage.release();
}

void MainWindow::on_imageboxSaveImage()
{
    ImageWriter *iw=new ImageWriter;
    //保存方式：0-png;1-bmp;2-jpg;
    iw->method=2;

    QObject *ptrSender=sender();
    if(ptrSender==ui->imagebox1)
        iw->qimg=pixmapShow1.pixmap().toImage();
    else if(ptrSender==ui->imagebox2)
        iw->qimg=pixmapShow2.pixmap().toImage();

    mainTP.start(iw);
}

void MainWindow::on_buttonProcess_clicked()
{
    if(imgProcessor1->img_filenames->size()==0
        && !cam1->isCapturing && !cam2->isCapturing)
    {
        isDetecting=true;
        imgProcessor1->isDetecting=isDetecting;
        imgProcessor1->startProcessOnce();
        return;
    }

    if(ui->buttonProcess->text()=="Process" && isDetecting==false)
    {
        isDetecting=true;
        imgProcessor1->isDetecting=isDetecting;
        imgProcessor2->isDetecting=isDetecting;
        ui->buttonProcess->setText("StopProcess");
        ui->condComboBox->setEnabled(false);

        imgProcessor1->ipcMutex.unlock();
        imgProcessor2->ipcMutex.unlock();

//        if (ui->buttonOpenImageList->text() == "Next")
//            if(!this->m_imgprocsThread->isInterruptionRequested())
//                emit startPicsProcessRequest();

    }
    else if (ui->buttonProcess->text() == "StopProcess" && isDetecting == true)
    {
        isDetecting=false;
        imgProcessor1->isDetecting=isDetecting;
        imgProcessor2->isDetecting=isDetecting;
        ui->buttonProcess->setText("Process");
        ui->condComboBox->setEnabled(true);

        imgProcessor1->ipcMutex.tryLock();
        imgProcessor2->ipcMutex.tryLock();
    }
}

void MainWindow::on_buttonReset_clicked()
{
//    ui->buttonOpenImageList->setText("OpenImageList");
//    ui->buttonOpenVideo->setText("OpenVideo");
    cam1->isCapturing=false;
    cam2->isCapturing=false;
    printf("reset clicked!");

    ui->buttonOpenImgList->setText("OpenImgList");
    isDetecting = false;
    ui->buttonProcess->setText("Process");
    ui->buttonStartCapture->setText("StartCapture");
    imgProcessor1->resetPar();
    ui->buttonOpenAIProject->setEnabled(true);
}

void MainWindow::on_buttonStartCapture_clicked()
{  
    if(ui->buttonStartCapture->text()=="StartCapture")
    {
        cam1->isCapturing=true;
        cam2->isCapturing=true;
        ui->buttonStartCapture->setText("StopCapture");

        if(!cam1->hasStarted)
            cam1->startCamera();
        else
            cam1->camMutex.unlock();

        cam2->StartGrabbing();
    }
    else if(ui->buttonStartCapture->text()=="StopCapture")
    {
        cam1->isCapturing=false;
        cam2->isCapturing=false;

        ui->buttonStartCapture->setText("StartCapture");

        cam1->camMutex.tryLock();
        cam2->StopGrabbing();
    }
}


void MainWindow::slotimagebox1Refresh()
{
    imgProcessor1->img_output1.copyTo(img_output1);
    QImage disImage;
//    窗体1显示
    if(img_output1.channels()==1)
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_Grayscale8);
    else
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_BGR888);
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));
    int gap2recv=time1.elapsed();
    time1.start();
    //界面状态显示
    QString tempStr;
    tempStr="1st camera refresh time(processed): "+QString::number(gap2recv)+
            "ms. In which process consumed: "+QString::number(imgProcessor1->onceRunTime)+
            "ms.\n";
    strOutput1+=tempStr;
    double fps=1000/double(gap2recv);
    tempStr="1st camera FPS(processed): "+QString::number(fps,'f',1)+".\n";
    strOutput1+=tempStr;
    ui->textBrowser1->setText(strOutput1);
    strOutput1.clear();
    tempStr.clear();
}

void MainWindow::slotimagebox2Refresh()
{
//    if(!imgProcessor2->ipcMutex.tryLock())
//        return;
    imgProcessor2->img_output1.copyTo(img_output2);
//    imgProcessor2->ipcMutex.unlock();

    QImage disImage;
//    窗体2显示
    if(img_output2.channels()==1)
        disImage = QImage(img_output2.data,img_output2.cols,img_output2.rows,
                          img_output2.cols*img_output2.channels(),QImage::Format_Grayscale8);
    else
        disImage = QImage(img_output2.data,img_output2.cols,img_output2.rows,
                          img_output2.cols*img_output2.channels(),QImage::Format_BGR888);
    pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
    int gap2recv=time2.elapsed();
    time2.start();
    //界面状态显示
    QString tempStr;
    tempStr="2nd camera refresh time(onGPU "+QString::number(int(imgProcessor2->onGPU))+
            "): "+QString::number(gap2recv)+"ms. In which process consumed: "+
            QString::number(imgProcessor2->onceRunTime)+"ms.\n";
    strOutput2+=tempStr;
    double fps=1000/double(gap2recv);
    tempStr="2nd camera FPS(processed): "+QString::number(fps,'f',1)+".\n";
    strOutput2+=tempStr;
    ui->textBrowser2->setText(strOutput2);
    strOutput2.clear();
    tempStr.clear();

}

void MainWindow::slotGetOneFrame1(QImage img)
{
    if(cam1->isCapturing && !isDetecting)
    {
        //窗体1显示
        pixmapShow1.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox1->Adapte();
        int gap2recv=time1.elapsed();
        time1.start();
        QString tempStr;
        tempStr="\n1st camera refresh time: "+QString::number(gap2recv)+" ms.\n";
        strOutput1+=tempStr;
        double fps=1000/double(gap2recv);
        tempStr="1st camera FPS: "+QString::number(fps,'f',1)+".\n";
        strOutput1+=tempStr;
        ui->textBrowser1->setText(strOutput1);
        strOutput1.clear();
        tempStr.clear();
    }

    if(cam1->isCapturing && isDetecting)
    {
        if(img.format()==QImage::Format_RGB888)
            img_input1=Mat(img.height(), img.width(), CV_8UC3,
                           img.bits(), img.bytesPerLine());
        else if(img.format()==QImage::Format_RGB32 ||
                img.format()==QImage::Format_RGBA8888)
            img_input1=Mat(img.height(), img.width(), CV_8UC4,
                           img.bits(), img.bytesPerLine());
        else
            img_input1=Mat(img.height(), img.width(), CV_8UC1,
                           img.bits(), img.bytesPerLine());
        if(imgProcessor1->ipcMutex.tryLock())
        {
            img_input1.copyTo(imgProcessor1->img_input1);
            imgProcessor1->ipcMutex.unlock();
            emit startCam1Request();
        }
    }

    if(ui->checkBoxSaving->isChecked())
    {
        if(!cmp1->hasStarted)
        {
            cmp1->width=img.width();
            cmp1->height=img.height();
            cmp1->startCamera();
        }
        else
        {
            cmp1->img=img;
            cmp1->getOneFrame();
//            if(cmp1->camMutex.tryLock())
//            {
//                cmp1->img=img;
//                cmp1->camMutex.unlock();
//                emit saveCam1Request();
//            }
//            else
//                cout<<"Cam1 Save skipped!"<<endl;
        }
    }
}

void MainWindow::slotGetOneFrame2(QImage img)
{
    if(cam2->isCapturing && !isDetecting)
    {
        //窗体2显示
        pixmapShow2.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox2->Adapte();
        int gap2recv=time2.elapsed();
        time2.start();
        QString tempStr;
        tempStr="\n2nd camera refresh time: "+QString::number(gap2recv)+" ms.\n";
        strOutput2+=tempStr;
        double fps=1000/double(gap2recv);
        tempStr="2nd camera FPS: "+QString::number(fps,'f',1)+".\n";
        strOutput2+=tempStr;
        ui->textBrowser2->setText(strOutput2);
        strOutput2.clear();
        tempStr.clear();
    }

    if(cam2->isCapturing && isDetecting)
    {
        if(img.format()==QImage::Format_RGB888)
            img_input2=Mat(img.height(), img.width(), CV_8UC3,
                           img.bits(), img.bytesPerLine());
        else if(img.format()==QImage::Format_RGB32 ||
                img.format()==QImage::Format_RGBA8888)
            img_input2=Mat(img.height(), img.width(), CV_8UC4,
                           img.bits(), img.bytesPerLine());
        else
            img_input2=Mat(img.height(), img.width(), CV_8UC1,
                           img.bits(), img.bytesPerLine());
        if(imgProcessor2->ipcMutex.tryLock())
        {
            img_input2.copyTo(imgProcessor2->img_input1);
            imgProcessor2->ipcMutex.unlock();
            emit startCam2Request();
        }
    }

    if(ui->checkBoxSaving->isChecked())
    {
        cmp2->img=img;
        cmp2->getOneFrame();
//        if(cmp2->camMutex.tryLock())
//        {
//            cmp2->img=img;
//            cmp2->camMutex.unlock();
//            emit saveCam2Request();
//        }
//        else
//            cout<<"Cam2 Save skipped!"<<endl;
    }
}

void MainWindow::on_buttonOpenImgList_clicked()
{
    if(ui->buttonOpenImgList->text()=="OpenImgList")
    {
        QString filename = QFileDialog::getExistingDirectory();
        QDir *dir=new QDir(filename);
        QStringList filter;
        filter<<"*.png"<<"*.jpg"<<"*.jpeg"<<"*.bmp";

        imgProcessor1->img_filenames =new QList<QFileInfo>(dir->entryInfoList(filter));

        imgProcessor1->processFilePic();
        if(imgProcessor1->img_filenames->count()>0)
            ui->buttonOpenImgList->setText("Next");
    }
    else if(ui->buttonOpenImgList->text()=="Next")
    {
        if (!imgProcessor1->processFilePic())
            ui->buttonOpenImgList->setText("OpenImgList");
    }
}

void MainWindow::on_condComboBox_activated(int index)
{
    ui->buttonReset->click();
    workCond = (WorkConditionsEnum)index;
    switch (workCond) {
    case HDRfrom2Img:
    case HDRfrom2ImgGPU:
        connect(imgProcHDR,&IMG_HDR::outputMulImgAIRequest,
                this,&MainWindow::slotimageboxesRefresh);
        imgProcessor1=imgProcHDR;
        break;
    case HDRfrom1Img:
    {
        connect(this,&MainWindow::startCam1Request,
                imgProcHDR,&IMG_HDR::startProcessOnce);
        connect(imgProcHDR,&IMG_HDR::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        imgProcessor1=imgProcHDR;
        break;
    }
    case InferenceRKNN:
    {
        if(!imgProcRKNN->hasInited)
        {
            QByteArray qba = proj_path.toLocal8Bit();
            imgProcRKNN->model_name=qba.data();
            imgProcRKNN->iniImgProcessor();
            if(imgProcRKNN->hasInited)
                ui->buttonOpenAIProject->setEnabled(false);
        }
        else
            ui->buttonOpenAIProject->setEnabled(false);
        connect(this,&MainWindow::startCam1Request,
                imgProcRKNN,&RKNN_INFERENCER::startProcessOnce);
        connect(imgProcRKNN,&RKNN_INFERENCER::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        imgProcessor1=imgProcRKNN;
        break;
    }
    case GeneralProcess:
    default:
    {
        if(imgProcessor1!=nullptr)
            delete imgProcessor1;
        imgProcessor1=new Image_Processing_Class;
        connect(this,&MainWindow::startCam1Request,
                imgProcessor1,&Image_Processing_Class::startProcessOnce);
        connect(imgProcessor1,&Image_Processing_Class::outputImgProcessedRequest,
                this,&MainWindow::slotimagebox1Refresh);
        break;
    }
    }
    imgProcessor1->workCond=workCond;

    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    iniRW->setValue("WorkCondition/WorkCondition",index);
    iniRW->setValue("InferenceRKNN/ModelPath",proj_path);
}
