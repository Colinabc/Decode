#include "tcp_client.h"

//#define DECODE_TCP_PORT 6778   //要访问的解码TCP端口
#define MAX_RECV_LEN 4096 //一次接收的最大长度


std::queue<std::string> head_queue;
std::mutex head_lock;
// std::queue<std::string> data_queue;
// std::mutex data_lock;

struct message_head     //接收的tcp数据头
{
    unsigned int image_size;        //图片数据长度
    unsigned int image_width;       //宽
    unsigned int image_height;      //高
    unsigned int image_channel;     //色域通道数
    unsigned int fps;               //视频原始FPS
    char addr[256];                 //rtsp
    char time[256];                 //时间戳
};


void draw_label(cv::Mat &img, std::string label, cv::Point p1, std::vector<int> color)
{
    int font = cv::FONT_HERSHEY_COMPLEX;
    double font_scale = 2;
    int thickness = 2;
    int baseline;
    cv::Size text_size = cv::getTextSize(label, font, font_scale, thickness, &baseline);

    cv::Point o = p1;
    if (o.y - text_size.height < 0)
    {
        o.y = p1.y + text_size.height;
    }
    if (o.x + text_size.width > img.cols)
    {
        o.x = img.cols - text_size.width;
    }

    cv::Point o1 = {o.x, o.y - text_size.height};
    cv::Point o2 = {o.x + text_size.width, o.y};
    //cv::rectangle(img, o1, o2, cv::Scalar(color[0],color[1],color[2]), -1);
    cv::putText(img, label, o, font, font_scale, cv::Scalar(color[0], color[1], color[2]), thickness);
}

void draw_img(cv::Mat &img, std::string jsondata,std::string rtsp, std::vector<int> taskList)
{
    try
    {
        auto data = nlohmann::json::parse(jsondata);
        auto tasks_dict = nlohmann::json::parse(tasks_json);
        auto color_dict = nlohmann::json::parse(color_json);
        int task = data.at("task");
        std::string rtsp_in = data.at("addr");
        if(std::find(taskList.begin(),taskList.end(),task) != taskList.end() && rtsp == rtsp_in) //check rtsp+task
        {
            int results = data.at("result_warning");
            std::string label = "";
            int size_line = 2;
            for (int i = 0; i < tasks_dict["task_list"].size(); i++)
            {
                if (task == tasks_dict["task_list"][i].at("task_key"))
                {
                    label = tasks_dict["task_list"][i].at("task_value");
                    break;
                }
            }
            //std::cout << label<<std::endl;
            if (data["results"].size() == 0)
            {
                if (results == 0)
                {
                    std::vector<int> color = color_dict.at("no_alarm_font");
                    cv::Point p(0, int(img.rows / 2));
                    draw_label(img, "WATCHING", p, color);
                }
                else
                {
                    std::vector<int> color = color_dict.at("alarm_font");
                    cv::Point p(0, int(img.rows / 2));
                    draw_label(img, label, p, color);
                }
                return;
            }
            for (int i = 0; i < data["results"].size(); i++)
            {
                if (data["results"][i]["b_box"].size() != 0)
                {
                    cv::Point p1(data["results"][i]["b_box"][0], data["results"][i]["b_box"][1]);
                    cv::Point p2(data["results"][i]["b_box"][2], data["results"][i]["b_box"][3]);
                    if (data["results"][i].at("cls") == 0)
                    {
                        std::vector<int> color = color_dict.at("no_alarm_rectangle");
                        cv::rectangle(img, p1, p2, cv::Scalar(color[0], color[1], color[2]), size_line);
                    }
                    else
                    {
                        std::vector<int> color = color_dict.at("alarm_rectangle");
                        cv::rectangle(img, p1, p2, cv::Scalar(color[0], color[1], color[2]), size_line);
                        std::vector<int> color_label = color_dict.at("alarm_font");
                        draw_label(img, label, p1, color_label);
                    }
                    if (data["results"][i]["areas"].size() != 0)
                    {
                        for (int j = 0; j < data["results"][i]["areas"].size(); j++)
                        {
                            std::vector<std::vector<cv::Point>> contours;
                            for (int k = 0; k < data["results"][i]["areas"][j].size(); k++)
                            {
                                std::vector<cv::Point> contour;
                                for (int t = 0; t < data["results"][i]["areas"][j][k].size(); t++)
                                {
                                    cv::Point p(data["results"][i]["areas"][j][k][t][0], data["results"][i]["areas"][j][k][1]);
                                    contour.push_back(p);
                                }
                                contours.push_back(contour);
                            }
                            std::vector<int> color = color_dict.at("alarm_areas");
                            cv::polylines(img, contours, true, cv::Scalar(color[0], color[1], color[2]), size_line, cv::LINE_AA);
                            cv::fillPoly(img, contours, cv::Scalar(color[0], color[1], color[2]));
                        }
                    }
                }
            }
       }

    }
    catch (...)
    {
        return;
    }
}


