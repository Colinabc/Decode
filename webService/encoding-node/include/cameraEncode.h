#ifndef CAMERAENCODE_H
#define CAMERAENCODE_H


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>
extern "C"
{

#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libavutil/hwcontext.h>
}
#include <stdbool.h>
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include <chrono>
#include <string>
#include <queue>
#include <map>
#include <mutex>
#include <exception>
#include<iostream>
#include "tcp_client.h"
#include "common.h"

void sw_encode(ADDR_TASK_PAIR task_pair);
void hw_encode(ADDR_TASK_PAIR task_pair);

#endif