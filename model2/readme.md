# Model2工程说明文档
​    本工程是一个多视频/相机输入源，多任务包含: 安全帽检测、工作服检测、人员入侵检测、烟火检测以及皮肤裸露检测等多种场景检测功能的实现.
## 1. 环境依赖
> ```
> glog
> gflags
> opencv3.4.0
> ＃ffmpeg库
> lavformat
> lavfilter
> lavdevice
> lswresample
> lswscale
> lavutil
> lavcodec
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

cd ../thirdparty/lib
export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
cd ../build
./sample2

编译结果:
工程目录结构如下:
├── caisa_config.json
├── CMakeLists.txt
├── include
│   └── decode_detection.h
├── main.cpp
├── Readme.md
├── results
├── src
│   └── decode_detection.cpp
└── thirdparty(通用安监算法库文件夹)
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
  bool fetch(FetchedPackage &package);   //输出结果-FetchedPackage
  void push(PushedPackage &package，string key);     //输入数据-PushedPackage

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
​      在Model2文件夹下放有main.cpp样例代码
```
#include"decode_detection.h"
＃define cameraNum 4
//配置文件路径
DEFINE_string(config_json, "../caisa_config.json", "");

int main()
{
   //输入相机IP,视频流地址URL
    string cameraIP[cameraNum]={};
    string URL[cameraNum]={};
     #1.相机输入源
    /*URL[cameraNum]={
            "rtsp://admin:letmein1@192.168.1.160:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.161:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.162:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.163:554/h264/ch1/main/av_stream"
};*/
   　#2.视频输入源
    for(int i=0;i<cameraNum;i++)
    {
       cameraIP[i]="192.168.1."+to_string(i+1);
       URL[i] = "../../../testing_data/VideoData/"+to_string(i+1)+".mp4";
    }
  
 //注册检测任务,4种
    Scheduler scdu(FLAGS_config_json);
    auto cc = make_shared<CourierClothing>(FLAGS_config_json);
    scdu.enroll(cc, Task::PERSON_CLOTHING);

    auto ci = make_shared<CourierInvasion>(FLAGS_config_json);
    scdu.enroll(ci, Task::PERSON_INVASION);

    auto cs = make_shared<CourierSkin>(FLAGS_config_json);
    scdu.enroll(cs, Task::SKIN_EXPOSURE);

    auto cf = make_shared<CourierFire>(FLAGS_config_json);
    scdu.enroll(cf, Task::FIRE_SMOKE);
    scdu.start();
　　
　　//开启多任务检测线程
    vector<thread>v_th;
    for(int i=0;i<cameraNum;i++)
    {
        v_th.push_back(std::thread(hardwareDecode,ref(scdu),URL[i],cameraIP[i]) );
    }
    
    for(int i=0;i<cameraNum;i++)
    {
        v_th[i].detach();
    }
    
    //通过getResult函数获取检测结果，并将结果图片存入results文件夹
    std::thread ft1(getResult,ref(scdu));   
    ft1.detach();     
    
    while(1)
     {
       usleep(10);
     }
    scdu.stop();
    DLOG(INFO) << "STOP"<<endl; 
    google::ShutdownGoogleLogging();
   
    return 0;
}
```
## 3.总结
上述为多输入源，多任务检测的使用示例，可参考上述逻辑进行修改.