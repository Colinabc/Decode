#include "run.h"


void thread_tcp_service()
{
    tcp_service();
}

void thread_computing()
{
    run();
}



int main(int argc, char* argv[])
{   

    int ret = load_computing_config();
    if(ret < 0){
        printf("CN Init Filed\n");
        return ret; 
    }

    std::thread send_handle = std::thread(thread_tcp_service);   
    std::thread computing_handle = std::thread(thread_computing);  
    computing_handle.join();
    send_handle.join(); 
    
    return 0;
}