#ifndef RKNN_INFERENCER_H
#define RKNN_INFERENCER_H

#include <QObject>

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
#include "utils/postprocess.h"
#include "utils/mpp_decoder.h"
#include "utils/mpp_encoder.h"
#include "utils/drawing.h"

#define BUILD_VIDEO_RTSP true
#if defined(BUILD_VIDEO_RTSP)
#include "mk_mediakit.h"
#endif

#define OUT_VIDEO_PATH "out.h264"

class RKNN_INFERENCER : public QObject
{
    Q_OBJECT
public:
    typedef struct {
      rknn_context rknn_ctx;
      rknn_input_output_num io_num;
      rknn_tensor_attr* input_attrs;
      rknn_tensor_attr* output_attrs;
      int model_channel;
      int model_width;
      int model_height;
      FILE* out_fp;
      MppDecoder* decoder;
      MppEncoder* encoder;
    } rknn_app_context_t;

    typedef struct {
      int width;
      int height;
      int width_stride;
      int height_stride;
      int format;
      char* virt_addr;
      int fd;
    } image_frame_t;

    explicit RKNN_INFERENCER(QObject *parent = nullptr);

    static void dump_tensor_attr(rknn_tensor_attr *attr);
    double __get_us(timeval t);
    static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
    static unsigned char *read_file_data(const char *filename, int *model_size);
    static int write_data_to_file(const char *path, char *data, unsigned int size);
    static int init_model(const char *model_path, rknn_app_context_t *app_ctx);
    static int release_model(rknn_app_context_t *app_ctx);
    static int inference_model(rknn_app_context_t *app_ctx, image_frame_t *img, detect_result_group_t *detect_result);
    int process_video_file(rknn_app_context_t *ctx, const char *path);
    int full_test(int argc, char **argv);
    int process_video_rtsp(rknn_app_context_t *ctx, const char *url);
signals:

};

void mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data);
void API_CALL on_track_frame_out(void *user_data, mk_frame frame);
void API_CALL on_mk_play_event_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[],
                                    int track_count);
void API_CALL on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[], int track_count);

#endif // RKNN_INFERENCER_H
