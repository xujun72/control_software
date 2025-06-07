#include "config.hpp"
#include "ground_comm.hpp"
//与地面通讯函数

httplib::Client cli("localhost", 8080);

Detect_device_state devicestate_geometry;       //几何
Detect_device_state devicestate_inspection;     //巡检


static void groundcomm_init()
{
    devicestate_geometry.detectId = DETECT_GEOMETRY;
    devicestate_inspection.detectId = DETECT_INSPECTION;
    devicestate_geometry.detectState = 0;
    devicestate_geometry.deviceWarn = 0;
    devicestate_geometry.warnCode = 0;
    devicestate_inspection.detectState = 0;
    devicestate_inspection.deviceWarn = 0;
    devicestate_inspection.warnCode = 0;
}

//发送故障信息给地面
void groundcomm_fault_post()
{
    std::cout << "groundcomm_fault_post start  " << std::endl;
    while(1)
    {
        //巡检
        if((!inspection_faultdata_queue.empty()) || (!geometry_faultdata_queue.empty())) 
        {
            if(!inspection_faultdata_queue.empty())
            {
                Detect_fault_data inspection_current = inspection_faultdata_queue.front();
                nlohmann::json j_inspection;
                
                j_inspection["faultId"]      = inspection_current.faultId;
                j_inspection["detectSys"]    = inspection_current.detectSys;
                j_inspection["faultTime"]    = inspection_current.faultTime;
                j_inspection["subsecSta"]    = inspection_current.subsecSta;
                j_inspection["subsecEnd"]    = inspection_current.subsecEnd;
                j_inspection["subsecDis"]    = inspection_current.subsecDis;
                j_inspection["trainNo"]      = inspection_current.trainNo;
                j_inspection["deviceNo"]     = inspection_current.deviceNo;
                j_inspection["faultPartId"]  = inspection_current.faultPartId;
                j_inspection["faultTypeId"]  = inspection_current.faultTypeId;
                j_inspection["faultDegree"]  = inspection_current.faultDegree;

                // 嵌套结构也手动写入
                j_inspection["referPos"] = {
                    {"x", inspection_current.referPos.x},
                    {"y", inspection_current.referPos.y},
                    {"w", inspection_current.referPos.w},
                    {"h", inspection_current.referPos.h}
                };

                j_inspection["faultPos"] = {
                    {"x", inspection_current.faultPos.x},
                    {"y", inspection_current.faultPos.y},
                    {"w", inspection_current.faultPos.w},
                    {"h", inspection_current.faultPos.h}
                };

                j_inspection["ImgName"] = inspection_current.ImgName;
                j_inspection["refName"] = inspection_current.refName;
                


                // 发起 POST 请求
                auto res = cli.Post("/api/user", j_inspection.dump(), "application/json");

                if (res && res->status == 200) //success
                {
                    inspection_faultdata_queue.pop();
                    std::cout << "11111响应状态码: " << res->status << std::endl;
                    std::cout << "11111响应内容: " << res->body << std::endl;

                    // 解析 JSON 响应
                    auto resp_json = nlohmann::json::parse(res->body);
                    if (resp_json.contains("result")) {
                        std::cout << "11111服务器返回结果: " << resp_json["result"] << std::endl;
                    }
                } 
                else 
                {
                    
                    std::cerr << "111111111请求失败或服务器无响应。" << std::endl;
                    continue;
                }
            }

            if(!geometry_faultdata_queue.empty())
            {
                Detect_fault_data geometry_current = geometry_faultdata_queue.front();
                nlohmann::json j_eometry;
                
                j_eometry["faultId"]      = geometry_current.faultId;
                j_eometry["detectSys"]    = geometry_current.detectSys;
                j_eometry["faultTime"]    = geometry_current.faultTime;
                j_eometry["subsecSta"]    = geometry_current.subsecSta;
                j_eometry["subsecEnd"]    = geometry_current.subsecEnd;
                j_eometry["subsecDis"]    = geometry_current.subsecDis;
                j_eometry["trainNo"]      = geometry_current.trainNo;
                j_eometry["deviceNo"]     = geometry_current.deviceNo;
                j_eometry["faultPartId"]  = geometry_current.faultPartId;
                j_eometry["faultTypeId"]  = geometry_current.faultTypeId;
                j_eometry["faultDegree"]  = geometry_current.faultDegree;

                // 嵌套结构也手动写入
                j_eometry["referPos"] = {
                    {"x", geometry_current.referPos.x},
                    {"y", geometry_current.referPos.y},
                    {"w", geometry_current.referPos.w},
                    {"h", geometry_current.referPos.h}
                };

                j_eometry["faultPos"] = {
                    {"x", geometry_current.faultPos.x},
                    {"y", geometry_current.faultPos.y},
                    {"w", geometry_current.faultPos.w},
                    {"h", geometry_current.faultPos.h}
                };

                j_eometry["ImgName"] = geometry_current.ImgName;
                j_eometry["refName"] = geometry_current.refName;

                // 发起 POST 请求
                auto res = cli.Post("/api/user", j_eometry.dump(), "application/json");

                if (res && res->status == 200) //success
                {
                    geometry_faultdata_queue.pop();
                    std::cout << "22222响应状态码: " << res->status << std::endl;
                    std::cout << "22222响应内容: " << res->body << std::endl;

                    // 解析 JSON 响应
                    auto resp_json = nlohmann::json::parse(res->body);
                    if (resp_json.contains("result")) {
                        std::cout << "222222服务器返回结果: " << resp_json["result"] << std::endl;
                    }
                } 
                else 
                {
                    std::cerr << "2222222请求失败或服务器无响应。" << std::endl;
                    continue;
                }
            }
            
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    }
    
}


//发送状态信息-心跳
static void groundcomm_sendstateinfo()
{
    // 构造 JSON 请求体
    nlohmann::json j;
    j["detectId"] = devicestate_inspection.detectId;
    j["trainNo"] = devicestate_inspection.trainNo;
    j["stateTime"] = devicestate_inspection.stateTime;
    j["detectState"] = devicestate_inspection.detectState;
    j["deviceWarn"] = devicestate_inspection.deviceWarn;
    j["warnCode"] = devicestate_inspection.warnCode;

    // 发起 POST 请求
    auto res = cli.Post("/api/user", j.dump(), "application/json");

    if (res && res->status == 200) //success
    {
        std::cout << "33333响应状态码: " << res->status << std::endl;
        std::cout << "33333响应内容: " << res->body << std::endl;

        // 解析 JSON 响应
        auto resp_json = nlohmann::json::parse(res->body);
        if (resp_json.contains("result")) {
            std::cout << "33333服务器返回结果: " << resp_json["result"] << std::endl;
        }
    } else {
        std::cout << "33333请求失败或服务器无响应。" << std::endl;
    }
}

//周期发送状态数据
void groundcomm_main()
{
    std::cout << "groundcomm_main start  " << std::endl;
    groundcomm_init();

    while(1)
    {
        groundcomm_sendstateinfo();
        std::cout << "groundcomm_sendstateinfo start  " << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void groundcomm_set_stateTime(std::string time,uint8_t device_id)
{
    if(device_id == DETECT_GEOMETRY)
    {
        devicestate_geometry.stateTime = time;
    }
    else if(device_id == DETECT_INSPECTION)
    {
        devicestate_inspection.stateTime = time;
    }
}

void groundcomm_set_faultdata(RecvDataItem faultdata,uint8_t device_id)
{
    if(device_id == DETECT_INSPECTION)
    {
        Detect_fault_data inspection_current;
        inspection_current.ImgName = faultdata.path;
        inspection_current.detectSys = DETECT_INSPECTION;

        std::time_t now = std::time(nullptr);                       // 当前时间戳
        std::tm* localTime = std::localtime(&now);                  // 转换为本地时间结构
        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y%m%d-%H:%M:%S");         // 格式化时间
        inspection_current.faultTime = oss.str();
        inspection_current.faultPartId = faultdata.faultlist[0].picinfo[0].part_label;
        inspection_current.faultTypeId = faultdata.faultlist[0].picinfo[0].fault_label;
        inspection_current.faultPos.x = faultdata.faultlist[0].picinfo[0].x;
        inspection_current.faultPos.y = faultdata.faultlist[0].picinfo[0].y;
        inspection_current.faultPos.w = faultdata.faultlist[0].picinfo[0].w;
        inspection_current.faultPos.h = faultdata.faultlist[0].picinfo[0].h;
        inspection_current.subsecDis = pic_distance(faultdata.path);
        inspection_faultdata_queue.push(inspection_current);

    }
}

