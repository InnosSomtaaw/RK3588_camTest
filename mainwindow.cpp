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

    isDetecting = false;
    //图像处理类初始化
    initImageProcess();

    //初始化程序工况
    initWorkCondition();

    //初始化网络相机设置
    initCamSettings();

    //初始化输入输出图像列表
    initImageInOuts();

    //默认AI路径初始化
//    initDefaultAIPaths();

    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h264/ch1/sub/av_stream");
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/main/av_stream");

    //数字相机类初始化（rtsp读取）
    initVideoPlayers();

    //RKNN类初始化
    rknnInfer=new RKNN_INFERENCER;

    //文本输出初始化
    initTextBrowsers();

    initBaseSettings();
}

MainWindow::~MainWindow()
{
//    if(imgProcess_main->ai_ini_flg)
//        imgProcess_main->this_aue->cppReleaseAI();

    delete ui;
}

void MainWindow::initCamSettings()
{
//    QDomDocument doc;
//    QFile file(_CAMS_SETTING_FILES);
//    if(!file.open(QFile::ReadOnly|QFile::Text))
//    {
//        cout<<"camera settings file can't be found."<<endl;
//        return;
//    }
//    file.close();
//    QString str, name;
//    if(doc.setContent(&file))
//    {
//        int i = 0;
//        QDomElement docElement = doc.documentElement();
//        QDomNode n = docElement.firstChild();
//        while(!n.isNull())
//        {
//            QDomElement e = n.toElement();
//            if(!e.isNull())
//            {
//                str = e.tagName();
////                name = e.attribute("name");
//                if(str == "webCam")
//                {
//                    camUserNames.push_back(e.attribute("userName"));
//                    camPasswords.push_back(e.attribute("password"));
//                    camIPs.push_back(e.attribute("IP"));
//                    camPorts.push_back(e.attribute("port"));
//                    camChannels.push_back(e.attribute("channel"));
//                    camCodes.push_back(e.attribute("code"));
//                    urls.push_back("rtsp://"+camUserNames[i]+":"+camPasswords[i]+"@"+camIPs[i]+":"+camPorts[i]+"/"+
//                            camCodes[i]+"/"+camChannels[i]+"/main/av_stream");
//                    i++;
//                }
//                else if(str == "videoFile")
//                {
//                    urls.push_back(e.attribute("path"));
//                }
//            }
//            n = n.nextSibling();
//        }
//    }
}

void MainWindow::initImageInOuts()
{
//    camCount = 4;
//    for(int i = 0; i<camCount; i++)
//    {
//        Mat imgIn, imgOut;
//        QImage qImg;
//        img_inputs.push_back(imgIn.clone());
//        img_outputs.push_back(imgOut.clone());
//        qimgs.push_back(qImg);
//        counts.push_back(0);
//        isCapturing1s.push_back(false);
//        ws.push_back(0);
//        hs.push_back(0);
//    }
//    maxCount = 10000;
}

void MainWindow::initImageBoxes()
{
    ui->imagebox1->setScene(&scene1);
    scene1.addItem(&pixmapShow1);

    ui->imagebox2->setScene(&scene2);
    scene2.addItem(&pixmapShow2);

    ui->imagebox3->setScene(&scene3);
    scene3.addItem(&pixmapShow3);

    connectImgBox();
}

void MainWindow::initDefaultAIPaths()
{
    proj_path = ".\\AI_Initial_Data\\TGS_train\\TGS_train.proj";
    img_path = ".\\AI_Initial_Data\\TGS_train\\srcImg.bmp";
    node_name = "hasBoxTGS hasNoBox noBoxTGS ";
}

void MainWindow::initImageProcess()
{
    imgProcess_main = new Image_Processing_Class;
    connectIPC();
    m_imgprocsThread = new QThread();
    imgProcess_main->moveToThread(m_imgprocsThread);
    m_imgprocsThread->start();

    imgProcess_main->save_count = 1;
}

