#include "decode_detection.h"

#define RGB_IMG 0

int Convert24Image(char *p32Img, char *p24Img, int dwSize32)
{
    if(p32Img != NULL && p24Img != NULL && dwSize32>0)
    {
        int dwSize24;
        dwSize24=(dwSize32 * 3)/4;
        char *pTemp,*ptr;
        char* pTemp_r=p32Img;
        char* pTemp_g=p32Img+1;
        char* pTemp_b=p32Img+2;
        char* ptr_r = p24Img;
        char* ptr_g = p24Img + 1;
        char* ptr_b = p24Img + 2;
        int ival=0;
        for (int index = 0; index < dwSize32/4 ; index++)
        {
            memcpy(ptr_r, pTemp_r, 3);
            pTemp_r+=4;
            ptr_r+=3;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

CAMERASTATE checkCameraState(string URL,string cameraID)
{
    int ret = 0;
    AVInputFormat *inputFmt = NULL;
    AVFormatContext *ifmt_ctx = NULL;
    AVDictionary*  options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if ((ret = avformat_open_input(&ifmt_ctx, URL.c_str(), inputFmt, &options)) < 0) {
        return NOT_CONNECTABLE_STATE;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        avformat_close_input(&ifmt_ctx);
        return NOT_CONNECTABLE_STATE;
    }

    avformat_close_input(&ifmt_ctx);
    return CONNECTABLE_STATE;

}

static enum AVPixelFormat get_vaapi_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == AV_PIX_FMT_VAAPI)
        {
            return *p;
        }
    }

    fprintf(stderr, "[HW_DECODE] Unable to decode this file using VA-API.\n");
    return AV_PIX_FMT_NONE;
}


