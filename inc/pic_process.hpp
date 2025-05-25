#ifndef PIC_PROCESS_HPP
#define PIC_PROCESS_HPP
#include "config.hpp"

void pic_rename(std::string newname,std::string oldname);
//计算距离，返回距离值
uint32_t pic_distance(std::string& pic_name);

#endif