void tcp_client(std::string ip, int port)
{
    generate_start_flag();
    recording_breathe_log("",-1);
    int count =0;
    while (1)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0); //若成功则返回一个sockfd（套接字描述符）
        if (sockfd < 0)
        {
            recording_breathe_log(ip,port);
            sleep(reconnTime);
            continue;
        }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            recording_breathe_log(ip,port);
            close(sockfd);
            sleep(reconnTime);
            continue;
        }
        std::cout<<"connect succcess!"<<std::endl;
        int ret = 0;
        int num = 0;
        while (1)
        {
            usleep(0.01);
            char recv_first[32] = {0};
            ret = recv(sockfd, recv_first, 1, 0);
            if (ret <= 0)
            {
                break;
            }
            if (strcmp(recv_first, "{") == 0)
            {
                char recvdata[2] = {0};
                char head_str[32] = {0};
                while (1)
                {
                    ret = recv(sockfd, recvdata, 1, 0);
                    if (strcmp(recvdata, "}") != 0)
                    {
                        strcat(head_str, recvdata);
                    }
                    else
                    {
                        break;
                    }
                }
                int head_len = atoi(head_str); //head数据长度，char==>int
                //printf("head_len:%d\n",head_len);
                void *buf = (void *)malloc(head_len);
                memset(buf, 0, head_len);
                char *ptr = (char *)buf;
                int total_len = head_len;
                while (1)
                {
                    ret = recv(sockfd, (void *)ptr, total_len, 0);
                    if (ret < total_len)
                    {
                        total_len = total_len - ret;
                        ptr += ret;
                    }
                    else
                    {
                        total_len = total_len - ret;
                        ptr += ret;
                        break;
                    }
                }
                if (total_len != 0)
                {
                    break;
                }
                std::string head = "";
                head = std::string((char *)buf, head_len);
                free(buf);
                buf = NULL;
                head_lock.lock();
                if(head_queue.size() >= queue_len){
                    head_queue.pop();
                }
                head_queue.push(head);
                head_lock.unlock();
                continue;
            }
            else if (strcmp(recv_first, "[") == 0)
            {
                char recvdata[2] = {0};
                char head_str[32] = {0};
                while (1)
                {
                    memset(recvdata, 0, sizeof(recvdata));
                    ret = recv(sockfd, recvdata, 1, 0);
                    if (strcmp(recvdata, "]") != 0)
                    {
                        strcat(head_str, recvdata);
                    }
                    else
                    {
                        break;
                    }
                }
                int head_len = atoi(head_str);
                //printf("data_len:%d\n",head_len);
                memset(recvdata, 0, sizeof(recvdata));
                ret = recv(sockfd, recvdata, 1, 0);
                if (ret <= 0)
                {
                    break;
                }
                if (strcmp(recvdata, "[") != 0)
                {
                    break;
                }
                void *buf = (void *)malloc(head_len);
                memset(buf, 0, head_len);
                char *ptr = (char *)buf;
                int total_len = head_len;
                while (1)
                {
                    ret = recv(sockfd, (void *)ptr, total_len, 0);
                    if (ret < total_len)
                    {
                        total_len = total_len - ret;
                        ptr += ret;
                    }
                    else
                    {
                        total_len = total_len - ret;
                        ptr += ret;
                        break;
                    }
                }
                if (total_len != 0)
                {
                    break;
                }
                std::string data = "";
                data = std::string((char *)buf, head_len);
                free(buf);
                buf = NULL;
                memset(recvdata, 0, sizeof(recvdata));
                ret = recv(sockfd, recvdata, 1, 0);
                if (ret <= 0)
                {
                    break;
                }
                if (strcmp(recvdata, "]") != 0)
                {
                    break;
                }
                // data_lock.lock();
                // if(data_queue.size() >= queue_len)
                // {
                //     data_queue.pop();
                // }
                // data_queue.push(data);
                // data_lock.unlock();
                continue;
            }
            else
            {
                break;
            }
        }
        close(sockfd);
        sleep(reconnTime);
    }

    std::cout << "===thread_tcp_client end!===" << std::endl;
    return;
}



