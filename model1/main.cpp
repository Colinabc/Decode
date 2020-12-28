#include<string>
#include<thread>
#include<chrono>
#include<iostream>
#include<algorithm>

#include <opencv2/opencv.hpp>

#include<gflags/gflags.h>
#include<glog/logging.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstddef>

// 安监算法库
#include"image_loader.hh"
#include "adapter/adpt_utils.hh"
#include "scheduler.hh"
#include "courier_fire.hh"
#include "courier_skin.hh"
#include "courier_invasion.hh"

using namespace std;
using namespace cv;
using namespace libadapter;

//配置文件路径
DEFINE_string(config_json,"../caisa_config.json","");
DEFINE_string(config_json_vest,"../caisa_config_vest.json","");
DEFINE_string(config_json_smokephone,"../caisa_config_smokephone.json","");
DEFINE_string(config_json_oilleak,"../caisa_config_leakoil.json","");
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

void draw_label(cv::Mat& img, const string& label, cv::Point& p1,cv::Point& p2, const cv::Scalar& color)
{
    int font = cv::FONT_HERSHEY_COMPLEX;
    double font_scale = 0.8;
    int thickness = 2;
    int baseLine ;
    cv::Size text_size = cv::getTextSize(label,font,font_scale,thickness,&baseLine);

    cv::Point o = p1;
    if(o.y - text_size.height < 0) o.y = p2.y + text_size.height;
    if(o.y > img.rows) o.y = p2.y;
    if(o.x + text_size.width > img.cols) o.x = img.cols - text_size.width;

    cv::Point o1 = {o.x, o.y - text_size.height};
    cv::Point o2 = {o.x + text_size.width, o.y};
    cv::rectangle(img, o1, o2,color, -1);
    cv::putText(img, label, o, font, font_scale, COLOR_WHITE, thickness);
}

void draw_person_uniform(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_uniform: "<<fp.results.size();

   for(auto& result: fp.results)
   {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
	   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

	   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2);
	   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

       string label ="";
      if(result.bbox.cls == 1)
      {
         label = "No Helmet!";
      }
      if(result.bbox.cls == 2)
      {
    	 label = "No RedUniform!";
      }
      if(result.bbox.cls == 3)
      {
    	 label = "No Helemt&RedUniform!";
      }
     cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
   }
}

void draw_person_vest(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_vest: "<<fp.results.size();

   for(auto& result: fp.results)
   {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
	   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

	   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2);
	   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 +2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

      string label ="";
      if(result.bbox.cls == 2|| result.bbox.cls ==3)
      {
    	 label = "No Vest!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
   }
}

void draw_person_invasion(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_invasion: "<<fp.results.size();
    for(auto& result: fp.results)
    {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
	   cv::rectangle(img,origin1,origin1+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,-(result.bbox.y2 - result.bbox.y1)/15),cv::Scalar(0,0,255),cv::FILLED);

	   cv::Point origin2(result.bbox.x1 - 1,result.bbox.y2 + 1);
	   cv::rectangle(img,origin2,origin2+ cv::Point(result.bbox.x2 - result.bbox.x1 + 2,(result.bbox.y2 - result.bbox.y1)/30),cv::Scalar(0,0,255),cv::FILLED);

      string label ="";
      if(result.bbox.cls == 2|| result.bbox.cls ==3)
      {
    	 label = "Invasion!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
    }
}

void draw_person_skin(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_person_skin: "<<fp.results.size();
  vector<vector<cv::Point>>contours;

  for(auto &result : fp.results)
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

        for(auto& pts:result.ptss)
	      {
             for(auto& pt : pts)
             {
                cv::circle(img,cv::Point(pt.x,pt.y),2,cv::Scalar(255,0,255),cv::FILLED);
             }
          }
    }
}

