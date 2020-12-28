#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->retSave,SIGNAL(stateChanged(int)),this, SLOT(onStateChanged(int)));
    ui->taskBar->setOrientation(Qt::Horizontal);  // 水平方向
    ui->taskBar->setMinimum(0);  // 最小值
    ui->taskBar->setMaximum(100);  // 最大值

    ImgSave_Flag = false;
    this->taskType = NO_TASK_TYPE;
    this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
    this->inputDataSourceInfo.ipCameraUrl.clear();
    this->inputDataSourceInfo.localPicturesPath.clear();
    this->inputDataSourceInfo.localVideoName.clear();

}

MainWindow::~MainWindow()
{
    qDebug()<< __FUNCTION__ << " : "<< __LINE__ ;

    if(this->pCameraDecode != NULL)
    {
        this->pCameraDecode->stop();
        delete this->pCameraDecode;
        this->pCameraDecode = NULL;
    }

    qDebug()<< __FUNCTION__ << " : "<< __LINE__ ;
    if(this->pGeneralSecurityMonitor != NULL)
    {
        this->pGeneralSecurityMonitor->stop();
        delete this->pGeneralSecurityMonitor;
        this->pGeneralSecurityMonitor = NULL;
    }
    if(this->globalArgs != NULL)
    {
      delete this->globalArgs->sendQueue;
      delete this->globalArgs->pushQueue;
      delete this->globalArgs->resultQueue;
      delete this->globalArgs;
    }
    delete ui;
}

void MainWindow::onStateChanged(int state)
{
    if (state == Qt::Checked) // "选中"
    {
        ImgSave_Flag = true;
        std::cout<<"true"<<std::endl;
    }
    else if(state == Qt::PartiallyChecked) // "半选"
    {
        ImgSave_Flag = true;
        std::cout<<"半选"<<std::endl;
    }
    else // 未选中 - Qt::Unchecked
    {
        ImgSave_Flag = false;
        std::cout<<"false"<<std::endl;
    }
}

void MainWindow::on_DisplayVideoButton_clicked()
{
    qDebug() << __FUNCTION__ << ": " << __LINE__ ;

    // file choice
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开视频文件"),
                                                    QDir::currentPath(),
                                                    tr("视频文件(*.avi *.mp4 *.wmv);;所有文件(*.*)"));
    this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;

    if (fileName.isEmpty())
    {
        QMessageBox::warning(this, "警告!", "视频文件打开失败!");
    }
    else
    {
        QMessageBox::information(this, "消息!", "视频文件打开成功!");

        this->inputDataSourceType = LOCAL_VIDEO_TYPE;
        this->inputDataSourceInfo.localVideoName = fileName;

        // ui disable button
        //ui->DisplayVideoButton->setDisabled(true);
        //ui->DisplayCameraButton->setDisabled(true);
        //ui->DisplayLocalTestButton->setDisabled(true);
        //ui->DisplayLocalNextButton->setDisabled(true);
    }
}

void MainWindow::on_DisplayCameraButton_clicked()
{
    qDebug() << __FUNCTION__ << ": " << __LINE__ ;
   this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
    bool ok;
    //QString defaultText = "rtsp://admin:letmein1@192.168.0.240:554/h264/ch36/main/av_stream";
    QString url = QInputDialog::getText(this,
                                        tr("摄像头 RTSP "),
                                        tr("请输入摄像头 RTSP 地址"),
                                        QLineEdit::Normal,
                                        this->text,
                                        &ok);
    this->text = url;
    if(ok && !url.isEmpty())
    {
        this->inputDataSourceType = IP_CAMERA_TYPE;
        this->inputDataSourceInfo.ipCameraUrl = url;

        // ui disable button
       // ui->DisplayVideoButton->setDisabled(true);
       // ui->DisplayCameraButton->setDisabled(true);
       // ui->DisplayLocalTestButton->setDisabled(true);
       // ui->DisplayLocalNextButton->setDisabled(true);

      //  ui->DisplayVideoButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
      //  ui->DisplayCameraButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
      //  ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
      //  ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
    }
    else
    {
        QMessageBox::warning(this, "警告!", "输入的RTSP地址有问题!");
    }

}