void MainWindow::initVideoPlayers()
{
    isCapturing1=false;
    isCapturing2=false;

    //数字相机类初始化（mpp读取）
    mppPlayer1=new MPP_PLAYER;
    mppPlayer1->videoURL = urls[0];
    mppPlayer1->video_type=264;
    connect(mppPlayer1,&MPP_PLAYER::sig_GetOneFrame,
            this,&MainWindow::slotGetOneFrame1);
    rknnThread1 = new QThread();
    mppPlayer1->moveToThread(rknnThread1);
    rknnThread1->start();
    mppPlayer2=new MPP_PLAYER;
    mppPlayer2->videoURL = urls[1];
    mppPlayer2->video_type=265;
    connect(mppPlayer2,&MPP_PLAYER::sig_GetOneFrame,
            this,&MainWindow::slotGetOneFrame2);
    rknnThread2 = new QThread();
    mppPlayer2->moveToThread(rknnThread2);
    rknnThread2->start();

//    mPlayer1 = new VideoPlayer;
//    mPlayer1->videoURL = urls[0];
//    mPlayer2 = new VideoPlayer;
//    mPlayer2->videoURL = urls[1];
//    connect(mPlayer1,SIGNAL(sig_GetOneFrame(QImage)),
//            this,SLOT(slotGetOneFrame1(QImage)));
//    connect(mPlayer2, SIGNAL(sig_GetOneFrame(QImage)),
//            this, SLOT(slotGetOneFrame2(QImage)));
}

void MainWindow::initBaseSettings()
{


//    FileStorage fs(_BASE_SETTING_FILES, FileStorage::READ);
//    if(!fs.isOpened())
//        return;
//    int cond = fs["workCondition"];
//    workCond = (WorkConditionsEnum)cond;
//    imgProcess_main->setWorkCond(workCond);
//    cout<<workCond<<endl;
//    FileNode fnode = fs["baseSettings"];

//    for(FileNodeIterator it = fnode.begin(); it!=fnode.end(); it++)
//    {
//        Point2f p;
//        *it>>p;
////        cout<<p.x<<", "<<p.y<<endl;
//        basePoints.push_back(p);
//    }
//    return;
}

void MainWindow::initTextBrowsers()
{
    ui->textBrowser1->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->textBrowser1->setText("Waiting for works.");
}

void MainWindow::initWorkCondition()
{
    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    workCond=WorkConditionsEnum(iniRW->value("WorkCondition/WorkCondition").toInt());
    imgProcess_main->workCond=workCond;
    ui->condComboBox->setCurrentIndex(workCond);
//    setBases(workCond*2);
}

void MainWindow::on_buttonOpenAIProject_clicked()
{
    proj_path = QFileDialog::getOpenFileName(this,tr("Open Project"),".\\AI_Initial_Data\\",
                                             tr("Project File(*.proj)"));
    if (proj_path.isEmpty())
        return;
    proj_path = proj_path.replace("/","\\");

    img_path = QFileDialog::getOpenFileName(this,tr("Open Sample"),".\\AI_Initial_Data\\",
                                            tr("Sample Picture(*.bmp)"));
    if (img_path.isEmpty())
        return;

    QStringList node_nameList = QFileDialog::getOpenFileNames(this,tr("Open Nodes"),".\\AI_Initial_Data\\",
                                                              tr("Node Files(*.te)"));
    if(node_nameList.isEmpty())
        return;

    node_name.clear();
    for(int i=0;i<node_nameList.size();i++)
    {
        QFileInfo fi = QFileInfo(node_nameList[i]);
        QByteArray ba;
        ba.append(fi.fileName());
        QList<QByteArray> baList = ba.split('.');
        QString str = QString(baList[0]);
        node_name.append(str+" ");
    }
}

void MainWindow::on_buttonProcessOnce_clicked()
{
    this->isDetecting = false;

//    if(this_mb->isConnected)
//        this_mb->startProcessOnce();

    QImage qimg1 = pixmapShow1.pixmap().toImage();

    Mat temp_img;
    temp_img = Mat(qimg1.height(),qimg1.width(),
                CV_8UC4,qimg1.bits(),
                qimg1.bytesPerLine());
    cvtColor(temp_img,img_input1,COLOR_BGRA2BGR);
//    cvtColor(temp_img,img_input1,CV_BGRA2BGR);
    temp_img.release();

    if(img_input1.empty())
        return;

    imgProcess_main->img_input1 = img_input1;
//    imgProcess_main->img_input2 = img_input1.clone();

    //测试AI多工程
//    if(img_input2.empty())
//        imgProcess_main_t->img_input1 = img_input1.clone();
//    else
//        imgProcess_main_t->img_input1 = img_input2;

    if(!this->m_imgprocsThread->isInterruptionRequested())
        emit startProcessOnceRequest();
    //测试AI多工程
//    if(!this->m_imgprocsThread->isInterruptionRequested())
//        emit startProcessOnceRequest1();
//    if(!this->m_imgprocsThread_t->isInterruptionRequested())
//        emit startProcessOnceRequest2();
}