void softwareDecode(Scheduler &s,string URL,string cameraIP )
{
    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    int ret = 0, got_picture, video_stream = -1;

    AVInputFormat *inputFmt = NULL;
    AVFormatContext *ifmt_ctx = NULL;
    AVDictionary*  options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if (( ret = avformat_open_input(&ifmt_ctx, URL.c_str(), inputFmt, &options) ) < 0) {
        fprintf(stderr, "[SW_DECODE] Cannot open input file '%s' \n", URL.c_str());
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "[SW_DECODE] Cannot find input stream information. \n");
        return;
    }

    for (uint i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = i;
            break;
        }
    }

    video_stream = ret;
    AVStream *video = ifmt_ctx->streams[video_stream];

    AVCodecContext *pCodecCtx = ifmt_ctx->streams[video_stream]->codec;

    AVCodec *decoder = avcodec_find_decoder(pCodecCtx->codec_id);

    if(avcodec_open2(pCodecCtx, decoder, NULL)<0)
    {
        printf("[SW_DECODE] Could not open codec.\n");
        return;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        printf("[SW_DECODE] failed to allocated memory for AVPacket. \n");
        return;
    }

    printf("format %s, duration %d us, bitrate %lld %d %d.\n",
           ifmt_ctx->iformat->name,
           ifmt_ctx->duration / 1000LL,
           ifmt_ctx->bit_rate,
           video->r_frame_rate.num/video->r_frame_rate.den,//frame per second
           1//video->codec->framerate.num/video->codec->framerate.den
           );

    AVFrame *sFrame = av_frame_alloc();
    if (!sFrame)
    {
        printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }

    int frame_rate = 1000/( video->r_frame_rate.num/video->r_frame_rate.den);

    int number = 0;
    int limit = 0;

    ret = 1;
    got_picture = 1;
    auto t1 = std::chrono::steady_clock::now();
    const int wait_time = s.CONF_["demo_wait_time_ms"]; 
    int frameID=0;
    while(ret >= 0)
    {
        if(got_picture)
        {
            t1 = std::chrono::steady_clock::now();
            got_picture = 0;
        }
        ret = av_read_frame(ifmt_ctx, pPacket);
   
        if (pPacket->stream_index == video_stream)
        {
            avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, pPacket);

            if (got_picture)
            {
                if((pFrame->pict_type != AV_PICTURE_TYPE_P&&number != limit))
                {
                    number++;
                }
                if(pFrame->pict_type == AV_PICTURE_TYPE_I)
                {
                    struct SwsContext *img_convert_ctx;
                    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, SWS_BICUBIC, NULL, NULL, NULL);
                    sFrame->data[0] = (uint8_t*)malloc(sizeof(uint8_t)*4*pCodecCtx->width*pCodecCtx->height);
                    avpicture_fill((AVPicture *)sFrame, sFrame->data[0], AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, pCodecCtx->width, pCodecCtx->height);

                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  sFrame->data, sFrame->linesize);

                    cv::Mat argb(pFrame->height, pFrame->width, CV_8UC4, sFrame->data[0]);

                    int lSize = argb.rows * argb.cols * argb.channels();
                    int size_out = sizeof(char) * lSize * 3 / 4;
                    char buffer_out[size_out];

                    Convert24Image((char *)argb.data, buffer_out, lSize);

                    cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() ); 		                           
                      //传入I帧图片,注册检测任务类型
                     PushedPackage pp;
                     if(frameID == 10000)
                      {
                         frameID = 0;
                      }
                     for(int i=0;i<8;i++)
                     {
                         pp.image = rgb.clone();                  
                         pp.camera_ip = cameraIP;
                         pp.task = tasks[i];   
                         pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

                         mt.lock();
                         s.push(pp);
                         mt.unlock();

                         FetchedPackage fp;
                          mt.lock();
					      s.fetch(fp,pp.key);
                          mt.unlock();
                         
                          cv::Mat tmp = rgb.clone();
	                      draw(tmp,fp);
                         
                          if(!fp.results.empty())
						   {
		                     if(fp.task != Task::FIRE_SMOKE)
		                      { 
				                stringstream ss;
		                        mt_prefix.lock();
				                ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
			                    mt_prefix.unlock();

								string dir  = s.CONF_["output_dir"]+cameraIP;
							    if(opendir(dir.c_str()) == NULL)
							     {
								    int ret = mkdir(dir.c_str(),0775);
							     }
								string save_path = dir + ss.str();
								cout<<"Save to"<<save_path<<endl;
								cv::imwrite(save_path,tmp);
		                      }
		                     else
		                      {
					  		    if( fp.cls == 1 )
								   {
								      stringstream ss;
			 					      mt_prefix.lock();
								      ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
								      mt_prefix.unlock();
									 	
						              string dir  = s.CONF_["output_dir"];
									  if(opendir(dir.c_str()) == NULL)
									 	 {
											int ret = mkdir(dir.c_str(),0775);
										 }
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
									  string save_path = dir + ss.str();
									  cout<<"Save to"<<save_path<<endl;
									  cv::imwrite(save_path,tmp);
								    }
				               }
                            }
                          this_thread::sleep_for(chrono::milliseconds(wait_time));                   
                     }
                    
                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);
                }
                if((pFrame->pict_type != AV_PICTURE_TYPE_I && number == limit))
                {
                    number = 0;

                    struct SwsContext *img_convert_ctx;
                    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, SWS_BICUBIC, NULL, NULL, NULL);
                    sFrame->data[0] = (uint8_t*)malloc(sizeof(uint8_t)*4*pCodecCtx->width*pCodecCtx->height);
                    avpicture_fill((AVPicture *)sFrame, sFrame->data[0], AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, pCodecCtx->width, pCodecCtx->height);

                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  sFrame->data, sFrame->linesize);

                    cv::Mat argb(pFrame->height, pFrame->width, CV_8UC4, sFrame->data[0]);

                    int lSize = argb.rows * argb.cols * argb.channels();
                    int size_out = sizeof(char) * lSize * 3 / 4;
                    char buffer_out[size_out];

                    Convert24Image((char *)argb.data, buffer_out, lSize);

                    cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                    //传入P帧图片,注册检测任务类型
                     PushedPackage pp;
                     if(frameID == 10000)
                      {
                         frameID = 0;
                      }
                     for(int i=0;i<8;i++)
                     {
                         pp.image = rgb.clone();                  
                         pp.camera_ip = cameraIP;
                         pp.task = tasks[i];     
                         pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

                         mt.lock();
                         s.push(pp);
                         mt.unlock();
  
                         FetchedPackage fp;
                          mt.lock();
                          s.fetch(fp,pp.key);
                          mt.unlock();
                          cv::Mat tmp = rgb.clone();
                          draw(tmp,fp);
                          if(!fp.results.empty())
                          {
				              if(fp.task != Task::FIRE_SMOKE)
				              {
				                 stringstream ss;
				                 ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		   
								 string dir = s.CONF_["output_dir"];
								 if(opendir(dir.c_str()) == NULL)
								  {
								      int ret = mkdir(dir.c_str(),0775);
								  }
                                dir+=cameraIP;
								if(opendir(dir.c_str()) == NULL)
								{
								   int ret = mkdir(dir.c_str(),0775);
								}
								 string save_path = dir + ss.str();
								 cout<<"Save to"<<save_path<<endl;
								 cv::imwrite(save_path,tmp);
				               }
		                     else
		                       {
						   			if(fp.cls == 1)
									{
						                stringstream ss;
						           		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
			   
										 string dir  = s.CONF_["output_dir"];
										 if(opendir(dir.c_str()) == NULL)
										  {
											  int ret = mkdir(dir.c_str(),0775);
										  }
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
								    }
		                        }
                           }
                          this_thread::sleep_for(chrono::milliseconds(wait_time));                 
                     }
                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);
                }

                frameID++;

                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();

                std::cout << "SW decode time per frame: " << dr_ms << std::endl;

                if((frame_rate - dr_ms) > 0)
                  {
                     usleep(int( (frame_rate - dr_ms)*1000 ));
                  }
            }
        }
        else
        {
            av_packet_unref(pPacket);
            continue;
        }
        av_packet_unref(pPacket);
    }
    av_packet_unref(pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sFrame);
    av_packet_free(&pPacket);
    avformat_close_input(&ifmt_ctx);

   std::cout<<"SoftWareDecode Finish!"<<std::endl;
}