void MainWindow::on_DisplayLocalTestButton_clicked()
{
    qDebug() << __FUNCTION__ << ": " << __LINE__ ;
    this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;

    QString input_images_path = QFileDialog::getExistingDirectory(this, "选择目录", QDir::currentPath(), QFileDialog::ShowDirsOnly |QFileDialog::ReadOnly);

    if(input_images_path.compare("") != 0 && !input_images_path.isEmpty())
    {
        this->inputDataSourceType = LOCAL_PICTURES_TYPE;
        this->inputDataSourceInfo.localPicturesPath = input_images_path;

        QMessageBox::information(this, "消息!", "图片路径打开成功!");

        // ui disable button
        //ui->DisplayVideoButton->setDisabled(true);
        //ui->DisplayCameraButton->setDisabled(true);
        //ui->DisplayLocalTestButton->setDisabled(true);
        //ui->DisplayLocalNextButton->setDisabled(true);

        //ui->DisplayVideoButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
       // ui->DisplayCameraButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
       // ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
       // ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");

    }
    else
    {
        QMessageBox::warning(this, "警告!", "图片路径打开失败!");
    }

}

void MainWindow::on_DisplayLocalNextButton_clicked()
{
    qDebug() << __FUNCTION__ << ": " << __LINE__ ;

    if(this->inputLocalPictureList.size() > 0)
    {
        QString image = this->inputLocalPictureList.front();

        cv::Mat tmpImg = cv::imread(image.toStdString().c_str());

        //QImage rgbImg = QImage(tmpImg.data, tmpImg.cols, tmpImg.rows, tmpImg.step, QImage::Format_RGB888).rgbSwapped().copy();

        this->inputLocalPictureList.pop_front();

        if(this->globalArgs->start_flag == 0)
        {
            for(int i=0;i < FRAME_Num;i++)
            {
               this->globalArgs->frames_input[i] = cv::Mat::zeros(tmpImg.rows,tmpImg.cols,CV_8UC3);
            }
            this->globalArgs->start_flag = 1;
        }

        if(this->globalArgs->refresh_index == FRAME_Num)
        {
            this->globalArgs->refresh_index = 0;
        }

        if(this->globalArgs->sendQueue->size() < 2)
        {
           FRAME_INFO frameInfo;
           frameInfo.frame_img = this->globalArgs->frames_input[ this->globalArgs->refresh_index ].clone();
           frameInfo.frame_id = this->globalArgs->refresh_index;

           this->globalArgs->detection_locker.lock();
           this->globalArgs->sendQueue->push(frameInfo);
           this->globalArgs->detection_locker.unlock();

           qDebug()<<__FUNCTION__<<" "<<__LINE__;
        }

        this->globalArgs->index_locker.lock();
        this->globalArgs->frames_input[this->globalArgs->refresh_index] = tmpImg.clone();
        this->globalArgs->refresh_index++;
        this->globalArgs->index_locker.unlock();

        //emit sendImage(rgbImg);

        ui->DisplayLocalNextButton->setDisabled(true);
        ui->DisplayLocalNextButton->setEnabled(true);

        ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    }
    else
    {
        // QMessageBox::warning(this, "警告!", "缓存暂无可用图片!");
        // ui->DisplayLocalTestButton->setEnabled(true);

        // get images list
        QDir fileDir(this->inputDataSourceInfo.localPicturesPath);

        QStringList filter;
        filter << "*.jpg";

        fileDir.setNameFilters(filter);

        QFileInfoList imageInfoList = fileDir.entryInfoList(filter);

        for(int inf = 0; inf < imageInfoList.count(); inf ++)
        {
            this->inputLocalPictureList << imageInfoList.at(inf).filePath();
        }
    }
}

void MainWindow::DisplayLocalNextButton_enabel()
{
    qDebug() << __FUNCTION__ << ": " << __LINE__ ;

    ui->DisplayLocalNextButton->setEnabled(true);
}

void MainWindow::on_startGetPoint_clicked()
{
 // CameraDecode tmpStausCheck;
  //CAMERASTATE status = tmpStausCheck.checkCameraState(this->inputDataSourceInfo.ipCameraUrl);
 // if(status == NOT_CONNECTABLE_STATE)
  //{
  //   QMessageBox::critical(NULL,"Info","camera disconnect!");
 // }
 // else
  {
      ui->ResultLabel->setEnablePointsGet();
      ui->startGetPoint->setDisabled(true);
      ui->stopGetPoint->setEnabled(true);
      ui->clearROI->setEnabled(true);
      //ui->startGetPoint->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
      //ui->stopGetPoint->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
  }
}

void MainWindow::on_stopGetPoint_clicked()
{
    if(ui->ResultLabel->setDisablePointGet())
    {
        ui->startGetPoint->setEnabled(true);
        ui->stopGetPoint->setDisabled(true);
        ui->clearROI->setEnabled(true);
        //ui->startGetPoint->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
        //ui->stopGetPoint->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
    }
}