void MainWindow::on_imagebox_refresh()
{
//    if(!imgProcess_main->ipcMutex.tryLock())
//        return;

//    img_outputs[0] = imgProcess_main->img_output1;
////    if(ws[0]>0 && hs[0]>0)
////        cv::resize(img_outputs[0],img_outputs[0],Size(ws[0],hs[0]));

//    cvtColor(img_outputs[0], img_outputs[0], COLOR_BGR2RGB);//图像格式转换
//    vector<QImage> disImages;

//    //窗体显示
//    for(int i = 0; i<4; i++)
//    {
//        if(img_outputs[i].empty())
//            continue;
//        QImage disImage;
//        disImage = QImage(img_outputs[i].data,img_outputs[i].cols,img_outputs[i].rows,
//                             QImage::Format_RGB888);
//        switch(i)
//        {
//        case 0:
////            disImage = disImage.scaled(ui->imagebox1->width()-5, ui->imagebox1->height()-5,
////                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//            pixmapShow1.setPixmap(QPixmap::fromImage(disImage));
////            ui->imagebox1->Adapte();
//            break;
//        case 1:
////            disImage = disImage.scaled(ui->imagebox2->width()-5, ui->imagebox2->height()-5,
////                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//            pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
////            ui->imagebox2->Adapte();
//            break;
//        case 2:
////            disImage = disImage.scaled(ui->imagebox3->width()-5, ui->imagebox3->height()-5,
////                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//            pixmapShow3.setPixmap(QPixmap::fromImage(disImage));
////            ui->imagebox3->Adapte();
//            break;
//        }
//    }
////    pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
//    imgProcess_main->ipcMutex.unlock();

//    //界面状态显示
////    ui->buttonProcessOnce->setEnabled(true);
//    ui->textSavingCount->setStyleSheet("background-color: rgb(255,255,255)");

//    if (imgProcess_main->isSavingImage)
//        ui->textSavingCount->setStyleSheet("background-color: rgb(176,196,222)");
//    else
//        ui->textSavingCount->setStyleSheet("background-color: rgb(255,255,255)");
}

void MainWindow::on_imageboxes_refresh()
{
    if(!imgProcess_main->ipcMutex.tryLock())
        return;

    QImage disImage;

    img_input1=imgProcess_main->img_input1;
    //窗体1显示
    if(img_input1.channels()==1)
        disImage = QImage(img_input1.data,img_input1.cols,img_input1.rows,
                          img_input1.cols*img_input1.channels(),QImage::Format_Grayscale8);
    else
    {
        cvtColor(img_input1, img_input1, COLOR_BGR2RGB);//图像格式转换
        disImage = QImage(img_input1.data,img_input1.cols,img_input1.rows,
                          img_input1.cols*img_input1.channels(),QImage::Format_RGB888);
    }
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));

    switch (workCond){
    case ResizeShow:
        if(!isDetecting)
            break;
        img_output1 = imgProcess_main->img_output1;
        //窗体2显示
        if(img_output1.channels()==1)
            disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                              img_output1.cols*img_output1.channels(),QImage::Format_Grayscale8);
        else
        {
            cvtColor(img_output1, img_output1, COLOR_BGR2RGB);//图像格式转换
            disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                              img_output1.cols*img_output1.channels(),QImage::Format_RGB888);
        }
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        break;
    case HDRfrom1ImgGPU:
    case HDRfrom1Img:
        //窗体2显示
        if(imgProcess_main->img_output2.channels()==1)
            disImage = QImage(imgProcess_main->img_output2.data,
                              imgProcess_main->img_output2.cols,
                              imgProcess_main->img_output2.rows,
                              imgProcess_main->img_output2.cols*imgProcess_main->img_output3.channels(),
                              QImage::Format_Grayscale8);
        else
            disImage = QImage(imgProcess_main->img_output2.data,
                              imgProcess_main->img_output2.cols,
                              imgProcess_main->img_output2.rows,
                              imgProcess_main->img_output2.cols*imgProcess_main->img_output3.channels(),
                              QImage::Format_RGB888);
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        break;
    case HDRfrom2ImgGPU:
    case HDRfrom2Img:
        //窗体2显示
        disImage = QImage(img_input2.data,img_input2.cols,img_input2.rows,
                          img_input2.cols*img_input2.channels(),QImage::Format_RGB888);
        pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
        //窗体3显示
        if(imgProcess_main->img_output3.channels()==1)
            disImage = QImage(imgProcess_main->img_output3.data,
                              imgProcess_main->img_output3.cols,
                              imgProcess_main->img_output3.rows,
                              imgProcess_main->img_output3.cols*imgProcess_main->img_output3.channels(),
                              QImage::Format_Grayscale8);
        else
            disImage = QImage(imgProcess_main->img_output3.data,
                              imgProcess_main->img_output3.cols,
                              imgProcess_main->img_output3.rows,
                              imgProcess_main->img_output3.cols*imgProcess_main->img_output3.channels(),
                              QImage::Format_RGB888);

        pixmapShow3.setPixmap(QPixmap::fromImage(disImage));
        ui->imagebox3->Adapte();
    default:
        break;
    }

    imgProcess_main->ipcMutex.unlock();


    //界面状态显示
