/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *漏2025-2035 our Corporation All rights reserved.*         *
*        *************************************************         *
*                                                                  *
* FileName    : common.h                                           *
*                                                                  *
* Author      : walter                                              *
*                                                                  *
* Email       : mwttry@163.com                               *
*                                                                  *
* Date        : 2018-1-23                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef ___TRDP_PD_H
#define ___TRDP_PD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "private.h"
#include "protocol.h"
#include "trdp.h"
#include "trdp_if_light.h"
#include "tau_marshall.h"
#include "vos_utils.h"
#include "common.h"
#include "vos_thread.h"
#include "vos_sock.h"
#include "trdp_types.h"


typedef struct  _TRDP_PD_FROM_TCMS
{
    uint16  tcms_active;    
    uint8   year;       
    uint8   month; 
    uint8   day;  
    uint8   hour; 
    uint8   minute;  
    uint8   second;
    uint8   train_no;

    uint8   car_sleep_mode:1;
    uint8   car_wake_mode:1; 
    uint8   door_state:1; 
    uint8   tlds_sleep_mode:1;       //几何系统休眠指令
    uint8   tlds_wake_mode:1;        //几何系统唤醒指令
    uint8   tds_sleep_mode:1;        //巡检系统休眠指令
    uint8   tds_wake_mode:1;         //巡检系统唤醒指令
    uint8   res1:1;

    uint16  train_speed;
    uint16  start_station_id;
    uint16  current_station_id;
    uint16  next_station_id;
    uint16  des_station_id;
    uint16  total_run_dist;          //总里程

    uint8   mode_auto_valid:1;        //自动模式有效
    uint8   up_down:1;                //上行下行
    uint8   em_supply:1;              //应急供电
    uint8   res2:5;

    uint32  current_station_dist;   //当前站距离
    uint16  cui_wd;                 //车轮轮径
    
    uint8   resarry[16];             //预留
}TrdpPdFromTcms;

typedef struct  _TRDP_PD_TO_TCMS
{
    uint16  tlds_active;            //几何系统生命信号
    uint16  tds_active;             //巡检系统生命信号
    uint8  tlds_device_inspection;  //几何系统自检    
    uint8  tds_device_inspection;   //巡检系统自检

    uint8  tlds_2d_module_status:1;  //几何系统2D模块状态
    uint8  tlds_IxInertiaModuleSts:1; //几何系统Ix惯性模块状态
    uint8  res1:6;

    uint8  tds_IusTrackImg1Abnormal; //巡检系统轨道图像1异常
    uint8  tds_IusTrackImg2Abnormal; //巡检系统轨道图像2异常
    uint8  tds_IusTrackImg3Abnormal; //巡检系统轨道图像3异常
    uint8  tds_IusTrackImg4Abnormal; //巡检系统轨道图像4异常
    uint8  tds_IusTrackImg5Abnormal; //巡检系统轨道图像5异常
    uint8  tds_IusTrackImg6Abnormal; //巡检系统轨道图像6异常

    uint16  tlds_IuiSSWVersion;      //几何系统软件版本 
    uint16  tds_IuiSSWVersion;    //巡检系统软件版本

    uint8  tds_IxRailCracks:1;      //巡检系统轨道裂纹
    uint8  tds_IxRailMisalignment :1;   //巡检系统轨道错位
    uint8  tds_wave:1;              //巡检系统波浪磨耗和鱼鳞纹
    uint8  tds_IxBoltsMissing:1;    //巡检系统螺栓缺失
    uint8  tds_IxBoltsLoose:1;       //巡检系统螺栓松动

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
    uint8  res2:6;
    uint8  resarry[16]; //预留
}TrdpPdToTcms;

typedef struct _rpkgToTcms{
	TrdpPdToTcms recv;
	int rlen;
}rpkgToTcms;
extern rpkgToTcms pdReplyToTcms;

typedef struct _rpkgFromTcms{
	TrdpPdFromTcms recv;
	int rlen;
}rpkgFromTcms;
extern rpkgFromTcms pdFromTcms;
uint32 common_get_local_ip_addr(void);
uint16 common_get_buffer_size(void *v);
extern void dbgOut(void *pRefCon,TRDP_LOG_T category,const CHAR8 *pTime,const CHAR8 *pFile,UINT16 LineNumber,const CHAR8 *pMsgStr);

#ifdef __cplusplus
}
#endif

#endif