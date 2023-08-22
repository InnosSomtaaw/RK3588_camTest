#ifndef MPP_PLAYER_H
#define MPP_PLAYER_H

#include <QThread>
#include <QImage>
#include <QMutex>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"
#include "rknn_api.h"
#include "../RKNN/utils/postprocess.h"
#include "../RKNN/utils/mpp_decoder.h"
#include "../RKNN/utils/mpp_encoder.h"
#include "../RKNN/utils/drawing.h"
#include "mk_mediakit.h"

//#include <opencv2/opencv.hpp>
//using namespace cv;

class MPP_PLAYER : public QObject
{
    Q_OBJECT

public:
    explicit MPP_PLAYER();
    ~MPP_PLAYER();

    QString videoURL;
    int video_type;
    QMutex mppMutex;
    MppDecoder *decoder;
    mk_player player;
    bool hasStarted;

    int width_stride, height_stride,width, height, format,fd;
    void *data;

    void startPlay();
    void getFrame();

signals:
    void sig_GetOneFrame(QImage); //每获取到一帧图像 就发送此信号
};

void API_CALL mpp_decoder_frame_callback1(void *user_data, int width_stride, int height_stride,
                                         int width, int height, int format, int fd, void *data);
void API_CALL on_track_frame_out1(void *user_data, mk_frame frame);
void API_CALL on_mk_play_event_func1(void *user_data, int err_code, const char *err_msg,
                                     mk_track tracks[],int track_count);
void API_CALL on_mk_shutdown_func1(void *user_data, int err_code, const char *err_msg,
                                   mk_track tracks[], int track_count);

#endif // MPP_PLAYER_H
