#include "config.hpp"
#include "algorithm_process.hpp"

extern "C" {
    #include "udp_socket.h"
    #include "trdp.h"
}
#include <atomic>
#include <chrono>

std::queue<Detect_fault_data> inspection_faultdata_queue;
std::queue<Detect_fault_data> geometry_faultdata_queue;

using json = nlohmann::json;

std::atomic<bool> running_rec(true);

// 接收线程函数
static void alg_receiver(zmq::socket_t& socket) {
    std::cout << "alg_receiver: " << std::endl;
    while (running_rec) {
        zmq::message_t content;
        
        // 非阻塞接收帧消息
        if (socket.recv(content, zmq::recv_flags::dontwait)) {
            // 处理 JSON 数据
            std::string data(static_cast<char*>(content.data()), content.size());
            try {
                auto resp = json::parse(data);
                RecvDataItem result;
                result.path = resp["path"].get<std::string>();
                result.error = resp["error"].get<std::string>();

                for (const auto& item : resp["fault_list"]) 
                {
                    Recv_faultlist fault;
                    fault.name = item["name"];
                    //通过图片命名获取距离
                    distance_file_recv = pic_distance(fault.name);
                    //通过枕木计算距离
                    alg_calc_distance(result.path,fault.name);

                    for (const auto& bbox_item : item["bbox"]) 
                    {
                        Recv_picinfo picinfo;

                        // 解析为字符串后转换为 int / float
                        picinfo.part_label  = std::stoi(bbox_item[0].get<std::string>());
                        picinfo.fault_label = std::stoi(bbox_item[1].get<std::string>());
                        picinfo.x = std::stoi(bbox_item[2].get<std::string>());
                        picinfo.y = std::stoi(bbox_item[3].get<std::string>());
                        picinfo.w = std::stoi(bbox_item[4].get<std::string>());
                        picinfo.h = std::stoi(bbox_item[5].get<std::string>());

                        if((picinfo.part_label == 3) && (picinfo.fault_label == 10))
                        {
                            alg_addwood_number(picinfo.h);
                        }

                        fault.picinfo.push_back(picinfo);
                    }

                    result.faultlist.push_back(fault);
                }
                recvQueue.push(result);

                //将算法识别的故障信息写到地面故障结构体，等待发往地面
                groundcomm_set_faultdata(result,DETECT_INSPECTION);
                std::cout << "Path: " << result.path << "\n";
                for (const auto& f : result.faultlist) {
                    std::cout << "Image: " << f.name << "\n";
                    for (const auto& p : f.picinfo) {
                        std::cout << "  Part: " << p.part_label << ", Fault: " << p.fault_label
                                << ", Pos: (" << p.x << "," << p.y << "), Size: " << p.w << "x" << p.h
                                << ", Conf: " << p.conf << "\n";
                    }
                }

            } catch (...) {
                std::cerr << "Invalid JSON received\n";
            }
        }
    }
}

static void alg_monitor_udpdata(void) {
    SendDataItem newdata;
    while(1)
    {
        if(PicPathFromIspection.newdata_flag == 1)
        {
            newdata.path = std::string(reinterpret_cast<const char*>(PicPathFromIspection.tds_pic_path), PicPathFromIspection.len);
            newdata.keys.assign(1, "key");
            sendQueue.push(newdata);
            udp_clear_newdataflag();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }   
}

void alg_clearwood(void)
{
    wood_number = 0;
}

//计算枕木数
static void alg_addwood_number(int h_val)
{
    uint8_t num;
    num = h_val * 10 / WOOD_HEIGHT;

    wood_number += num;
}

//通过枕木计算距离
static void alg_calc_distance(std::string& folder_name,std::string& file_name)
{
    std::string newname;

    if (!folder_name.empty() && folder_name.back() != '/') {
        folder_name += '/';
    }

    distance_wood_calc = wood_number / 10 * WOOD_SPACE_DISTANCE;
    //
    if(abs(distance_file_recv-distance_wood_calc) < 5)
    {
        newname = pic_filename_updatedistance(file_name,distance_wood_calc);
        pic_rename(folder_name+newname,folder_name+file_name);
    }
}


void alg_main()
{
    zmq::context_t ctx(1);
    zmq::socket_t socket(ctx, ZMQ_DEALER);

    // 设置发送接收高水位，0表示不限制
    socket.setsockopt(ZMQ_SNDHWM, 0);
    socket.setsockopt(ZMQ_RCVHWM, 0);
    socket.connect(TCP_ADDR);
    // 启动接收线程
    std::thread t_rec(alg_receiver, std::ref(socket));
    t_rec.detach();
    // 创建监听进程，等待udp新数据接受
    std::thread t_monitor(alg_monitor_udpdata);
    t_monitor.detach();
    // 主线程持续发送请求
    while (true) {

    #ifdef TEST_VERSION
        json req;
        req["path"] = "/to/your/path/1";
        req["key1"] = "key1";
        
        zmq::message_t msg(req.dump().size());
        memcpy(msg.data(), req.dump().c_str(), req.dump().size());
        
        socket.send(msg, zmq::send_flags::dontwait); // 非阻塞发送
        std::cout << "Sent request: " << req["path"] << std::endl;
        SendDataItem item = sendQueue.pop();//阻塞式，等待新的数据
    #else
        SendDataItem item = sendQueue.pop();//阻塞式，等待新的数据

        json req;
        req["path"] = item.path;
        req["key1"] = "key1";
        
        zmq::message_t msg(req.dump().size());
        memcpy(msg.data(), req.dump().c_str(), req.dump().size());
        
        socket.send(msg, zmq::send_flags::dontwait); // 非阻塞发送
        std::cout << "Sent request: " << req["path"] << std::endl;
    #endif
    }
}
