#ifndef RUN_H
#define RUN_H

// gflags
#include <gflags/gflags.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
// c++ ifstream
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include<ctime>
// json
#include "json.hpp"
// c signal
#include "ctrl_c.h"
#include "cameraDecode.h"
#include "Imageswitch.h"
#include "common.h"
#include "tcp_service.h"

using namespace std;

//extern int *pconn;
extern std::map<std::string, std::vector<COMPUTNGPARA>> cameraIP_clientIP_map;
extern std::map<std::string, int> clientIP_index_map;            //客户端IP与其对应的index关系映射
extern std::map<std::string, std::queue<FRAMEINFO>> *presultMap; //全局变量
extern std::map<std::string, mutex *> *presultMap_mtx;
extern std::map<std::string, std::queue<FRAMEINFO>> resultMap; //全局变量
extern std::map<std::string, mutex *> resultMap_mtx;

void run(); //初始化算法
void release();
void dn_breathe_log(std::vector<cameraDecode* >& dn_vec);
#endif