//    ui->buttonProcessOnce->setEnabled(true);
    ui->textSavingCount->setStyleSheet("background-color: rgb(255,255,255)");

    if (imgProcess_main->isSavingImage)
        ui->textSavingCount->setStyleSheet("background-color: rgb(176,196,222)");
    else
        ui->textSavingCount->setStyleSheet("background-color: rgb(255,255,255)");
    /*QString str;
    str.sprintf("x: %f, y: %f, z: %f", imgProcess_main->pt1.x/1000, imgProcess_main->pt1.y/1000, imgProcess_main->pt1.z/1000);*/

    QString str;
    QString tempStr;
    for(int i = 0; i<2; i++)
    {
        tempStr.sprintf("The %dth camera:\n", i+1);
        str += tempStr;
        for(int k = 0; k<imgProcess_main->ptsVec[i].size(); k++)
        {
            tempStr.sprintf("x: %4d, y: %4d\n", imgProcess_main->ptsVec[i][k].x, imgProcess_main->ptsVec[i][k].y);
            str += tempStr;
        }
    }
    ui->textBrowser1->setText(str);
    str.clear();
}

void MainWindow::on_mainwindowStatus_inform()
{
//    if(ui->buttonOpenImageList->text()=="Next")
//        imgProcess_main->mainwindowIsNext = true;
//    else
//        imgProcess_main->mainwindowIsNext = false;

    if(!isDetecting)
        imgProcess_main->mainwindowIsStopProcess = true;
    else
        imgProcess_main->mainwindowIsStopProcess = false;

    if(imgProcess_main->img_filenames->isEmpty())
        return;

    if(imgProcess_main->img_filenames->count()>0)
        qDebug()<<imgProcess_main->img_filenames->at(0).fileName();
    else if(imgProcess_main->img_filenames->count()==0)
        ui->buttonReset->click();

}

void MainWindow::connectIPC()
{
    connect(this,&MainWindow::startPicsProcessRequest,
                               imgProcess_main,&Image_Processing_Class::startPicsProcess);

    connect(this, SIGNAL(startMulCamProcessRequest(QImage, int)),
            imgProcess_main, SLOT(startMulCamProcess(QImage, int)));
    connect(this, SIGNAL(startMulCamTempRequest(QImage, int)),
            imgProcess_main, SLOT(startMulCamTemp(QImage, int)));

    connect(imgProcess_main, &Image_Processing_Class::outputMulImgAIRequest,
            this, &MainWindow::on_imageboxes_refresh);

    connect(imgProcess_main,&Image_Processing_Class::outputImgProcessedRequest,
            this,&MainWindow::on_imageboxes_refresh);

    connect(imgProcess_main,&Image_Processing_Class::mainwindowStatusRequest,
            this,&MainWindow::on_mainwindowStatus_inform);

    connect(this,&MainWindow::changeProcParasRequest,
            imgProcess_main,&Image_Processing_Class::changeProcPara);
}

void MainWindow::connectImgBox()
{
    connect(this->ui->imagebox1,&PictureView::outputImgProperty_request,
            this,&MainWindow::on_imagebox1_OpenImage);
    connect(this->ui->imagebox2,&PictureView::outputImgProperty_request,
            this,&MainWindow::on_imagebox2_OpenImage);
}

