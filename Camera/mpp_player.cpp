#include "mpp_player.h"

MPP_PLAYER::MPP_PLAYER()
{
    isCapturing = false; hasFinished = false;hasStarted=false;
    video_type=265;
    decoder=new MppDecoder;
    player = mk_player_create();
    hasAllocated=false;
}

MPP_PLAYER::~MPP_PLAYER()
{
    isCapturing=false;
    hasFinished=true;
    hasStarted=false;
    hasAllocated=false;
    if (decoder != nullptr)
    {
      delete(decoder);
      decoder = nullptr;
    }
    if (player)
        mk_player_release(player);
    if(rgb_buf!=nullptr)
        free(rgb_buf);
}

void MPP_PLAYER::startCamera()
{
    QByteArray qba = camURL.toLocal8Bit();
    const char *video_name=qba.data();

    decoder->Init(video_type, 30 ,(void *)this);
    decoder->SetCallback(mpp_decoder_frame_callback);

    printf("app_ctx=%p decoder=%p\n", this, &this->decoder);

    mk_config config;
    memset(&config, 0, sizeof(mk_config));
    config.log_mask = LOG_CONSOLE;
    mk_env_init(&config);
    mk_player_set_on_result(player, on_mk_play_event_func, (void *)this);
    mk_player_set_on_shutdown(player, on_mk_shutdown_func, (void *)this);
    mk_player_play(player, video_name);

    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));

    hasStarted=true;
}

void MPP_PLAYER::run()
{
    if(!hasAllocated)
    {
        rgb_buf = malloc(sizeof(uchar)*width * height * 3);
        hasAllocated=true;
    }
    src = wrapbuffer_virtualaddr((void*)data, width, height, format, width_stride, height_stride);
    dst = wrapbuffer_virtualaddr((void*)rgb_buf, width, height, RK_FORMAT_RGB_888);
    STATUS = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != STATUS) {
        printf("%d, check error! %s", __LINE__, imStrError(STATUS));
        return;
    }
    rfd=-1;
    STATUS = imcvtcolor(src, dst,RK_FORMAT_YCbCr_420_SP,RK_FORMAT_RGB_888,
                                  IM_YUV_TO_RGB_BT601_FULL,1,&rfd);

    QImage image = QImage((uchar*)rgb_buf,width,height,width*3,QImage::Format_RGB888);

    emit sigGetOneFrame(image);  //发送信号
}

void API_CALL mpp_decoder_frame_callback(void *user_data, int width_stride, int height_stride,
                                         int width, int height, int format, int fd, void *data)
{
    MPP_PLAYER *ctx=(MPP_PLAYER*)user_data;
    ctx->width_stride=width_stride;
    ctx->height_stride=height_stride;
    ctx->width=width;
    ctx->height=height;
    ctx->format=format;
    ctx->fd=fd;
    ctx->data=data;
//    ctx->getOneFrame();
    ctx->start();
}

void API_CALL on_track_frame_out(void *user_data, mk_frame frame)
{
    MPP_PLAYER *ctx=(MPP_PLAYER*)user_data;
//    printf("on_track_frame_out ctx=%p\n", ctx);
    const char* data = mk_frame_get_data(frame);
    size_t size = mk_frame_get_data_size(frame);
//    printf("decoder=%p\n", ctx->decoder);
    ctx->decoder->Decode((uint8_t*)data, size, 0);
}

void API_CALL on_mk_play_event_func(void *user_data, int err_code, const char *err_msg,
                                     mk_track tracks[],int track_count)
{
    MPP_PLAYER *ctx = (MPP_PLAYER *) user_data;
    if (err_code == 0) {
        //success
        printf("play success!");
        int i;
        for (i = 0; i < track_count; ++i) {
            if (mk_track_is_video(tracks[i])) {
                log_info("got video track: %s", mk_track_codec_name(tracks[i]));
                //监听track数据回调
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
            }
        }
    } else {
        printf("play failed: %d %s", err_code, err_msg);
    }
}

void API_CALL on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg,
                                   mk_track tracks[], int track_count)
{
    printf("play interrupted: %d %s", err_code, err_msg);
}


