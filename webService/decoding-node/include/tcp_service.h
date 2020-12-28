#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

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
#include "run.h"

int select_socket(int conn);
void tcp_send(int *pconnArray, std::map<std::string, std::queue<FRAMEINFO>> *presultMap, std::map<std::string, std::mutex *> *presultMap_mtx);
void tcp_create(int dn_port, int *pconnArray, std::map<std::string, int> clientIP_index_map);

#endif