#ifndef GENERALSECURITYMONITOR_H
#define GENERALSECURITYMONITOR_H

// qt use
#include <QObject>
#include <QWidget>
#include <QThread>
#include <QString>
#include <QDebug>
#include <QQueue>
#include<QDateTime>
#include<QDir>
#include<QMessageBox>
#include <qtextcodec.h>
// opencv use
#include "opencv2/opencv.hpp"

// public struct
#include "publicheader.h"

// std lib
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include<vector>
#include<queue>

// general security monitor
#include "adapter/adpt_utils.hh"
#include "scheduler.hh"
#include "courier_fire.hh"
#include "courier_skin.hh"
#include "courier_invasion.hh"
#include"timerthread.h"

class GeneralSecurityMonitor : public QThread
{
    Q_OBJECT

public:
     GeneralSecurityMonitor(QObject *parent = Q_NULLPTR);
    //explicit GeneralSecurityMonitor(QString configFile,QString configFile_vest,GLOBAL_ARGS* args, QObject *parent = Q_NULLPTR);
     GeneralSecurityMonitor(QString configFilePath,GLOBAL_ARGS* args, QObject *parent = Q_NULLPTR);
    ~GeneralSecurityMonitor();

    void init();

    void init(QString configFilePath);

    int Convert24Image(char *p32Img, char *p24Img, int dwSize32);

    void setCurrentTask(TASK_TYPE taskType);

    void drawResults();
    void pushImageToDetect();
    void fetchResult();
    void stop();
    bool enroll();

signals:
    void resultToDisplay(QImage);

public slots:
    void receiPoints(QVector<QMap<QString, QPoint> > pointsMap);
    void receiFinishPoints(bool flag);


//public slots:
  //  void pushImageToDetect(QImage image);

protected:
    void run();

private:
    bool stoped;
    TASK_TYPE currentTaskType;
    libadapter::Task currentTask;

    QString configFilePath;

    libadapter::Scheduler *pScdu = NULL;

    GLOBAL_ARGS* globalArgs = NULL;

    string prefix_task[12]={"safetyHelmet","reduniform","blueuniform","vest","invasion","skin","fire","personSmoke","personPhone","oilLeakIn","oilLeakOut","personFall"};

    vector<float>roiPoints;
};

#endif // GENERALSECURITYMONITOR_H