// int recv_data_from_socket(int sockfd, char *data, int data_len)
// {
// 	int count = 0;
// 	int rest_size = data_len;
// 	char* ptr = data;
// 	int ret;
// 	while(1){
//         ret = recv(sockfd, (void *)ptr, rest_size, 0);
//         if(ret <= 0){
//             count = 0;
//             break;
//         }
//         ptr += ret;
//         count += ret;
//         if(count == data_len){
//             break;
//         }	          
//         rest_size -= ret;  
// 	}
// 	return count;
// }


// void tcp_client_dn(std::string ip, int port)
// {
//     std::cout <<"=====ip:"<<ip<<std::endl;
//     std::cout <<"=====port:"<<port<<std::endl;
//     while(1)
//     {
//         int sockfd = socket(AF_INET, SOCK_STREAM, 0);//若成功则返回一个sockfd（套接字描述符）
//         if (sockfd < 0)
//         {
          
//             continue;
//         }
        
//         struct sockaddr_in server_addr;
//         server_addr.sin_family = AF_INET;
//         server_addr.sin_port = htons(port);
//         server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
//         if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
//         {          
//             continue;
//         }
//         std::cout <<"=====connect"<<std::endl;
//         message_head head;
//         int len = 0;
//         int num = 0;
//         //auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//         while (1)
//         {
//             usleep(0.001);
//             memset(&head, 0x0, sizeof(struct message_head));
//             try{
//                 len = recv_data_from_socket(sockfd, (char*)(&head), sizeof(struct message_head));
//             }catch(...){      
//                 printf("catch head error\n");
//                 break;
//             }          
//             if(len != sizeof(struct message_head))
//             {

//                 printf("recv head error, len:%d\n",len);
//                 break;
//             }
//             int length = head.image_size;
//             void* buf = (void*)malloc(length);
//             memset(buf, 0x0, length);
//             try{
//                 len = recv_data_from_socket(sockfd, (char *)buf, length);
//             }catch(...){      
//                 printf("catch data error\n");
//                 free(buf);
//                 buf = NULL;
//                 break;
//             }     
//             if(len == length)
//             {
//                 try{
//                     std::string img_data = std::string((char*)buf, length);
//                     cv::Mat img = Base2Mat(img_data);
//                     std::string rtsp = head.addr;
//                     auto iter = addr_queue_map.find(rtsp);
//                     if(iter != addr_queue_map.end())
//                     {
//                         //std::cout << "======="<<std::endl;
//                         addr_mtx_map[rtsp]->lock();
//                         if(addr_queue_map[rtsp].size() >= queue_len){
//                             addr_queue_map[rtsp].pop();
//                         }       
//                         addr_queue_map[rtsp].push(img);                               
//                         addr_mtx_map[rtsp]->unlock();
//                     }                
//                 }catch(...){      
//                     printf("catch img error\n");
//                     free(buf);
//                     buf = NULL;
//                     break;
//                 }     
                
//             }else{
//                 printf("recv incomplete data\n");
//                 free(buf);
//                 buf = NULL;
//                 break;
//             }
//             free(buf);
//             buf = NULL;
//         }
//         std::cout<<"Server Closed!:"<<ip<<std::endl;
//         close(sockfd);
    
//     }  

//     //std::cout << "===thread_tcp_client end!===" <<std::endl;
//     return;
// }
