#include "common.h"

std::string cn_init_local_path = "../config/cn_init_local.config";  //本地配置文件路径
std::string cn_init_path = "";  //启动设置配置文件路径
std::string cn_start_path = "";   //计算节点异常启动文件
std::string cn_breathe_path = "";   //计算节点心跳log文件
std::string config_json_path = "";   //算法配置文件
std::string sg_dir = "";
int queue_len = 0;                   //所有队列长度
int breathe_time_step = 0;           //心跳时间间隔
int tcp_timeout = 0;                 //TCP断线重连时间
int cn_port = 0;                     //本地TCP服务端口
int log_list_len = 0;                //心跳日志中端口列表最大长度
int zone_offset = 0;                 //时区偏移，单位（小时）
std::map<std::string, int> decoding_ip_port_map;  //解码服务ip和端口数组
std::map<int, std::vector<float>> tasks_info;     //要启用的算法任务和阈值数组
std::map<std::string, struct Ip_Info> ip_info_map;  //摄像头对应的信息map
std::mutex ip_info_map_mtx;                         //摄像头对应的信息map锁


std::string get_localtime()     //获取本地当前时间,默认为北京时间
{
    auto time_m = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::string milliseconds_str = std::to_string(time_m % 1000);
    if (milliseconds_str.length() < 3) {
		milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
	}

    auto time_s = time_m/1000 + zone_offset*3600;       //东八区偏移
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_s), "%Y-%m-%dT%H:%M:%S.");
    std::string time_now =  ss.str() + milliseconds_str;
    std::cout<<"======time_now:"<<time_now<<"======"<<std::endl;
    return time_now;
}

void generate_start_flag()
{ 
    //std::cout<<cn_start_path<<std::endl;
    std::string json_new = "";
    std::fstream fs;
    fs.open(cn_start_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
    fs.write(json_new.c_str(), json_new.length());
    fs.close(); 
}


void recording_breathe_log(std::string ip, int port)
{
    std::string json_new = "";
    std::fstream fs(cn_breathe_path); 
    if(fs.fail()){      //创建
        nlohmann::json data;
        data["time_step"] = breathe_time_step;
        std::string str_time = get_localtime();
        data["fresh_air"] = str_time;      
        data["connection_failure"] = {};
        json_new = data.dump(4);
        std::fstream fs3;
        fs3.open(cn_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
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
        
        std::ofstream ofs1 (cn_breathe_path, std::ios::out | std::ios::trunc); 
        ofs1.close();
        std::fstream fs3;
        fs3.open(cn_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
        fs3.write(json_new.c_str(), json_new.length());
        fs3.close();
    }
}


int load_computing_config() 
{
    std::fstream fs0(cn_init_local_path); 
    if(fs0.fail()){ 
        return -1;
    }
    std::stringstream ss0; 
    ss0 << fs0.rdbuf();
    std::string json_in0 = ss0.str(); 
    fs0.close();
    try{
        auto data0 = nlohmann::json::parse(json_in0);       
        cn_init_path = data0.at("cn_init_path");
        cn_start_path = data0.at("cn_start_path");
        cn_breathe_path = data0.at("cn_breathe_path");
        queue_len = data0.at("queue_len");
        log_list_len = data0.at("log_list_len");
        zone_offset = data0.at("zone_offset");
    }catch(...){      
        return -1;
    } 


    std::fstream fs(cn_init_path); 
    if(fs.fail()){        
        return -1;
    }
    std::stringstream ss; 
    ss << fs.rdbuf();
    std::string json_in = ss.str(); 
    fs.close();
 
    ip_info_map_mtx.lock();
    ip_info_map.erase(ip_info_map.begin(),ip_info_map.end());
    ip_info_map_mtx.unlock();
    try{  
        auto data = nlohmann::json::parse(json_in);  
        tcp_timeout = data.at("tcp_timeout");
        breathe_time_step = data.at("breathe_time_step");
        sg_dir = data.at("sg_path");
        cn_port = data.at("cn_port");    
        config_json_path = sg_dir + "/deployment.json";
        std::vector<int> tasks_vec;
        for(int i=0;i<data["tasks_info"].size();i++){
            int task = data["tasks_info"][i].at("task");
            std::vector<float> threshold_vec;
            for(int j=0;j<data["tasks_info"][i]["threshold"].size();j++){
                float threshold = data["tasks_info"][i]["threshold"][j];
                threshold_vec.push_back(threshold);
            }
            tasks_info.insert(std::map<int, std::vector<float>>::value_type(task, threshold_vec));  
            tasks_vec.push_back(task);  
        }

        std::fstream fs1(config_json_path); 
        if(fs1.fail()){ 
            return -1;
        }
        std::stringstream ss1; 
        ss1 << fs1.rdbuf(); 
        std::string json_config = ss1.str(); 
        fs1.close();
        try{
            auto data_config = nlohmann::json::parse(json_config); 
            data_config.at("configs_path") = sg_dir + "/configs";
            data_config.at("models_path") = sg_dir + "/models";
            data_config.at("tasks_list") = tasks_vec;
            std::string out_json = data_config.dump(4);
            std::ofstream ofs2 (config_json_path, std::ios::out | std::ios::trunc); 
            ofs2.close();
            std::fstream fs2;
            fs2.open(config_json_path, std::fstream::binary | std::fstream::out | std::fstream::app); 
            fs2.write(out_json.c_str(), out_json.length());
            fs2.close();
        }catch(...){      
            return -1;
        } 

        for(int i=0;i<data["decoding_node_list"].size();i++){
            std::string ip = data["decoding_node_list"][i].at("ip");
            int port = data["decoding_node_list"][i].at("port");
            decoding_ip_port_map.insert(std::map<std::string, int>::value_type(ip, port));
            for(int j=0;j<data["decoding_node_list"][i]["video_computing_pair"].size();j++)
            {
                std::string rtsp = data["decoding_node_list"][i]["video_computing_pair"][j].at("addr");
                Ip_Info info;
                info.task_key = 0;
                if(data["decoding_node_list"][i]["video_computing_pair"][j]["tasks"].size() != data["decoding_node_list"][i]["video_computing_pair"][j]["roi_list"].size()){
                    return -1;
                }
                for(int k=0;k<data["decoding_node_list"][i]["video_computing_pair"][j]["tasks"].size();k++){
                    info.tasks.push_back(data["decoding_node_list"][i]["video_computing_pair"][j]["tasks"][k]);
                }
                for(int k=0;k<data["decoding_node_list"][i]["video_computing_pair"][j]["roi_list"].size();k++){
                    std::vector<float> roi;
                    for(int t=0;t<data["decoding_node_list"][i]["video_computing_pair"][j]["roi_list"][k].size();t++){
                        roi.push_back(data["decoding_node_list"][i]["video_computing_pair"][j]["roi_list"][k][t]);
                    }
                    info.rois.push_back(roi);
                }
                ip_info_map_mtx.lock();
                ip_info_map.insert(std::map<std::string, struct Ip_Info>::value_type(rtsp, info));    
                ip_info_map_mtx.unlock();      
            }
        }       
    }catch(...){      
        return -1;
    }   

    return 0;
}