#include "mpp_compressor.h"

MPP_COMPRESSOR::MPP_COMPRESSOR()
{
    isCapturing = false; hasFinished = false;hasStarted=false;
    encoder=new MppEncoder;
}

MPP_COMPRESSOR::~MPP_COMPRESSOR()
{
    isCapturing=false;
    hasFinished=true;
    if (encoder != nullptr)
    {
      delete(encoder);
      encoder = nullptr;
    }
    if(enc_data!=nullptr)
        free(enc_data);
}

void MPP_COMPRESSOR::startCamera()
{
    memset(&enc_params, 0, sizeof(MppEncoderParams));
    enc_params.width = width;
    enc_params.height = height;
    enc_params.fmt = MPP_FMT_YUV420SP;
    enc_params.type = MPP_VIDEO_CodingMJPEG;
    enc_params.fps_out_num=1;
    enc_params.fps_out_den=5000;

    //Align to 16
    width_stride=(width+16-1)&~(16-1);
    height_stride=(height+16-1)&~(16-1);

    encoder->Init(enc_params,(void *)this);
    encoder->SetCallback(mpp_encoder_frame_callback);

    mpp_frame = NULL;
    mpp_frame_fd = 0;
    mpp_frame_addr = NULL;
    enc_data_size = 0;
    enc_buf_size = encoder->GetFrameSize();
    enc_data = (char*)malloc(enc_buf_size);
    mpp_frame = encoder->GetInputFrameBuffer();
    mpp_frame_fd = encoder->GetInputFrameBufferFd(mpp_frame);
    mpp_frame_addr = encoder->GetInputFrameBufferAddr(mpp_frame);
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));

    hasStarted=true;
}

void MPP_COMPRESSOR::getOneFrame()
{
//    if(!camMutex.tryLock())
//        return;
    usrtimer1.start();

    dst = wrapbuffer_virtualaddr(mpp_frame_addr, width, height, RK_FORMAT_YCbCr_420_SP,
                                 width_stride,height_stride);
    rfd=-1;
    switch (img.format()) {
    case QImage::Format_RGB888:
    case QImage::Format_BGR888:
    {
        src = wrapbuffer_virtualaddr((void*)img.bits(), width, height, RK_FORMAT_RGB_888,
                                     width_stride,height_stride);
        STATUS = imcvtcolor(src, dst,RK_FORMAT_RGB_888,RK_FORMAT_YCbCr_420_SP,
                            IM_RGB_TO_YUV_BT601_FULL,0,&rfd);
    }
        break;
    case QImage::Format_RGB32:
    case QImage::Format_RGBA8888:
    {
        src = wrapbuffer_virtualaddr((void*)img.bits(), width, height, RK_FORMAT_BGRA_8888,
                                     width_stride,height_stride);
        STATUS = imcvtcolor(src, dst,RK_FORMAT_BGRA_8888,RK_FORMAT_YCbCr_420_SP,
                            IM_RGB_TO_YUV_BT601_FULL,0,&rfd);
    }
    }

//    STATUS = imcheck(src, dst, src_rect, dst_rect);
//    if (IM_STATUS_NOERROR != STATUS) {
//        printf("%d, check error! %s", __LINE__, imStrError(STATUS));
//        cout<<" Used time: "<<usrtimer1.elapsed()<<" ms."<<endl;
//        return;
//    }

    cout<<rfd<<" RGA cvtcolor time: "<<usrtimer1.elapsed()<<" ms."<<endl;

    this->start();
//    camMutex.unlock();

//     Encode to file
//    enc_data_size = encoder->Encode(mpp_frame, enc_data, enc_buf_size);

//    cout<<"Encode time: "<<usrtimer1.elapsed()<<" ms."<<endl;

//    //按需保存图片：
//    current_date_time =QDateTime::currentDateTime();
//    current_date =current_date_time.toString("yyyy_MM_dd");
//    current_time =current_date_time.toString("hh_mm_ss_zzz");
//    dir_str = "./SaveFile/"+current_date+"/";
//    if (!dir.exists(dir_str))
//        dir.mkpath(dir_str);
//    fn=dir_str+current_time+"_autosaved.jpg";
//    qba = fn.toLocal8Bit();
//    fp = fopen(qba.data(), "w");
//    fwrite(enc_data, 1, enc_data_size,fp);
//    //    fflush(fp);
//    fclose(fp);

//    cout<<"Save time(width:"<<width<<",height:"<<
//              height<<"): "<<usrtimer1.elapsed()<<" ms."<<endl;
}

void MPP_COMPRESSOR::run()
{
    usrtimer2.start();

    STATUS=imsync(rfd);

    // Encode to file
    enc_data_size = encoder->Encode(mpp_frame, enc_data, enc_buf_size);

    cout<<"Encode time: "<<usrtimer2.elapsed()<<" ms."<<endl;

//    //按需保存图片：
//    current_date_time =QDateTime::currentDateTime();
//    current_date =current_date_time.toString("yyyy_MM_dd");
//    current_time =current_date_time.toString("hh_mm_ss_zzz");
//    dir_str = "./SaveFile/"+current_date+"/";
//    if (!dir.exists(dir_str))
//        dir.mkpath(dir_str);
//    fn=dir_str+current_time+"_autosaved.jpg";
//    qba = fn.toLocal8Bit();
//    fp = fopen(qba.data(), "w");
//    fwrite(enc_data, 1, enc_data_size,fp);
////    fflush(fp);
//    fclose(fp);

//    cout<<"Save time(width:"<<width<<",height:"<<
//              height<<"): "<<usrtimer2.elapsed()<<" ms."<<endl;
}

void mpp_encoder_frame_callback(void *userdata, const char *data, int size)
{
    MPP_COMPRESSOR *ctx=(MPP_COMPRESSOR*)userdata;

    //按需保存图片：
    ctx->current_date_time =QDateTime::currentDateTime();
    ctx->current_date =ctx->current_date_time.toString("yyyy_MM_dd");
    ctx->current_time =ctx->current_date_time.toString("hh_mm_ss_zzz");
    ctx->dir_str = "./SaveFile/"+ctx->current_date+"/";
    if (!ctx->dir.exists(ctx->dir_str))
        ctx->dir.mkpath(ctx->dir_str);
    ctx->fn=ctx->dir_str+ctx->current_time+"_autosaved.jpg";
    ctx->qba = ctx->fn.toLocal8Bit();
    ctx->fp = fopen(ctx->qba.data(), "w");
    fwrite(data, 1, size,ctx->fp);
    fclose(ctx->fp);
}
