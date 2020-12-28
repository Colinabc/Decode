#include "run.h"

std::string getCurpath()
{
  pid_t pid = getpid();
  char strProcessPath[1024] = {0};
  if (readlink("/proc/self/exe", strProcessPath, 1024) <= 0)
  {
    return NULL;
  }
  char *strProcessName = strrchr(strProcessPath, '/');

  if (!strProcessName)
  {
    printf("当前进程ID：%d\n", pid);
    printf("当前进程名：\n");
    printf("当前进程路径：%s\n", strProcessPath);
  }
  else
  {
    printf("当前进程ID：%d\n", pid);
    printf("当前进程名：%s\n", ++strProcessName);
    printf("当前进程路径：%s\n", strProcessPath);
  }
  std::string path = strProcessPath;
  return path.substr(0, path.find("/build/decoding"));
}

int main(int argc, char *argv[])
{
  // ctrl c signal set
  install_interrupt_callback();

  absPath = getCurpath();
  std::cout << getCurpath() << std::endl;
  //1.读取配置参数
  int ret = load_decoding_config();
  if (ret < 0)
  {
    std::cout << "Fail to load config file!" << std::endl;
    return ret;
  }
  else
  {
    std::cout << "Success to load config file!" << std::endl;
  }

  //2.开启解码线程
  run();

  //3.开启socket accept线程
  std::thread th_tcp_create(tcp_create, dn_port, connArray, clientIP_index_map);
  th_tcp_create.detach();

  //4.开启socket send线程
  std::thread th_tcp_send(tcp_send, connArray, presultMap, presultMap_mtx);
  th_tcp_send.detach();

  //5.确保子线程先退出，主线程后退出
  while (!STOP_ALL)
  {
    sleep(1);
  }

  release();

  std::cout << "Exit Application!" << std::endl;

  return 0;
}