//void MainWindow::on_buttonOpenVideo_clicked()
//{
//    if(ui->buttonOpenVideo->text()=="OpenVideo" && isCapturing1s[0]==false)
//    {
//        QString videoName = QFileDialog::getOpenFileName(this,tr("Open Video"),".",
//                                                      tr("Video File(*.mp4 *.avi *.wmv *dav)"));
//        if (videoName.isEmpty())
//            return;

//        mPlayer1->videoURL = videoName;
//        isCapturing1s[0] = true;
//        mPlayer1->isCapturing1 = isCapturing1s[0];
//        mPlayer1->startPlay();

//        ui->buttonOpenVideo->setText("Stop");
//    }
//    else if(ui->buttonOpenVideo->text()=="Stop" && isCapturing1s[0]==true)
//    {
//        isCapturing1s[0] = false;
//        mPlayer1->isCapturing1 = isCapturing1s[0];
//        ui->buttonOpenVideo->setText("Continue");
//    }
//    else if(ui->buttonOpenVideo->text()=="Continue" && isCapturing1s[0]==false)
//    {
//        isCapturing1s[0] = true;
//        mPlayer1->isCapturing1 = isCapturing1s[0];
//        ui->buttonOpenVideo->setText("Stop");
//    }
//}

void MainWindow::on_imagebox1_OpenImage()
{
    QString fileName = ui->imagebox1->img_Name_input;
    if (fileName.isEmpty())
        return;

//    Mat srcImage = imread(fileName.toLatin1().data());//读取图片数据
    Mat srcImage = imread(fileName.toLocal8Bit().toStdString());//读取图片数据

    img_input1 = srcImage.clone();
    imgProcess_main->img_input1=img_input1;

    cvtColor(srcImage, srcImage, COLOR_BGR2RGB);//图像格式转换
    QImage disImage = QImage(srcImage.data,srcImage.cols,srcImage.rows,
                             QImage::Format_RGB888);

    //窗体1显示
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));

    ui->imagebox1->Adapte();
    srcImage.release();
}

void MainWindow::on_imagebox2_OpenImage()
{
    QString fileName = ui->imagebox2->img_Name_input;
    if (fileName.isEmpty())
        return;

//    Mat srcImage = imread(fileName.toLatin1().data());//读取图片数据
    Mat srcImage = imread(fileName.toLocal8Bit().toStdString());//读取图片数据

    img_input2 = srcImage.clone();
    imgProcess_main->img_input2=img_input2;

    cvtColor(srcImage, srcImage, COLOR_BGR2RGB);//图像格式转换
    QImage disImage = QImage(srcImage.data,srcImage.cols,srcImage.rows,
                             QImage::Format_RGB888);

    //窗体2显示
    pixmapShow2.setPixmap(QPixmap::fromImage(disImage));
    ui->imagebox2->Adapte();
    srcImage.release();
}

void MainWindow::on_buttonProcess_clicked()
{
    if(imgProcess_main->img_filenames->size()==0
        && !isCapturing1 && !isCapturing2)
    {
        imgProcess_main->startProcessOnce();
        return;
    }

    if(ui->buttonProcess->text()=="Process" && isDetecting==false)
    {
        isDetecting = true;
        ui->buttonProcess->setText("StopProcess");
        ui->condComboBox->setEnabled(false);

//        if (ui->buttonOpenImageList->text() == "Next")
//            if(!this->m_imgprocsThread->isInterruptionRequested())
//                emit startPicsProcessRequest();

    }
    else if (ui->buttonProcess->text() == "StopProcess" && isDetecting == true)
    {
        isDetecting = false;
        ui->buttonProcess->setText("Process");
        ui->condComboBox->setEnabled(true);
    }
}

void MainWindow::on_buttonReset_clicked()
{
//    ui->buttonOpenImageList->setText("OpenImageList");
//    ui->buttonOpenVideo->setText("OpenVideo");
    isCapturing1=false;
    isCapturing2=false;
    printf("reset clicked!");
//    mPlayer1->isCapturing = isCapturing1;
//    mPlayer1->videoURL = urls[0];
//    mPlayer2->isCapturing = isCapturing2;
//    mPlayer2->videoURL = urls[1];

    ui->buttonOpenImgList->setText("OpenImgList");
    isDetecting = false;
    ui->buttonProcess->setText("Process");
    ui->buttonStartCapture->setText("StartCapture");
    imgProcess_main->resetPar();
}