void MainWindow::on_clearROI_clicked()
{
    ui->ResultLabel->clearROI();
    ui->startGetPoint->setEnabled(true);
    ui->stopGetPoint->setEnabled(true);
    ui->clearROI->setDisabled(true);
    emit finshPoints(true);
}



void MainWindow::on_StartRun_clicked()
{
    if(this->pGeneralSecurityMonitor == NULL)
    {
        QString fileName =QString::fromStdString("../GeneralSecurityMonitoring_UI/config");

        if (fileName.isEmpty())
        {
            QMessageBox::warning(this, "警告!", "配置文件打开失败!");
        }
        else
        {
           // QMessageBox::information(this, "消息!", "配置文件打开成功!");

            this->globalArgs = new GLOBAL_ARGS();
            this->globalArgs->sendQueue = new std::queue<FRAME_INFO>();
            this->globalArgs->resultQueue = new std::queue<libadapter::FetchedPackage>();
            this->globalArgs->pushQueue = new std::queue<libadapter::PushedPackage>();

            this->pGeneralSecurityMonitor = new GeneralSecurityMonitor(fileName,this->globalArgs);
            //sleep(5);
            connect(ui->ResultLabel,SIGNAL(getPointMapSignal(QVector< QMap<QString,QPoint>>)),this->pGeneralSecurityMonitor,SLOT(receiPoints(QVector< QMap<QString,QPoint> >)) );
            connect(this,SIGNAL(finshPoints(bool)),this->pGeneralSecurityMonitor,SLOT(receiFinishPoints(bool)) );
            this->pGeneralSecurityMonitor->setCurrentTask(this->taskType);
            ui->taskBar->setValue(50);
            this->status = this->pGeneralSecurityMonitor->enroll();
            std::cout<<"status: "<<status<<std::endl;

            if(!status)
            {
                if(this->pGeneralSecurityMonitor !=NULL)
                {
                    std::cout<<"111"<<std::endl;
                    this->pGeneralSecurityMonitor->stop();
                    delete this->pGeneralSecurityMonitor;
                    this->pGeneralSecurityMonitor =NULL;
                }
                if( this->globalArgs != NULL )
                {
                    this->globalArgs->sendQueue = NULL;
                    this->globalArgs->resultQueue = NULL;
                    this->globalArgs->pushQueue = NULL;
                    this->globalArgs = NULL;
                }
                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("模型加载超时,请退出程序,重置板卡!");
                QMessageBox::warning(this,title,info,
                        QMessageBox::Yes);
                return;
            }
            else
            {
                ui->taskBar->setValue(100);
                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("模型加载完成!");
                QMessageBox::information(this,title,info,
                        QMessageBox::Yes);
                this->pGeneralSecurityMonitor->start();
            }
        }
    }

    if(this->taskType != NO_TASK_TYPE)
    {
        switch(this->inputDataSourceType)
        {
        case LOCAL_VIDEO_TYPE:
        {
            // get local video image (decode)
            this->pCameraDecode = new CameraDecode(this->inputDataSourceInfo.localVideoName, SOFTWARE_LOOP_DECODE,this->globalArgs);
            // display result image
            connect(this->pGeneralSecurityMonitor, SIGNAL(resultToDisplay(QImage)), ui->ResultLabel, SLOT(showImage(QImage)));

            this->pGeneralSecurityMonitor->setCurrentTask(this->taskType);

            this->pCameraDecode->start();

            // disable or enable all button
            ui->StartRun->setDisabled(true);
            ui->resetCard->setDisabled(true);
            ui->DisplayVideoButton->setDisabled(true);
            ui->DisplayCameraButton->setDisabled(true);
            ui->DisplayLocalTestButton->setDisabled(true);
            ui->DisplayLocalNextButton->setDisabled(true);
            ui->StopRun->setEnabled(true);


            ui->StartRun->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->resetCard->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayVideoButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayCameraButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->StopRun->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");

            ui->clearROI->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->startGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->stopGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
        }
            break;
        case IP_CAMERA_TYPE:
        {
            // get ip camera image (decode)
            this->pCameraDecode = new CameraDecode(this->inputDataSourceInfo.ipCameraUrl, SOFTWARE_DECODE,this->globalArgs);

            // display result image
            connect(this->pGeneralSecurityMonitor, SIGNAL(resultToDisplay(QImage)), ui->ResultLabel, SLOT(showImage(QImage)));


            this->pGeneralSecurityMonitor->setCurrentTask(this->taskType);

            this->pCameraDecode->start();

            // disable or enable all button
            ui->StartRun->setDisabled(true);
            ui->resetCard->setDisabled(true);
            ui->DisplayVideoButton->setDisabled(true);
            ui->DisplayCameraButton->setDisabled(true);
            ui->DisplayLocalTestButton->setDisabled(true);
            ui->DisplayLocalNextButton->setDisabled(true);
            ui->StopRun->setEnabled(true);

            ui->StartRun->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->resetCard->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayVideoButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayCameraButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->StopRun->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");

            ui->clearROI->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->startGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->stopGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
        }
            break;
        case LOCAL_PICTURES_TYPE:
        {
            // get images list
            QDir fileDir(this->inputDataSourceInfo.localPicturesPath);

            QStringList filter;
            filter << "*.jpg";

            fileDir.setNameFilters(filter);

            QFileInfoList imageInfoList = fileDir.entryInfoList(filter);

            for(int inf = 0; inf < imageInfoList.count(); inf ++)
            {
                this->inputLocalPictureList << imageInfoList.at(inf).filePath();
            }

            this->pGeneralSecurityMonitor->setCurrentTask(this->taskType);

            // display result image
            connect(this->pGeneralSecurityMonitor, SIGNAL(resultToDisplay(QImage)), ui->ResultLabel, SLOT(showImage(QImage)));

            // disable or enable all button
            ui->StartRun->setDisabled(true);
            ui->resetCard->setDisabled(true);
            ui->DisplayVideoButton->setDisabled(true);
            ui->DisplayCameraButton->setDisabled(true);
            ui->DisplayLocalTestButton->setDisabled(true);
            ui->DisplayLocalNextButton->setEnabled(true);
            ui->StopRun->setEnabled(true);

            ui->StartRun->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->resetCard->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayVideoButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayCameraButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
            ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
            ui->StopRun->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");

            ui->clearROI->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->startGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
            ui->stopGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");

        }
            break;
        case NO_INPUT_DATA_SOURCE_TYPE:
            QMessageBox::warning(this, "警告!", "输入源未选择!");
            break;
        }
        ui->comboBox->setDisabled(true);
    }
    else
    {
        QMessageBox::warning(this, "警告!", "任务类型未选中!");
    }
}

