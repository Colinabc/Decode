#ifndef DECODE_DETECTION_H
#define DECODE_DETECTION_H

// Decode use
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
//check dir
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>

//decode
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
}

// opencv use
#include <opencv2/opencv.hpp>

// std time
#include <chrono>
#include<string>
#include<thread>
#include<mutex>

//detection
#include "adapter/adpt_utils.hh"
#include "scheduler.hh"
#include "courier_fire.hh"
#include "courier_skin.hh"
#include "courier_invasion.hh"

using namespace std;
using namespace cv;
using namespace libadapter;

std::mutex mt;	
std::mutex mt_prefix;
//相机状态
typedef enum CameraState
{
    NOT_CONNECTABLE_STATE = 0,
    CONNECTABLE_STATE
}CAMERASTATE;

//解码类型
typedef enum DecodeType
{
    SOFTWARE_DECODE = 0,
    HARDWARE_DECODE,
    SOFTWARE_LOOP_DECODE,
    HARDWARE_LOOP_DECODE
}DECODE_TYPE;


//多个任务检测
enum Task tasks[8] = {Task::PERSON_CLOTHING,
                Task::PERSON_CLOTHING_VEST,
                Task::PERSON_INVASION,
                Task::SKIN_EXPOSURE,
                Task::FIRE_SMOKE,
                Task::SMOKE,
                Task::PHONE,
                Task::LEAK_OIL
};

string prefix[8] = {"uniform-","vest-","invasion-","skin-","fire-","personSmoke-","personPhone-","oilLeak-"};
vector<cv::Scalar> colors = {
	COLOR_BLACK,COLOR_YELLOW,COLOR_PINK,COLOR_RED};

//图像格式转换
int Convert24Image(char *p32Img, char *p24Img, int dwSize32);
CAMERASTATE checkCameraState(string URL,string cameraID);

//输入图片数据
void softwareDecode(Scheduler &s,string URL,string cameraIP);
void softwareLoopDecode(Scheduler &s,string URL,string cameraIP);
void hardwareDecode(Scheduler &s,string URL,string cameraIP);
void hardwareLoopDecode(Scheduler &s,string URL,string cameraIP);

//绘图
void draw_label(cv::Mat& img, const string& label, cv::Point& p1,cv::Point& p2, const cv::Scalar& color);
void draw_person_clothing(cv::Mat& img, FetchedPackage& fp);
void draw_person_invasion(cv::Mat& img, FetchedPackage& fp);
void draw_person_skin(cv::Mat& img, FetchedPackage& fp);
void draw_fire(cv::Mat& img, FetchedPackage& fp);
void draw(cv::Mat& img, FetchedPackage& fp);

#endif // DECODE_DETECTION_H