void draw_fire(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_fire: "<<fp.cls;
  if(fp.cls == 1)
   {
       string label = "";
       label="Warning: Fire!";
       cv::Point origin1(img.cols/16,img.rows/16);
       cv::Point origin2(14*img.cols/16,14*img.rows/16);

       cv::putText(img,label,cv::Point(img.cols/2,img.rows/2),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(0,0,255),2);
       cv::rectangle(img,origin1,origin2,cv::Scalar(0,0,255),2);
   }
}

void draw_person_smoke(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_smoke: "<<fp.results.size();
    for(auto& result: fp.results)
    {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);

      string label ="";
      if(result.bbox.cls == 2)
      {
    	 label = "personSmoke!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
    }
}

void draw_person_phone(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_phone: "<<fp.results.size();
    for(auto& result: fp.results)
    {
	   cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);

	   cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);

      string label ="";
      if(result.bbox.cls == 1)
      {
    	 label = "personPhone!";
      }
      cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(255,255,255),2);
    }
}

void draw_oil_leak(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_oil_leak: "<<fp.results.size();
  vector<vector<cv::Point>>contours;

  for(auto &result : fp.results)
   {
       if(!result.ptss.empty())
       {
           cv::rectangle(img,cv::Point(result.bbox.x1, result.bbox.y1),cv::Point(result.bbox.x2, result.bbox.y2),cv::Scalar(0,0,255),2);
           cv::Point origin1(result.bbox.x1 - 1,result.bbox.y1);
		   
           //show task types
           string label = "";
           label="Oil Leak!";
          // float scale =(float)(result.bbox.x2-result.bbox.x1)/(result.bbox.y2-result.bbox.y1);
           cv::putText(img,label,origin1+cv::Point( (result.bbox.x2-result.bbox.x1)/14,-(result.bbox.y2 - result.bbox.y1)/60),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(255,255,255),2);
       }

        for(auto& pts:result.ptss)
	      {
			vector<cv::Point>contour;
             for(auto& pt : pts)
             {
				 cv::Point p(pt.x,pt.y);
				 contour.push_back(p);
             }
			contours.push_back(contour);
          }
    }
	cv::polylines(img,contours,true,cv::Scalar(0,0,255),2,cv::LINE_AA);
	cv::fillPoly(img, contours,cv::Scalar(255,0,255));
}

void draw(cv::Mat& img, FetchedPackage& fp)
{
     switch(fp.task)
     {
        case Task::PERSON_CLOTHING:
            draw_person_uniform(img,fp);
            break;
        case Task::PERSON_CLOTHING_VEST:
            draw_person_vest(img,fp);
            break;
        case Task::PERSON_INVASION:
            draw_person_invasion(img,fp);
            break;
        case Task::SKIN_EXPOSURE:
            draw_person_skin(img,fp);
            break;
        case Task::FIRE_SMOKE:
            draw_fire(img,fp);
            break;
		 case Task::SMOKE:
			 draw_person_smoke(img,fp);
			 break;
		 case Task::PHONE:
			 draw_person_phone(img,fp);
			 break;
		 case Task::LEAK_OIL:
			 draw_oil_leak(img,fp);
			 break;
        default:
            cout<<"Unsupported task: "<<fp.task<<endl;
            break;
     }
}

void enroll_tasks(Scheduler &s)
{
    //uniform
    auto cc = make_shared<CourierClothing>(FLAGS_config_json);
    s.enroll(cc,tasks[0]);
    std::cout<<"1======>"<<std::endl;
   //vest
    auto cv = make_shared<CourierClothing>(FLAGS_config_json_vest);
    s.enroll(cv,tasks[1]);
   std::cout<<"2======>"<<std::endl;
    //invasion
    auto ci = make_shared<CourierInvasion>(FLAGS_config_json);
    s.enroll(ci, tasks[2]);
    std::cout<<"3======>"<<std::endl;
    //skin exposure
    auto cs = make_shared<CourierSkin>(FLAGS_config_json);
    s.enroll(cs, tasks[3]);
    std::cout<<"4======>"<<std::endl;
    //smoke
    auto cf = make_shared<CourierFire>(FLAGS_config_json);
    s.enroll(cf, tasks[4]);
	
	//person smoke
	auto cm = make_shared<libadapter::CourierPhone>(FLAGS_config_json_smokephone);
	s.enroll(cm, tasks[5]);
	
	//person phone
	auto cp = make_shared<libadapter::CourierPhone>(FLAGS_config_json_smokephone);
	s.enroll(cp, tasks[6]);

	//oil leak
	auto co = make_shared<libadapter::CourierOil>(FLAGS_config_json_oilleak);
	s.enroll(co, tasks[7]);
}

