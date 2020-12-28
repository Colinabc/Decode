#ifndef PUBLICHEADER_H
#define PUBLICHEADER_H

#include <QMetaType>
#include <QVector>
#include <QString>
#include <QDir>

// opencv use
#include "opencv2/opencv.hpp"
#include<queue>
#include<mutex>
#include<QTimer>
#include "scheduler.hh"
#define FRAME_Num 30

extern bool ImgSave_Flag;

typedef enum task_type{
    NO_TASK_TYPE = 0,
    SAFETY_HELMET_DETECTION_TYPE,
    WORK_REDCLOTHING_DETECTION_TYPE,
    WORK_BLUECLOTHING_DETECTION_TYPE,
    REFLECT_VEST_DETECTION_TYPE,
    PERSONNEL_INTRUSION_DETECTION_TYPE,
    SKIN_EXPOSURE_DETECTION_TYPE,
    SMOKE_DETECTION_TYPE,
    PERSONSMOKE_DETECTION_TYPE,
    PERSONPHONE_DERECTION_TYPE,
    OILLEAK_DETECTION_IN_TYPE,
    OILLEAK_DETECTION_OUT_TYPE,
    PERSONFALL_DETECTION_TYPE
}TASK_TYPE;

typedef enum input_data_source_type{
    NO_INPUT_DATA_SOURCE_TYPE = 0,
    LOCAL_VIDEO_TYPE,
    IP_CAMERA_TYPE,
    LOCAL_PICTURES_TYPE,
}INPUT_DATA_SOURCE_TYPE;

typedef struct input_data_source_info{
   QString ipCameraUrl;
   QString localVideoName;
   QString localPicturesPath;
}INPUT_DATA_SOURCE_INFO;


typedef struct frame_info
{
    unsigned int frame_id;
    cv::Mat frame_img;

}FRAME_INFO;

typedef struct global_args
{
    int refresh_index=0;
    int last_index=0;
    int start_flag=0;
    cv::Mat frames_input[FRAME_Num];
    bool push_fetch_flag = true;
    bool decode_flag = true;

    /*
    shared_ptr<libadapter::CourierClothing> ch;
    shared_ptr<libadapter::CourierClothing> cc_r;
    shared_ptr<libadapter::CourierClothing> cc_b;
    shared_ptr<libadapter::CourierClothing> cv;
    shared_ptr<libadapter::CourierClothing> ci;
    shared_ptr<libadapter::CourierSkin> cs;
    shared_ptr<libadapter::CourierFire> cf;
    shared_ptr<libadapter::CourierPhone>cp;//phone
    shared_ptr<libadapter::CourierPhone>cm;//smoke
    shared_ptr<libadapter::CourierOil>co_in;//oil_in
    shared_ptr<libadapter::CourierOil>co_out;//oil_out
    */

    std::queue<FRAME_INFO>* sendQueue;
    std::queue<libadapter::FetchedPackage>* resultQueue;
    std::queue<libadapter::PushedPackage>* pushQueue;

    std::mutex detection_locker;
    std::mutex push_locker;
    std::mutex result_locker;
    std::mutex flag_locker;
    std::mutex index_locker;

} GLOBAL_ARGS;

QString mkMutiDir(const QString path);

#endif // PUBLICHEADER_H
