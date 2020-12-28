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
#include <map>
#include <mutex>
#include <thread>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "imageswitch.h"
#include "json.h"
#include "common.h"


extern std::queue<std::string> head_queue;
extern std::mutex head_lock;


void tcp_client(std::string ip, int port);
void draw_img(cv::Mat &img, std::string jsondata, std::string rtsp, std::vector<int> taskList);
//void tcp_client_dn(std::string ip, int port);

#endif
