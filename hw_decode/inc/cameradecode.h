#ifndef CAMERADECODE_H
#define CAMERADECODE_H


// Decode use
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>

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
#include "opencv2/opencv.hpp"

// std time
#include <chrono>
#include<string>

using namespace std;

typedef enum CameraState
{
    NOT_CONNECTABLE_STATE = 0,
    CONNECTABLE_STATE
}CAMERASTATE;

typedef enum DecodeType
{
    SOFTWARE_DECODE = 0,
    HARDWARE_DECODE,
    SOFTWARE_LOOP_DECODE,
    HARDWARE_LOOP_DECODE
}DECODE_TYPE;

class CameraDecode
{

public:
CameraDecode();
~CameraDecode();

public:

    int Convert24Image(char *p32Img, char *p24Img, int dwSize32);
    CAMERASTATE checkCameraState(string URL,string cameraID);

    void softwareDecode(string URL,string cameraID,bool saveImg);
    void hardwareDecode(string URL,string cameraID,bool savImg);
    void softwareLoopDecode(string URL,string cameraID,bool saveImg);
    void hardwareLoopDecode(string URL,string cameraID,bool saveImg);

    void stop();

private:
    bool stoped;

};

#endif // CAMERADECODE_H
