#include "common.h"

std::string tasks_json = "";
std::string color_json = "";
std::string en_init_local_path = ""; //本地配置文件路径
std::string en_init_path = "";       //启动设置配置文件路径
std::string en_start_path = "";      //计算节点异常启动文件
std::string en_breathe_path = "";  
std::string absPath = "";

int log_list_len = 0;                //心跳日志中端口列表最大长度
int zone_offset = 0;                 //时区偏移，单位（小时）
int breathe_time_step = 0;
int addr_timeout = 0;                //摄像头断线重连时间
int queue_len = 0;                   //所有队列长度
int reconnTime = 0;
int en_port = 0;
int computing_node_port = 0;
//std::string hwType = "";
std::string computing_node = "";
// std::map<std::string, std::queue<cv::Mat>> addr_queue_map;
// std::map<std::string, std::mutex*> addr_mtx_map;
std::vector<ADDR_TASK_PAIR>task_pair_vec;



std::string get_localtime()
{
  auto time_m = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string milliseconds_str = std::to_string(time_m % 1000);
  if (milliseconds_str.length() < 3)
  {
    milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
  }
  auto time_s = time_m / 1000 + zone_offset * 3600; //东八区偏移
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_s), "%Y-%m-%dT%H:%M:%S.");
  std::string time_now = ss.str() + milliseconds_str;
  return time_now;
}

int getJson(std::string path,std::string& jsonStr)
{
    std::fstream fs(path); 
    if(fs.fail())
    {        
        return -1;
    }
    std::stringstream ss; 
    ss << fs.rdbuf();
    jsonStr = ss.str(); 
    fs.close();
    return 0;
}

void generate_start_flag()
{
  std::string json_new = "";
  std::fstream fs(en_start_path);
  fs.open(en_start_path, std::fstream::binary | std::fstream::out | std::fstream::app);
  fs.write(json_new.c_str(), json_new.length());
  fs.close();
}

int load_encoding_config(std::string& hwType)
{
    task_pair_vec.clear();
    //en_init.config文件
    en_init_local_path = "../config/en_init_local.config";
    std::fstream fs0(en_init_local_path);
    if (fs0.fail())
    {
        return -1;
    }
    std::stringstream ss0;
    ss0 << fs0.rdbuf();
    std::string json_in0 = ss0.str();
    fs0.close();
    try
    {
        auto data0 = nlohmann::json::parse(json_in0);
        en_init_path = absPath+ (std::string)(data0.at("en_init_path"));
        en_start_path = absPath+ (std::string)(data0.at("en_start_path"));
        en_breathe_path = absPath+ (std::string)(data0.at("en_breathe_path"));
        queue_len = data0.at("queue_len");
        log_list_len = data0.at("log_list_len");
        zone_offset = data0.at("zone_offset");
        addr_timeout = data0.at("addr_timeout");      
    }
    catch (...)
    {
        return -1;
    }
    std::fstream fs1(en_init_path);
    if (fs1.fail())
    {
        return -1;
    }
    std::stringstream ss1;
    ss1 << fs1.rdbuf();
    std::string json_in1 = ss1.str();
    fs1.close();
    try
    {   
        auto data1 = nlohmann::json::parse(json_in1);
        breathe_time_step = data1["breathe_time_step"].get<int>();
        reconnTime = data1["reconn_time_step"].get<int>();
        hwType = data1["hardware_type"].get<std::string>();
        en_port = data1["port"].get<int>();
        computing_node = data1["computing_node"].get<std::string>();
        computing_node_port = data1["computing_node_port"].get<int>();
        //保存客户端IP列表
        for (int index = 0; index < data1["addr_task_pair"].size(); index++)
        {
            ADDR_TASK_PAIR tmp_addrtaskPair;
            tmp_addrtaskPair.rtsp = data1["addr_task_pair"][index]["rtsp"].get<std::string>();
            std::string rtmp_name = data1["addr_task_pair"][index]["rtmp"].get<std::string>();
            tmp_addrtaskPair.rtmp = rtmp_name;
            //tmp_addrtaskPair.rtmp = "rtmp://" + computing_node + ":" + std::to_string(en_port) + "/live/" + rtmp_name;
            std::vector<int> tmp_taskList = data1["addr_task_pair"][index]["tasks"];
            tmp_addrtaskPair.taskList = tmp_taskList;
            task_pair_vec.push_back(tmp_addrtaskPair);
            // for(int i=0;i<tmp_taskList.size();i++)
            // {
            //    std::cout<<tmp_taskList[i]<<std::endl;
            // }
            //ip_task ====> queue<cv::Mat>
            // std::queue<cv::Mat> queueMat;
            // addr_queue_map.insert(std::map<std::string,std::queue<cv::Mat>>::value_type(tmp_addrtaskPair.rtsp,queueMat));
            // addr_mtx_map.insert(std::map<std::string,std::mutex*>::value_type(tmp_addrtaskPair.rtsp,new std::mutex()));
        }
    }
    catch(...)
    {
        return -1;
    }
    int ret = getJson("../config/tasks.json",tasks_json);
    if(ret < 0)
    {
        return -1;
    }
    ret = getJson("../config/color.json",color_json);
    if(ret < 0)
    {
        return -1;
    }
    return 0;
}

void recording_breathe_log(std::string ip, int port)
{
    std::string json_new = "";
    std::fstream fs(en_breathe_path); 
    if(fs.fail()){      //创建
        nlohmann::json data;
        data["time_step"] = breathe_time_step;
        std::string str_time = get_localtime();
        data["fresh_air"] = str_time;      
        data["connection_failure"] = {};
        json_new = data.dump(4);
        std::fstream fs3;
        fs3.open(en_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
        fs3.write(json_new.c_str(), json_new.length());
        fs3.close();

    }else{             //添加
        std::stringstream ss; 
        ss << fs.rdbuf(); 
        std::string json_status_config = ss.str(); 
        fs.close();
        try{
            auto data = nlohmann::json::parse(json_status_config); 
            std::string str_time = get_localtime();
            if(ip == ""){
                data["fresh_air"] = str_time;
            }else{
                nlohmann::json addr_json;
                addr_json["ip"] = ip;
                addr_json["port"] = port;
                addr_json["time"] = str_time;
                if(data["connection_failure"].size() < log_list_len){
                    data["connection_failure"].push_back(addr_json);
                }else{
                    data["connection_failure"].erase(data["connection_failure"].begin());
                    data["connection_failure"].push_back(addr_json);
                }                          
            }           
            json_new = data.dump(4);
        }catch(...){
            return;
        }  
        
        std::ofstream ofs1 (en_breathe_path, std::ios::out | std::ios::trunc); 
        ofs1.close();
        std::fstream fs3;
        fs3.open(en_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
        fs3.write(json_new.c_str(), json_new.length());
        fs3.close();
    }
}