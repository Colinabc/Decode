#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileInfoList>
#include <QDir>
#include <QStringList>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include<QProgressBar>
#include<QScreen>
#include<QDesktopWidget>
#include<QTimer>
#include "generalsecuritymonitor.h"
#include "cameradecode.h"
#include "publicheader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void sendImage(QImage);
    void finshPoints(bool);

private slots:
    void on_DisplayVideoButton_clicked();

    void on_DisplayCameraButton_clicked();

    void on_DisplayLocalTestButton_clicked();

    void on_DisplayLocalNextButton_clicked();

    void DisplayLocalNextButton_enabel();

    void on_StartRun_clicked();

    void on_StopRun_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_startGetPoint_clicked();
    void on_stopGetPoint_clicked();
    void on_clearROI_clicked();

    void on_resetCard_clicked();
    void onStateChanged(int state);


private:
    Ui::MainWindow *ui;

    INPUT_DATA_SOURCE_TYPE inputDataSourceType;
    INPUT_DATA_SOURCE_INFO inputDataSourceInfo;
    TASK_TYPE   taskType;

    CameraDecode *pCameraDecode = NULL;

    QStringList inputLocalPictureList;

    GeneralSecurityMonitor *pGeneralSecurityMonitor = NULL;
    QString text = "rtsp://admin:letmein1@192.168.0.240:554/h264/ch36/main/av_stream";
    GLOBAL_ARGS* globalArgs = NULL;
    bool status;
};

#endif // MAINWINDOW_H
