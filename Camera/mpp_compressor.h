#ifndef MPP_COMPRESSOR_H
#define MPP_COMPRESSOR_H

#include "Camera/general_camera.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <QFileDialog>
#include <qdatetime.h>
#include <QElapsedTimer>

#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"
#include "rknn_api.h"
#include "../RKNN/utils/postprocess.h"
#include "../RKNN/utils/mpp_decoder.h"
#include "../RKNN/utils/mpp_encoder.h"
#include "../RKNN/utils/drawing.h"
#include "mk_mediakit.h"

class MPP_COMPRESSOR : public General_Camera
{
    Q_OBJECT
public:
    MPP_COMPRESSOR();
    ~MPP_COMPRESSOR();

    //计时器
    QElapsedTimer usrtimer1,usrtimer2;

    int width_stride, height_stride,width, height, format,fd;
    QImage img;
    const char* data;
    int size,framNum;
    QMutex encodeMutex;

    FILE* fp;
    QDateTime current_date_time;
    QString current_date,current_time, dir_str;
    QDir dir;
    QString fn;
    QByteArray qba;

    void startCamera() override;

public slots:
    void getOneFrame() override;

protected:
    void run() override;

private:
    MppEncoderParams enc_params;
    MppEncoder *encoder;

    void* mpp_frame;
    void* mpp_frame_addr;
    char* enc_data;
    int mpp_frame_fd,enc_data_size,enc_buf_size,rfd;

    // init rga context
    rga_buffer_t src,dst;
    im_rect src_rect,dst_rect;
    IM_STATUS STATUS;
};

void API_CALL mpp_encoder_frame_callback(void* userdata, const char* data, int size);

#endif // MPP_COMPRESSOR_H
