#include "run.h"

//int *pconn=NULL;
std::map<std::string, std::vector<COMPUTNGPARA>> cameraIP_clientIP_map;
std::map<std::string, int> clientIP_index_map;            //客户端IP与其对应的index关系映射
std::map<std::string, std::queue<FRAMEINFO>> *presultMap; //全局变量
std::map<std::string, mutex *> *presultMap_mtx;
std::map<std::string, std::queue<FRAMEINFO>> resultMap; //全局变量
std::map<std::string, mutex *> resultMap_mtx;
std::vector<cameraDecode *> decode_vec;

void dn_breathe_log(std::vector<cameraDecode *>& dn_vec)
{
  while(1)
  {
    std::string json_new = "";
    std::fstream fs(dn_breathe_path);   
    if (fs.fail())
    {
      std::cout<<"open failure!"<<std::endl;
      nlohmann::json data;
      data["time_step"] = breathe_time_step;
      std::string str_time = get_localtime();
      data["fresh_air"] = str_time;
      data["connection_failure"] ={};

      json_new = data.dump(4);
      std::fstream fs1;
      fs1.open(dn_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app);
      fs1.write(json_new.c_str(), json_new.length());
      fs1.close();
    }
    else
    {
      std::cout<<"open success!"<<std::endl;
      std::stringstream ss;
      ss << fs.rdbuf();
      std::string json_status_config = ss.str();
      fs.close();
      try
      {
        auto data = nlohmann::json::parse(json_status_config);
        std::cout<<"dn_vec.size: "<<dn_vec.size()<<std::endl;

        for(int i=0; i < dn_vec.size();i++)
        {
           if(dn_vec[i]->getbreatheInfo()!= NULL)
           {
              data["fresh_air"] = get_localtime();
              nlohmann::json addr_json;
              if( !dn_vec[i]->getbreatheInfo()->connectionFailure.empty() )
              {
                addr_json["addr"] = dn_vec[i]->getbreatheInfo()->connectionFailure; 
                if(data["connection_failure"].size() < log_list_len)
                {
                  data["connection_failure"].push_back(addr_json);
                }
                else
                {
                  data["connection_failure"].erase(data["connection_failure"].begin());
                  data["connection_failure"].push_back(addr_json);
                }
              }                      
           }
        }     
        json_new = data.dump(4);

      }
      catch (...)
      {
        return;
      }
      std::ofstream ofs1(dn_breathe_path, std::ios::out | std::ios::trunc);
      ofs1.close();
      std::fstream fs3;
      fs3.open(dn_breathe_path, std::fstream::binary | std::fstream::out | std::fstream::app);
      fs3.write(json_new.c_str(), json_new.length());
      fs3.close();
    }
    sleep( breathe_time_step*60 );//呼吸间隔min
  }
}

void run() //初始化算法
{
  generate_start_flag();
  decode_vec.clear();
  cameraIP_clientIP_map.clear();
  clientIP_index_map.clear();
  resultMap.clear();
  resultMap_mtx.clear();

  presultMap = &resultMap;
  presultMap_mtx = &resultMap_mtx;

  //1.1 客户端IP列表
  for (int index = 0; index < computingNodesIP.size(); index++)
  {
    clientIP_index_map.insert(make_pair(computingNodesIP[index], index));
  }
  //1.2 解码节点列表
  for (int i = 0; i < decodeAddrList.size(); i++)
  {
    vector<COMPUTNGPARA> clientIP_vec;

    auto tmp = decodeAddrList[i];
    std::string tmp_addr = tmp.addr; //相机IP
    std::string tmp_decoding_type = tmp.decoding_type;
    auto tmp_computingNodes_vec = tmp.computingNodes_vec;
    for (int j = 0; j < tmp_computingNodes_vec.size(); j++)
    {
      auto tmp_computingNodes = tmp_computingNodes_vec[j];
      clientIP_vec.push_back(tmp_computingNodes);
    }

    cameraIP_clientIP_map.insert(make_pair(tmp_addr, clientIP_vec)); //每个解码节点: 相机IP ==> 多个clientIP

    cameraDecode *pcamDecode = new cameraDecode(hwType, tmp_addr, tmp_decoding_type, breathe_time_step, reconnTime, reconnCount, presultMap, presultMap_mtx, false);
    decode_vec.push_back(pcamDecode);
  }

  //1.3 呼吸日志初始化
  std::thread th_breathe(dn_breathe_log, std::ref(decode_vec));
  th_breathe.detach();

  //1.4 创建解码线程
  for (int k = 0; k < decode_vec.size(); k++)
  {
    decode_vec[k]->run();
  }

}
void release()
{
  for (int k = 0; k < decode_vec.size(); k++)
  {
    delete decode_vec[k];
    decode_vec[k] = NULL;
  }
  std::cout << "Release camera pointer！" << std::endl;
}