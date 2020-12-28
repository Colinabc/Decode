#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
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


void tcp_service();

#endif