void hardwareDecode(Scheduler &s,string URL,string cameraIP)
{
    int ret = 0;

    AVBufferRef *hw_device_ctx = NULL;
    ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "[HW_DECODE] Failed to create a VAAPI device. \n");
        return;
    }

    AVDictionary*  options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    AVFormatContext *ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, URL.c_str(), NULL, &options)) < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot open input file '%s' \n", URL.c_str());
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot find input stream information. \n");
        return;
    }

    AVCodec *decoder = NULL;
    ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (ret < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot find a video stream in the input file. \n");
        return;
    }
    int video_stream = ret;

    AVCodecContext *decoder_ctx = NULL;
    if (!(decoder_ctx = avcodec_alloc_context3(decoder)))
        return;

    AVStream *video = ifmt_ctx->streams[video_stream];

    if ( (ret = avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0) ) {
        fprintf(stderr, "[HW_DECODE] avcodec_parameters_to_context error. \n");
        return;
    }

    decoder_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    if (!decoder_ctx->hw_device_ctx) {
        fprintf(stderr, "[HW_DECODE] A hardware device reference create failed.\n");
        return;
    }

    decoder_ctx->get_format = get_vaapi_format;

    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0)
        fprintf(stderr, "[HW_DECODE] Failed to open codec for decoding. \n");

    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }

    AVFrame *sw_frame = av_frame_alloc();
    if (!sw_frame)
    {
        printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        printf("[HW_DECODE] failed to allocated memory for AVPacket. \n");
        return;
    }

    int response = 0;
    int number = 0;
    int limit = 0;
    int frame_rate = 1000/(video->r_frame_rate.num/video->r_frame_rate.den);
    int got_picture = 1;
    auto t1 = std::chrono::steady_clock::now();
    ret = 1;
    const int wait_time = s.CONF_["demo_wait_time_ms"]; 
    int frameID = 0;
    while(ret >= 0 )
    {
        if(got_picture)
        {
            t1 = std::chrono::steady_clock::now();
            got_picture = 0;
        }
        ret = av_read_frame(ifmt_ctx, pPacket);

        if (pPacket->stream_index == video_stream)
        {
            response = avcodec_send_packet(decoder_ctx, pPacket);

            if(response < 0)
                printf("[HW_DECODE] Error while sending a packet to the decoder. \n");
                    
            while (response >= 0)
            {
                response = avcodec_receive_frame(decoder_ctx, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    //printf("[HW_DECODE] Output invalid or has flashed decoder don`t have new frame !\n");
                    break;
                } else if (response < 0) {
                    printf("[HW_DECODE] Error while receiving a frame from the decoder. \n");
                    return;
                }
                if((pFrame->pict_type == AV_PICTURE_TYPE_P && number != limit))
                {
                    number++;
                }
                if((pFrame->pict_type == AV_PICTURE_TYPE_I))
                {
                    if (response >= 0) {

                        if(AV_PIX_FMT_VAAPI == pFrame->format)
                        {

                            sw_frame->format = AV_PIX_FMT_ARGB;
                            if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0) {
                                fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                                break;
                            }
                        }

                        cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);

                        int lSize = argb.rows * argb.cols * argb.channels();
                        int size_out = sizeof(char) * lSize * 3 / 4;
                        char buffer_out[size_out];

                        Convert24Image((char *)argb.data, buffer_out, lSize);
                        cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                      //传入I帧图片,注册检测任务类型
                       PushedPackage pp;
		               if(frameID == 10000)
		                  {
		                     frameID = 0;
		                  }
			           for(int i=0;i<8;i++)
			            {
			              pp.image = rgb.clone();                  
			              pp.camera_ip = cameraIP;
			              pp.task = tasks[i];  
                          pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

		                  mt.lock();   
			              s.push(pp);
		                  mt.unlock();
		                 
		                  FetchedPackage fp;
		                  mt.lock();
		                  s.fetch(fp,pp.key);
		                  mt.unlock();
		                 
		                  cv::Mat tmp = rgb.clone();
		                  draw(tmp,fp);
		                  if(!fp.results.empty())
		                  {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"]+cameraIP;
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
		                     }
			              this_thread::sleep_for(chrono::milliseconds(wait_time));                 
			             }                        
                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }
                if((pFrame->pict_type != AV_PICTURE_TYPE_I && number == limit))
                {
                    number = 0;
                    if (response >= 0) {

                        if(AV_PIX_FMT_VAAPI == pFrame->format)
                        {

                            sw_frame->format = AV_PIX_FMT_ARGB;
                            if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0) {
                                fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                                break;
                            }
                        }

                        cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);

                        int lSize = argb.rows * argb.cols * argb.channels();
                        int size_out = sizeof(char) * lSize * 3 / 4;
                        char buffer_out[size_out];
                        Convert24Image((char *)argb.data, buffer_out, lSize);
                        cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                                    
                      //传入P帧图片,注册检测任务类型
                     PushedPackage pp;
	                 if(frameID == 10000)
	                   {
	                     frameID = 0;
	                   }
	                 for(int i=0;i<8;i++)
	                  {
			              pp.image = rgb.clone();                  
			              pp.camera_ip = cameraIP;
			              pp.task = tasks[i]; 
                          pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);
    
		                  mt.lock();   
			              s.push(pp);
		                  mt.unlock();
		                 
		                  FetchedPackage fp;
		                  mt.lock();
		                  s.fetch(fp,pp.key);
		                  mt.unlock();
		                 
		                  cv::Mat tmp = rgb.clone();
		                  draw(tmp,fp);

                          if(!fp.results.empty())
                          {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"];
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
                          }
	                      this_thread::sleep_for(chrono::milliseconds(wait_time));                 
	                  }
                          
                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }
                frameID++;
                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();                              
                std::cout << "HW Decode time per frame: "<<dr_ms<< std::endl;      
 		
               if((frame_rate - dr_ms) > 0)
		         {
		             usleep(int( (frame_rate - dr_ms)*1000 ));
		         }
            }
        }
       else
	    {
	      av_packet_unref(pPacket);
           continue;
	    } 
        av_packet_unref(pPacket);
    }

    avformat_close_input(&ifmt_ctx);
    avcodec_free_context(&decoder_ctx);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sw_frame);
    av_buffer_unref(&hw_device_ctx);
}

