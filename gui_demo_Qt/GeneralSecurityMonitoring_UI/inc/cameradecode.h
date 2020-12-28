#ifndef CAMERADECODE_H
#define CAMERADECODE_H

// qt use
#include <QObject>
#include <QWidget>
#include <QThread>
#include <QString>
#include <QDebug>
#include <QQueue>
#include<QImage>

// Decode use
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
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
#include"publicheader.h"


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


class CameraDecode : public QThread
{
    Q_OBJECT

public:
    explicit CameraDecode(QObject *parent = Q_NULLPTR);
    explicit CameraDecode(QString URL, DECODE_TYPE decodeType = HARDWARE_DECODE, GLOBAL_ARGS* args = NULL, QObject *parent =Q_NULLPTR );
    ~CameraDecode();

    int Convert24Image(char *p32Img, char *p24Img, int dwSize32);

    CAMERASTATE checkCameraState(QString URL);

    void stop();

signals:
    void sendImage(FRAME_INFO frameInfo);

protected:
    void run();

    void softwareDecode();
    void hardwareDecode();

    void softwareLoopDecode();
    void hardwareLoopDecode();

private:
    QString URL;

    DECODE_TYPE decodeType;
    bool stoped;

    GLOBAL_ARGS* globalArgs=NULL;
    FRAME_INFO frameInfo;

};

#endif // CAMERADECODE_H
