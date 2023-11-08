#include "rknn_inferencer.h"

RKNN_INFERENCER::RKNN_INFERENCER()
{
    hasInited=false;
    isDetecting=false;
    onGPU=false;
    save_count = 1;
    onceRunTime=0;

    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
}

RKNN_INFERENCER::~RKNN_INFERENCER()
{
    // release
    deinitPostProcess();
    ret = rknn_destroy(ctx);
    if (model_data)
      free(model_data);
    if (resize_buf)
      free(resize_buf);
    if(input_attrsDyn!=nullptr)
        delete[] input_attrsDyn;
    if(output_attrsDyn!=nullptr)
        delete[] output_attrsDyn;
}

void RKNN_INFERENCER::dump_tensor_attr(rknn_tensor_attr* attr)
{
  std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
  for (int i = 1; i < attr->n_dims; ++i) {
    shape_str += ", " + std::to_string(attr->dims[i]);
  }

  printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride = %d, size_with_stride=%d, fmt=%s, "
         "type=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems, attr->size, attr->w_stride,
         attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
         get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

unsigned char* RKNN_INFERENCER::load_data(FILE* fp, size_t ofst, size_t sz)
{
  unsigned char* data;
  int            ret;

  data = NULL;

  if (NULL == fp) {
    return NULL;
  }

  ret = fseek(fp, ofst, SEEK_SET);
  if (ret != 0) {
    printf("blob seek failure.\n");
    return NULL;
  }

  data = (unsigned char*)malloc(sz);
  if (data == NULL) {
    printf("buffer malloc failure.\n");
    return NULL;
  }
  ret = fread(data, 1, sz, fp);
  return data;
}

unsigned char* RKNN_INFERENCER::load_model(const char* filename, int* model_size)
{
  FILE*          fp;
  unsigned char* data;

  fp = fopen(filename, "rb");
  if (NULL == fp) {
    printf("Open file %s failed.\n", filename);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  data = load_data(fp, 0, size);

  fclose(fp);

  *model_size = size;
  return data;
}

int RKNN_INFERENCER::saveFloat(const char* file_name, float* output, int element_size)
{
  FILE* fp;
  fp = fopen(file_name, "w");
  for (int i = 0; i < element_size; i++) {
    fprintf(fp, "%.6f\n", output[i]);
  }
  fclose(fp);
  return 0;
}

double RKNN_INFERENCER::__get_us(struct timeval t)
{
    return (t.tv_sec * 1000000 + t.tv_usec);
}

void RKNN_INFERENCER::iniImgProcessor()
{
    /* Create the neural network */
    printf("Loading mode...\n");
    model_data_size = 0;
    model_data      = load_model(model_name, &model_data_size);
    ret                            = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
    if (ret < 0) {
      printf("rknn_init error ret=%d\n", ret);
      return;
    }

    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0) {
      printf("rknn_init error ret=%d\n", ret);
      return;
    }
    printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);

    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0) {
      printf("rknn_init error ret=%d\n", ret);
      return;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    input_attrsDyn=new rknn_tensor_attr[io_num.n_input];
    for (int i = 0; i < io_num.n_input; i++) {
      input_attrsDyn[i].index = i;
      ret                  = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrsDyn[i]), sizeof(rknn_tensor_attr));
      if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return;
      }
      dump_tensor_attr(&(input_attrsDyn[i]));
    }

    output_attrsDyn=new rknn_tensor_attr[io_num.n_output];
    for (int i = 0; i < io_num.n_output; i++) {
      output_attrsDyn[i].index = i;
      ret                   = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrsDyn[i]), sizeof(rknn_tensor_attr));
      dump_tensor_attr(&(output_attrsDyn[i]));
    }

    if (input_attrsDyn[0].fmt == RKNN_TENSOR_NCHW) {
      printf("model is NCHW input fmt\n");
      channel = input_attrsDyn[0].dims[1];
      height  = input_attrsDyn[0].dims[2];
      width   = input_attrsDyn[0].dims[3];
    } else {
      printf("model is NHWC input fmt\n");
      height  = input_attrsDyn[0].dims[1];
      width   = input_attrsDyn[0].dims[2];
      channel = input_attrsDyn[0].dims[3];
    }

    printf("model input height=%d, width=%d, channel=%d\n", height, width, channel);

    memset(inputs, 0, sizeof(inputs));
    inputs[0].index        = 0;
    inputs[0].type         = RKNN_TENSOR_UINT8;
    inputs[0].size         = width * height * channel;
    inputs[0].fmt          = RKNN_TENSOR_NHWC;
    inputs[0].pass_through = 0;

    hasInited=true;