void softwareLoopDecode(Scheduler &s,string URL,string cameraIP)
{
    av_register_all();
    avformat_network_init();
    avdevice_register_all();

restart:
    int ret = 0, got_picture, video_stream = -1;

    AVInputFormat *inputFmt = NULL;
    AVFormatContext *ifmt_ctx = NULL;
    AVDictionary*  options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if ((ret = avformat_open_input(&ifmt_ctx, URL.c_str(), inputFmt, &options)) < 0) {
        fprintf(stderr, "[SW_DECODE] Cannot open input file '%s' \n", URL.c_str());
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "[SW_DECODE] Cannot find input stream information. \n");
        return;
    }

    for (uint i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = i;
            break;
        }
    }

    video_stream = ret;
    AVStream *video = ifmt_ctx->streams[video_stream];

    AVCodecContext *pCodecCtx = ifmt_ctx->streams[video_stream]->codec;

    AVCodec *decoder = avcodec_find_decoder(pCodecCtx->codec_id);

    if(avcodec_open2(pCodecCtx, decoder, NULL)<0)
    {
        printf("[SW_DECODE] Could not open codec.\n");
        return;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        printf("[SW_DECODE] failed to allocated memory for AVPacket. \n");
        return;
    }

    printf("format %s, duration %d us, bitrate %lld %d %d.\n",
           ifmt_ctx->iformat->name,
           ifmt_ctx->duration / 1000LL,
           ifmt_ctx->bit_rate,
           video->r_frame_rate.num/video->r_frame_rate.den,
           1//video->codec->framerate.num/video->codec->framerate.den
           );

    AVFrame *sFrame = av_frame_alloc();
    if (!sFrame)
    {
        printf("[SW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }
    
    //ms
    int frame_rate = 1000/( video->r_frame_rate.num/video->r_frame_rate.den);

    int number = 0;
    int limit = 0;

    ret = 1;
    got_picture = 1;
    auto t1 = std::chrono::steady_clock::now();

    const int wait_time = s.CONF_["demo_wait_time_ms"]; 
    int frameID=0;
    while(ret >= 0)
    {
        if(got_picture)
        {
            t1 = std::chrono::steady_clock::now();
            got_picture = 0;
        }

        ret = av_read_frame(ifmt_ctx, pPacket);

        if (pPacket->stream_index == video_stream)
        {
            avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, pPacket);

            if (got_picture)
            {
                if((pFrame->pict_type != AV_PICTURE_TYPE_P&&number != limit))
                {
                    number++;
                }
                if(pFrame->pict_type == AV_PICTURE_TYPE_I)
                {
                    struct SwsContext *img_convert_ctx;
                    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, SWS_BICUBIC, NULL, NULL, NULL);
                    sFrame->data[0] = (uint8_t*)malloc(sizeof(uint8_t)*4*pCodecCtx->width*pCodecCtx->height);
                    avpicture_fill((AVPicture *)sFrame, sFrame->data[0], AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, pCodecCtx->width, pCodecCtx->height);

                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  sFrame->data, sFrame->linesize);
                    cv::Mat argb(pFrame->height, pFrame->width, CV_8UC4, sFrame->data[0]);

                    int lSize = argb.rows * argb.cols * argb.channels();
                    int size_out = sizeof(char) * lSize * 3 / 4;
                    char buffer_out[size_out];

                    Convert24Image((char *)argb.data, buffer_out, lSize);
                    cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                      //传入I帧图片,注册检测任务类型
                     PushedPackage pp;
                     if(frameID==10000)
                       {
                          frameID=0;
                       }
                     for(int i=0;i<8;i++)
                     {                         
                         pp.image = rgb.clone();                  
                         pp.camera_ip = cameraIP;
                         pp.task = tasks[i];     
                         pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

		                 mt.lock();   
			             s.push(pp);
		                 mt.unlock();
		                 
		                 FetchedPackage fp;
		                 mt.lock();
		                 s.fetch(fp,pp.key);
		                 mt.unlock();
		                 
		                 cv::Mat tmp = rgb.clone();
		                 draw(tmp,fp);

                          if(!fp.results.empty())
                          {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"];
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
                                    dir+=cameraIP;
									if(opendir(dir.c_str()) == NULL)
									{
									   int ret = mkdir(dir.c_str(),0775);
									}
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
                           }
                         this_thread::sleep_for(chrono::milliseconds(wait_time));                 
                     }

                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);
                }
                
                if((pFrame->pict_type != AV_PICTURE_TYPE_I && number == limit))
                {
                    number = 0;

                    struct SwsContext *img_convert_ctx;
                    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, SWS_BICUBIC, NULL, NULL, NULL);
                    sFrame->data[0] = (uint8_t*)malloc(sizeof(uint8_t)*4*pCodecCtx->width*pCodecCtx->height);
                    avpicture_fill((AVPicture *)sFrame, sFrame->data[0], AV_PIX_FMT_BGRA/*AV_PIX_FMT_ARGB*/, pCodecCtx->width, pCodecCtx->height);

                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  sFrame->data, sFrame->linesize);
                    cv::Mat argb(pFrame->height, pFrame->width, CV_8UC4, sFrame->data[0]);

			        int lSize = argb.rows * argb.cols * argb.channels();
			        int size_out = sizeof(char) * lSize * 3 / 4;
			        char buffer_out[size_out];

			        Convert24Image((char *)argb.data, buffer_out, lSize);
			        cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                      //传入P帧图片,注册检测任务类型
                     PushedPackage pp;
                     if(frameID==10000)
                        {
   							frameID=0;
                        }
                     for(int i=0;i<8;i++)
                     {
                         pp.image = rgb.clone();                  
                         pp.camera_ip = cameraIP;
                         pp.task = tasks[i];     
		                 pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

                         mt.lock();   
			             s.push(pp);
		                 mt.unlock();
		                 
		                 FetchedPackage fp;
		                 mt.lock();
		                 s.fetch(fp,pp.key);
		                 mt.unlock();
		                 
		                 cv::Mat tmp = rgb.clone();
		                 draw(tmp,fp);

                          if(!fp.results.empty())
                          {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"]+cameraIP;
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
                          }
                         this_thread::sleep_for(chrono::milliseconds(wait_time));                 
                     }

                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);

                }
                 frameID++;
                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();
                std::cout << "SW decode time per frame: " << dr_ms << std::endl;

                if((frame_rate - dr_ms) > 0)
                 {
                    usleep( int( (frame_rate - dr_ms)*1000 ));
                }
            }

        }
        else
        {
            av_packet_unref(pPacket);
            continue;
        }
        av_packet_unref(pPacket);
    }
    av_packet_unref(pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sFrame);
    av_packet_free(&pPacket);
    avformat_close_input(&ifmt_ctx);
    if(ret<0)
    {
        goto restart;
    }
}

