#include "cameraDecode.h"

cameraDecode::cameraDecode()
{
  this->stoped = false;
}
cameraDecode::cameraDecode(std::string hardwareType, std::string addr, std::string decoding_type, int breathe_time, int reconn_time, int reconn_count, std::map<std::string, queue<FRAMEINFO>> *presultMap, std::map<std::string, std::mutex *> *presultMap_mtx, bool imgFlag = false)
{
  this->imgFlag = imgFlag;
  this->cameraFlag = false;
  this->stoped = false;
  this->hardwareType = hardwareType;
  this->addr = addr;
  this->decoding_type = decoding_type;
  this->breathe_time = breathe_time;
  this->reconn_time = reconn_time;
  this->reconn_count = reconn_count;
  this->breatheInfo = new BREATHEINFO();
  this->presultMap = presultMap;
  this->presultMap_mtx = presultMap_mtx;

  std::queue<FRAMEINFO> tmp_frameInfo;
  this->presultMap->insert(make_pair(addr, tmp_frameInfo));
  this->presultMap_mtx->insert(make_pair(addr, new std::mutex()));
}

cameraDecode::~cameraDecode()
{
  this->stop();
}

void cameraDecode::saveImage(std::string savePath, std::string cameraIP, cv::Mat img)
{
  if (opendir(savePath.c_str()) == NULL)
  {
    int ret = mkdir(savePath.c_str(), 0775);
  }
  savePath = savePath + "/" + cameraIP;

  if (opendir(savePath.c_str()) == NULL)
  {
    int ret = mkdir(savePath.c_str(), 0775);
  }

  string time = this->get_time_now();
  cv::imwrite(savePath + "/" + time + ".png", img);
}

BREATHEINFO* cameraDecode::getbreatheInfo()
{
  return this->breatheInfo;
}
int cameraDecode::convert24Image(char *p32Img, char *p24Img, int dwSize32)
{
  if (p32Img != NULL && p24Img != NULL && dwSize32)
  {
    int dwSize24;
    dwSize24 = (dwSize32 * 3) / 4;
    char *psrc_r = p32Img;
    char *pdst_r = p24Img;
    for (int index = 0; index < dwSize32 / 4; index++)
    {
      memcpy(pdst_r, psrc_r, 3);
      psrc_r += 4;
      pdst_r += 3;
    }
  }
  else
  {
    return 0;
  }
  return 1;
}

string cameraDecode::get_time_now()
{
  time_t tt;
  time(&tt);
  tt = tt + 8 * 3600; // transform the time zone
  tm *t = gmtime(&tt);
  std::string time_str = "";
  char time_data[64] = {0};
  sprintf(time_data, "%d-%02d-%02d-%02d-%02d-%02d",
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday,
          t->tm_hour,
          t->tm_min,
          t->tm_sec);
  time_str = time_data;
  return time_str;
}

static enum AVPixelFormat get_cuda_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
  const enum AVPixelFormat *p;
  for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++)
  {
    if (*p == AV_PIX_FMT_CUDA)
    {
      return *p;
    }
  }
  fprintf(stderr, "[HW_DECODE] Unable to decode by using CUDA-API \n");
  return AV_PIX_FMT_NONE;
}

static enum AVPixelFormat get_vaapi_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
  const enum AVPixelFormat *p;
  for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++)
  {
    if (*p == AV_PIX_FMT_VAAPI)
    {
      return *p;
    }
  }
  fprintf(stderr, "[HW_DECODE] Unable to decode by using CUDA-API \n");
  return AV_PIX_FMT_NONE;
}

