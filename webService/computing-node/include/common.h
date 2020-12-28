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
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include "json.h"


struct Ip_Info
{
    int task_key;          //任务计数
    std::vector<int> tasks; //任务列表
    std::vector<std::vector<float>> rois;  
};


//导出部分配置参数
extern int queue_len;
extern int breathe_time_step;
extern int tcp_timeout;
extern int cn_port;
extern std::string sg_dir;
extern std::map<std::string, int> decoding_ip_port_map;
extern std::map<int, std::vector<float>> tasks_info;
extern std::map<std::string, struct Ip_Info> ip_info_map;
extern std::mutex ip_info_map_mtx;


std::string get_localtime();         //获取格式化后的UTC时间，格式：%Y-%m-%dT%H:%M:%S.MS
void generate_start_flag();   //生成启动异常文件
void recording_breathe_log(std::string ip, int port);   //记录心跳日志
int load_computing_config();  //加载计算节点初始化配置文件

#endif