void hardwareLoopDecode(Scheduler &s,string URL,string cameraIP)
{
restart:
    int ret = 0;

    AVBufferRef *hw_device_ctx = NULL;
    ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "[HW_DECODE] Failed to create a VAAPI device. \n");
        return;
    }

    AVDictionary*  options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    AVFormatContext *ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, URL.c_str(), NULL, &options)) < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot open input file '%s' \n", URL.c_str());
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot find input stream information. \n");
        return;
    }

    AVCodec *decoder = NULL;
    ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (ret < 0) {
        fprintf(stderr, "[HW_DECODE] Cannot find a video stream in the input file. \n");
        return;
    }
    int video_stream = ret;

    AVCodecContext *decoder_ctx = NULL;
    if (!(decoder_ctx = avcodec_alloc_context3(decoder)))
        return;

    AVStream *video = ifmt_ctx->streams[video_stream];

    if ((ret = avcodec_parameters_to_context(decoder_ctx, video->codecpar)) < 0) {
        fprintf(stderr, "[HW_DECODE] avcodec_parameters_to_context error. \n");
        return;
    }

    decoder_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    if (!decoder_ctx->hw_device_ctx) {
        fprintf(stderr, "[HW_DECODE] A hardware device reference create failed.\n");
        return;
    }

    decoder_ctx->get_format = get_vaapi_format;

    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0)
        fprintf(stderr, "[HW_DECODE] Failed to open codec for decoding. \n");

    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }

    AVFrame *sw_frame = av_frame_alloc();
    if (!sw_frame)
    {
        printf("[HW_DECODE] failed to allocated memory for AVFrame. \n");
        return;
    }
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        printf("[HW_DECODE] failed to allocated memory for AVPacket. \n");
        return;
    }


    int response = 0;
    int number = 0;
    int limit = 0;
    int frame_rate = 1000/(video->r_frame_rate.num/video->r_frame_rate.den);
    int got_picture = 1;
    auto t1 = std::chrono::steady_clock::now();
    ret = 1;
    const int wait_time = s.CONF_["demo_wait_time_ms"]; 
    int frameID=0;
    while(ret >= 0)
    {
        if(got_picture)
        {
            t1 = std::chrono::steady_clock::now();
            got_picture = 0;
        }
        ret = av_read_frame(ifmt_ctx, pPacket);

        if (pPacket->stream_index == video_stream)
        {
            response = avcodec_send_packet(decoder_ctx, pPacket);
            if(response < 0)
                printf("[HW_DECODE] Error while sending a packet to the decoder. \n");

            while (response >= 0)
            {
                response = avcodec_receive_frame(decoder_ctx, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    //printf("[HW_DECODE] Output invalid or has flashed decoder don`t have new frame !\n");
                    break;
                } else if (response < 0) {
                    printf("[HW_DECODE] Error while receiving a frame from the decoder. \n");
                    return;
                }
                if((pFrame->pict_type == AV_PICTURE_TYPE_P && number != limit))
                {
                    number++;
                }
                if((pFrame->pict_type == AV_PICTURE_TYPE_I))
                {
                    if (response >= 0) {

                        if(AV_PIX_FMT_VAAPI == pFrame->format)
                        {

                            sw_frame->format = AV_PIX_FMT_ARGB;
                            if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0) {
                                fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                                goto end;
                            }
                        }

                        cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);

                        int lSize = argb.rows * argb.cols * argb.channels();
                        int size_out = sizeof(char) * lSize * 3 / 4;
                        char buffer_out[size_out];

                        Convert24Image((char *)argb.data, buffer_out, lSize);
                        cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
			            std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                       //传入I帧图片,注册检测任务类型
                     PushedPackage pp;
                    if(frameID==10000)
                       {
                          frameID=0;
                       }
	                for(int i=0;i<8;i++)
	                {                         
	                     pp.image = rgb.clone();                  
	                     pp.camera_ip = cameraIP;
	                     pp.task = tasks[i];    
					     pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);

                         mt.lock();   
			             s.push(pp);
		                 mt.unlock();
		                 
		                 FetchedPackage fp;
		                 mt.lock();
		                 s.fetch(fp,pp.key);
		                 mt.unlock();
		                 
		                 cv::Mat tmp = rgb.clone();
		                 draw(tmp,fp);

                          if(!fp.results.empty())
                          {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"];
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
                                    dir+=cameraIP;
									if(opendir(dir.c_str()) == NULL)
									{
									   int ret = mkdir(dir.c_str(),0775);
									}
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
                           }
	                   this_thread::sleep_for(chrono::milliseconds(wait_time));                 
	                }
                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }
                if((pFrame->pict_type != AV_PICTURE_TYPE_I && number == limit))
                {
                    number = 0;
                    if (response >= 0) {

                        if(AV_PIX_FMT_VAAPI == pFrame->format)
                        {
                            sw_frame->format = AV_PIX_FMT_ARGB;
                            if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0) {
                                fprintf(stderr, "[HW_DECODE] Error transferring the data to system memory\n");
                                goto end;
                            }
                        }

                        cv::Mat argb(sw_frame->height, sw_frame->width, CV_8UC4, sw_frame->data[0]);

                        int lSize = argb.rows * argb.cols * argb.channels();
                        int size_out = sizeof(char) * lSize * 3 / 4;
                        char buffer_out[size_out];

                        Convert24Image((char *)argb.data, buffer_out, lSize);
                        cv::Mat rgb(argb.rows, argb.cols, CV_8UC3, buffer_out);
                        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
                         //传入P帧图片,注册检测任务类型
                      PushedPackage pp;
                    if(frameID==10000)
                       {
                          frameID=0;
                       }
	                 for(int i=0;i<8;i++)
	                  {                         
	                     pp.image = rgb.clone();                  
	                     pp.camera_ip = cameraIP;
	                     pp.task = tasks[i];   
				         pp.key = pp.camera_ip+"_"+to_string(frameID)+"_"+to_string(pp.task);  

                         mt.lock();   
			             s.push(pp);
		                 mt.unlock();
		                 
		                 FetchedPackage fp;
		                 mt.lock();
		                 s.fetch(fp,pp.key);
		                 mt.unlock();
		                 
		                 cv::Mat tmp = rgb.clone();
		                 draw(tmp,fp);

                          if(!fp.results.empty())
                          {
		                      if(fp.task != Task::FIRE_SMOKE)
		                       {
		                    		stringstream ss;
		                            mt_prefix.lock();
		                    		ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
		                            mt_prefix.unlock();
									string dir  = s.CONF_["output_dir"];
									if(opendir(dir.c_str()) == NULL)
									  {
										 int ret = mkdir(dir.c_str(),0775);
									  }
                                    dir+=cameraIP;
									if(opendir(dir.c_str()) == NULL)
									{
									   int ret = mkdir(dir.c_str(),0775);
									}
									string save_path = dir + ss.str();
									cout<<"Save to"<<save_path<<endl;
									cv::imwrite(save_path,tmp);
		                        }
		                       else
		                        {
								  if(fp.cls == 1)
									{
										stringstream ss;
										mt_prefix.lock();
										ss<<"/"<<prefix[i]<<to_string(ms.count())<<".png";
										mt_prefix.unlock();
										string dir  = s.CONF_["output_dir"];
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
                                        dir+=cameraIP;
										if(opendir(dir.c_str()) == NULL)
										{
										   int ret = mkdir(dir.c_str(),0775);
										}
										string save_path = dir + ss.str();
										cout<<"Save to"<<save_path<<endl;
										cv::imwrite(save_path,tmp);
									}
		                         }
                           }
	                   this_thread::sleep_for(chrono::milliseconds(wait_time));                 
	                }

                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }
                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();
                std::cout << "SW decode time per frame: " << dr_ms << std::endl;
                
                 if((frame_rate - dr_ms) > 0)
                  {
                     usleep(int( (frame_rate - dr_ms)*1000 ));
                  }
            }
        }
        av_packet_unref(pPacket);
    }
