#ifndef CAMERADECODE_H
#define CAMERADECODE_H

//decode use
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
//check dir
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>
extern "C"
{
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

#include "opencv2/opencv.hpp"
#include <thread>
#include <chrono>
#include <string>
#include <queue>
#include <map>
#include <mutex>
#include "common.h"

using namespace std;

class cameraDecode
{
public:
    explicit cameraDecode();
    explicit cameraDecode(std::string hardwareType, std::string addr, std::string decoding_type, int breathe_time, int reconn_time,int reconn_count,std::map<std::string, queue<FRAMEINFO>> *presultMap, std::map<std::string, std::mutex *> *presultMap_mtx, bool imgFlag);
    ~cameraDecode();
    int convert24Image(char *p32Img, char *p24Img, int dwSize32);
    
    //获取当前时间戳
    string get_time_now();
    CAMERASTATUS checkCameraStatus(string addr);
    void stop();
    BREATHEINFO* getbreatheInfo();
    void run();
    void decode_CPU();
    void decode_iGPU();
    void decode_GPU();
    void saveImage(string savePath, string cameraIP, cv::Mat);

private:
    bool imgFlag;
    std::string addr;
    bool stoped;
    std::string hardwareType;
    std::string decoding_type;
    int breathe_time;
    int reconn_time;
    int reconn_count;
    bool cameraFlag;
    BREATHEINFO* breatheInfo;

    std::map<std::string, queue<FRAMEINFO>> *presultMap;
    std::map<std::string, std::mutex *> *presultMap_mtx;
};
#endif // CAMERADECODE_H