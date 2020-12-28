#include "run.h"

using namespace libadapter;

extern std::queue<struct Frame_Info> frame_info_queue;  //接收队列
extern std::mutex frame_info_queue_mtx;                //接收队列锁
extern std::queue<std::string> send_head_queue; //发送队列数据头
extern std::mutex send_head_queue_mtx;          //发送队列数据头锁
extern std::queue<cv::Mat> send_data_queue;      //发送队列数据体
extern std::mutex send_data_queue_mtx;               //发送队列数据体锁


void writejson(FetchedPackage fp, Frame_Info frame_Info, int task, std::vector<float> roi_vec)
{
    nlohmann::json str;    
    str["addr"] = frame_Info.addr;
    str["image_width"] = frame_Info.frame.cols;
    str["image_height"] = frame_Info.frame.rows;
    str["image_channel"] = frame_Info.frame.channels();
    str["fps"] = frame_Info.fps;
    str["task"] = task;
    str["time"] = frame_Info.time;
    str["result_warning"] = fp.cls;
    str["roi"] = roi_vec;
    if(fp.results.size() == 0)
    {
        std::vector<int> empty_vec;
        str["results"] = empty_vec;
    }else
    {
        for(auto &result: fp.results) 
        {
            nlohmann::json str_result;
            str_result["cls"] = result.bbox.cls;
            std::vector<int> b_box;
            b_box.push_back(result.bbox.x1);
            b_box.push_back(result.bbox.y1);
            b_box.push_back(result.bbox.x2);
            b_box.push_back(result.bbox.y2);
            str_result["b_box"] = b_box;
            if(result.bbox.scores.size() == 0){
                std::vector<float> scores;
                str_result["scores"] = scores;
            }else{
                str_result["scores"] = result.bbox.scores;
            }         
            if(result.ptss.size() == 0){
                std::vector<int> empty_vec;
                str_result["areas"] = empty_vec;
            }else{
                for(auto &pts: result.ptss) 
                {
                    std::vector<std::vector<int>> points;
                    for(auto pt: pts) 
                    {
                        std::vector<int> point;
                        point.push_back(pt.x);
                        point.push_back(pt.y);
                        points.push_back(point);
                    }
                    str_result["areas"].push_back(points);
                }
            }          
            str["results"].push_back(str_result);
        }
    }
    
  
    std::string out = str.dump();
    std::string json_head = "{" + std::to_string(out.length()) + "}" + out;  //json头
 
    send_head_queue_mtx.lock();
    if (send_head_queue.size() >= queue_len){
        send_head_queue.pop();
    }                                 
    send_head_queue.push(json_head);
    send_head_queue_mtx.unlock();  

    send_data_queue_mtx.lock();
    if (send_data_queue.size() >= queue_len){
        send_data_queue.pop();
    }                                 
    send_data_queue.push(frame_Info.frame.clone());
    send_data_queue_mtx.unlock();  
    
    return;
}