void MainWindow::on_textSavingCount_textChanged()
{
    QString qstr = ui->textSavingCount->toPlainText();
    switch (workCond) {
    case HDRfrom1Img:
    case HDRfrom1ImgGPU:
        emit changeProcParasRequest(qstr,workCond);
        break;
    default:
        imgProcess_main->max_save_count = qstr.toInt();
        break;
    }
}

void MainWindow::on_buttonStartCapture_clicked()
{  
//    isCapturing1=true;
//    char *input[]={"","./model/RK3588/yolov5s-640-640.rknn",
//                  "rtsp://admin:Lead123456@192.168.137.98:554/h264/ch1/sub/av_stream",
//                  "264"};
//    rknnInfer->full_test(4,input);
//    return;

    if(ui->buttonStartCapture->text()=="StartCapture")
    {
        isCapturing1=true;
        isCapturing2=true;
        ui->buttonStartCapture->setText("StopCapture");

        if(!mppPlayer1->hasStarted)
            mppPlayer1->startPlay();
        else
            mppPlayer1->mppMutex.unlock();

//        mppPlayer2->startPlay();

//        mPlayer1->isCapturing = isCapturing1;
//        mPlayer1->startPlay();
//        mPlayer2->isCapturing1 = isCapturing1s[1];
//        mPlayer2->startPlay();

//        mNVR->isCapturing1=isCapturing1s[0];
//        mNVR->cameraPlay4();

        ui->editCheckBox->setEnabled(false);
    }
    else if(ui->buttonStartCapture->text()=="StopCapture")
    {
        isCapturing1=false;
        isCapturing2=false;

        ui->buttonStartCapture->setText("StartCapture");

        mppPlayer1->mppMutex.lock();
//        mppPlayer2->mppMutex.lock();
//        mPlayer1->isCapturing = isCapturing1;
//        mPlayer2->isCapturing1 = isCapturing1s[1];

        if((!imgProcess_main->img_inputs[0].empty())&(!imgProcess_main->img_inputs[2].empty()))
        {
            ui->editCheckBox->setEnabled(true);
        }
    }
}

void MainWindow::on_submitBtn_clicked()
{
//    cv::destroyAllWindows();
//    int index = workCond*2;
//    basePoints[index].x = ui->lineEditX1->text().toInt();
//    basePoints[index].y = ui->lineEditY1->text().toInt();
//    index++;
//    basePoints[index].x = ui->lineEditX2->text().toInt();
//    basePoints[index].y = ui->lineEditY2->text().toInt();

//    FileStorage fs(_BASE_SETTING_FILES, FileStorage::WRITE);
//    if(!fs.isOpened())
//        return;
//    fs<<"workCondition"<< (int)workCond;

//    fs<<"baseSettings"<<"[";
//    for(int i = 0; i<basePoints.size(); i++)
//    {
//        fs<<basePoints[i];
//    }
//    fs<<"]";
//    ui->editCheckBox->setCheckState(Qt::Unchecked);

}

void MainWindow::onMouseFront(int event, int x, int y, int flags)
{
    if(event == EVENT_LBUTTONDOWN)
    {
        QString xstr, ystr;
        xstr.sprintf("%d", x);
        ystr.sprintf("%d", y);
        ui->lineEditX1->setText(xstr);
        ui->lineEditY1->setText(ystr);
        Mat temp = imgProcess_main->img_inputs[0].clone();
        drawMarker(temp, Point(x,y), Scalar(0,0,255));
        imshow("frontLeftImg", temp);
    }
}

void MainWindow::onMouseBack(int event, int x, int y, int flags)
{
    if(event == EVENT_LBUTTONDOWN)
    {
        QString xstr, ystr;
        xstr.sprintf("%d", x);
        ystr.sprintf("%d", y);
        ui->lineEditX2->setText(xstr);
        ui->lineEditY2->setText(ystr);
        Mat temp = imgProcess_main->img_inputs[2].clone();
        drawMarker(temp, Point(x,y), Scalar(0,0,255));
        imshow("backLeftImg", temp);
    }
}

void on_mouse_front(int event, int x, int y, int flags, void* params)
{
    MainWindow* win = static_cast<MainWindow*>(params);
    win->onMouseFront(event, x, y, flags);

}

