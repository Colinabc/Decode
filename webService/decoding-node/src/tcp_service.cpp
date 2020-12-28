#include "tcp_service.h"

int select_socket(int conn)
{
  fd_set fds;
  timeval tv;
  FD_ZERO(&fds);
  FD_SET(conn, &fds);
  tv.tv_sec = tv.tv_usec = 0;
  int ret = select(conn + 1, &fds, NULL, NULL, &tv);
  return ret;
}

//cameraIP ==> clientIP ==> conn
void tcp_create(int dn_port, int *pconnArray, std::map<std::string, int> clientIP_index_map)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); //若成功则返回一个套接字描述符
  if (sockfd < 0)
  {
    printf("create socket error \n");
    return;
  }
  struct timeval timeout = {1, 0}; //设置阻塞超时时间1秒  .SO_RCVTIMEO
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&timeout, sizeof(struct timeval)) != 0)
  {
    printf("set accept timeout failed \n");
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(dn_port);           //服务端绑定端口
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //绑定服务端IP
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
  {
    printf("bind error! \n");
    close(sockfd);
    return;
  }
  if (listen(sockfd, 2000) == -1) //参数2：监听进程数量
  {
    close(sockfd);
    printf("listen error! \n");
    return;
  }

  socklen_t addr_len = sizeof(server_addr);
  int nRet = getsockname(sockfd, (struct sockaddr *)&server_addr, &addr_len);
  if (nRet == -1)
  {
    printf("get sock_addr error ! \n");
    return;
  }
  printf("start listen server at: %s:%d \n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
  while (1)
  {
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    struct timeval timeout1 = {1, 0}; //设置阻塞超时时间1秒  .SO_RCVTIMEO
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout1, sizeof(struct timeval)) != 0)
    {
      printf("set accept timeout failed \n");
    }
    int conn = accept(sockfd, (struct sockaddr *)&client_addr, &length);

    if (conn < 0)
    {
      continue;
    }

    //实时更新套接字,判断客户端IP是否在配置列表里面
    int tmpIndex = 0;
    for (std::map<std::string, int>::iterator iter1 = clientIP_index_map.begin(); iter1 != clientIP_index_map.end(); iter1++)
    {
      if(iter1->first == inet_ntoa(client_addr.sin_addr))
      {
        tmpIndex = clientIP_index_map[inet_ntoa(client_addr.sin_addr)];
        connArray_mtx.lock();
        pconnArray[tmpIndex] = conn;
        connArray_mtx.unlock();
      }
    }
    std::cout << "accept conn:=====> " << conn << std::endl;
  }
  std::cout << "exit while" << std::endl;

  close(sockfd);
  return;
}

//发送数据，map: conn ===> queue
void tcp_send(int *pconnArray, std::map<std::string, std::queue<FRAMEINFO>> *presultMap, std::map<std::string, std::mutex *> *presultMap_mtx)
{
  auto time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  auto time_start1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  int count = 0;
  while (1)
  {
    usleep(0.001);
    //一次遍历每路相机对应的队列buffer
    for (std::map<std::string, std::queue<FRAMEINFO>>::iterator iter1 = presultMap->begin(); iter1 != presultMap->end(); iter1++)
    {
      std::string cameraIP = iter1->first;
      if (!iter1->second.empty())
      {
        (*presultMap_mtx)[cameraIP]->lock();
        FRAMEINFO tmp_img_info;
        tmp_img_info = iter1->second.front();
        iter1->second.pop();

        (*presultMap_mtx)[cameraIP]->unlock();

        std::string imgData = Mat2Base64(tmp_img_info.frame, "jpg"); //将Mat转成string发送
        MESSAGEHEAD headData;
        headData.image_width = tmp_img_info.frame.cols;
        headData.image_height = tmp_img_info.frame.rows;
        headData.image_channel = tmp_img_info.frame.channels();
        //headData.image_size = headData.image_width * headData.image_height * headData.image_channel;
        headData.image_size = imgData.size();
        headData.fps = tmp_img_info.fps;
        //std::cout << "\tfps: " << headData.fps << std::endl;
        int i;
        for (i = 0; i < tmp_img_info.url.length(); i++)
        {
          headData.addr[i] = tmp_img_info.url[i];
        }
        headData.addr[i] = '\0'; // cameraIP

        int j;
        std::string time = get_localtime();
        for (j = 0; j < time.length(); j++)
        {
          headData.time[j] = time[j];
        }
        headData.time[j] = '\0'; // timestamp

        //cameraIP ==> 多个client(IP+extract_rate),固定不变
        for (std::map<std::string, std::vector<COMPUTNGPARA>>::iterator iter2 = cameraIP_clientIP_map.begin(); iter2 != cameraIP_clientIP_map.end(); iter2++)
        {
          if (iter2->first == cameraIP)
          {

            //std::cout << "\tcameraIP: " << cameraIP << std::endl;
            for (auto &ip_extract : iter2->second)
            {

              //std::cout << "\tclientIP: " << ip_extract.ip << "\t count: " << ip_extract.count << std::endl;
              int tmp_index = clientIP_index_map[ip_extract.ip]; //client IP

              connArray_mtx.lock();
              int tmp_conn = pconnArray[tmp_index];
              connArray_mtx.unlock();

              //seng image
              // std::vector<char> mats;
              //cv::Mat tmp_img = tmp_img_info.frame.clone();
              //mats.insert(mats.end(), tmp_img.data, tmp_img.data + headData.image_size);
              //不同客户端，抽帧不同
              if (ip_extract.count == ip_extract.extraction_rate - 1)
              {
                int disconn_flag = select_socket(tmp_conn);
                if (disconn_flag == 0)
                {
                  if (tmp_conn != -1)
                  {
                    int ret1 = send(tmp_conn, &headData, sizeof(MESSAGEHEAD), 0); //发送头
                    if (ret1 <= 0)
                    {
                      close(tmp_conn);
                      connArray_mtx.lock();
                      pconnArray[tmp_index] = -1;
                      connArray_mtx.unlock();
                      continue;
                    }
                    int ret2 = send(tmp_conn, (char *)imgData.c_str(), headData.image_size, 0); // 发送体
                    if (ret2 <= 0)
                    {
                      close(tmp_conn);
                      connArray_mtx.lock();
                      pconnArray[tmp_index] = -1;
                      connArray_mtx.unlock();
                      continue;
                    }
                    ip_extract.count = 0;
                    count++;

                    auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                    auto time_now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                    auto tmp = (time_now - time_start);
                    time_start = time_now;
                    
                    if ((time_now1 - time_start1) >= 1000) //统计1s发送图片总数
                    {
                      time_start1 = time_now1;
                      std::cout << "tcp send: " << std::endl;
                      std::cout << " \tsend Num Per Second: " << count << std::endl;
                      std::cout << "\tsend conn: =====> " << tmp_conn << " \tindex: " << tmp_index << std::endl;
                      count = 0;
                    }
                  }
                }
                else
                {
                  close(tmp_conn);
                  connArray_mtx.lock();
                  pconnArray[tmp_index] = -1;
                  connArray_mtx.unlock();
                }
              }
              else
              {
                ip_extract.count++;
              }
            }
          }
        }
      }
    }
  }
  printf("Client Closed ! \n");
  return;
}