end:
    avformat_close_input(&ifmt_ctx);
    avcodec_free_context(&decoder_ctx);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sw_frame);
    av_buffer_unref(&hw_device_ctx);

    if(ret<0)
    {
        goto restart;
    }
    return;
}


void draw_label(cv::Mat& img, const string& label, cv::Point& p1,cv::Point& p2, const cv::Scalar& color)
{
    int font = cv::FONT_HERSHEY_COMPLEX;
    double font_scale = 0.8;
    int thickness = 2;
    int baseLine ;
    cv::Size text_size = cv::getTextSize(label,font,font_scale,thickness,&baseLine); 

    cv::Point o = p1;
    if(o.y - text_size.height < 0) o.y = p2.y + text_size.height;
    if(o.y > img.rows) o.y = p2.y;
    if(o.x + text_size.width > img.cols) o.x = img.cols - text_size.width;
 
    cv::Point o1 = {o.x, o.y - text_size.height};
    cv::Point o2 = {o.x + text_size.width, o.y};
    cv::rectangle(img, o1, o2,color, -1);
   
    cv::putText(img, label, o, font, font_scale, COLOR_WHITE, thickness);
}


void draw_person_clothing(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_clothing: "<<fp.results.size(); 
   for(auto& result: fp.results)
   {
      cv::Point p1(result.bbox.x1, result.bbox.y1);
      cv::Point p2(result.bbox.x2, result.bbox.y2);
      cv::rectangle(img, p1, p2, colors[result.bbox.cls],2);
      cout<< "rectangle: "<<p1.x<<" "<<p1.y<<" "<<p2.x<<" "<<p2.y<<endl;   
   
      stringstream ss;

      if(result.bbox.cls == 1)      
      {
         ss<<" No Helmet!";
      }
      if(result.bbox.cls == 2) 
      {
    	 ss<<"No RedUniform!";
      }
      if(result.bbox.cls == 3) 
      {
    	 ss<<"No Helemt&RedUniform!";
      }
      draw_label(img, ss.str(),p1,p2,colors[result.bbox.cls]);
   }

}

