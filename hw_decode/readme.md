# hw_decode工程说明文档
​    本工程是对视频/相机输入源进行解码的工程样例
## 1. 环境依赖
> ```
> opencv3.4.0
> # ffmpeg
> lavformat
> lavfilter
> lavdevice
> lswresample
> lswscale
> lavutil
> lavcodec
> ```

## 2. 使用方法
### 2.1 算法源码编译及安装
```
编译命令:
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
./sample

编译结果:
工程目录结构如下:
├── camera_config.json
├── CMakeLists.txt
├── inc
│   ├── cameradecode.h
│   └── json.hpp
├── main.cpp
├── readme.md
├── results
└── src
    └── cameradecode.cpp
```
### 2.2 数据结构和相关函数
#### 2.2.1 cameradecode类
```
class Scheduler {
public:
    int Convert24Image(char *p32Img, char *p24Img, int dwSize32);//图像格式转换
    CAMERASTATE checkCameraState(string URL,string cameraID);//相机连接状态检查
    void softwareDecode(string URL,string cameraID,bool saveImg);//软解码
    void hardwareDecode(string URL,string cameraID,bool savImg);//硬解码
    void softwareLoopDecode(string URL,string cameraID,bool saveImg);//软解码
    void hardwareLoopDecode(string URL,string cameraID,bool saveImg);//硬解码
    void stop();//停止解码

private:
    bool stoped;
}
```
### 2.3 样例
​      在hw_decode文件夹下放有main.cpp样例代码
```
#include<string>
#include<thread>
#include<iostream>
#include<fstream>
#include<map>
#include<vector>

#include"cameradecode.h"
#include"json.hpp"

using namespace std;
using json = nlohmann::json;

int main()
{
     //读取相机配置文件
    string config_json_path = "../camera_config.json";
    ifstream fcj(config_json_path);
    
    json CONF_;
    fcj>>CONF_;
     int count=CONF_["cameraNum"];
     cout<<"count: "<<count<<endl;
     map<string,string>cameraInfo;
     for(int i=0;i<count;i++)
       {
          cameraInfo.insert(pair<string,string>("camera"+to_string(i+1),CONF_["camera"+to_string(i+1)]["IP"])); 
       }
      
      //根据设备数量初始化解码对象
      CameraDecode decode[count];
      DECODE_TYPE decode_type = HARDWARE_DECODE;
     
     //此处为硬解码方式
      vector<thread>my_threads;
    if(decode_type == HARDWARE_DECODE)
      {
         for(int i=0;i<count;i++)
         {
           my_threads.push_back( thread( std::bind(&CameraDecode::hardwareDecode,&decode[i],cameraInfo["camera"+to_string(i+1)],"camera"+to_string(i+1),true)) );
         }
         for(auto& i:my_threads)
         {
             i.detach();
         }        
      } 
       while(1)
      {
         usleep(1);
      } 
    return 0;
}
```
## 3.总结
上述为视频/相机输入源，多路线程解码的使用示例，可参考上述逻辑进行修改.