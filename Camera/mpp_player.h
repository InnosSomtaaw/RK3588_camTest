#ifndef MPP_PLAYER_H
#define MPP_PLAYER_H

#include "Camera/general_camera.h"
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

class MPP_PLAYER : public General_Camera
{
    Q_OBJECT

public:
    MPP_PLAYER();
    ~MPP_PLAYER();

    int video_type;
    MppDecoder *decoder;
    int width_stride, height_stride,width, height, format,fd;
    void *data;

    void startCamera() override;
    void run() override;

private:
    mk_player player;
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect      src_rect;
    im_rect      dst_rect;
    bool hasAllocated;
    void *rgb_buf=nullptr;
    int rfd;
    IM_STATUS STATUS;
};

void API_CALL mpp_decoder_frame_callback(void *user_data, int width_stride, int height_stride,
                                         int width, int height, int format, int fd, void *data);
void API_CALL on_track_frame_out(void *user_data, mk_frame frame);
void API_CALL on_mk_play_event_func(void *user_data, int err_code, const char *err_msg,
                                     mk_track tracks[],int track_count);
void API_CALL on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg,
                                   mk_track tracks[], int track_count);


#endif // MPP_PLAYER_H