static enum AVPixelFormat get_format(AVCodecContext *ctx)
{
  AVPixelFormat pixFormat;
  switch (ctx->pix_fmt)
  {
  case AV_PIX_FMT_YUVJ420P:
    pixFormat = AV_PIX_FMT_YUV420P;
    break;
  case AV_PIX_FMT_YUVJ422P:
    pixFormat = AV_PIX_FMT_YUV422P;
    break;
  case AV_PIX_FMT_YUVJ444P:
    pixFormat = AV_PIX_FMT_YUV444P;
    break;
  case AV_PIX_FMT_YUVJ440P:
    pixFormat = AV_PIX_FMT_YUV440P;
    break;
  default:
    pixFormat = ctx->pix_fmt;
    break;
  }
  return pixFormat;
}

void cameraDecode::stop()
{
  delete this->breatheInfo;
  this->breatheInfo = NULL;
  this->stoped = true;
  sleep(1);
}

CAMERASTATUS cameraDecode::checkCameraStatus(string addr)
{
  int ret = 0;
  AVInputFormat *inputFmt = NULL;
  AVFormatContext *ifmt_ctx = NULL;
  AVDictionary *options = NULL;
  av_dict_set(&options, "rtsp_transport", "tcp", 0);

  if ((ret = avformat_open_input(&ifmt_ctx, addr.c_str(), inputFmt, &options)) < 0)
  {
    return NOT_CONNECTABLE_STATE;
  }
  if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
  {
    avformat_close_input(&ifmt_ctx);
    return NOT_CONNECTABLE_STATE;
  }
  avformat_close_input(&ifmt_ctx);
  return CONNECTABLE_STATE;
}

