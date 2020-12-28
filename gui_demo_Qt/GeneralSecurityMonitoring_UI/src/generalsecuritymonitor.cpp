#include "generalsecuritymonitor.h"

GeneralSecurityMonitor::GeneralSecurityMonitor(QObject *parent)
{
    this->stoped = false;
}

GeneralSecurityMonitor::GeneralSecurityMonitor(QString configFilePath,GLOBAL_ARGS* args, QObject *parent)
{
    this->stoped = false;
    this->configFilePath = configFilePath;
    this->globalArgs = args;
    this->init();
}

GeneralSecurityMonitor::~GeneralSecurityMonitor()
{
    this->stop();
}

void GeneralSecurityMonitor::init()
{
    this->init(this->configFilePath);
}
bool GeneralSecurityMonitor::enroll()
{
    std::string  tmp_configFilePath = this->configFilePath.toStdString();;
    libadapter::Task selectTask = this->currentTask;

    //std::string  tmp_configFilePath = configFilePath.toStdString();
    std::string  tmpConfigFile_helmet = tmp_configFilePath+"/caisa_config_helmet.json";
    std::string  tmpConfigFile_redUniform = tmp_configFilePath+"/caisa_config_redUniform.json";
    std::string  tmpConfigFile_blueUniform = tmp_configFilePath+"/caisa_config_blueUniform.json";
    std::string  tmpConfigFile_vest = tmp_configFilePath+"/caisa_config_vest.json";
    std::string  tmpConfigFile_invasion = tmp_configFilePath+"/caisa_config_invasion.json";
    std::string  tmpConfigFile_skin = tmp_configFilePath+"/caisa_config_skin.json";
    std::string  tmpConfigFile_firesmoke = tmp_configFilePath+"/caisa_config_firesmoke.json";
    std::string  tmpConfigFile_smokephone = tmp_configFilePath+"/caisa_config_smokephone.json";
    std::string  tmpConfigFile_oilleak_in = tmp_configFilePath+"/caisa_config_leakoil_indoor.json";
    std::string  tmpConfigFile_oilleak_out = tmp_configFilePath+"/caisa_config_leakoil_outdoor.json";
    std::string  tmpConfigFile_personFall = tmp_configFilePath+"/caisa_config_fall.json";

    switch (selectTask)
    {
        case libadapter::Task::PERSON_HELMET:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_helmet);
            shared_ptr<libadapter::CourierClothing> ch = make_shared<libadapter::CourierClothing>(tmpConfigFile_helmet);
            ch->set_batch_size(4);
            this->pScdu->enroll(ch,libadapter::Task::PERSON_HELMET);
            break;
        }
        case libadapter::Task::PERSON_RED_UNIFORM:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_redUniform);
            shared_ptr<libadapter::CourierClothing> cc_r = make_shared<libadapter::CourierClothing>(tmpConfigFile_redUniform);
            cc_r->set_batch_size(4);
            this->pScdu->enroll(cc_r, libadapter::Task::PERSON_RED_UNIFORM);
            break;
        }
       case libadapter::Task::PERSON_BLUE_UNIFORM:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_blueUniform);
            shared_ptr<libadapter::CourierClothing> cc_b = make_shared<libadapter::CourierClothing>(tmpConfigFile_blueUniform);
            cc_b->set_batch_size(4);
            this->pScdu->enroll(cc_b, libadapter::Task::PERSON_BLUE_UNIFORM);
            break;
        }
       case libadapter::Task::PERSON_VEST:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_vest);
            shared_ptr<libadapter::CourierClothing> cv = make_shared<libadapter::CourierClothing>(tmpConfigFile_vest);
            cv->set_batch_size(4);
            this->pScdu->enroll(cv, libadapter::Task::PERSON_VEST);
            break;
        }
       case libadapter::Task::PERSON_INVASION:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_invasion);
            shared_ptr<libadapter::CourierClothing> ci = make_shared<libadapter::CourierInvasion>(tmpConfigFile_invasion);
            ci->set_batch_size(4);
            this->pScdu->enroll(ci, libadapter::Task::PERSON_INVASION);
            break;
        }
       case libadapter::Task::SKIN_EXPOSURE:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_skin);
            shared_ptr<libadapter::CourierSkin> cs = make_shared<libadapter::CourierSkin>(tmpConfigFile_skin);
            cs->set_batch_size(4);
            this->pScdu->enroll(cs, libadapter::Task::SKIN_EXPOSURE);
            break;
        }
       case libadapter::Task::FIRE_SMOKE:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_firesmoke);
            shared_ptr<libadapter::CourierFire> cf = make_shared<libadapter::CourierFire>(tmpConfigFile_firesmoke);
            cf->set_batch_size(4);
            this->pScdu->enroll(cf, libadapter::Task::FIRE_SMOKE);
            break;
        }
       case libadapter::Task::PHONE:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_smokephone);
            shared_ptr<libadapter::CourierPhone>cp = make_shared<libadapter::CourierPhone>(tmpConfigFile_smokephone);
            cp->set_batch_size(4);
            this->pScdu->enroll(cp, libadapter::Task::PHONE);
            break;
        }
       case libadapter::Task::SMOKE:
        {
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_smokephone);
            shared_ptr<libadapter::CourierPhone>cm = make_shared<libadapter::CourierPhone>(tmpConfigFile_smokephone);
            cm->set_batch_size(4);
            this->pScdu->enroll(cm, libadapter::Task::SMOKE);
            break;
        }
       case libadapter::Task::LEAK_OIL_INDOOR:
        {
            //indoor oil
           this->pScdu = new libadapter::Scheduler(tmpConfigFile_oilleak_in);
           shared_ptr<libadapter::CourierOil>co_in = make_shared<libadapter::CourierOil>(tmpConfigFile_oilleak_in);
           co_in->set_batch_size(4);
           this->pScdu->enroll(co_in, libadapter::Task::LEAK_OIL_INDOOR);
           break;
        }
       case libadapter::Task::LEAK_OIL:
        {
            //Outdoor oil
           this->pScdu = new libadapter::Scheduler(tmpConfigFile_oilleak_out);
           shared_ptr<libadapter::CourierOil>co_out = make_shared<libadapter::CourierOil>(tmpConfigFile_oilleak_out);
           co_out->set_batch_size(4);
           this->pScdu->enroll(co_out, libadapter::Task::LEAK_OIL);
           break;
        }
    case libadapter::Task::FALL_DOWN:
         {
            //person falldown
            this->pScdu = new libadapter::Scheduler(tmpConfigFile_personFall);
            shared_ptr<libadapter::CourierFall>cfall = make_shared<libadapter::CourierFall>(tmpConfigFile_personFall);
            cfall->set_batch_size(4);
            this->pScdu->enroll(cfall, libadapter::Task::FALL_DOWN);
            break;
         }
        default:
            break;
        }
     bool ret = this->pScdu->start_wait_for(20);
     return ret;
}
void GeneralSecurityMonitor::init(QString configFilePath)
{
    if(this->pScdu != NULL)
    {
        this->pScdu->stop();
        delete this->pScdu;
        this->pScdu = NULL;
    }
}

