#include "cameraEncode.h"

// std::queue<cv::Mat> frame_queue;
// std::mutex frame_mtx;

int set_hwframe_ctx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx)
{
    AVBufferRef *hw_frames_ref;
    AVHWFramesContext *frames_ctx = NULL;
    int err = 0;
    if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
        printf("Failed to create VAAPI frame context.\n");
        return -1;
    }
    frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
    frames_ctx->format    = AV_PIX_FMT_VAAPI;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->width     = ctx->width;
    frames_ctx->height    = ctx->height;
    frames_ctx->initial_pool_size = 20;  //该值为默认值，不能随意改动
    if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
        printf("Failed to initialize VAAPI frame context.\n");
        av_buffer_unref(&hw_frames_ref);
        return err;
    }
    ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    if (!ctx->hw_frames_ctx){
        err = AVERROR(ENOMEM);
    }   

    av_buffer_unref(&hw_frames_ref);
    return err;
}

void yuvI420ToNV12(char *I420,int w,int h,char *NV12)
{
    memcpy(NV12,I420,w*h);//y分量
    for(int i = 0,j = 0;i<w*h/4;i++,j+=2)
    {
        memcpy(NV12+w*h+j,I420+w*h+i,1);//u分量
        memcpy(NV12+w*h+j+1,I420+w*h+i+w*h/4,1);//v分量
    }
}

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		AV_CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame){
			ret=0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}