void draw_person_vest(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_vest: "<<fp.results.size(); 
   for(auto& result: fp.results)
   {
      cv::Point p1(result.bbox.x1, result.bbox.y1);
      cv::Point p2(result.bbox.x2, result.bbox.y2);
      cv::rectangle(img, p1, p2, colors[result.bbox.cls],2);
      cout<< "rectangle: "<<p1.x<<" "<<p1.y<<" "<<p2.x<<" "<<p2.y<<endl;   
   
      stringstream ss;

      if(result.bbox.cls == 2 || result.bbox.cls == 3) 
      {
    	 ss<<"No Vest!";
      }
      draw_label(img, ss.str(),p1,p2,colors[result.bbox.cls]);
   }

}


void draw_person_invasion(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_invasion: "<<fp.results.size();
    for(auto& result: fp.results)
    {
         cv::Point p1(result.bbox.x1, result.bbox.y1);
         cv::Point p2(result.bbox.x2, result.bbox.y2);
         cv::rectangle(img, p1, p2, COLOR_RED,2);
 
         stringstream ss;
         ss<<"Invasion!";
         draw_label(img,ss.str(),p1,p2,COLOR_RED);
    }
}

void draw_person_skin(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_person_skin: "<<fp.results.size();
  vector<vector<cv::Point>>contours;

  for(auto &result : fp.results)
   {
        cv::Point p1(result.bbox.x1, result.bbox.y1);
	    cv::Point p2(result.bbox.x2, result.bbox.y2);
	    cv::rectangle(img, p1, p2, COLOR_RED,2);
        
        for(auto& pts:result.ptss)
	     {
 	        vector<cv::Point>contour;
             for(auto& pt : pts)
             {
                cv::Point p(pt.x,pt.y);
                contour.push_back(p);
             } 
             contours.push_back(contour);
          }
    if(!contours.empty())
    {
		 stringstream ss;
		 ss<<"skin exposure ! ";
		 draw_label(img,ss.str(),p1,p2,COLOR_RED);
    }

   }

   cv::polylines(img,contours,true,COLOR_WHITE,2,cv::LINE_AA);
   cv::fillPoly(img, contours,COLOR_PINK);	
}