void test(Scheduler& s)
{
    const int num_ip = s.CONF_["demo_way_camera"];//相机数量
    const int step = s.CONF_["demo_step"];//间隔step张，取一张图进行检测
    const int wait_time = s.CONF_["demo_wait_time_ms"];//间隔时间 
    string ip = "192.168.0.";
    vector<string>ips;
    for(int i=0;i<num_ip;i++)
     {
        ips.push_back(ip+to_string(i));
     }
   //加载数据集
  ImageLoader *loaders[1];
  loaders[0] = new ImageLoader(
      s.CONF_["oil_folder"], s.CONF_["oil_name_txt"]);
 /* loaders[1] = new ImageLoader(
      s.CONF_["invasion_folder"], s.CONF_["invasion_name_txt"]);
  loaders[2] = new ImageLoader(
      s.CONF_["skin_folder"], s.CONF_["skin_name_txt"]);
  loaders[3] = new ImageLoader(
      s.CONF_["fire_folder"], s.CONF_["fire_name_txt"]);*/

    int k=0;
    while(k >=0 )
    {
        //循环读取图片
       for(int ip_i=0;ip_i < num_ip;ip_i++){
            for(int j=0;j<8;j++){
                   int index = k%loaders[0]->size();
                   string path = loaders[0]->operator[](index);
                   cout<<path<<endl;
 
                   cv::Mat img = cv::imread(path);

                   PushedPackage pp;
                   pp.image = img.clone();
                   pp.camera_ip = ips[ip_i];
                   pp.task = tasks[j];
                   pp.key = pp.camera_ip+"_"+to_string(index)+"_"+to_string(j) ;

                   s.push(pp);//调用push函数，将图片传入进行检测

                   FetchedPackage fp;
                   if(!s.fetch(fp,pp.key) )
                      {
                         this_thread::sleep_for(chrono::milliseconds(10));
                         continue;
                      }
                     
                     if(!fp.results.empty()||fp.cls == 1)
                        {
                         draw(img,fp); 
		                 stringstream ss;
		                 ss<<"/"<<prefix[j]<<k<<".png"; 
		                 string dir  = s.CONF_["output_dir"];
						 if(opendir(dir.c_str()) == NULL)
						     {
						        int ret = mkdir(dir.c_str(),0775);
						     }
		                 dir+=ips[ip_i];
						 if(opendir(dir.c_str()) == NULL)
						     {
						        int ret = mkdir(dir.c_str(),0775);
						     }
		                 string save_path = dir + ss.str();
		                 cout<<"Save to"<<save_path<<endl;
		                 cv::imwrite(save_path,img);
                        }
             }
       }
        k+=step;
    }
}


int main(int argc,char* argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv,true);
    google::InitGoogleLogging(argv[0]); 
    
    Scheduler scdu(FLAGS_config_json);
    enroll_tasks(ref(scdu));
    try
	{
	  scdu.start();
	}
	catch(...)
	{
	  std::cout<<"start failure" <<std::endl;
	}
    

    test(ref(scdu));
    scdu.stop();
    DLOG(INFO)<<"STOP"<<endl;

    google::ShutdownGoogleLogging();
    return 0;
}
