#include "mpp_player.h"

MPP_PLAYER::MPP_PLAYER()
{
    decoder=new MppDecoder;
    player = mk_player_create();
    hasStarted=false;
}

MPP_PLAYER::~MPP_PLAYER()
{
    if (decoder != nullptr)
    {
      delete(decoder);
      decoder = nullptr;
    }
    if (player)
        mk_player_release(player);
}

void MPP_PLAYER::startPlay()
{
    QByteArray qba = videoURL.toLocal8Bit();
    const char *video_name=qba.data();

    decoder->Init(video_type, 30 ,(void *)this);
    decoder->SetCallback(mpp_decoder_frame_callback1);

    printf("app_ctx=%p decoder=%p\n", this, &this->decoder);

    mk_config config;
    memset(&config, 0, sizeof(mk_config));
    config.log_mask = LOG_CONSOLE;
    mk_env_init(&config);
    mk_player_set_on_result(player, on_mk_play_event_func1, (void *)this);
    mk_player_set_on_shutdown(player, on_mk_shutdown_func1, (void *)this);
    mk_player_play(player, video_name);
    hasStarted=true;
}

void MPP_PLAYER::getFrame()
{
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect      src_rect;
    im_rect      dst_rect;
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));

    printf("cvtcolor with RGA!\n");
    void *rgb_buf=nullptr;
    rgb_buf = malloc(sizeof(uchar)*width * height * 3);
    memset(rgb_buf, 0, sizeof(uchar)*width * height * 3);

    src = wrapbuffer_virtualaddr((void*)data, width, height, format, width_stride, height_stride);
    dst = wrapbuffer_virtualaddr((void*)rgb_buf, width, height, RK_FORMAT_RGB_888);
    int ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret) {
      printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
      return;
    }
    IM_STATUS STATUS = imcvtcolor(src, dst,RK_FORMAT_YCbCr_420_SP,RK_FORMAT_RGB_888,
                                  IM_YUV_TO_RGB_BT709_LIMIT);

    QImage disImage = QImage((uchar*)rgb_buf,width,height,width*3,QImage::Format_RGB888);
//    QImage image = disImage.copy(); //把图像复制一份 传递给界面显示
    emit sig_GetOneFrame(disImage);  //发送信号
}

void API_CALL mpp_decoder_frame_callback1(void *user_data, int width_stride, int height_stride,
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
    ctx->getFrame();
}

void API_CALL on_track_frame_out1(void *user_data, mk_frame frame)
{
    MPP_PLAYER *ctx=(MPP_PLAYER*)user_data;
    printf("on_track_frame_out ctx=%p\n", ctx);
    const char* data = mk_frame_get_data(frame);
    size_t size = mk_frame_get_data_size(frame);
    printf("decoder=%p\n", ctx->decoder);
    ctx->decoder->Decode((uint8_t*)data, size, 0);
}

void API_CALL on_mk_play_event_func1(void *user_data, int err_code, const char *err_msg,
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
                mk_track_add_delegate(tracks[i], on_track_frame_out1, user_data);
            }
        }
    } else {
        printf("play failed: %d %s", err_code, err_msg);
    }
}

void API_CALL on_mk_shutdown_func1(void *user_data, int err_code, const char *err_msg,
                                   mk_track tracks[], int track_count)
{
    printf("play interrupted: %d %s", err_code, err_msg);
}
