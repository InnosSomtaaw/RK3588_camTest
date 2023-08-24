#ifndef RKNN_INFERENCER_H
#define RKNN_INFERENCER_H

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define _BASETSD_H

#include "RgaUtils.h"
#include "im2d.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "./utils/postprocess.h"
#include "rga.h"
#include "rknn_api.h"

#include "ImageProcess/image_processing.h"

#define PERF_WITH_POST 1

class RKNN_INFERENCER : public Image_Processing_Class
{
    Q_OBJECT
public:
    explicit RKNN_INFERENCER(QObject *parent = nullptr);
    ~RKNN_INFERENCER();

    bool hasInited;
    int            status     = 0;
    char*          model_name = NULL;
    rknn_context   ctx;
    size_t         actual_size        = 0;
    int            img_width          = 0;
    int            img_height         = 0;
    int            img_channel        = 0;
    const float    nms_threshold      = NMS_THRESH;
    const float    box_conf_threshold = BOX_THRESH;
    struct timeval start_time, stop_time;
    int            ret;
    int onceRunTime;

    static void dump_tensor_attr(rknn_tensor_attr* attr);
    static unsigned char* load_data(FILE* fp, size_t ofst, size_t sz);
    static unsigned char* load_model(const char* filename, int* model_size);
    static int saveFloat(const char* file_name, float* output, int element_size);

    double __get_us(struct timeval t);
    bool InitRKNN();

public slots:
    int RunOnce();

private:
    int            model_data_size = 0;
    unsigned char* model_data= NULL;

    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect      src_rect;
    im_rect      dst_rect;

    void* resize_buf = nullptr;
    int channel = 3;
    int width   = 0;
    int height  = 0;

    rknn_sdk_version version;
    rknn_input_output_num io_num;
    rknn_tensor_attr *input_attrsDyn,*output_attrsDyn;

    rknn_input inputs[1];
};

#endif // RKNN_INFERENCER_H
