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
    //图像处理类初始化
    initImageProcess();
    //初始化网络相机设置
    initCamSettings();
    //初始化输入输出图像列表
    initImageInOuts();
    //数字相机类初始化
    initVideoPlayers();
    //文本输出初始化
    initTextBrowsers();
    //初始化程序工况
    initWorkCondition();
    //默认AI路径初始化
    initDefaultAIPaths();
    //初始化参数设置
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
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h264/ch1/sub/av_stream");
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/main/av_stream");

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

    connect(this->ui->imagebox1,&PictureView::outputImgProperty_request,
            this,&MainWindow::on_imagebox1_OpenImage);
    connect(this->ui->imagebox2,&PictureView::outputImgProperty_request,
            this,&MainWindow::on_imagebox2_OpenImage);
}

void MainWindow::initDefaultAIPaths()
{
    //RKNN类初始化
    rknnInfer=new RKNN_INFERENCER;
    connect(this,&MainWindow::startRkOnceRequest,
            rknnInfer,&RKNN_INFERENCER::RunOnce);
    connect(rknnInfer,&RKNN_INFERENCER::outputImgProcessedRequest,
            this,&MainWindow::on_imagebox1_refresh);
    rknnThread=new QThread;
    rknnInfer->moveToThread(rknnThread);
    rknnThread->start();

    if(workCond==InferenceRKNN)
    {
        QByteArray qba = proj_path.toLocal8Bit();
        rknnInfer->model_name=qba.data();
        rknnInfer->InitRKNN();
        if(rknnInfer->hasInited)
            ui->buttonOpenAIProject->setEnabled(false);
    }

}

void MainWindow::initImageProcess()
{
    isDetecting = false;
    imgProcess_main = new Image_Processing_Class;
    connect(this,&MainWindow::startPicsProcessRequest,
            imgProcess_main,&Image_Processing_Class::startPicsProcess);

    connect(imgProcess_main, &Image_Processing_Class::outputMulImgAIRequest,
            this, &MainWindow::on_imageboxes_refresh);

    connect(imgProcess_main,&Image_Processing_Class::outputImgProcessedRequest,
            this,&MainWindow::on_imageboxes_refresh);

    connect(imgProcess_main,&Image_Processing_Class::mainwindowStatusRequest,
            this,&MainWindow::on_mainwindowStatus_inform);

    connect(this,&MainWindow::changeProcParasRequest,
            imgProcess_main,&Image_Processing_Class::changeProcPara);

    connect(this, SIGNAL(startMulCamProcessRequest(QImage, int)),
            imgProcess_main, SLOT(startMulCamProcess(QImage, int)));
    connect(this, SIGNAL(startMulCamTempRequest(QImage, int)),
            imgProcess_main, SLOT(startMulCamTemp(QImage, int)));
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
    mppPlayer=new MPP_PLAYER;
    mppPlayer->videoURL = urls[0];
    mppPlayer->video_type=264;
    connect(mppPlayer,&MPP_PLAYER::sig_GetOneFrame,
            this,&MainWindow::slotGetOneFrame1);
    playerThread=new QThread();
    mppPlayer->moveToThread(playerThread);
    playerThread->start();

    //数字相机类初始化（ffmpeg读取）
//    ffPlayer=new VideoPlayer;
//    ffPlayer->videoURL=urls[1];
//    connect(ffPlayer,&VideoPlayer::sig_GetOneFrame,
//            this,&MainWindow::slotGetOneFrame2);
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

    strOutput1="place holder 1...\n";
    strOutput2="\nplace holder 2...\n";
    ui->textBrowser1->setText(strOutput1+strOutput2);
}

