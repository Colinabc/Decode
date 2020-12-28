#include"decode_detection.h"
#define cameraNum 4

//配置文件路径
DEFINE_string(config_json, "../caisa_config.json", "");
DEFINE_string(config_json_vest,"../caisa_config_vest.json","");
DEFINE_string(config_json_smokephone,"../caisa_config_smokephone.json","");
DEFINE_string(config_json_oilleak,"../caisa_config_leakoil.json","");

int main()
{
    vector<string>URL={
            "rtsp://admin:letmein1@192.168.1.160:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.161:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.162:554/h264/ch1/main/av_stream",
	    "rtsp://admin:letmein1@192.168.1.163:554/h264/ch1/main/av_stream"
};
    vector<string>cameraIP={"192.168.1.160","192.168.1.161","192.168.1.162","192.168.1.163"};
 //注册检测任务,8种
    Scheduler scdu(FLAGS_config_json);
    auto cc = make_shared<CourierClothing>(FLAGS_config_json);
    scdu.enroll(cc, Task::PERSON_CLOTHING);

    auto cv = make_shared<CourierClothing>(FLAGS_config_json_vest);
    scdu.enroll(cv, Task::PERSON_CLOTHING_VEST);

    auto ci = make_shared<CourierInvasion>(FLAGS_config_json);
    scdu.enroll(ci, Task::PERSON_INVASION);

    auto cs = make_shared<CourierSkin>(FLAGS_config_json);
    scdu.enroll(cs, Task::SKIN_EXPOSURE);

    auto cf = make_shared<CourierFire>(FLAGS_config_json);
    scdu.enroll(cf, Task::FIRE_SMOKE);
	
	//person smoke
	auto cm = make_shared<libadapter::CourierPhone>(FLAGS_config_json_smokephone);
	scdu.enroll(cm, Task::SMOKE);
	
	//person phone
	auto cp = make_shared<libadapter::CourierPhone>(FLAGS_config_json_smokephone);
	scdu.enroll(cp, Task::PHONE);
	
	//oil leak
	auto co = make_shared<libadapter::CourierOil>(FLAGS_config_json_oilleak);
	scdu.enroll(co, Task::LEAK_OIL);	
	
    scdu.start();

    vector<thread>v_th;
    for(int i=0;i<cameraNum;i++)
    {
        v_th.push_back( std::thread(softwareDecode,ref(scdu),URL[i],cameraIP[i]) );
    }
    
    for(int i=0;i<cameraNum;i++)
    {
        v_th[i].detach();
    }
        
    while(1)
     {
       usleep(10);
     }
    scdu.stop();
    DLOG(INFO) << "STOP"<<endl; 
    google::ShutdownGoogleLogging();
   
    return 0;
}
