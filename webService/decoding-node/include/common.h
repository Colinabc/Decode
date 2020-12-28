#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <opencv2/opencv.hpp>
#include <queue>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <stdio.h>
#include <mutex>
#include "json.hpp"

typedef struct computing_para
{
   std::string ip;
   int extraction_rate;
   int count;
} COMPUTNGPARA;

typedef struct decode_para
{
   std::string addr;
   std::string decoding_type;
   std::vector<COMPUTNGPARA> computingNodes_vec;
} DECODE_PARA;

typedef struct frame_info
{
   std::string url;
   cv::Mat frame;
   unsigned int fps;
} FRAMEINFO;

typedef struct message_head
{
   unsigned int image_size;    //图片数据长度 
   unsigned int image_width;   //宽
   unsigned int image_height;  //高
   unsigned int image_channel; //色域通道数
   unsigned int fps;           //视频原始的FPS
   char addr[256];             //视频地址
   char time[256];             //时间戳
} MESSAGEHEAD;

typedef struct breathe_Info
{
   std::string connectionFailure;
   std::string timeStamp;
} BREATHEINFO;

typedef enum cameraStatus
{
   NOT_CONNECTABLE_STATE = 0,
   CONNECTABLE_STATE
} CAMERASTATUS;

extern std::string dn_init_local_path;
extern std::string dn_init_path;
extern std::string dn_start_path;
extern std::string dn_breathe_path;
extern std::string absPath;

extern int breathe_time_step;
extern int reconnTime;
extern int reconnCount;
extern std::string hwType;
extern int dn_port;
extern int connArray[256];
extern std::mutex connArray_mtx;
extern std::vector<std::string> computingNodesIP; //客户端IP列表
extern std::vector<DECODE_PARA> decodeAddrList; //解码列表

extern int zone_offset;  //时区偏移，单位（小时）
extern int log_list_len; //心跳日志中端口列表最大长度
extern int queue_len;

std::string get_localtime();
void generate_start_flag(); //未启动的摄像头列表
int load_decoding_config();

#endif // COMMON_H
