#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <zmq.hpp>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "httplib.h"
#include "json.hpp"

//计算距离的误差值 >= errorvalue时，使用巡检数据。< errorvalue时，使用算法值
#define DISTANCE_ERROR_VALUE       5

//检测系统        1：巡检；2：几何
#define DETECT_INSPECTION           1
#define DETECT_GEOMETRY             2


#define COMM_STATE_OK               0
#define COMM_STATE_NG               1
//设备通讯状态
struct device_comm_state {
    uint8_t insp_state:1;       //巡检状态
    uint8_t geom_state:1;       //几何状态
    uint8_t grou_state:1;       //地面状态
    uint8_t algo_state:1;       //算法状态
    uint8_t tcms_state:1;       //TCMS状态
    uint8_t other_state:3;      //其他设备状态
};

//检测设备状态 地面通讯使用
struct Detect_device_state {
    uint8_t detectId;               //1：巡检；2：几何
    std::string trainNo;            //
    std::string stateTime;          //YYYYMMDD-hh:mm:ss
    uint8_t detectState;            //0：离线；1：在线
    uint32_t deviceWarn;            //取32bit，每一位对应设备/功能模块，0表示设备在线，1表示设备异常/离线
    uint16_t warnCode;              //地面端对应维护字典
};

//检测故障信息 地面通讯使用
struct fault_pos {
    int x, y, w, h;
};


struct Detect_fault_data {
    int faultId;                    //故障编码
    int detectSys;                  //检测系统 1：巡检；2：几何
    std::string faultTime;          //故障检出时间 YYYYMMDD-hh:mm:ss
    int subsecSta;                  //子区间起点 采用站点编码，配置编码对应站点名称字典
    int subsecEnd;                  //子区间终点
    int subsecDis;                  //子位置距离
    std::string trainNo;            //设备编号（车号）
    int deviceNo;                   //子设备编号（采集相机）
    int faultPartId;                //轨道部件id
    int faultTypeId;                //故障类型id
    int faultDegree;                //影响程度
    fault_pos referPos;             //故障在模板图片上的坐标
    fault_pos faultPos;             //故障在故障图片上的坐标
    std::string ImgName;            //故障图片名称
    std::string refName;            //参考图片名称
};

extern std::queue<Detect_fault_data> inspection_faultdata_queue;    //巡检故障数据队列
extern std::queue<Detect_fault_data> geometry_faultdata_queue;      //几何故障数据队列



//枕木数量计算 分辨率0.5
extern uint16_t sleeper_number;


struct Recv_picinfo {
    int part_label;
    int fault_label;
    int x, y, w, h;
    float conf;
    float da, dl, dw, dh, dd, dc;
};

struct Recv_faultlist {
    std::string name;
    std::vector<Recv_picinfo> picinfo;
    std::vector<std::string> keys;
};

//接收算法模块数据
struct RecvDataItem {
    std::string path;
    std::string error;
    std::vector<Recv_faultlist> faultlist; // 存储多组检测数据 
};

//发送数据到算法模块
struct SendDataItem {
    std::string path;
    std::vector<std::string> keys;  
};

class RecvDataQueue {
private:
    std::queue<RecvDataItem> queue_recvdata;
    std::mutex mutex_recvdata;
    std::condition_variable cond_recvdata;

public:
    void push(const RecvDataItem& item) {
        std::lock_guard<std::mutex> lock(mutex_recvdata);
        queue_recvdata.push(item);
        cond_recvdata.notify_one();
    }

    RecvDataItem pop() {
        std::unique_lock<std::mutex> lock(mutex_recvdata);
        cond_recvdata.wait(lock, [this] { return !queue_recvdata.empty(); });
        RecvDataItem item = queue_recvdata.front();
        queue_recvdata.pop();
        return item;
    }
};

class SendDataQueue {
private:
    std::queue<SendDataItem> queue_senddata;
    std::mutex mutex_senddata;
    std::condition_variable cond_senddata;

public:
    void push(const SendDataItem& item) {
        std::lock_guard<std::mutex> lock(mutex_senddata);
        queue_senddata.push(item);
        cond_senddata.notify_one();
    }

    SendDataItem pop() {
        std::unique_lock<std::mutex> lock(mutex_senddata);
        cond_senddata.wait(lock, [this] { return !queue_senddata.empty(); });  //阻塞等待数据
        SendDataItem item = queue_senddata.front();//找到第一个数据
        queue_senddata.pop();//删除数据
        return item;
    }
};

extern RecvDataQueue recvQueue;
extern SendDataQueue sendQueue;

#endif // CONFIG_HPP