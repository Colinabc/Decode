# Model１工程说明文档
​    本工程是一个多图片输入源，多任务包含: 安全帽检测、工作服检测、人员入侵检测、烟火检测以及皮肤裸露检测等多种场景检测功能的实现.
## 1. 环境依赖
> ```
> glog
> gflags
> opencv3.4.0
> #通用安监算法库
> lscheduler
> ladapter
> ```

## 2. 使用方法
### 2.1 算法源码编译及安装
```
编译命令:
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
//将安监视算法库加入系统变量
cd ../thirdparty/lib
export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
cd ../build
./sample1

编译结果:
工程目录结构如下:
├── caisa_config.json
├── CMakeLists.txt
├── main.cpp
├── readme.md
├── results
└── thirdparty
│   ├── include
│   ├── adapter
│   │   ├── adapter.hh
│   │   ├── adpt_utils.hh
│   │   ├── attr_adapter.hh
│   │   ├── deeplab_adapter.hh
│   │   ├── fire_adapter.hh
│   │   └── yolo_adapter.hh
│   ├── courier_base.hh
│   ├── courier_clothing.hh
│   ├── courier_fire.hh
│   ├── courier.hh
│   ├── courier_invasion.hh
│   ├── courier_skin.hh
│   ├── cr_colors.hh
│   ├── cr_common.hh
│   ├── image_loader.hh
│   ├── package_ex.hh
│   ├── package.hh
│   ├── pipeline.hh
│   ├── scheduler.hh
│   ├── subcourier_attr.hh
│   ├── subcourier_det.hh
│   ├── subcourier.hh
│   ├── subcourier_skin.hh
│   ├── third_party
│   │   ├── attribute
│   │   │   ├── image_classification_m.hh
│   │   │   ├── image_processor_m.hh
│   │   │   └── json.hpp
│   │   ├── libyolo
│   │   │   ├── bbox.hh
│   │   │   ├── nms.hh
│   │   │   ├── param.hh
│   │   │   ├── post_process.hh
│   │   │   ├── pre_process.hh
│   │   │   └── yolo.hh
│   │   ├── skin_exposure
│   │   │   ├── my_clock.hh
│   │   │   └── skin_exposure.hh
│   │   └── utils
│   │       ├── dclock.hh
│   │       └── device.hh
│   └── version.hh
└── lib
    ├── libadapter.so
    ├── libajutils.so
    ├── libattr.so
    ├── libscheduler.so
    ├── libskin.so
    └── libyolo.so

```
### 2.2 数据结构和相关函数
#### 2.2.1 Scheduler类
```
class Scheduler {
public:
  Scheduler(string config_json_path);     //导入配置文件进行初始化
  bool fetch(FetchedPackage &package，string key);   //输出结果-FetchedPackage
  void push(PushedPackage &package);     //输入数据-PushedPackage

  void enroll(shared_ptr<Courier> courier, Task task); //注册检测任务类型
  void start(Task task);                 //执行检测任务
  void stop(Task task);                  //结束检测任务
  void remove(Task task);                //移除特定检测任务
  void start();
  void stop();
  void remove();
  json CONF_;
private:
  vector<shared_ptr<Courier>> couriers_;
  vector<Task> tasks_;
}
}
```
#### 2.2.2 Task枚举
```
enum Task {
  PERSON_CLOTHING,              //工作服检测
  PERSON_INVASION,              //人员入侵检测
  PERSON_CLOTHING_VEST,  　　　　//反光衣检测
  SKIN_EXPOSURE,                //皮肤裸露检测
  OIL_LEAKAGE,                  //漏油检测
  FIRE_SMOKE                    //烟火检测
};
```
#### 2.2.3  PushedPackage/FetchedPackage类
```
struct Package {
  Task task;
  int frame_id;
  std::string camera_ip;
  std::string key;

  virtual ~Package() {}
};

//输入数据信息
struct PushedPackage: Package {
  cv::Mat image;   

  // ~PushedPackage() {}
};
//输出结果信息
struct FetchedPackage: Package {
  int cls;
  int frame_id;
  std::string key;
  cv::Mat image;
  std::vector<CrResult> results;

  /*------- you don't need to know ------- */
  bool finished;
  int num_finished;
  int num_rests;
  int create_time;

  FetchedPackage() {}
  FetchedPackage(PushedPackage& pp) {
    cls = 0;
    task = pp.task;
    frame_id = pp.frame_id;
    key = pp.key;
    image = pp.image;
    finished = false;
  }
};
```
### 2.3 样例
​      在Model1文件夹下放有main.cpp样例代码
```
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
//多个任务检测
enum Task tasks[4] = {Task::PERSON_CLOTHING,
                Task::PERSON_INVASION,
                Task::SKIN_EXPOSURE,
                Task::FIRE_SMOKE};
string prefix[4] = {"clothing-","invasion-","skin-","fire-"};
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

void draw_person_clothing(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_clothing: "<<fp.results.size();

   for(auto& result: fp.results)
   {
      cv::Point p1(result.bbox.x1, result.bbox.y1);
      cv::Point p2(result.bbox.x2, result.bbox.y2);
      cv::rectangle(img, p1, p2, colors[result.bbox.cls],2);
      cout<< "rectangle: "<<p1.x<<" "<<p1.y<<" "<<p2.x<<" "<<p2.y<<endl;

      stringstream ss;
      ss<<"Person: "<<int(result.bbox.scores[0]*100);

      if(result.bbox.cls == 1|| result.bbox.cls == 3)
      {
         ss<<" Helmet: "<<int(result.bbox.scores[1]*100);
      }
      if(result.bbox.cls == 2|| result.bbox.cls == 3)
      {
    	 ss<<" RedUniform: "<<int(result.bbox.scores[2]*100);
      }
      draw_label(img, ss.str(),p1,p2,colors[result.bbox.cls]);
   }
}

void draw_person_invasion(cv::Mat& img, FetchedPackage& fp)
{
    DLOG(INFO) << "draw_person_invasion: "<<fp.results.size();
    for(auto& result: fp.results)
    {
         cv::Point p1(result.bbox.x1, result.bbox.y1);
         cv::Point p2(result.bbox.x2, result.bbox.y2);
         cv::rectangle(img, p1, p2, COLOR_RED,2);

         stringstream ss;
         ss<<"Person Invasion: "<<int(result.bbox.scores[0]*100);
         draw_label(img,ss.str(),p1,p2,COLOR_RED);
    }
}

void draw_person_skin(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_person_skin: "<<fp.results.size();
  vector<vector<cv::Point>>contours;

  for(auto &result : fp.results)
   {
        cv::Point p1(result.bbox.x1, result.bbox.y1);
      	cv::Point p2(result.bbox.x2, result.bbox.y2);
       	cv::rectangle(img, p1, p2, COLOR_RED,2);
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
         stringstream ss;
         ss<<"Skin Exposure";
         draw_label(img,ss.str(),p1,p2,COLOR_RED);
    }

   cv::polylines(img,contours,true,COLOR_WHITE,2,cv::LINE_AA);
   cv::fillPoly(img, contours,COLOR_PINK);
}


void draw_fire(cv::Mat& img, FetchedPackage& fp)
{
  DLOG(INFO) << "draw_fire: "<<fp.cls;
  int font = cv::FONT_HERSHEY_COMPLEX;
  double font_scale = 1;
  int thickness = 2;
  int baseline;
  string label = "Fire!";

  if(fp.cls == 0)
  {
       label = "Monitoring...";
  }

   cv::Size text_size = cv::getTextSize(label,font, font_scale, thickness,&baseline);

   cv::Point o1(0,0);
   cv::Point o2(text_size.width, text_size.height);
   cv::Point o(0,text_size.height);
   cv::rectangle(img,o1,o2,COLOR_WHITE,-1);
   cv::putText(img,label,o,font,font_scale,COLOR_RED,thickness);
}

void draw(cv::Mat& img, FetchedPackage& fp)
{
     switch(fp.task)
     {
        case Task::PERSON_CLOTHING:
            draw_person_clothing(img,fp);
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
        default:
            cout<<"Unsupported task: "<<fp.task<<endl;
            break;
     }
}

void enroll_tasks(Scheduler &s)
{
    auto cc = make_shared<CourierClothing>(FLAGS_config_json);
    s.enroll(cc,tasks[0]);

    auto ci = make_shared<CourierInvasion>(FLAGS_config_json);
    s.enroll(ci, tasks[1]);

    auto cs = make_shared<CourierSkin>(FLAGS_config_json);
    s.enroll(cs, tasks[2]);

    auto cf = make_shared<CourierFire>(FLAGS_config_json);
    s.enroll(cf, tasks[3]);
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
      s.CONF_["clothing_folder"], s.CONF_["clothing_name_txt"]);
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
            for(int j=0;j<3;j++){
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
        k+=step;
    }
}


int main(int argc,char* argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv,true);
    google::InitGoogleLogging(argv[0]); 
    
    Scheduler scdu(FLAGS_config_json);
    enroll_tasks(ref(scdu));

    scdu.start();

    test(ref(scdu));
    scdu.stop();
    DLOG(INFO)<<"STOP"<<endl;

    google::ShutdownGoogleLogging();
    return 0;
}
```
## 3.总结
上述为图片输入源，多任务检测的使用示例，可参考上述逻辑进行修改.