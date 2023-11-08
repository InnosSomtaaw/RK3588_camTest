#ifndef GENERAL_CAMERA_H
#define GENERAL_CAMERA_H

#include <sched.h>

#include <iostream>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include <QString>
#include <QImage>
#include <QMutex>

QT_BEGIN_NAMESPACE
using namespace std;
QT_END_NAMESPACE

class General_Camera : public QObject,public QRunnable
{
    Q_OBJECT

public:
    General_Camera();
    ~General_Camera();

    int isCapturing,hasFinished,hasStarted;
    QString camURL;
    QMutex camMutex;
    cpu_set_t myCPU;
    int cpuNo;

    virtual void startCamera();
    virtual void getOneFrame();

signals:
    void sigGetOneFrame(QImage);

};

#endif // GENERAL_CAMERA_H