void hw_encode(ADDR_TASK_PAIR task_pair)
{   
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    while (1)
    {
        //原地址
        std::string inUrl = task_pair.rtsp;
        const char *out_filename = task_pair.rtmp.c_str();
        std::cout <<"========:"<<inUrl<<std::endl;
        std::cout <<"========:"<<out_filename<<std::endl;
        std::vector<int> taskList = task_pair.taskList;
        /// 1 使用opencv打开源视频流
        cv::VideoCapture cap = cv::VideoCapture(inUrl);
        if (!cap.isOpened())
        {
            sleep(addr_timeout);
            continue;
        }
        std::cout <<" cap open success" << std::endl;
        //获取视频帧的尺寸
        int inWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int inHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        int fps = cap.get(cv::CAP_PROP_FPS);

        int ret;
        int i_frame = 0;          
        AVCodec *codec;
        AVCodecContext *pCodecCtx;
        AVFrame *frame_mem_gpu;
        AVFrame *frame_mem;
        AVPacket pkt;
        AVOutputFormat* fmt;
        AVStream* video_st;
        AVFormatContext *pFormatCtx;
        AVBufferRef *device_ctx_ref;
        AVDictionary *options = NULL;
                
        /* Open a device and create an AVHWDeviceContext */ //"/dev/dri/renderD128"
        ret = av_hwdevice_ctx_create(&device_ctx_ref, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
        if (ret < 0) {
            printf("Failed to create a VAAPI device\n");
            sleep(addr_timeout);
            continue;
        }

        pFormatCtx = avformat_alloc_context();
        avformat_alloc_output_context2(&pFormatCtx, NULL, "flv", out_filename);
        fmt = pFormatCtx->oformat;
        if (avio_open(&pFormatCtx->pb, out_filename, AVIO_FLAG_READ_WRITE) < 0){
            sleep(addr_timeout);
            continue;
        }    
        video_st = avformat_new_stream(pFormatCtx, 0);
        if (video_st==NULL){
            sleep(addr_timeout);
            continue;
        }
        video_st->codecpar->codec_tag = 0;
        
        codec = avcodec_find_encoder_by_name("h264_vaapi");
        if (!codec) {
            printf ("Codec not found.\n");
            sleep(addr_timeout);
            continue;
        }          
        pCodecCtx = avcodec_alloc_context3(codec);
        if (!pCodecCtx) {
            printf( "Could not allocate video codec context.\n");
            sleep(addr_timeout);
            continue;
        }
        pCodecCtx = video_st->codec;
        pCodecCtx->width        = inWidth;
        pCodecCtx->height       = inHeight;
        pCodecCtx->pix_fmt      = AV_PIX_FMT_VAAPI;
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        pCodecCtx->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小 50kB
        pCodecCtx->time_base.num = 1;  
	    pCodecCtx->time_base.den = fps; 
        pCodecCtx->gop_size = 250;
        pCodecCtx->max_b_frames = 0;
        pCodecCtx->qmin  = 30;
        pCodecCtx->qmax  = 40;
        
        if ((ret = set_hwframe_ctx(pCodecCtx, device_ctx_ref)) < 0) {
            printf("Failed to set hwframe context.\n");
            sleep(addr_timeout);
            continue;
        }
        if(pCodecCtx->codec_id == AV_CODEC_ID_H264) {
            //av_dict_set(&options, "preset", "fast", 0);
            av_dict_set(&options, "tune", "zerolatency", 0);
        }
        av_dump_format(pFormatCtx, 0, out_filename, 1); //打印关于输入或输出格式的详细信息
        if (avcodec_open2(pCodecCtx, codec, &options) < 0) {
            printf ("Could not open codec.\n");
            sleep(addr_timeout);
            continue;
        }

        /* Allocate an AVFrame on memory and set the params */
        frame_mem = av_frame_alloc();
        if(!frame_mem){
            printf ("Could not allocate AVFrame\n");
            sleep(addr_timeout);
            continue;
        }
        frame_mem->width  = pCodecCtx->width;
        frame_mem->height = pCodecCtx->height;
        frame_mem->format = AV_PIX_FMT_NV12;
        ret = av_frame_get_buffer(frame_mem, 0);
        if (ret < 0) {
            printf("Could not allocate AVFrame buffer on memory.\n");
            sleep(addr_timeout);
            continue;
        }
        
        /* Allocate an AVFrame on GPU memory and set the params */
        frame_mem_gpu = av_frame_alloc();
        if (!frame_mem_gpu) {
            printf ("Could not allocate GPU AVFrame.\n");
            sleep(addr_timeout);
            continue;
        }
        frame_mem_gpu->format = pCodecCtx->pix_fmt;
        frame_mem_gpu->width  = pCodecCtx->width;
        frame_mem_gpu->height = pCodecCtx->height;
        ret = av_hwframe_get_buffer(pCodecCtx->hw_frames_ctx, frame_mem_gpu, 0 );
        if (ret < 0) {
            printf("Could not allocate AVFrame buffer on GPU memory.\n");
            sleep(addr_timeout);
            continue;
        }
        if (!frame_mem_gpu->hw_frames_ctx) {
            printf("hw_frames_ctx error.\n");
            sleep(addr_timeout);
            continue;
        }
        //写入封装头
        ret = avformat_write_header(pFormatCtx, NULL);
        if (ret != 0)
        {
            printf("avformat_write_header error\n");
            sleep(addr_timeout);
            continue;
        }
             
        uint8_t* picture_buf;
        int picture_size;
        AVFrame *frame_yuv;
        frame_yuv = av_frame_alloc();
        picture_size = avpicture_get_size(AV_PIX_FMT_YUV420P, inWidth, inHeight);
        picture_buf = (uint8_t *)av_malloc(picture_size);
        avpicture_fill((AVPicture *)frame_yuv, picture_buf, AV_PIX_FMT_YUV420P, inWidth, inHeight);
        int cout_timeout = 0;
        int num = 0;
        
        av_new_packet(&pkt,picture_size);
        std::cout<<"=======start========="<<std::endl;
        while(1)
        {
            cv::Mat frame;
            cap.read(frame);
            if (frame.empty())
            {
                if(cout_timeout >= 5)
                {
                    printf("restart addr start\n");
                    cap.release();
                    cap = cv::VideoCapture(inUrl);
                    if (!cap.isOpened())
                    {
                        printf("restart addr failure\n");
                        break;
                    }
                    cout_timeout = 0;
                    printf("restart addr end\n");
                }else{
                    cout_timeout ++;
                }             
                continue;
            }

            head_lock.lock();
            if(!head_queue.empty())
            {
                std::string jsondata = head_queue.front();      
                head_queue.pop();
                head_lock.unlock();              
                draw_img(frame, jsondata, inUrl, taskList); //check rstp+task;  
                if(num == 10000){
                    num == 0;
                }                    
                num ++;
                //printf("num:%d\n",num);
            }else{
                head_lock.unlock();
            }
            
            cv::Mat yuvImg;
            cv::cvtColor(frame, yuvImg, CV_BGR2YUV_I420);
            memcpy(picture_buf, yuvImg.data, picture_size*sizeof(unsigned char));
            yuvI420ToNV12((char*)picture_buf, inWidth, inHeight, (char*)frame_mem->data[0]);
            frame_mem_gpu->pts = i_frame;
            //frame_mem_gpu->pts = i_frame*(video_st->time_base.den)/((video_st->time_base.num)*fps);

            ret = av_hwframe_transfer_data( frame_mem_gpu, frame_mem, 0 );
            if(ret < 0){
                printf("Copy data to a hw surface failure\n");
                break;
            }
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;     
            /*Supply a raw video or audio frame to the encoder.*/
            ret = avcodec_send_frame(pCodecCtx, frame_mem_gpu);
            if (ret < 0){
                printf("avcodec_send_frame failure\n");
                break;
            }
            int got_picture = 0;
            while(1)
            {
                ret = avcodec_receive_packet(pCodecCtx, &pkt);
                if (ret == AVERROR(EAGAIN)){
                    break;
                }          
                if (ret < 0){
                    break;
                }
                //printf("Write frame_mem_gpu %3d (size=%5d)\n", i_frame, pkt.size);
                int ret = avcodec_encode_video2(pCodecCtx, &pkt, frame_mem_gpu, &got_picture);
                if(ret < 0){
                    break;
                }
                if (got_picture==1)
                {                  
                    i_frame++;
                    pkt.stream_index = video_st->index;
                    printf("Succeed to encode frame start: %5d\tsize:%5d\n",i_frame,pkt.size);
                    ret = av_write_frame(pFormatCtx, &pkt);
                    printf("Succeed to encode frame end\n");    
                    //av_free_packet(&pkt);                               
                }              
            }
            av_packet_unref(&pkt);  
        }
       
        cap.release();
        ret = flush_encoder(pFormatCtx, 0);
        if (ret < 0) {
            printf("Flushing encoder failed\n");
            return;
        }   
        //Write file trailer
        av_write_trailer(pFormatCtx);   
        //Clean      
        av_free(picture_buf);
        av_frame_free(&frame_yuv);
        av_frame_free(&frame_mem_gpu);
        av_freep(&frame_mem->data[0]);
        av_frame_free(&frame_mem);
        avcodec_close(video_st->codec);
        avcodec_close(pCodecCtx);
        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);

        std::cout << "=======error=====" << std::endl;
        sleep(addr_timeout);
        continue;
        
    }

    return;
}

