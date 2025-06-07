#ifndef _UDP_SOCKET_H
#define _UDP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "private.h"

#define SOCKET_BUFF_LEN 1024
#define LOCAL_SERVER_PATH	"/tmp/SERVER_PATH"
#define LOCAL_CLIENT_PATH	"/tmp/CLIENT_PATH"

#define RETRY_RECV_TIMES 500
#define UDP_PROTOCOL_STX 0XAE
enum UDP_CMD_TYPE
{
	UDP_CMD_CAR_INFO = 0x01,
	UDP_CMD_CAR_PIC_PATH = 0x02,
	UDP_CMD_CAR_REPLY = 0x03,	
};

typedef struct _UDP_PROTOCOL_INFO
{
	//byte1
	uint8 start;	
	uint8 cmd;
	//byte2
	uint16 lenth;
	//byte3
    uint8 data[SOCKET_BUFF_LEN];
 	//byte3
 	uint16 crc;
	//unsigned char crc2;
	
}UDP_PROTOCOL_INFO;

typedef struct  _INFO_FROM_TDS
{
    uint16 tds_active;             //巡检系统生命信号
    uint8  tds_pic_deal_finish; //巡检系统Ix惯性模块状态 
    uint8  tds_device_inspection;   //巡检系统自检
    uint8  tds_IusTrackImg1Abnormal; //巡检系统Ius轨道图像1异常
    uint8  tds_IusTrackImg2Abnormal; //巡检系统Ius轨道图像2异常
    uint8  tds_IusTrackImg3Abnormal; //巡检系统Ius轨道图像3异常
    uint8  tds_IusTrackImg4Abnormal; //巡检系统Ius轨道图像4异常
    uint8  tds_IusTrackImg5Abnormal; //巡检系统Ius轨道图像5异常
    uint8  tds_IusTrackImg6Abnormal; //巡检系统Ius轨道图像6异常
    uint16  tds_IuiSSWVersion;    //巡检系统Iui软件版本

    uint8  resarry[10]; //预留
}INFO_FROM_TDS;

typedef struct  _PIC_PATH_FROM_TDS
{

    uint8  tds_pic_path[512];   //巡检系统自检
    uint16 len;
    uint8 newdata_flag;
}PIC_PATH_FROM_TDS;


typedef struct  _INFO_FROM_ALGORITHM
{
    uint8  tds_IxRailCracks:1;      //巡检系统Ix轨道裂纹
    uint8  tds_IxRailMisalignment :1;   //巡检系统Ix轨道错位
    uint8  tds_wave:1;              //巡检系统波浪磨耗和鱼鳞纹
    uint8  tds_IxBoltsMissing:1;    //巡检系统Ix螺栓缺失
    uint8  tds_IxBoltsLoose:1;       //巡检系统Ix螺栓松动
    uint8  tds_IxSurfaceAbrasions1:1;    //巡检系统夹板断裂
    uint8  tds_IxSurfaceAbrasions2:1;   //轨下橡胶垫板移位
    uint8  tds_IxSurfaceAbrasions3:1;   //弹条折断、缺失

    uint8  tds_IxSurfaceAbrasions4:1;   //弹条松脱
    uint8  tds_IxSurfaceAbrasions5:1;   //轨道板断裂
    uint8  tds_IxSurfaceAbrasions6:1;   //道床积水
    uint8  tds_IxSurfaceAbrasions7:1;   //道床异物
    uint8  tds_IxSurfaceAbrasions8:1;   //道床裂缝
    uint8  tds_IxSurfaceAbrasions9:1;   //道床掉块
    uint8  tds_IxSurfaceAbrasions10:1;  //浮置板道床密封条破损、缺失
    uint8  tds_IxSurfaceAbrasions11:1;  //水沟有垃圾、淤泥等异物

    uint8  tds_IxSurfaceAbrasions12:1;  //道床范围内各类盖板缺失、翘起
    uint8  tds_IxSurfaceAbrasions13:1;  //轨枕掉块
    uint8  tds_IxSurfaceAbrasions14:1;  //轨枕裂缝
    uint8  tds_IxSurfaceAbrasions15:1;  //尖轨与基本轨秘贴部位有缝隙
    uint8  tds_IxSurfaceAbrasions16:1;  //尖轨与基本轨之间有异物
    uint8  tds_IxSurfaceAbrasions17:1;  //轮缘槽内有异物
    uint8  tds_IxSurfaceAbrasions18:1;  //滑床台脱焊移位
    uint8  tds_IxSurfaceAbrasions19:1;  //支撑、限位器、支距扣板缺失

    uint8  tds_IxSurfaceAbrasions20:1;  //钢轨接头有狭缝
    uint8  tds_IxSurfaceAbrasions21:1;  //护轨螺栓、调整片松脱
    uint8  tds_IxSurfaceAbrasions22:1;  //钢轨顶面有油料堆积
    uint8  tds_IxSurfaceAbrasions23:1;  //涂油器出油板、感应器松脱
    uint8  tds_IxSurfaceAbrasions24:1;  //轨距拉杆脱落
    uint8  tds_IxSurfaceAbrasions25:1;  //防脱护轨螺栓脱落
    uint8  tds_IxSurfaceAbrasions26:1;  //轮缘槽宽度变化
    uint8  res1:1;  //预留
}INFO_FROM_ALGORITHM;
    
typedef struct  _INFO_FROM_TLDS
{
    uint16  tlds_active;            //几何系统生命信号
    uint8  tlds_device_inspection;  //几何系统自检    
    uint8  tlds_2d_module_status:1;  //几何系统2D模块状态
    uint8  tlds_IxInertiaModuleSts:1; //几何系统Ix惯性模块状态
    uint8  res1:6;  

    uint16  tlds_IuiSSWVersion;      //几何系统Iui软件版本

    uint8  res2:6;
    uint8  resarry[16]; //预留
}INFO_FROM_TLDS;

int udp_init(void);
extern PIC_PATH_FROM_TDS PicPathFromIspection;
extern int ParseUdpInspectionInfo(uint8 *buffer, uint16 len);
extern int SendMultidata(uint16 multi_port,uint8 *sndbuff,uint16 datalen);
extern int CreateMultiRecv(uint16 multircv_port,uint8 eth_num,uint8* multi_addr);
extern int get_eth_num(void);
extern int SendSingleData(uint16 port,uint8 *serverip,uint8 *sndbuff,uint16 datalen);
extern int SendTcmsDataToIandG(uint8 *sndbuff,uint16 datalen);
extern int CreateSingleRecv(uint16 port);
extern int getLocalHostIP(char *ipaddr, int interface);
int create_localServer(char *serverpath);
extern int send_broadcastdata(uint16 broadcast_port,uint8 *sndbuff,uint16 datalen);

void udp_clear_newdataflag();

#ifdef __cplusplus
}
#endif

#endif