void pushing(Scheduler& s) 
{ 
    recording_breathe_log("",-1);         //记录开始心跳时间
    auto time_m = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    auto time_start = time_m/1000;
    int key_id = 0;
    while(1) 
    {    
        usleep(0.001);
        frame_info_queue_mtx.lock();                  
        if (!frame_info_queue.empty())
        {
            auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            auto time_b = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            auto time_breathe = time_b/1000;
            if((time_breathe-time_start) >= (breathe_time_step*60)){
                time_start = time_breathe;
                recording_breathe_log("",-1);         
            }
           
            Frame_Info frame_Info = frame_info_queue.front(); 
            frame_info_queue.pop();                              
            frame_info_queue_mtx.unlock(); 

            ip_info_map_mtx.lock(); 
            std::map<std::string, struct Ip_Info>::iterator ite = ip_info_map.find(frame_Info.addr);
            if(ite == ip_info_map.end()){
                ip_info_map_mtx.unlock();
                continue;
            }
       
            if(ip_info_map[frame_Info.addr].task_key >= ip_info_map[frame_Info.addr].tasks.size()){
                ip_info_map[frame_Info.addr].task_key = 0;
            }
            int task_key = ip_info_map[frame_Info.addr].task_key;
            int task_id = ip_info_map[frame_Info.addr].tasks[ip_info_map[frame_Info.addr].task_key];
            std::vector<float> roi_vec = ip_info_map[frame_Info.addr].rois[ip_info_map[frame_Info.addr].task_key];           
            ip_info_map[frame_Info.addr].task_key ++;              
            ip_info_map_mtx.unlock(); 

            std::string task = "";
            if (task_id == 1){
                task = "helmet";
            }else if (task_id == 2){
                task = "uniform";
            }else if (task_id == 3){
                task = "vest";
            }else if (task_id == 6){
                task = "invasion";
            }else if (task_id == 7){
                task = "skin";
            }else if (task_id == 8){
                task = "fire";
            }
            if (key_id == 100000){
                key_id = 0;
            }
            key_id ++;     

            PushedPackage pp;    
            pp.image = frame_Info.frame.clone();        
            pp.key = to_string(key_id);
            pp.camera_ip = frame_Info.addr;
            if (task == "helmet"){
                pp.task = Task::PERSON_HELMET;
            }else if (task == "uniform"){
                pp.task = Task::PERSON_UNIFORM;
            }else if (task == "vest"){
                pp.task = Task::PERSON_VEST;
            }else if (task == "invasion"){
                pp.task = Task::PERSON_INVASION;
            }else if (task == "skin"){
                pp.task = Task::SKIN_EXPOSURE;
            }else if (task == "fire"){
                pp.task = Task::FIRE_SMOKE;
            } 
           
            s.push(pp);  
            FetchedPackage fp;
            s.fetch(fp, to_string(key_id)); 
            
            //std::cout<<"key_id:"<<key_id<<" ,task:"<<task<<" ,addr:"<<frame_Info.addr<<std::endl;
            writejson(fp, frame_Info, task_id, roi_vec);   
            //auto time3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            //std::cout<<"===time3-1:"<<time3-time1<<", id:"<<task_id<<std::endl;    
        }
        else{
            frame_info_queue_mtx.unlock();
        }    
    }
    std::cout<<"===thread_pushing end!==="<<std::endl;
    return;
}


void enroll_tasks(Scheduler &s) 
{      

    std::string config_uniform_path = sg_dir + "/configs/caisa_uniform_config.json"; //算法配置文件路径(工作服)
    std::string config_vest_path = sg_dir + "/configs/caisa_vest_config.json";  //算法配置文件路径(反光衣)
    auto ca = make_shared<CourierClothing>(config_uniform_path);
    s.enroll(ca, Task::PERSON_HELMET);
    auto cb = make_shared<CourierClothing>(config_uniform_path);
    s.enroll(cb, Task::PERSON_UNIFORM);
    auto cc = make_shared<CourierClothing>(config_vest_path);
    s.enroll(cc, Task::PERSON_VEST);
    auto cf = make_shared<CourierInvasion>(config_uniform_path);
    s.enroll(cf, Task::PERSON_INVASION);
    auto cg = make_shared<CourierSkin>(config_uniform_path);
    s.enroll(cg, Task::SKIN_EXPOSURE);
    auto ch = make_shared<CourierFire>(config_uniform_path);
    s.enroll(ch, Task::FIRE_SMOKE);

}

void thread_tcp_client_recv(std::string decoding_ip, int port)
{
    std::cout<<decoding_ip<<"=============="<<port<<std::endl;
    tcp_client(decoding_ip, port);
}


void thread_tcp_client_recv_list()
{
    for(auto iter = decoding_ip_port_map.begin(); iter != decoding_ip_port_map.end(); iter++)
    {
        std::thread tcp_handle = std::thread(thread_tcp_client_recv, iter->first, iter->second);
        tcp_handle.detach();
    }
}

void run()  //初始化算法
{   

    std::string config_uniform_path = sg_dir + "/configs/caisa_uniform_config.json"; //算法配置文件路径(工作服)
    std::string config_vest_path = sg_dir + "/configs/caisa_vest_config.json";  //算法配置文件路径(反光衣)
    
    Scheduler scdu(config_uniform_path);
    enroll_tasks(scdu);      
    scdu.start();
    generate_start_flag();
    std::thread recv_handle = std::thread(thread_tcp_client_recv_list);
    recv_handle.detach();
    pushing(scdu);

    return;
}
