#ifndef ANJIAN_H
#define ANJIAN_H

#include <unistd.h>
#include <sys/time.h> 
#include <string.h>
#include <string>
#include <thread>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <chrono>
#include <random>
#include <queue>
#include <mutex>
#include <map>
#include <vector>
#include <time.h>
#include <iostream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include "json.h"
#include "image_loader.hh"
#include "adapter/adpt_utils.hh"
#include "scheduler.hh"
#include "tcp_service.h"
#include "tcp_client.h"
#include "common.h"


void run();  //初始化算法


#endif