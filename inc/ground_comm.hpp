#ifndef GROUND_COMM_HPP
#define GROUND_COMM_HPP


#include "pic_process.hpp"

void groundcomm_main();
void groundcomm_set_stateTime(std::string time,uint8_t device_id);
void groundcomm_fault_post();
void groundcomm_set_faultdata(RecvDataItem faultdata,uint8_t device_id);


#endif