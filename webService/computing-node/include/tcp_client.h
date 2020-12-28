#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <string.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "common.h"
#include "imageswitch.h"

struct Frame_Info   //根据接收的tcp数据头组装的帧信息
{
    cv::Mat frame;
    int fps;
    std::string addr;
    std::string time;
};


void tcp_client(std::string ip, int port);

#endif
