#include "tcp_client.h"

//#define DECODE_TCP_PORT 6778   //要访问的解码TCP端口
#define MAX_RECV_LEN 4096      //一次接收的最大长度

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

std::queue<struct Frame_Info> frame_info_queue;  //接收队列
std::mutex frame_info_queue_mtx;         


int recv_data_from_socket(int sockfd, char *data, int data_len)
{
	int count = 0;
	int rest_size = data_len;
	char* ptr = data;
	int ret;
	while(1){
        ret = recv(sockfd, (void *)ptr, rest_size, 0);
        if(ret <= 0){
            count = 0;
            break;
        }
        ptr += ret;
        count += ret;
        if(count == data_len){
            break;
        }	          
        rest_size -= ret;  
	}
	return count;
}


void tcp_client(std::string ip, int port)
{
    while(1)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);//若成功则返回一个sockfd（套接字描述符）
        if (sockfd < 0)
        {
            recording_breathe_log(ip, port);
            sleep(tcp_timeout);
            continue;
        }
        // struct timeval timeout = {1,0};  
        // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) != 0){
        //     printf("set accept timeout failed");
        // }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        {
            recording_breathe_log(ip, port);
            close(sockfd);
            sleep(tcp_timeout);
            continue;
        }

        message_head head;
        int len = 0;
        int num = 0;
        auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        while (1)
        {
            usleep(0.001);
            memset(&head, 0x0, sizeof(struct message_head));
            try{
                len = recv_data_from_socket(sockfd, (char*)(&head), sizeof(struct message_head));
            }catch(...){      
                printf("catch head error\n");
                break;
            }          
            if(len != sizeof(struct message_head))
            {

                printf("recv head error, len:%d\n",len);
                break;
            }
            int length = head.image_size;
            void* buf = (void*)malloc(length);
            memset(buf, 0x0, length);
            try{
                len = recv_data_from_socket(sockfd, (char *)buf, length);
            }catch(...){      
                printf("catch data error\n");
                free(buf);
                buf = NULL;
                break;
            }     
            if(len == length)
            {
                try{
                    std::string img_data = std::string((char*)buf, length);
                    cv::Mat img = Base2Mat(img_data);
                    
                    Frame_Info info;
                    info.frame = img.clone();
                    info.addr = head.addr;
                    info.fps = head.fps;
                    info.time = head.time;
                    frame_info_queue_mtx.lock();
                    if(frame_info_queue.size() >= queue_len){
                        frame_info_queue.pop();
                    }       
                    frame_info_queue.push(info);                               
                    frame_info_queue_mtx.unlock();
                    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    if(((time2-time1)/1000) >= 1){
                        std::cout<<"recv_fps:"<<num<<" ,ip:"<<ip<<std::endl;
                        time1 = time2;
                        num = 0;
                    }                      
                    num ++;
                }catch(...){      
                    printf("catch img error\n");
                    free(buf);
                    buf = NULL;
                    break;
                }     
                
            }else{
                printf("recv incomplete data\n");
                free(buf);
                buf = NULL;
                break;
            }
            free(buf);
            buf = NULL;
        }
        std::cout<<"Server Closed!:"<<ip<<std::endl;
        close(sockfd);
        sleep(tcp_timeout);     
       
    }  

    //std::cout << "===thread_tcp_client end!===" <<std::endl;
    return;
}
