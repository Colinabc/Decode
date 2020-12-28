#include "common.h"

std::string dn_init_local_path = ""; //本地配置文件路径
std::string dn_init_path = "";                                     //启动设置配置文件路径
std::string dn_start_path = "";                                    //计算节点异常启动文件
std::string dn_breathe_path = "";                                  //计算节点心跳log文件
std::string absPath = "";

int breathe_time_step = 0;
int reconnTime = 0;
int reconnCount = 0;
std::string hwType = "";
int dn_port = 0;
int connArray[256]={0};
std::mutex connArray_mtx;

std::vector<std::string> computingNodesIP; //客户端IP列表
std::vector<DECODE_PARA> decodeAddrList; //解码列表

int zone_offset = 0;  //时区偏移，单位（小时）
int log_list_len = 0; //心跳日志中端口列表最大长度
int queue_len = 0;   //图像队列长度

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

void generate_start_flag()
{
  std::string json_new = "";
  std::fstream fs(dn_start_path);
  fs.open(dn_start_path, std::fstream::binary | std::fstream::out | std::fstream::app);
  fs.write(json_new.c_str(), json_new.length());
  fs.close();
}

int load_decoding_config()
{
  decodeAddrList.clear();
  computingNodesIP.clear();
  dn_init_local_path = absPath+"/config/dn_init_local.config";
  
  std::fstream fs0(dn_init_local_path);
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
    dn_init_path = absPath+ (std::string)(data0.at("dn_init_path"));
    dn_start_path = absPath+ (std::string)(data0.at("dn_start_path"));
    dn_breathe_path = absPath+ (std::string)(data0.at("dn_breathe_path"));
    zone_offset = data0.at("zone_offset");
    log_list_len = data0.at("log_list_len");
    queue_len = data0.at("queue_len");
    reconnCount = data0.at("reconnCount");
  }
  catch (...)
  {
    return -1;
  }

  std::fstream fs1(dn_init_path);
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
    dn_port = data1["port"].get<int>();

    //初始化套接字数组
    int connSize = data1["computing_node_list"].size();
    for (int i = 0; i < connSize; i++)
    {
      connArray[i] = -1;
    }

    //保存客户端IP列表
    for (int index = 0; index < data1["computing_node_list"].size(); index++)
    {
      std::string tmp = data1["computing_node_list"][index].get<std::string>();
      computingNodesIP.push_back(tmp);
    }

    //保存所有计算节点参数
    for (int addrIndex = 0; addrIndex < data1["decoding_addr_list"].size(); addrIndex++)
    {
      DECODE_PARA tmp_decodePara;
      auto tmp = data1["decoding_addr_list"][addrIndex];
      tmp_decodePara.addr = tmp["addr"].get<std::string>(); //camera url
      tmp_decodePara.decoding_type = tmp["decoding_type"].get<std::string>();

      for (int nodeIndex = 0; nodeIndex < tmp["computing_nodes"].size(); nodeIndex++)
      {
        COMPUTNGPARA tmp_computingNodes;
        auto tmp_node = tmp["computing_nodes"][nodeIndex];
        tmp_computingNodes.ip = tmp_node["ip"].get<std::string>();
        tmp_computingNodes.extraction_rate = tmp_node["extraction_rate"].get<int>();
        tmp_computingNodes.count = 0;

        tmp_decodePara.computingNodes_vec.push_back(tmp_computingNodes);
      }
      decodeAddrList.push_back(tmp_decodePara);
    }
  }
  catch (...)
  {
    return -1;
  }
  return 0;
}