void cameraDecode::decode_CPU()
{
  av_register_all();
  avformat_network_init();
  avdevice_register_all();

  while (!this->stoped)
  {
    usleep(0.001);
    int ret = 0, got_picture, video_stream = -1;
    AVInputFormat *inputFmt = NULL;
    AVFormatContext *ifmt_ctx = NULL;
    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "1000000", 0);

    if ((ret = avformat_open_input(&ifmt_ctx, this->addr.c_str(), inputFmt, &options)))
    {
      fprintf(stderr, "[SW_DECODE] cannot open input file '%s' \n", this->addr.c_str());
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
    {
      fprintf(stderr, "[SW_DECODE] cannot open input stream information. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    for (uint i = 0; i < ifmt_ctx->nb_streams; i++)
    {
      if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      {
        video_stream = i;
        break;
      }
    }

    AVStream *video = ifmt_ctx->streams[video_stream];
    AVCodecContext *pCodecCtx = ifmt_ctx->streams[video_stream]->codec;
    AVCodec *decoder = avcodec_find_decoder(pCodecCtx->codec_id);
    if (avcodec_open2(pCodecCtx, decoder, NULL) < 0)
    {
      printf("[SW_DECODE] cannot open codec \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
      printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
      printf("[SW_DECODE] failed to allocated memory for AVPacket. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }
    printf("format %s, duration %d us, bitrate %lld %d %d.\n",
           ifmt_ctx->iformat->name,
           ifmt_ctx->duration / 1000LL,
           ifmt_ctx->bit_rate,
           video->r_frame_rate.num / video->r_frame_rate.den,
           1 //video->codec->framerate.num/video->codec->framerate.den
    );
    AVFrame *sFrame = av_frame_alloc();
    if (!sFrame)
    {
      printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    int frame_rate = video->r_frame_rate.num / video->r_frame_rate.den;
    int number = 0;
    int limit = 0;
    struct SwsContext *img_convert_ctx;
    ret = 1;
    got_picture = 1;
    long time_start = 0;
    auto tmp1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    auto time_start_breathe = tmp1 / 1000;
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    while (!this->stoped)
    {
      usleep(1);
      auto tmp2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      auto time_now = tmp2 / 1000;

      ret = av_read_frame(ifmt_ctx, pPacket);
      
      //定时呼吸
      if ((time_now - time_start_breathe) >= 1) //秒
      {
        time_start_breathe = time_now;
        //int ret = av_read_frame(ifmt_ctx, pPacket);
        this->breatheInfo->timeStamp = get_localtime();
        this->breatheInfo->connectionFailure ="";
        if(ret < 0)
        {
          this->breatheInfo->connectionFailure = this->addr;
        }
      }
      if (ret >= 0)
      {
        time_t timestamp;  
        time_start = time(&timestamp);

        if (pPacket->stream_index == video_stream)
        {
          avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, pPacket);
          if (got_picture)
          {
            //get BGR img
            //img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
            img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, get_format(pCodecCtx), pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
            sFrame->data[0] = (uint8_t *)malloc(sizeof(uint8_t) * 3 * pCodecCtx->width * pCodecCtx->height);
            avpicture_fill((AVPicture *)sFrame, sFrame->data[0], AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);

            if ((this->decoding_type == "iFrame") && (pFrame->pict_type == AV_PICTURE_TYPE_I))
            {
              sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, sFrame->data, sFrame->linesize);

              cv::Mat rgb(pFrame->height, pFrame->width, CV_8UC3, sFrame->data[0]);
              for (std::map<std::string, std::queue<FRAMEINFO>>::iterator iter1 = this->presultMap->begin(); iter1 != this->presultMap->end(); iter1++)
              {
                if (iter1->first == this->addr)
                {
                  FRAMEINFO tmp_info;
                  tmp_info.frame = rgb.clone();
                  tmp_info.url = iter1->first;
                  tmp_info.fps = frame_rate;

                  (*presultMap_mtx)[iter1->first]->lock();
                  if ((*presultMap)[iter1->first].size() < queue_len) //限制图像队列大小
                  {
                    (*presultMap)[iter1->first].push(tmp_info);
                  }
                  (*presultMap_mtx)[iter1->first]->unlock();
                }
              }

              if (this->imgFlag)
              {
                std::string savePath = "../results";
                int pos1 = this->addr.find("@");
                int pos2 = this->addr.find_last_of(":");
                std::string IP = this->addr.substr(pos1 + 1, pos2 - pos1 - 1);
                this->saveImage(savePath, IP, rgb);
              }
            }
            else if (this->decoding_type == "full")
            {
              sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, sFrame->data, sFrame->linesize);
              cv::Mat rgb(pFrame->height, pFrame->width, CV_8UC3, sFrame->data[0]);

              for (std::map<std::string, std::queue<FRAMEINFO>>::iterator iter1 = this->presultMap->begin(); iter1 != this->presultMap->end(); iter1++)
              {
                if (iter1->first == this->addr)
                {
                  FRAMEINFO tmp_info;
                  tmp_info.frame = rgb.clone();
                  tmp_info.url = iter1->first;
                  tmp_info.fps = frame_rate;

                  (*presultMap_mtx)[iter1->first]->lock();
                  if ((*presultMap)[iter1->first].size() < queue_len) //限制图像队列大小
                  {
                    (*presultMap)[iter1->first].push(tmp_info);
                  }
                  (*presultMap_mtx)[iter1->first]->unlock();

                  if (this->imgFlag)
                  {
                    std::string savePath = "../results";
                    int pos1 = this->addr.find("@");
                    int pos2 = this->addr.find_last_of(":");
                    std::string IP = this->addr.substr(pos1 + 1, pos2 - pos1 - 1);
                    this->saveImage(savePath, IP, rgb);
                  }
                }
              }
            }
            free(sFrame->data[0]);
            sws_freeContext(img_convert_ctx);

            //防止解码速度过快
            //auto t2 =std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            //auto dr_ms = t2 -t1;
            //t1 = t2;
            //if ((1000.0 / frame_rate - dr_ms) > 0)
            //{
              //usleep(int((1000.0 / frame_rate - dr_ms) * 1000));
           // }
          }
          else
          {
            usleep(1);
            av_frame_unref(pFrame);
            av_packet_unref(pPacket);
            continue;
          }
          av_frame_unref(pFrame);
          av_packet_unref(pPacket);
        }
      }
      else
      {
        av_frame_unref(pFrame);
        av_packet_unref(pPacket);
        time_t timeStamp;
        long time_now = time(&timeStamp);
        if ((time_now - time_start) > (this->reconn_time)) //超过断线重连时间就退出循环
        {
          std::cout << "camera connect time out: " <<this->addr<< std::endl;
          break;
        }
      }
    } //对应 while(1)
    //释放
    av_packet_unref(pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sFrame);
    av_packet_free(&pPacket);
    avformat_close_input(&ifmt_ctx);
  }
}

/*
void cameraDecode::decode_GPU()
{
  while (1)
  {
    int ret =0;
    AVBufferRef* hw_device_ctx = NULL;
    ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA,NULL,NULL,0);
    if(ret < 0)
    {
      fprintf(stderr, "[HW_DECODE] Failed to create a CUDA device.\n"); 
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    AVDictionary* options = NULL;
    av_dict_set(&options,"rtsp_transport","tcp",0);
    av_dict_set(&options,"rtsp_transport","1000000",0);
    AVFormatContext* ifmt_ctx = NULL;
    if((ret = avformat_open_input(&ifmt_ctx,this->addr.c_str(),NULL,&options)) < 0)
    {
        fprintf(stderr,"[HW_DECODE] Cannot open input file '%s' \n",this->addr.c_str());
        av_dict_free(&options);
        av_buffer_unref(&hw_device_ctx);
        continue;
    }
    if((ret = avformat_find_stream_info(ifmt_ctx,NULL)) < 0)
    {
        fprintf(stderr, "[HW_DECODE] Cannot open input stream information. \n");
        avformat_close_input(&ifmt_ctx);
        av_dict_free(&options);
        av_buffer_unref(&hw_device_ctx);
        continue;
    }

    AVCodec* decoder = NULL;
    ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO,-1,-1,&decoder,0);
    if(ret < 0)
    {
      fprintf(stderr, "[HW_DECODE] Cannot find a video stream in the input file. \n");
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    int video_stream = ret;
    AVCodecContext* decoder_ctx = NULL;
    if(!(decoder_ctx = avcodec_alloc_context3(decoder)))
    {
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    AVStream* video = ifmt_ctx->streams[video_stream];
    if((ret = avcodec_parameters_to_context(decoder_ctx, video->codecpar)) < 0 )
    {
        fprintf(stderr, "[HW_DECODE] avcodec_parameters_to_context error. \n ");
        avformat_close_input(&ifmt_ctx);
        av_dict_free(&options);
        av_buffer_unref(&hw_device_ctx);
        continue;
         ifmt_ctx->bit_rate,
         video->codec->framerate.num/video->codec->framerate.den );
    {
      fprintf(stderr, "[HW_DECODE] A hardware device reference create failed.\n");
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    decoder_ctx->get_format = get_cuda_format;
         ifmt_ctx->bit_rate,
         video->codec->framerate.num/video->codec->framerate.den );");
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }
    
    printf("format %s, duration %d us, bitrate %lld %d .\n",
         ifmt_ctx->iformat->name,
         ifmt_ctx->duration / 1000LL,
         ifmt_ctx->bit_rate,
         video->codec->framerate.num/video->codec->framerate.den );
    printf("Codec %s ID %d .\n", decoder->name, decoder->id);
    
    AVFrame* pFrame = av_frame_alloc();
    if(!pFrame)
    {
      printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    AVFrame* sw_frame = av_frame_alloc();
    if(!sw_frame)
    {
      printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
      av_frame_free(&pFrame);
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }

    AVPacket* pPacket = av_packet_alloc();
    if(!pPacket)
    {
      av_frame_free(&pFrame);
      av_frame_free(&sw_frame);
      avformat_close_input(&ifmt_ctx);
      av_dict_free(&options);
      av_buffer_unref(&hw_device_ctx);
      continue;
    }
    AVFrame *sFrame = av_frame_alloc();
    if (!sFrame)
    {
        printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
        av_frame_free(&pFrame);
        av_frame_free(&sw_frame);
        av_packet_free(&pPacket);
        avformat_close_input(&ifmt_ctx);
        av_dict_free(&options);
        av_buffer_unref(&hw_device_ctx);
        continue;
    }

    int response = 0;
    struct SwsContext* img_convert_ctx;
    img_convert_ctx = sws_getContext(decoder_ctx->width,decoder_ctx->height, AV_PIX_FMT_NV12,decoder_ctx->width,decoder_ctx->height,AV_PIX_FMT_BGR24,SWS_BICUBIC, NULL, NULL, NULL);
    sFrame->data[0] = (uint8_t*)malloc(sizeof(uint8_t) * 3 * decoder_ctx->width * decoder_ctx->height);
    avpicture_fill((AVPicture*)sFrame, sFrame->data[0], AV_PIX_FMT_BGR24,decoder_ctx->width,decoder_ctx->height);
    
    while( av_read_frame(ifmt_ctx,pPacket)>=0 )
    {
      usleep(0.001);
      if(pPacket->stream_index == video_stream)
      {
        response = avcodec_send_packet(decoder_ctx,pPacket);
        if(response < 0)
        {
          printf("[HW_DECODE] Error while sending a packet to the decoder. \n");
        }
        while (response >= 0)
        {
            response = avcodec_receive_frame(decoder_ctx,pFrame);
            if(response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                //printf("[HW_DECODE] Output invalid or has flashed decoder don`t have new frame !\n");
                break;
            }
            else if(response < 0)
            {
                printf("[HW_DECODE] Error while receiving a frame from the decoder. \n");
                break;
            }
            
            if(this->decoding_type == "iFrame")
            {
                if( (AV_PIX_FMT_CUDA == pFrame->format) && (pFrame->pict_type == AV_PICTURE_TYPE_I))
                 {
                    sw_frame->format = AV_PIX_FMT_NV12;
                    if((ret = av_hwframe_transfer_data(sw_frame,pFrame,0)) < 0) //从GPU拷贝数据到CPU
                    {
                      fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory \n");
                      break;
                    }
                 } 
            }
            else if(this->decoding_type == "full")
            {
                if( (AV_PIX_FMT_CUDA == pFrame->format) )
                {
                  sw_frame->format = AV_PIX_FMT_NV12;
                  if((ret = av_hwframe_transfer_data(sw_frame,pFrame,0)) < 0) //从GPU拷贝数据到CPU
                  {
                    fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory \n");
                    break;
                  }             
                }
            }
            sws_scale(img_convert_ctx,(const unsigned char* const*)sw_frame->data, sw_frame->linesize,0,
                      sw_frame->height, sFrame->data, sFrame->linesize);
            if(sw_frame->width >0 && sw_frame->height > 0)
            {
                cv::Mat src( sw_frame->height, sw_frame->width, CV_8UC3,sFrame->data[0] );
                cv::Mat img = src.clone();
            }
            av_frame_unref(pFrame);
        }
        
      }
      av_frame_unref(pFrame);
      av_packet_unref(pPacket);
    }  
    free(sFrame->data[0]);
    avcodec_free_context(&decoder_ctx);
    av_frame_free(&pFrame);
    av_frame_free(&sw_frame);
    av_frame_free(&sFrame);
    av_packet_free(&pPacket);
    avformat_close_input(&ifmt_ctx);
    av_buffer_unref(&hw_device_ctx);

  }
}
*/
void cameraDecode::decode_GPU() {}

//核显
void cameraDecode::decode_iGPU()
{
  while (!this->stoped)
  {
    int ret = 0;
    AVBufferRef *hw_device_ctx = NULL;
    ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
    if (ret < 0)
    {
      fprintf(stderr, "[HW_DECODE] Failed to create a VAAPI device. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "1000000", 0);  //
    AVFormatContext *ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, this->addr.c_str(), NULL, &options)) < 0)
    {
      fprintf(stderr, "[HW_DECODE] Cannot open input file '%s' \n", this->addr.c_str());
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
    {
      fprintf(stderr, "[HW_DECODE] Cannot find input stream information. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVCodec *decoder = NULL;
    ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (ret < 0)
    {
      fprintf(stderr, "[HW_DECODE] Cannot find a video stream in the input file. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }
    int video_stream = ret;

    AVCodecContext *decoder_ctx = NULL;
    if (!(decoder_ctx = avcodec_alloc_context3(decoder)))
    {
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVStream *video = ifmt_ctx->streams[video_stream];

    if ((ret = avcodec_parameters_to_context(decoder_ctx, video->codecpar)) < 0)
    {
      fprintf(stderr, "[HW_DECODE] avcodec_parameters_to_context error. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    decoder_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    if (!decoder_ctx->hw_device_ctx)
    {
      fprintf(stderr, "[HW_DECODE] A hardware device reference create failed.\n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    decoder_ctx->get_format = get_vaapi_format;
    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0)
    {
      fprintf(stderr, "[HW_DECODE] Failed to open codec for decoding. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
      printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVFrame *sw_frame = av_frame_alloc();
    if (!sw_frame)
    {
      printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
      printf("[HW_DECODE] failed to allocated memory for AVPacket. \n");
      sleep(this->reconn_time);
      static int count = 0;
      if (count < this->reconn_count)
      {
        count++;
        std::cout << "reconnect: " << count << std::endl;
        continue;
      }
      else
      {
        count = 0;
        break;
      }
    }

    int response = 0;
    long time_start = 0;
    int frame_rate = video->r_frame_rate.num / video->r_frame_rate.den;
    auto time_start_breathe = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000;

    while (!this->stoped) //循环抽取码流
    {
      auto time_now = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000;

      ret = av_read_frame(ifmt_ctx, pPacket);
      if ((time_now - time_start_breathe) >= 1) //秒
      {
        time_start_breathe = time_now;
        //int ret = av_read_frame(ifmt_ctx, pPacket);

        this->breatheInfo->timeStamp = get_localtime();
        this->breatheInfo->connectionFailure ="";
        if(ret < 0)
        {
          this->breatheInfo->connectionFailure = this->addr;
        }
      }

      if (ret >= 0)
      {
        time_t timeStamp;
        time_start = time(&timeStamp);

        if (pPacket->stream_index == video_stream)
        {
          response = avcodec_send_packet(decoder_ctx, pPacket);
          usleep(0.001);
          if (response < 0)
            printf("[HW_DECODE] Error while sending a packet to the decoder. \n");

          while (response >= 0)
          {
            response = avcodec_receive_frame(decoder_ctx, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
              //printf("[HW_DECODE] Output invalid or has flashed decoder don`t have new frame !\n");
              break;
            }
            else if (response < 0)
            {
              printf("[HW_DECODE] Error while receiving a frame from the decoder. \n");
              break;
            }

            if ((this->decoding_type == "iFrame") && (pFrame->pict_type == AV_PICTURE_TYPE_I))
            {
              if (response >= 0)
              {
                if (AV_PIX_FMT_VAAPI == pFrame->format)
                {
                  sw_frame->format = AV_PIX_FMT_ARGB;
                  if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0)
                  {
                    fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                    break;
                  }
                  cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);
                  int lSize = argb.rows * argb.cols * argb.channels();
                  int size_out = sizeof(char) * lSize * 3 / 4;
                  char buffer_out[size_out];
                  convert24Image((char *)argb.data, buffer_out, lSize);
                  cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);

                  for (std::map<std::string, std::queue<FRAMEINFO>>::iterator iter1 = this->presultMap->begin(); iter1 != this->presultMap->end(); iter1++)
                  {
                    if (iter1->first == this->addr)
                    {
                      FRAMEINFO tmp_info;
                      tmp_info.frame = rgb.clone();
                      tmp_info.url = iter1->first;
                      tmp_info.fps = frame_rate;

                      (*presultMap_mtx)[iter1->first]->lock();
                      if ((*presultMap)[iter1->first].size() < queue_len) //限制图像队列大小
                      {
                        (*presultMap)[iter1->first].push(tmp_info);
                      }
                      (*presultMap_mtx)[iter1->first]->unlock();
                    }
                  }

                  if (this->imgFlag)
                  {
                    std::string savePath = "../results";
                    int pos1 = this->addr.find("@");
                    int pos2 = this->addr.find_last_of(":");
                    std::string IP = this->addr.substr(pos1 + 1, pos2 - pos1 - 1);
                    this->saveImage(savePath, IP, rgb);
                  }
                }
              }
            }
            else if (this->decoding_type == "full")
            {
              if (response >= 0)
              {
                if (AV_PIX_FMT_VAAPI == pFrame->format)
                {
                  sw_frame->format = AV_PIX_FMT_ARGB;
                  usleep(1000);
                  if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0)
                  {
                    fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                    break;
                  }

                  cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);
                  int lSize = argb.rows * argb.cols * argb.channels();
                  int size_out = sizeof(char) * lSize * 3 / 4;
                  char buffer_out[size_out];
                  convert24Image((char *)argb.data, buffer_out, lSize);
                  cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);

                  for (std::map<std::string, std::queue<FRAMEINFO>>::iterator iter1 = this->presultMap->begin(); iter1 != this->presultMap->end(); iter1++)
                  {
                    if (iter1->first == this->addr)
                    {
                      FRAMEINFO tmp_info;
                      tmp_info.frame = rgb.clone();
                      tmp_info.url = iter1->first;
                      tmp_info.fps = frame_rate;

                      (*presultMap_mtx)[iter1->first]->lock();
                      if ((*presultMap)[iter1->first].size() < queue_len) //限制图像队列大小
                      {
                        (*presultMap)[iter1->first].push(tmp_info);
                      }
                      (*presultMap_mtx)[iter1->first]->unlock();
                    }
                  }

                  if (this->imgFlag)
                  {
                    std::string savePath = "../results";
                    int pos1 = this->addr.find("@");
                    int pos2 = this->addr.find_last_of(":");
                    std::string IP = this->addr.substr(pos1 + 1, pos2 - pos1 - 1);
                    this->saveImage(savePath, IP, rgb);
                  }
                }
              }
            }
            av_frame_unref(pFrame);
          }
        }
        av_frame_unref(pFrame);
        av_packet_unref(pPacket);
      }
      else
      {
        av_frame_unref(pFrame);
        av_packet_unref(pPacket);
        time_t timeStamp;
        long time_now = time(&timeStamp);
        if ((time_now - time_start) > (this->reconn_time))
        {
          std::cout << "camera connect time out: "<<this->addr << std::endl;
          break;
        }
      }
    } //while

    avformat_close_input(&ifmt_ctx);
    avcodec_free_context(&decoder_ctx);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sw_frame);
    av_buffer_unref(&hw_device_ctx);
  } //while
}

void cameraDecode::run()
{
  if (this->hardwareType == "CPU")
  {
    std::cout << "Choose CPU Mode" << std::endl;
    std::thread th1(&cameraDecode::decode_CPU, this);
    th1.detach();
  }
  else if (this->hardwareType == "GPU")
  {
    std::cout << "Choose GPU Mode" << std::endl;
    std::thread th2(&cameraDecode::decode_GPU, this);
    th2.detach();
  }
  else if (this->hardwareType == "iGPU")
  {
    std::cout << "Choose iGPU Mode" << std::endl;
    std::thread th3(&cameraDecode::decode_iGPU, this);
    th3.detach();
  }
  else
  {
    std::cout << "Decode Type Not Exist!" << std::endl;
  }
}