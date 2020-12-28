#ifndef MAIN_H
#define MAIN_H

#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <random>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <mutex>
#include <map>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "json.h"

typedef struct addr_task_pair
{
   std::string rtsp;
   std::string rtmp;
   std::vector<int>taskList;
} ADDR_TASK_PAIR;

extern std::string tasks_json;
extern std::string color_json;

extern int zone_offset;  //时区偏移，单位（小时）
extern int log_list_len; //心跳日志中端口列表最大长度
extern int queue_len;
extern int breathe_time_step;
extern int addr_timeout;
extern int reconnTime;
extern int en_port;
extern int computing_node_port;
//extern std::string hwType;   //编码类型
extern std::string computing_node;

extern std::vector<ADDR_TASK_PAIR> task_pair_vec;
// extern std::map<std::string, std::queue<cv::Mat>> addr_queue_map;
// extern std::map<std::string, std::mutex*> addr_mtx_map;



int load_encoding_config(std::string& hwType);
void generate_start_flag();
void recording_breathe_log(std::string ip, int port);   //记录心跳日志
int getJson(std::string path,std::string& jsonStr);

#endif