void draw_fire(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_fire: "<<fp.cls;
  int font = cv::FONT_HERSHEY_COMPLEX;
  double font_scale = 1;
  int thickness = 2;
  int baseline;
  string label = "Fire!";

  if(fp.cls == 1)
  {

   cv::Size text_size = cv::getTextSize(label,font, font_scale, thickness,&baseline);

   cv::Point o1(0,0);
   cv::Point o2(text_size.width, text_size.height);
   cv::Point o(0,text_size.height);
   cv::rectangle(img,o1,o2,COLOR_WHITE,-1);
   cv::putText(img,label,o,font,font_scale,COLOR_RED,thickness);
  }
}

void draw_person_smoke(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_smoke: "<<fp.results.size();
    for(auto& result: fp.results)
    {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);

      string label ="";
      if(result.bbox.cls == 2)
      {
    	 label = "personSmoke!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
    }
}

void draw_person_phone(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_phone: "<<fp.results.size();
    for(auto& result: fp.results)
    {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);

      string label ="";
      if(result.bbox.cls == 1)
      {
    	 label = "personPhone!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
    }
}

void draw_oil_leak(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_oil_leak: "<<fp.results.size();
  vector<vector<cv::Point>>contours;

  for(auto &result : fp.results)
   {
       if(!result.ptss.empty())
       {
           cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
           cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
		   
           //show task types
           string label = "";
           label="Oil Leak!";
          // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
           cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
       }

        for(auto& pts:result.ptss)
	      {
			vector<cv::Point>contour;
             for(auto& pt : pts)
             {
				 cv::Point p(pt.x,pt.y);
				 contour.push_back(p);
             }
			contours.push_back(contour);
          }
    }
	cv::polylines(img,contours,true,cv::Scalar(0,0,255),2,cv::LINE_AA);
	cv::fillPoly(img, contours,cv::Scalar(255,0,255));
}

void draw(cv::Mat& img, FetchedPackage& fp)
{
     switch(fp.task)
     {
        case Task::PERSON_CLOTHING:
            draw_person_clothing(img,fp);
            break;
        case Task::PERSON_CLOTHING_VEST:
           draw_person_vest(img,fp);
           break;
        case Task::PERSON_INVASION:
            draw_person_invasion(img,fp);            
            break;
        case Task::SKIN_EXPOSURE:
            draw_person_skin(img,fp);          
            break;     
        case Task::FIRE_SMOKE:
            draw_fire(img,fp) ;         
            break;
		 case Task::SMOKE:
			 draw_person_smoke(img,fp);
			 break;
		 case Task::PHONE:
			 draw_person_phone(img,fp);
			 break;
		 case Task::LEAK_OIL:
			 draw_oil_leak(img,fp);
			 break;
        default:
            cout<<"Unsupported task: "<<fp.task<<endl;
            break;
     }
}