//    qDebug()<<"RKNN Thread Id: "<<QThread::currentThreadId()<<Qt::endl;
}

void RKNN_INFERENCER::startProcessOnce()
{
//    sched_setaffinity(0,sizeof(myCPU),&myCPU);
    if (!ipcMutex.tryLock())
        return;
    usrtimer.start();

//    save_count++;
//    current_date_time =QDateTime::currentDateTime();
//    qDebug()<<save_count<<" RKNN start time: "<<
//              current_date_time.toString("hh_mm_ss_zzz")<<Qt::endl;

    img_input1.copyTo(img_output3);
    ipcMutex.unlock();

    if(img_output3.channels()==3)
        cvtColor(img_output3, img_output1, cv::COLOR_BGR2RGB);
    else if(img_output3.channels()==4)
        cvtColor(img_output3, img_output1, cv::COLOR_RGBA2RGB);
    img_width  = img_output1.cols;
    img_height = img_output1.rows;
//    printf("img width = %d, img height = %d\n", img_width, img_height);

    if (img_width != width || img_height != height)
    {
      resize(img_output1,img_output3,Size(height,width));
      inputs[0].buf = (void*)img_output3.data;

//      //printf("resize with RGA!\n");
//      resize_buf = malloc(height * width * channel);
//      memset(resize_buf, 0x00, height * width * channel);
//      src = wrapbuffer_virtualaddr((void*)img_output1.data, img_width, img_height, RK_FORMAT_RGB_888);
//      dst = wrapbuffer_virtualaddr((void*)resize_buf, width, height, RK_FORMAT_RGB_888);
//      ret = imcheck(src, dst, src_rect, dst_rect);
//      if (IM_STATUS_NOERROR != ret) {
//        printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
//        return;
//      }
//      IM_STATUS STATUS = imresize(src, dst);
//      inputs[0].buf = resize_buf;

    }
    else
    {
      inputs[0].buf = (void*)img_output1.data;
    }

//    gettimeofday(&start_time, NULL);
    rknn_inputs_set(ctx, io_num.n_input, inputs);
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < io_num.n_output; i++) {
      outputs[i].want_float = 0;
    }

    ret = rknn_run(ctx, NULL);
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
//    gettimeofday(&stop_time, NULL);
//    printf("once run use %f ms\n", (__get_us(stop_time) - __get_us(start_time)) / 1000);

    // post process
    float scale_w = (float)width / img_width;
    float scale_h = (float)height / img_height;

    detect_result_group_t detect_result_group;
    vector<float>    out_scales;
    vector<int32_t>  out_zps;
    for (int i = 0; i < io_num.n_output; ++i) {
      out_scales.push_back(output_attrsDyn[i].scale);
      out_zps.push_back(output_attrsDyn[i].zp);
    }
    post_process((int8_t*)outputs[0].buf, (int8_t*)outputs[1].buf, (int8_t*)outputs[2].buf,
            height, width, box_conf_threshold, nms_threshold,
            scale_w, scale_h, out_zps, out_scales, &detect_result_group);

    // Draw Objects
    char text[256];
    for (int i = 0; i < detect_result_group.count; i++) {
      detect_result_t* det_result = &(detect_result_group.results[i]);
//      sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
//      printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left, det_result->box.top,
//             det_result->box.right, det_result->box.bottom, det_result->prop);
      int x1 = det_result->box.left;
      int y1 = det_result->box.top;
      int x2 = det_result->box.right;
      int y2 = det_result->box.bottom;
      rectangle(img_output1, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0, 255), 3);
      putText(img_output1, text, Point(x1, y1 + 12), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
    }

    emit outputImgProcessedRequest();
    ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
    if (resize_buf!=nullptr)
      free(resize_buf);

    //    cout<<"Time of Once Run(ms): "<<
    //        usrtimer.elapsed()<<endl;

    onceRunTime=usrtimer.elapsed();

//    cout<<"Current RKNN infer thread: "<<QThread::currentThreadId()<<endl;
//    current_date_time =QDateTime::currentDateTime();
//    qDebug()<<save_count<<" RKNN end time: "<<current_date_time.toString("hh_mm_ss_zzz")
//           <<" used: "<<onceRunTime<<Qt::endl;
}
