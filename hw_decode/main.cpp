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
