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

uint16_t sleeper_number;

static void initdata()
{
    sleeper_number = 0;
}

void producer() {
    for (int i = 0; i < 5; ++i) {
        RecvDataItem item;
        item.path = "E:\\data\\img_" + std::to_string(i) + ".jpg";
        item.error = "";
        item.faultlist = {
            {
                "fault_" + std::to_string(i),
                {
                    {1, 0, 10, 20, 30, 40, 0.9f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f}
                },
                {"key1", "key2"}
            }
        };
        recvQueue.push(item);
        std::cout << "Produced: " << item.path << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// 模拟消费线程
// void consumer() {
//     for (int i = 0; i < 5; ++i) {
//         RecvDataItem item = recvQueue.pop();
//         std::cout << "Consumed: " << item.path
//                   << ", fault name: " << item.faultlist[0].name
//                   << ", key[0]: " << item.faultlist[0].keys[0] << std::endl;
//     }
// }

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