void on_mouse_back(int event, int x, int y, int flags, void* params)
{
    MainWindow* win = static_cast<MainWindow*>(params);
    win->onMouseBack(event, x, y, flags);
}

void MainWindow::on_editCheckBox_stateChanged(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->lineEditX1->setEnabled(false);
        ui->lineEditX2->setEnabled(false);
        ui->lineEditY1->setEnabled(false);
        ui->lineEditY2->setEnabled(false);
        ui->submitBtn->setEnabled(false);
    }
    else if(state == Qt::Checked)
    {
        ui->buttonReset->click();
        ui->lineEditX1->setEnabled(true);
        ui->lineEditX2->setEnabled(true);
        ui->lineEditY1->setEnabled(true);
        ui->lineEditY2->setEnabled(true);
        ui->submitBtn->setEnabled(true);
        imshow("frontLeftImg", imgProcess_main->img_inputs[0]);
        imshow("backLeftImg", imgProcess_main->img_inputs[2]);
        setMouseCallback("frontLeftImg", on_mouse_front, this);
        setMouseCallback("backLeftImg", on_mouse_back, this);
    }
}

void MainWindow::requestProcess(QImage img, int i)
{
    //对应startMulCamProcessRequest(QImage, int)
    if(imgProcess_main->ipcMutex.tryLock())
    {
        emit startMulCamProcessRequest(img, i);
        imgProcess_main->ipcMutex.unlock();
    }

    /*//对应startMulCamProcessRequest(vector<QImage>)
    qimgs[i] = img;
    if(counts[i]<maxCount)
        counts[i]++;
    else
        counts[i] = 0;

    for(int j = 1; j<4; i++)
    {
        if(counts[0] != counts[j])
            return;
    }

    if(imgProcess_main->ipcMutex.tryLock())
    {
        emit startMulCamProcessRequest(qimgs);
        imgProcess_main->ipcMutex.unlock();
    }*/
}

void MainWindow::slotGetOneFrame1(QImage img)
{
    if(isCapturing1 && !isDetecting)
    {
        //窗体1显示
        pixmapShow1.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox1->Adapte();
//        emit startMulCamTempRequest(img, 0);
//        QImage2Mat(img, img_inputs[0]);
    }

    if(isCapturing1 && isDetecting)
    {
        requestProcess(img, 0);
    }
}

void MainWindow::slotGetOneFrame2(QImage img)
{
    if(isCapturing2 && !isDetecting)
    {
        //窗体2显示
//        img = img.scaled(ui->imagebox2->width()-5, ui->imagebox2->height()-5,
//                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        pixmapShow2.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox2->Adapte();
//        QImage2Mat(img, img_inputs[1]);
    }

    if(isCapturing2 && isDetecting)
    {
        requestProcess(img, 1);
    }
}

void MainWindow::setBases(int index)
{
    QString xstr, ystr;
    xstr.sprintf("%d", basePoints[index].x);
    ystr.sprintf("%d", basePoints[index].y);
    ui->lineEditX1->setText(xstr);
    ui->lineEditY1->setText(ystr);
    index++;
    xstr.sprintf("%d",basePoints[index].x);
    ystr.sprintf("%d", basePoints[index].y);
    ui->lineEditX2->setText(xstr);
    ui->lineEditY2->setText(ystr);
}

void MainWindow::on_buttonOpenImgList_clicked()
{
    if(ui->buttonOpenImgList->text()=="OpenImgList")
    {
        QString filename = QFileDialog::getExistingDirectory();
        QDir *dir=new QDir(filename);
        QStringList filter;
        filter<<"*.png"<<"*.jpg"<<"*.jpeg"<<"*.bmp";

        imgProcess_main->img_filenames =new QList<QFileInfo>(dir->entryInfoList(filter));

        imgProcess_main->processFilePic();
        if(imgProcess_main->img_filenames->count()>0)
            ui->buttonOpenImgList->setText("Next");
    }
    else if(ui->buttonOpenImgList->text()=="Next")
    {
        if (!imgProcess_main->processFilePic())
            ui->buttonOpenImgList->setText("OpenImgList");
    }
}

void MainWindow::on_condComboBox_activated(int index)
{
    ui->buttonReset->click();
    workCond = (WorkConditionsEnum)index;
    imgProcess_main->workCond=workCond;
    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    iniRW->setValue("WorkCondition/WorkCondition",index);
}

