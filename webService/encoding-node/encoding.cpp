//#include "tcp_client.h"
#include "cameraEncode.h"

using namespace cv;
void thread_tcp_client()
{
    //计算节点IP+Port
    tcp_client(computing_node, computing_node_port);
    
}

// void thread_tcp_client_dn()
// {
//     int port = 6878;
//     tcp_client_dn(computing_node, port);
// }

void thread_encode(ADDR_TASK_PAIR task_pair, std::string hwType)  
{
    if(hwType == "CPU"){
        sw_encode(task_pair);
    }else if(hwType == "iGPU"){
        hw_encode(task_pair);
    }else{
        return;
    } 
}



int main()
{   
    std::string hwType = "";
    int ret = load_encoding_config(hwType);
    if(ret < 0){
        std::cout<<"EN Init Failure !"<<std::endl;
        return 0;
    }
    

    std::thread p1 = std::thread(thread_tcp_client);  
    //std::thread p2 = std::thread(thread_tcp_client_dn); 
    for(int i=0; i<task_pair_vec.size();i++)
    {
      ADDR_TASK_PAIR tmp_addrtaskPair = task_pair_vec[i];
      std::thread t = std::thread(thread_encode, tmp_addrtaskPair, hwType);
      t.detach();
    }
    
    p1.join();
    //p2.join();

    return 0;
}
