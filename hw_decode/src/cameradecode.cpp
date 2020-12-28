#include "cameradecode.h"

#define RGB_IMG 0

CameraDecode::CameraDecode()
{
    this->stoped = false;
}

CameraDecode::~CameraDecode()
{
    this->stop();
}

int CameraDecode::Convert24Image(char *p32Img, char *p24Img, int dwSize32)
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

CAMERASTATE CameraDecode::checkCameraState(string URL,string cameraID)
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


void CameraDecode::softwareDecode(string URL,string cameraID,bool saveImg)
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
    int index1 = 0;
    while(ret >= 0 && !this->stoped)
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
                     
                     //saveImage
                    if(saveImg)
		     {
                        string dir = "../results/"+cameraID;
                        if(opendir(dir.c_str()) == NULL)
                        {
                          int ret = mkdir(dir.c_str(),0775);
                        }

			string fileName =dir+"/"+to_string(index1++)+".jpg";
                        cv::imwrite(fileName,rgb);
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

                     //saveImage
                    if(saveImg)
		     {
                        string dir = "../results/"+cameraID;
                        if(opendir(dir.c_str()) == NULL)
                        {
                          int ret = mkdir(dir.c_str(),0775);
                        }

			string fileName =dir+"/"+to_string(index1++)+".jpg";
                        cv::imwrite(fileName,rgb);
                     }

                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);

                }

                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();
                std::cout << "SW decocde time per frame: " << dr_ms << std::endl;

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

void CameraDecode::hardwareDecode(string URL,string cameraID,bool saveImg)
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
    int index2=0;
    while(ret >= 0 && !this->stoped)
    {
        if(got_picture)
        {
            //t1 = std::chrono::steady_clock::now();
            got_picture = 0;
        }
        ret = av_read_frame(ifmt_ctx, pPacket);

        if (pPacket->stream_index == video_stream)
        {
            response = avcodec_send_packet(decoder_ctx, pPacket);
            sleep(1);

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

	                if(saveImg)
		        {			        
		          string dir = "../results/"+cameraID;
		          if(opendir(dir.c_str()) == NULL)
		          {
		            int ret = mkdir(dir.c_str(),0775);
		          }
			  string fileName =dir+"/"+to_string(index2++)+".jpg";
		          cv::imwrite(fileName,rgb);
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
                     //saveImage
	            if(saveImg)
		     {
	                string dir = "../results/"+cameraID;
	                if(opendir(dir.c_str()) == NULL)
	                {
	                  int ret = mkdir(dir.c_str(),0775);
	                }

			string fileName =dir+"/"+to_string(index2++)+".jpg";
	                cv::imwrite(fileName,rgb);
	             }
                          
                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }

                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();
                std::cout<<"HW decode time per frame: "<<dr_ms<<std::endl; 
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
        std::cout<<cameraID+"decoding"<<std::endl; 
        av_packet_unref(pPacket);
    }

    avformat_close_input(&ifmt_ctx);
    avcodec_free_context(&decoder_ctx);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&sw_frame);
    av_buffer_unref(&hw_device_ctx);

   std::cout<<"HardWareDecode Finish!"<<std::endl;
}

void CameraDecode::softwareLoopDecode(string URL,string cameraID,bool saveImg)
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

    int frame_rate = 1000/( video->r_frame_rate.num/video->r_frame_rate.den);

    int number = 0;
    int limit = 0;

    ret = 1;
    got_picture = 1;
    int index3 = 0;
    auto t1 = std::chrono::steady_clock::now();
    while(ret >= 0 && !this->stoped)
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
                     //saveImage
                    if(saveImg)
		     {
                        string dir = "../results/"+cameraID;
                        if(opendir(dir.c_str()) == NULL)
                        {
                          int ret = mkdir(dir.c_str(),0775);
                        }

			string fileName =dir+"/"+to_string(index3++)+".jpg";
                        cv::imwrite(fileName,rgb);
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

                    if(saveImg)
		     {
                        string dir = "../results/"+cameraID;
                        if(opendir(dir.c_str()) == NULL)
                        {
                          int ret = mkdir(dir.c_str(),0775);
                        }

			string fileName =dir+"/"+to_string(index3++)+".jpg";
                        cv::imwrite(fileName,rgb);
                     }
                    free(sFrame->data[0]);
                    sws_freeContext(img_convert_ctx);

                }

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
    if(!this->stoped)
    {
        goto restart;
    }
}

void CameraDecode::hardwareLoopDecode(string URL,string cameraID,bool saveImg)
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
    int index4=0;
    while(ret >= 0 && !this->stoped)
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

	              if(saveImg)
		       {
	                  static int index4 = 0;
	                  string dir = "../results/"+cameraID;
	                  if(opendir(dir.c_str()) == NULL)
	                  {
	                    int ret = mkdir(dir.c_str(),0775);
	                  }

			  string fileName =dir+"/"+to_string(index4++)+".jpg";
	                  cv::imwrite(fileName,rgb);
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
	               
                      if(saveImg)
		       {
	                  string dir = "../results/"+cameraID;
	                  if(opendir(dir.c_str()) == NULL)
	                  {
	                    int ret = mkdir(dir.c_str(),0775);
	                  }

			  string fileName =dir+"/"+to_string(index4++)+".jpg";
	                  cv::imwrite(fileName,rgb);
	               }
                        av_frame_unref(pFrame);
                        got_picture = 1;
                    }
                }
                double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();
                std::cout << "HW decode time per frame: " << dr_ms << std::endl;

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
    if(!this->stoped)
    {
        goto restart;
    }
    return;
}

void CameraDecode::stop()
{
    this->stoped = true;

    // for decode over
    sleep(1);
}