void MainWindow::on_StopRun_clicked()
{
    if(this->taskType != NO_TASK_TYPE)
    {
        switch(this->inputDataSourceType)
        {
        case LOCAL_VIDEO_TYPE:
            if(this->pCameraDecode != NULL)
            {
                this->pCameraDecode->stop();
                delete this->pCameraDecode;
                this->pCameraDecode = NULL;
            }

            if(this->pGeneralSecurityMonitor !=NULL)
            {
                this->pGeneralSecurityMonitor->stop();

                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("成功退出!");
                QMessageBox::information(this,title,info,
                        QMessageBox::Yes);

                delete this->pGeneralSecurityMonitor;
                this->pGeneralSecurityMonitor =NULL;
            }

             if( this->globalArgs != NULL )
             {
                 this->globalArgs->sendQueue = NULL;
                 this->globalArgs->resultQueue = NULL;
                 this->globalArgs->pushQueue = NULL;
                 this->globalArgs = NULL;
             }
            this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
            this->inputDataSourceInfo.localVideoName.clear();
            break;

        case IP_CAMERA_TYPE:
            if(this->pCameraDecode != NULL)
            {
                this->pCameraDecode->stop();
                delete this->pCameraDecode;
                this->pCameraDecode = NULL;
            }

            if(this->pGeneralSecurityMonitor !=NULL)
            {
                this->pGeneralSecurityMonitor->stop();

                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("成功退出!");
                QMessageBox::information(this,title,info,
                        QMessageBox::Yes);

                delete this->pGeneralSecurityMonitor;
                this->pGeneralSecurityMonitor =NULL;
            }
            if( this->globalArgs != NULL )
            {
                this->globalArgs->sendQueue = NULL;

                this->globalArgs->resultQueue = NULL;

                this->globalArgs->pushQueue = NULL;

                this->globalArgs = NULL;
            }
            this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
            this->inputDataSourceInfo.ipCameraUrl.clear();
            break;

        case LOCAL_PICTURES_TYPE:
            this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
            this->inputDataSourceInfo.localPicturesPath.clear();
            this->inputLocalPictureList.clear();
            break;

        case NO_INPUT_DATA_SOURCE_TYPE:
            this->inputDataSourceType = NO_INPUT_DATA_SOURCE_TYPE;
            this->inputDataSourceInfo.ipCameraUrl.clear();
            this->inputDataSourceInfo.localVideoName.clear();
            this->inputDataSourceInfo.localPicturesPath.clear();
            this->inputLocalPictureList.clear();
            break;
        }
        ui->comboBox->setCurrentText("未选择");
        ui->comboBox->setEnabled(true);
    }

    ui->StartRun->setEnabled(true);
    ui->DisplayVideoButton->setEnabled(true);
    ui->DisplayCameraButton->setEnabled(true);
    ui->DisplayLocalTestButton->setEnabled(true);
    ui->DisplayLocalNextButton->setEnabled(true);
    ui->StopRun->setDisabled(true);
    ui->comboBox->setEnabled(true);
    ui->resetCard->setEnabled(true);

    ui->StartRun->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    ui->DisplayVideoButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    ui->DisplayCameraButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    ui->DisplayLocalTestButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    ui->DisplayLocalNextButton->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");
    ui->StopRun->setStyleSheet("background-color: rgb(0, 49, 84);\n\ncolor: rgb(255, 255, 255);\n");
    ui->resetCard->setStyleSheet("background-color: rgb(13, 79, 114);\n\ncolor: rgb(255, 255, 255);\n");

    ui->clearROI->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
    ui->startGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
    ui->stopGetPoint->setStyleSheet("background-color: rgb(13, 95, 110);\n\ncolor: rgb(255, 255, 255);\n");
}

