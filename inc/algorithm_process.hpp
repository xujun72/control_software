#ifndef ALGORITHM_PROCESS_HPP
#define ALGORITHM_PROCESS_HPP


#include "ground_comm.hpp"
//与算法模块通讯
#define TCP_ADDR "tcp://127.0.0.1:5555"

void alg_clearwood(void);
static void alg_addwood_number(int h_val);
static void alg_calc_distance(std::string& folder_name,std::string& file_name);
void alg_main();

#endif