#include "config.hpp"
#include "savepic.hpp"
#include "algorithm_process.hpp"
#include "pic_process.hpp"
#include "ground_comm.hpp"

extern "C" {
    #include "udp_socket.h"
    #include "trdp.h"
}
RecvDataQueue recvQueue;
SendDataQueue sendQueue;

//枕木数量计算 分辨率0.1
uint16_t wood_number;
//站点更新，枕木重新计算
uint8_t newstation_flag;
//从文件名中获取距离
uint32_t distance_file_recv;
//通过枕木数量计算得出距离
uint32_t distance_wood_calc;

static void initdata()
{
    wood_number = 0;
    newstation_flag = 0;
    distance_file_recv = 0;
    distance_wood_calc = 0;
    memset(&PicPathFromIspection,0x00,sizeof(PIC_PATH_FROM_TDS));
}

void consumer() {
    while (true) {
        RecvDataItem item = recvQueue.pop(); // 等待直到队列非空，自动移除
        //处理数据
        std::cout << "Processing: " << item.path << std::endl;
        // TODO: 根据 item 内容执行具体操作
        // if (item.error == "exit") break; // 可选退出机制
    }
}

int main() {

    // SetConsoleOutputCP(CP_UTF8);
    initdata();
    udp_init();
    trdp_init(NULL);
    //算法主函数
    std::thread algmain(alg_main);
    
    // std::thread algsend(alg_sendpath); //图形处理发送函数，发送需要处理的文件夹给到算法

    // std::thread algrecv(alg_recvdata);//接收算法处理后数据
   // while(1)
    // {
    //     std::cout << "start..." << std::endl;

    //     std::this_thread::sleep_for(std::chrono::seconds(10));  // 延迟10秒

    //     std::cout << "end." << std::endl;

    //     std::cin.get();
    // }
    
    // algsend.join();
    // algrecv.join();


    // 修改图片名称
    // std::string newname;
    // std::string oldname;
    // newname = "I:/234.png";
    // oldname = "I:/222.png";
    // pic_rename(newname,oldname);
    // 修改图片名称

    //地面通讯线程 start
    std::thread groundcommmain(groundcomm_main);
    std::thread groundcommfault_post(groundcomm_fault_post);

    algmain.join();
    groundcommmain.join();
    groundcommfault_post.join();

    //地面通讯线程 end

    return 0;
}