int GeneralSecurityMonitor::Convert24Image(char *p32Img, char *p24Img, int dwSize32)
{
    if(p32Img != NULL && p24Img != NULL && dwSize32>0)
    {
        int dwSize24;
        dwSize24=(dwSize32 * 3)/4;
        char *pTemp,*ptr;
        char* pTemp_r=p32Img;
        char* pTemp_g=p32Img+1;
        char* pTemp_b=p32Img+2;
        char* ptr_r = p24Img;
        char* ptr_g = p24Img + 1;
        char* ptr_b = p24Img + 2;
        int ival=0;
        for (int index = 0; index < dwSize32/4 ; index++)
        {
            memcpy(ptr_r, pTemp_r, 3);
            pTemp_r+=4;
            ptr_r+=3;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

void GeneralSecurityMonitor::setCurrentTask(TASK_TYPE taskType)
{
    this->currentTaskType = taskType;
    switch (taskType)
    {
        case SAFETY_HELMET_DETECTION_TYPE:
            this->currentTask = libadapter::Task::PERSON_HELMET;
            break;
        case WORK_REDCLOTHING_DETECTION_TYPE:
            this->currentTask = libadapter::Task::PERSON_RED_UNIFORM;
            break;
        case WORK_BLUECLOTHING_DETECTION_TYPE:
            this->currentTask = libadapter::Task::PERSON_BLUE_UNIFORM;
            break;
        case PERSONNEL_INTRUSION_DETECTION_TYPE:
            this->currentTask = libadapter::Task::PERSON_INVASION;
            break;
        case SKIN_EXPOSURE_DETECTION_TYPE:
            this->currentTask = libadapter::Task::SKIN_EXPOSURE;
            break;
        case SMOKE_DETECTION_TYPE:
            this->currentTask = libadapter::Task::FIRE_SMOKE;
            break;
        case REFLECT_VEST_DETECTION_TYPE:
            this->currentTask = libadapter::Task::PERSON_VEST;
             break;
        case PERSONPHONE_DERECTION_TYPE:
             this->currentTask = libadapter::Task::PHONE;
             break;
        case PERSONSMOKE_DETECTION_TYPE:
             this->currentTask = libadapter::Task::SMOKE;
             break;
        case OILLEAK_DETECTION_IN_TYPE:
             this->currentTask = libadapter::Task::LEAK_OIL_INDOOR;
             break;
        case OILLEAK_DETECTION_OUT_TYPE:
             this->currentTask = libadapter::Task::LEAK_OIL; //outdoor
             break;
        case PERSONFALL_DETECTION_TYPE:
             this->currentTask = libadapter::Task::FALL_DOWN; //personfall
             break;
        case NO_TASK_TYPE:
             break;
        default:
             break;
    }
}

void GeneralSecurityMonitor::stop()
{
    this->stoped = true;
    if(this->pScdu != NULL)
    {
        this->pScdu->stop();
        msleep(1000);
        delete this->pScdu;
        this->pScdu = NULL;
        std::cout<<"exit scduler"<<std::endl;
    }
}

void GeneralSecurityMonitor::pushImageToDetect()
{
    while( !this->stoped )
    {
        if( this->globalArgs->sendQueue->empty() )
        {
            usleep(1000);
            continue;
        }

         libadapter::PushedPackage pp;

         this->globalArgs->detection_locker.lock();

         FRAME_INFO frameInfo ;
         frameInfo = this->globalArgs->sendQueue->front();
         this->globalArgs->sendQueue->pop();
         this->globalArgs->detection_locker.unlock();

         cv::Mat rgb = frameInfo.frame_img.clone();//rgb

         pp.image = rgb.clone();
         pp.camera_ip="192.168.0.1";
         pp.task = this->currentTask;

         pp.frame_id = frameInfo.frame_id;
         pp.key = prefix_task[ this->currentTaskType -1 ]+"_"+ pp.camera_ip +"_"+ to_string(pp.frame_id);

         if( this->globalArgs->push_fetch_flag )
         {
             if(this->pScdu->push(pp))
             {
                 this->globalArgs->push_locker.lock();
                 this->globalArgs->pushQueue->push(pp);
                 this->globalArgs->push_locker.unlock();

                 this->globalArgs->flag_locker.lock();
                 this->globalArgs->push_fetch_flag = false;
                 this->globalArgs->flag_locker.unlock();
             }
         }
    }

}

void GeneralSecurityMonitor::fetchResult()
{
    while(!this->stoped)
    {
         libadapter::FetchedPackage fp;

         fp.task = this->currentTask;

         //get result
         if(this->globalArgs->pushQueue->empty())
         {
              usleep(1000);
              continue;
         }

         this->globalArgs->push_locker.lock();
         libadapter::PushedPackage pp = this->globalArgs->pushQueue->front();
         this->globalArgs->pushQueue->pop();
         this->globalArgs->push_locker.unlock();
         
         if( this->pScdu->fetch(fp, pp.key))
         {
             this->globalArgs->result_locker.lock();
             if(this->globalArgs->resultQueue->size()<2)
             {
                 this->globalArgs->resultQueue->push(fp);
             }

             this->globalArgs->result_locker.unlock();

             this->globalArgs->flag_locker.lock();
             this->globalArgs->push_fetch_flag = true;
             this->globalArgs->flag_locker.unlock();
         }
    }
}


void GeneralSecurityMonitor::drawResults()
{
    libadapter::FetchedPackage fp;

    while (!this->stoped)
    {
       usleep(1000);
       auto t1 = std::chrono::steady_clock::now();
       this->globalArgs->index_locker.lock();
       cv::Mat img = this->globalArgs->frames_input[ (this->globalArgs->refresh_index - 30 + FRAME_Num ) % FRAME_Num ].clone();
       this->globalArgs->index_locker.unlock();

        if( !this->globalArgs->resultQueue->empty() )
        {
            this->globalArgs->result_locker.lock();
            fp = this->globalArgs->resultQueue->front();
            this->globalArgs->resultQueue->pop();
            this->globalArgs->result_locker.unlock();
        }
           QStringList list = QString::fromStdString(fp.key).split("_");//QString字符串分割函数
           QString path = "../GeneralSecurityMonitoring_UI/results/"+list[0];

           if(ImgSave_Flag)
           {
               mkMutiDir(path);
               qDebug()<<"path: ==========>"<<path;
           }

           // info 0: nothing 1: no-helmet, 2: no-unifrom / REFLECTIVE_CLOTHING, 3: no-helmet and no-unifrom / no-helmet and REFLECTIVE_CLOTHING
          switch(this->currentTaskType)
           {
               case SAFETY_HELMET_DETECTION_TYPE:
                    {
                       //draw result
                      if(!fp.results.empty())
                       {
                           for(auto result : fp.results)
                           {
                               if(result.bbox.cls == 1)
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                   //show text
                                   string label = "";

                                   label = "No Helmet!";
                                   //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
                               }
                               else
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,255,0),2);
                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,255,0),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,(result.bbox.y2 - result.bbox.y1)/60),cv::Scalar(0,255,0),cv::FILLED);

                                   //show text
                                   string label = "";
                                   //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" H: "+to_string( (int)(result.bbox.scores[1]*100))+"%";

                                   label = "Helmet";
                                   //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/10,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,0,0),2);
                               }
                           }
                           //save result
                           QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                           QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                           if(ImgSave_Flag&&fp.cls ==1)
                           {
                               cv::imwrite(fileName.toStdString(),img);
                           }
                       }
                       QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                       emit resultToDisplay(rgbImg);
                       break;
                    }

               case WORK_REDCLOTHING_DETECTION_TYPE:
                    {
                       //draw result
                      if(!fp.results.empty())
                      {
                           for(auto result : fp.results)
                           {
                               if(result.bbox.cls == 2)
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                   //show task types
                                   string label = "";
                                   //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                   label="No Red Uniform!";
                                   //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
                                  }
                               else
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,255,0),2);

                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,255,0),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,255,0),cv::FILLED);

                                   //show task types
                                   string label = "";
                                   //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                   label="Red Uniform ";
                                   //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/10,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,255,0),2);
                               }
                           }

                           //save result
                           QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                           QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                           if(ImgSave_Flag&&fp.cls==1)
                           {
                               cv::imwrite(fileName.toStdString(),img);
                           }
                      }

                      QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                      emit resultToDisplay(rgbImg);
                      break;
                    }
              case WORK_BLUECLOTHING_DETECTION_TYPE:
                   {
                      //draw result
                     if(!fp.results.empty())
                     {
                          for(auto result : fp.results)
                          {
                              if(result.bbox.cls == 2)
                              {
                                  cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

                                  cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                  cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                  cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                  cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                  //show task types
                                  string label = "";
                                  //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                  label="No Blue Uniform!";
                                  //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                  cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
                                 }
                              else
                              {
                                  cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,255,0),2);

                                  cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                  cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,255,0),cv::FILLED);

                                  cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                  cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,255,0),cv::FILLED);

                                  //show task types
                                  string label = "";
                                  //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                  label="Blue Uniform ";
                                  //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                  cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/10,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,255,0),2);
                              }
                          }

                          //save result
                          QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                          QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                          if(ImgSave_Flag&&fp.cls==1)
                          {
                              cv::imwrite(fileName.toStdString(),img);
                          }
                     }

                     QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                     emit resultToDisplay(rgbImg);
                     break;
                   }
               case REFLECT_VEST_DETECTION_TYPE:
                  {
                     //draw result
                    if(!fp.results.empty())
                    {
                         for(auto result : fp.results)
                         {
                             if(result.bbox.cls == 2)
                             {
                                 cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

                                 cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                 cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                 cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                 cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                 //show task types
                                 string label = "";
                                 //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                 label="No Vest!";
                                // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                 cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
                             }
                             else
                             {
                                 cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,255,0),2);

                                 cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                 cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,255,0),cv::FILLED);

                                 cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                 cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,255,0),cv::FILLED);

                                 //show task types
                                 string label = "";
                                 //label="P: "+to_string( (int)(result.bbox.scores[0]*100) ) +"%"+" U: "+to_string( (int)(result.bbox.scores[1]*100))+"%";
                                 label="Vest";
                                 //float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                 cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/10,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,0,0),2);
                             }
                         }

                         //save result
                         QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                         QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                         if(ImgSave_Flag&&fp.cls==1)
                         {
                             cv::imwrite(fileName.toStdString(),img);
                         }
                    }

                    QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                    emit resultToDisplay(rgbImg);
                    break;
                  }

               case PERSONNEL_INTRUSION_DETECTION_TYPE:
                     {
                       //draw result
                     if(!fp.results.empty())
                      {
                           for(auto result : fp.results)
                           {
                               cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
                               cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                               cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                               cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                               cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                               //show task types
                               string label = "";
                               label="Person!";
                              // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                               cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
                           }
                           //save result
                           QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                           QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                           if(ImgSave_Flag&&fp.cls==1)
                           {
                               cv::imwrite(fileName.toStdString(),img);
                           }
                     }
                       QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                       emit resultToDisplay(rgbImg);
                       break;
                     }

               case SKIN_EXPOSURE_DETECTION_TYPE:
                     {
                        if(!fp.results.empty())
                        {

                           for(auto result:fp.results)
                           {
                               if(!result.ptss.empty())
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);
                                   //show task types
                                   string label = "";
                                   label="Skin Exposure!";
                                  // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
                               }
                               else
                               {
                                   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,255,0),2);

                                   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,255,0),cv::FILLED);

                                   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,255,0),cv::FILLED);

                                   //show task types
                                   string label = "";
                                   label="OK";
                                  // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                   cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(0,0,0),2);
                               }

                               for(auto pts:result.ptss)
                               {
                                   for(auto pt : pts)
                                   {
                                       cv::circle(img,cv::Point(pt.x,pt.y),2,cv::Scalar(255,0,255),cv::FILLED);
                                   }
                               }
                           }

                           //save result
                           QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                           QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                           if(ImgSave_Flag&&fp.cls == 1)
                           {
                               cv::imwrite(fileName.toStdString(),img);
                           }
                        }

                        QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                        emit resultToDisplay(rgbImg);
                       break;
                     }

               case SMOKE_DETECTION_TYPE:
                     {
                       if(fp.cls == 1)
                       {
                           string label = "";
                           label="Warning: Fire!";
                          // cv::Point origin1(img.cols/16,img.rows/16);
                          // cv::Point origin2(14*img.cols/16,14*img.rows/16);

                           cv::putText(img,label,cv::Point(img.cols/2,img.rows/2),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(0,0,255),2);
                          // cv::rectangle(img,origin1,origin2,cv::Scalar(0,0,255),2);

                           //save result
                           QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                           QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                           if(ImgSave_Flag)
                           {
                               cv::imwrite(fileName.toStdString(),img);
                           }
                       }

                       QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                       emit resultToDisplay(rgbImg);

                       break;
                     }
              case PERSONSMOKE_DETECTION_TYPE:
                    {
                      if(!fp.results.empty())
                       {
                            for(auto result : fp.results)
                            {
                                if(result.bbox.cls == 2 )
                                {
                                    cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
                                    cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                    cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                    cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                    cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                    //show task types
                                    string label = "";
                                    label="Person Smoke!";
                                   // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                    cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
                                }
                            }
                            //save result
                            QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                            QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                            if(ImgSave_Flag&& fp.cls ==1)
                            {
                                cv::imwrite(fileName.toStdString(),img);
                            }
                      }

                       QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                       emit resultToDisplay(rgbImg);
                      break;
                    }

              case PERSONPHONE_DERECTION_TYPE:
                    {
                      if(!fp.results.empty())
                       {
                            for(auto result : fp.results)
                            {
                                if(result.bbox.cls == 1 )
                                {
                                    cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
                                    cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                    cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                    cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                    cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                    //show task types
                                    string label = "";
                                    label="Person Phone!";
                                   // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                    cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
                                }
                            }
                            //save result
                            QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                            QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                            if(ImgSave_Flag&& fp.cls==1)
                            {
                                cv::imwrite(fileName.toStdString(),img);
                            }
                      }
                       QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                       emit resultToDisplay(rgbImg);
                      break;
                    }
              case OILLEAK_DETECTION_OUT_TYPE:
              case OILLEAK_DETECTION_IN_TYPE:
                    {
                      if(!fp.results.empty())
                      {
                         vector< vector<cv::Point> >contours;
                         for(auto result:fp.results)
                         {
                             if(!result.ptss.empty()&& result.cls == 1)
                             {
                                // cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

                                 cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                 //show task types
                                 string label = "";
                                 if(this->currentTaskType == OILLEAK_DETECTION_OUT_TYPE)
                                 {
                                     label="Outdoor Oil Leak!";
                                 }
                                 else if(this->currentTaskType == OILLEAK_DETECTION_IN_TYPE && result.bbox.cls == 1)//box.cls check if indoor oil
                                 {
                                     label="Indoor Oil Leak!";
                                 }

                                // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                 cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(0,0,255),2);

                                for(auto pts:result.ptss)
                                   {
                                     vector<cv::Point>contour;
                                     for(auto pt : pts)
                                     {
                                         cv::Point p(pt.x,pt.y);
                                         contour.push_back(p);
                                     }
                                     contours.push_back(contour);
                                   }
                             }
                         }
                        cv::polylines(img,contours,true,cv::Scalar(0,0,255),4,cv::LINE_AA);
                        //cv::fillPoly(img, contours,cv::Scalar(255,0,255));
                         //save result
                         QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                         QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                         if(ImgSave_Flag&&fp.cls==1)
                         {
                             cv::imwrite(fileName.toStdString(),img);
                         }
                      }

                      QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                      emit resultToDisplay(rgbImg);
                     break;
                    }

              case PERSONFALL_DETECTION_TYPE:
                      {
                        //draw result
                          if(!fp.results.empty())
                           {
                                for(auto result : fp.results)
                                {
                                    if(result.cls == 1)
                                    {
                                        cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
                                        cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
                                        cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

                                        cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
                                        cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

                                        //show task types
                                        string label = "";
                                        label="Person Fall Down!";
                                       // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
                                        cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
                                    }
                                }
                                //save result
                                QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
                                QString fileName = path+"/"+QString::fromStdString(fp.key)+"_"+curTime+".png";
                                if(ImgSave_Flag&&fp.cls ==1)
                                {
                                    cv::imwrite(fileName.toStdString(),img);
                                }
                          }
                        QImage rgbImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888).rgbSwapped().copy();
                        emit resultToDisplay(rgbImg);
                        break;
                      }
           }
          double dr_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t1).count();

    }
}
void GeneralSecurityMonitor::receiPoints(QVector<QMap<QString, QPoint>> pointsMap)
{
    //keep update
    std::cout<<"recei points ========="<<std::endl;
    roiPoints.clear();
    for(int i=0;i < pointsMap.size();i++)
    {
        foreach (QString canvasSizeStr, pointsMap[i].keys())
        {
           QStringList canvasSizeStrList = canvasSizeStr.split(",");
           QString wStr = canvasSizeStrList[0];
           QString hStr = canvasSizeStrList[1];

           int oldW = wStr.toInt();
           int oldH = hStr.toInt();

           int oldPointX = pointsMap[i][canvasSizeStr].x();
           int oldPointY = pointsMap[i][canvasSizeStr].y();

           float nowPointX = oldPointX*1.0 / oldW;
           float nowPointY = oldPointY*1.0 / oldH;

           roiPoints.push_back(nowPointX);
           roiPoints.push_back(nowPointY);
        }
    }

    switch (this->currentTask)
    {
    case libadapter::Task::PERSON_HELMET:
         this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
         break;
    case libadapter::Task::PERSON_RED_UNIFORM:
          this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
          break;
   case libadapter::Task::PERSON_BLUE_UNIFORM:
         this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
         break;
   case libadapter::Task::PERSON_VEST:
         this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
         break;
   case libadapter::Task::PERSON_INVASION:
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::SKIN_EXPOSURE:
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::FIRE_SMOKE:
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::PHONE:
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::SMOKE:
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::LEAK_OIL_INDOOR:
        //indoor oil
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
   case libadapter::Task::LEAK_OIL:
        //Outdoor oil
        this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
        break;
    case libadapter::Task::FALL_DOWN:
         this->pScdu->set_roi(this->currentTask,"192.168.0.1",this->roiPoints);
         break;
    default:
        break;
    }
}
void GeneralSecurityMonitor::receiFinishPoints(bool flag)
{
    vector<float>tmp={0,0};
    if(flag == true)
    {
        switch (this->currentTask)
        {
        case libadapter::Task::PERSON_HELMET:
             this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
             break;
        case libadapter::Task::PERSON_RED_UNIFORM:
             this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
              break;
       case libadapter::Task::PERSON_BLUE_UNIFORM:
             this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
             break;
       case libadapter::Task::PERSON_VEST:
             this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
             break;
       case libadapter::Task::PERSON_INVASION:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::SKIN_EXPOSURE:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::FIRE_SMOKE:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::PHONE:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::SMOKE:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::LEAK_OIL_INDOOR:
            //indoor oil
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::LEAK_OIL:
            //Outdoor oil
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
       case libadapter::Task::FALL_DOWN:
            this->pScdu->set_roi(this->currentTask,"192.168.0.1",tmp);
            break;
        default:
            break;
        }
    }

}

void GeneralSecurityMonitor::run()
{
    std::thread th1(&GeneralSecurityMonitor::fetchResult, this);
    th1.detach();
    std::thread th2(&GeneralSecurityMonitor::drawResults,this);
    th2.detach();

    this->pushImageToDetect();
}