void MainWindow::on_resetCard_clicked()
{

   if( ui->resetCheck->checkState() == Qt::Checked)
   {
    int status = -1;
    status = system("cd ../../device-manager/DeviceManager-2.4.0-Source/build && ./rbdm -r 0");
    if (-1 == status)
    {
        QString title = QStringLiteral("消息通知");
        QString info = QStringLiteral("板卡重置失败!");
        QMessageBox::information(this,title,info,
                QMessageBox::Yes);
    }
    else
    {
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("板卡重置成功!");
                QMessageBox::information(this,title,info,
                        QMessageBox::Yes);
            }
            else
            {
                QString title = QStringLiteral("消息通知");
                QString info = QStringLiteral("板卡重置失败!");
                QMessageBox::information(this,title,info,
                        QMessageBox::Yes);
            }
        }
        else
        {
            //printf("exit status = [%d]\n", WEXITSTATUS(status));
            QString title = QStringLiteral("消息通知");
            QString info = QStringLiteral("板卡重置失败!");
            QMessageBox::information(this,title,info,
                    QMessageBox::Yes);
        }
    }
   }
   else
   {
       QString title = QStringLiteral("消息通知");
       QString info = QStringLiteral("请勾选后重置!");
       QMessageBox::warning(this,title,info,
               QMessageBox::Yes);
   }

}



void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if(QString::compare(arg1, "安全帽检测") == 0)
    {
        this->taskType = SAFETY_HELMET_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "红色工作服检测") == 0)
    {
        this->taskType = WORK_REDCLOTHING_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "蓝色工作服检测") == 0)
    {
        this->taskType = WORK_BLUECLOTHING_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "人员入侵检测") == 0)
    {
        this->taskType = PERSONNEL_INTRUSION_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "皮肤裸露检测") == 0)
    {
        this->taskType = SKIN_EXPOSURE_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "烟火检测") == 0)
    {
        this->taskType = SMOKE_DETECTION_TYPE;
    }
    else if(QString::compare(arg1,"反光衣检测")==0)
    {
        this->taskType = REFLECT_VEST_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "抽烟检测") == 0)
    {
        this->taskType = PERSONSMOKE_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "打电话检测") == 0)
    {
        this->taskType = PERSONPHONE_DERECTION_TYPE;
    }
    else if(QString::compare(arg1, "漏油检测-室外") == 0)
    {
        this->taskType = OILLEAK_DETECTION_OUT_TYPE;
    }
    else if(QString::compare(arg1, "漏油检测-室内") == 0)
    {
        this->taskType = OILLEAK_DETECTION_IN_TYPE;
    }
    else if(QString::compare(arg1, "人员倒地") == 0)
    {
        this->taskType = PERSONFALL_DETECTION_TYPE;
    }
    else if(QString::compare(arg1, "未选择") == 0)
    {
        this->taskType = NO_TASK_TYPE;
    }
    std::cout<< __FUNCTION__<< "  this->taskType "<<this->taskType;
}

