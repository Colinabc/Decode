#include "tcp_service.h"


#define QUEUE 200
#define MAX_RECV_LEN 4096  //一次接收的最大长度

std::queue<std::string> send_head_queue; //发送队列数据头
std::mutex send_head_queue_mtx;          //发送队列数据头锁
std::queue<cv::Mat> send_data_queue;      //发送队列数据体
std::mutex send_data_queue_mtx;               //发送队列数据体锁
std::mutex dif_lock;     //互斥锁,防止多客户端


int select_socket(int conn)
{
    fd_set fds;
    timeval tv;
    FD_ZERO(&fds); 
    FD_SET(conn,&fds);
    tv.tv_sec = tv.tv_usec = 0;
    int ret = select(conn+1,&fds,NULL,NULL,&tv);
    return ret;
}

void thread_send(int conn)
{
    int num = 0;
    while(1)
    {
        usleep(0.01);
        dif_lock.lock();
        send_head_queue_mtx.lock();
        if (!send_head_queue.empty())
        {            
            std::string json_head = send_head_queue.front();  
            send_head_queue.pop();
            send_head_queue_mtx.unlock(); 
            int disconn_flag = select_socket(conn);
            if(disconn_flag != 0){
                send_data_queue_mtx.lock();
                if (!send_data_queue.empty()){
                    cv::Mat img = send_data_queue.front();  
                    send_data_queue.pop();
                    send_data_queue_mtx.unlock(); 
                }else{
                    send_data_queue_mtx.unlock();
                }
                dif_lock.unlock();
                break;
            }
            
            int ret = send(conn, json_head.c_str(), json_head.length(),0);
            if(ret <= 0){
                send_data_queue_mtx.lock();
                if (!send_data_queue.empty()){
                    cv::Mat img = send_data_queue.front();  
                    send_data_queue.pop();
                    send_data_queue_mtx.unlock(); 
                }else{
                    send_data_queue_mtx.unlock();
                }
                dif_lock.unlock();
                break;
            }          
     
        }else{
            send_head_queue_mtx.unlock();
        }

        send_data_queue_mtx.lock();
        if (!send_data_queue.empty())
        {            
            cv::Mat img = send_data_queue.front();  
            send_data_queue.pop();
            send_data_queue_mtx.unlock();   
            std::string img_data = Mat2Base64(img, "jpg");  //转成jpg字符串  
            std::string json_data = "[" + std::to_string(img_data.length()) + "]" + "[" + img_data + "]";  //图片体   
            int disconn_flag = select_socket(conn);
            if(disconn_flag != 0){
                dif_lock.unlock();
                break;
            }
            int ret = send(conn, json_data.c_str(), json_data.length(),0);
            if(ret <= 0){
                dif_lock.unlock();
                break;
            }                
            num ++;
        }else{
            send_data_queue_mtx.unlock();
        }
        dif_lock.unlock();
    }
    printf("Client Closed!\n");
    close(conn); 
    return;
}


void tcp_service()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);//若成功则返回一个sockfd（套接字描述符）
    if (sockfd < 0)
    {       
        exit(1);
    }
    struct timeval timeout = {1,0};  
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&timeout, sizeof(struct timeval)) != 0){
        printf("set accept timeout failed");
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(cn_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        close(sockfd);
        exit(1);
    }
    if (listen(sockfd, QUEUE) == -1)
    {
        close(sockfd);
        exit(1);
    }
    socklen_t addr_len = sizeof(server_addr);
    int nRet = getsockname(sockfd, (struct sockaddr*)&server_addr, &addr_len);
    if (nRet == -1)
    {
        close(sockfd);
        exit(1);
    }
    printf("Starting send_tcp_service listen at:  %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    while (1)
    {   
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);   
        struct timeval timeout = {1,0};  
        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) != 0){
            printf("set accept timeout failed");
        }  
        int conn = accept(sockfd, (struct sockaddr*)&client_addr, &length);
        if (conn < 0){
            continue;
        }
        printf("===============conn:%d\n",conn);
        std::thread thread_handle = std::thread(thread_send, conn);
        thread_handle.detach();           
    }
 
    close(sockfd);
    std::cout << "===thread_send_tcp_service end!===" <<std::endl;
    return;
}