void sw_encode(ADDR_TASK_PAIR task_pair)
{   
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    while (1)
    {
        //原地址
        std::string inUrl = task_pair.rtsp;
        const char *out_filename = task_pair.rtmp.c_str();
        std::cout <<"========:"<<inUrl<<std::endl;
        std::cout <<"========:"<<out_filename<<std::endl;
        std::vector<int> taskList = task_pair.taskList;
        /// 1 使用opencv打开源视频流
        cv::VideoCapture cap = cv::VideoCapture(inUrl);
        if (!cap.isOpened())
        {
            sleep(addr_timeout);
            continue;
        }
        std::cout <<" cap open success" << std::endl;
        //获取视频帧的尺寸
        int inWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int inHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        int fps = cap.get(cv::CAP_PROP_FPS);
        
        AVFormatContext* pFormatCtx;
        AVOutputFormat* fmt;
        AVStream* video_st;
        AVCodecContext* pCodecCtx;
        AVCodec* pCodec;
        AVPacket pkt;
        uint8_t* picture_buf;
        AVFrame* pFrame;
        int picture_size;
        AVDictionary *options = NULL;
        int ret = 0;
    
        pFormatCtx = avformat_alloc_context();
        avformat_alloc_output_context2(&pFormatCtx, NULL, "flv", out_filename);
        fmt = pFormatCtx->oformat;
        if (avio_open(&pFormatCtx->pb, out_filename, AVIO_FLAG_READ_WRITE) < 0){
            sleep(addr_timeout);
            continue;
        }    
        video_st = avformat_new_stream(pFormatCtx, 0);
        if (video_st==NULL){
            sleep(addr_timeout);
            continue;
        }
        video_st->codecpar->codec_tag = 0;
        pCodecCtx = video_st->codec;
      
        //配置编码器参数
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //全局参数,4194304      
        pCodecCtx->codec_id = AV_CODEC_ID_H264;
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        pCodecCtx->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小 50kB
        pCodecCtx->width = inWidth;
        pCodecCtx->height = inHeight;
        pCodecCtx->time_base.num = 1;  
	    pCodecCtx->time_base.den = 25; 
        pCodecCtx->gop_size = 250;
        pCodecCtx->max_b_frames = 0;
        pCodecCtx->qmin  = 30;
        pCodecCtx->qmax  = 40;
 
        if(pCodecCtx->codec_id == AV_CODEC_ID_H264) {
            //av_dict_set(&options, "preset", "fast", 0);
            av_dict_set(&options, "tune", "zerolatency", 0);
        }
        av_dump_format(pFormatCtx, 0, out_filename, 1);
        pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
        if (!pCodec){
            printf("avcodec_find_encoder error\n");
            sleep(addr_timeout);
            continue;
        }
        //d 打开编码器上下文
        ret = avcodec_open2(pCodecCtx, pCodec, &options);
        if (ret != 0)
        {
            printf("avcodec_open2 error\n");
            sleep(addr_timeout);
            continue;
        }
        
        pFrame = av_frame_alloc();
        picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
        picture_buf = (uint8_t *)av_malloc(picture_size);
        avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
        
        //写入封装头
        ret = avformat_write_header(pFormatCtx, NULL);
        if (ret != 0)
        {
            printf("avformat_write_header error\n");
            sleep(addr_timeout);
            continue;
        }
        av_new_packet(&pkt,picture_size);
        int y_size = pCodecCtx->width * pCodecCtx->height;
        int framecnt = 0;
        int num = 0;
        int cout_timeout = 0;
        auto time_m = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        auto time_start = time_m/1000;
        while(1)
        {
            usleep(0.001);
            auto time_b = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            auto time_breathe = time_b/1000;
            if((time_breathe-time_start) >= (breathe_time_step*60)){
                time_start = time_breathe;
                recording_breathe_log("",-1);         
            }
      
            cv::Mat frame;
            cap.read(frame);
            if (frame.empty())
            {
                if(cout_timeout >= 5)
                {
                    printf("restart addr start\n");
                    cap.release();
                    cap = cv::VideoCapture(inUrl);
                    if (!cap.isOpened())
                    {
                        printf("restart addr failure\n");
                        break;
                    }
                    cout_timeout = 0;
                    printf("restart addr end\n");
                }else{
                    cout_timeout ++;
                }             
                continue;
            }

            head_lock.lock();
            if(!head_queue.empty())
            {
                std::string jsondata = head_queue.front();      
                head_queue.pop();
                head_lock.unlock();              
                draw_img(frame, jsondata, inUrl, taskList); //check rstp+task;  
                if(num == 10000){
                    num == 0;
                }                    
                num ++;
                //printf("num:%d\n",num);
            }else{
                head_lock.unlock();
            }
            cv::Mat yuvImg;
            cv::cvtColor(frame, yuvImg, CV_BGR2YUV_I420);
            memcpy(picture_buf, yuvImg.data, picture_size*sizeof(unsigned char));
            pFrame->data[0] = picture_buf;              // Y
            pFrame->data[1] = picture_buf+ y_size;      // U 
            pFrame->data[2] = picture_buf+ y_size*5/4;  // V

            pFrame->pts = framecnt*(video_st->time_base.den)/((video_st->time_base.num)*fps);
            //auto time_now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            int got_picture=0;
            //Encode
            int ret = avcodec_encode_video2(pCodecCtx, &pkt,pFrame, &got_picture);
            if(ret < 0){
                break;
            }
            if (got_picture==1)
            {
                printf("Succeed to encode frame start: %5d\tsize:%5d\n",framecnt,pkt.size);
                framecnt++;
                pkt.stream_index = video_st->index;
                ret = av_write_frame(pFormatCtx, &pkt);
                av_free_packet(&pkt);      
                // auto time_now2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                // std::cout <<"time:"<<time_now2 - time_now1<<std::endl;
                printf("Succeed to encode frame end\n");           
            }
            // addr_mtx_map[inUrl]->lock();
            // if(!addr_queue_map[inUrl].empty())
            // {
            //     cv::Mat frame = addr_queue_map[inUrl].front();
            //     addr_queue_map[inUrl].pop();
            //     addr_mtx_map[inUrl]->unlock();
                
            // }else{
            //     addr_mtx_map[inUrl]->unlock();
            // }
            
        }
        cap.release();
        ret = flush_encoder(pFormatCtx, 0);
        if (ret < 0) {
            printf("Flushing encoder failed\n");
            return;
        }
    
        //Write file trailer
        av_write_trailer(pFormatCtx);
        //Clean
        av_free(picture_buf);
        av_frame_free(&pFrame);
        avcodec_close(video_st->codec);
        avcodec_close(pCodecCtx);
        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);

        std::cout << "=======error=====" << std::endl;
        sleep(addr_timeout);
        continue;
        
    }

    return;
}