void MainWindow::initWorkCondition()
{
    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    workCond=WorkConditionsEnum(iniRW->value("WorkCondition/WorkCondition").toInt());
    proj_path=iniRW->value("InferenceRKNN/ModelPath").toString();
    imgProcess_main->workCond=workCond;
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

        rknnInfer->ipcMutex.unlock();

//        if (ui->buttonOpenImageList->text() == "Next")
//            if(!this->m_imgprocsThread->isInterruptionRequested())
//                emit startPicsProcessRequest();

    }
    else if (ui->buttonProcess->text() == "StopProcess" && isDetecting == true)
    {
        isDetecting = false;
        ui->buttonProcess->setText("Process");
        ui->condComboBox->setEnabled(true);

        rknnInfer->ipcMutex.lock();
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
    ui->buttonOpenAIProject->setEnabled(true);
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
    if(ui->buttonStartCapture->text()=="StartCapture")
    {
        isCapturing1=true;
        isCapturing2=true;
        ui->buttonStartCapture->setText("StopCapture");

        if(!mppPlayer->hasStarted)
            mppPlayer->startPlay();
        else
            mppPlayer->mppMutex.unlock();

//        ffPlayer->isCapturing=isCapturing2;
//        if(!ffPlayer->hasStarted)
//            ffPlayer->startPlay();

        ui->editCheckBox->setEnabled(false);
    }
    else if(ui->buttonStartCapture->text()=="StopCapture")
    {
        isCapturing1=false;
        isCapturing2=false;

        ui->buttonStartCapture->setText("StartCapture");

//        mppPlayer->mppMutex.tryLock();
        mppPlayer->mppMutex.lock();

//        ffPlayer->isCapturing=isCapturing2;

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

void MainWindow::on_imagebox1_refresh()
{
//    if(!rknnInfer->ipcMutex.tryLock())
//        return;

    rknnInfer->img_output1.copyTo(img_output1);
//    rknnInfer->ipcMutex.unlock();

    QImage disImage;
//    窗体1显示
    if(img_output1.channels()==1)
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_Grayscale8);
    else
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_BGR888);

    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));

//    sleep(1);
    int gap2recv=time1.elapsed();
    time1.start();
    //界面状态显示
    QString tempStr;
    tempStr="1st camera refresh time(processed): "+QString::number(gap2recv)+
            "ms. In which process consumed: "+QString::number(rknnInfer->onceRunTime)+
            "ms.\n";
    strOutput1+=tempStr;
    double fps=1000/double(gap2recv);
    tempStr="1st camera FPS(processed): "+QString::number(fps,'f',1)+".\n";
    strOutput1+=tempStr;
    ui->textBrowser1->setText(strOutput1+strOutput2);
    strOutput1.clear();
    tempStr.clear();
}

void MainWindow::on_imagebox2_refresh()
{

}

void MainWindow::slotGetOneFrame1(QImage img)
{
    if(isCapturing1 && !isDetecting)
    {
        //窗体1显示
        pixmapShow1.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox1->Adapte();
        int gap2recv=time1.elapsed();
        time1.start();
        QString tempStr;
        tempStr="1st camera refresh time: "+QString::number(gap2recv)+" ms.\n";
        strOutput1+=tempStr;
        double fps=1000/double(gap2recv);
        tempStr="1st camera FPS: "+QString::number(fps,'f',1)+".\n";
        strOutput1+=tempStr;
        ui->textBrowser1->setText(strOutput1+strOutput2);
        strOutput1.clear();
        tempStr.clear();
    }

    if(isCapturing1 && isDetecting)
    {
        img_input1=Mat(img.height(), img.width(), CV_8UC3,
                        img.bits(), img.bytesPerLine());
        if(rknnInfer->ipcMutex.tryLock())
        {
            img_input1.copyTo(rknnInfer->img_input1);
            rknnInfer->ipcMutex.unlock();
            emit startRkOnceRequest();
        }
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

//    if(isCapturing2 && isDetecting)
//    {

//    }
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

    switch (workCond) {
    case InferenceRKNN:
        if(!rknnInfer->hasInited)
        {
            QByteArray qba = proj_path.toLocal8Bit();
            rknnInfer->model_name=qba.data();
            rknnInfer->InitRKNN();
            if(rknnInfer->hasInited)
                ui->buttonOpenAIProject->setEnabled(false);
        }
        else
            ui->buttonOpenAIProject->setEnabled(false);
        break;
    default:
        break;
    }

    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    iniRW->setValue("WorkCondition/WorkCondition",index);
    iniRW->setValue("InferenceRKNN/ModelPath",proj_path);
}

