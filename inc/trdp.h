/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2025-2035 our Corporation All rights reserved.*         *
*        *************************************************         *
*                                                                  *
* FileName    : trdp.h                                             *
*                                                                  *
* Author      : walter                                              *
*                                                                  *
* Email       : mwttry@163.com                               *
*                                                                  *
* Date        : 2025-4-22                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef _TRDP_H
#define _TRDP_H
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*debug_handler)(void *);

typedef struct {
	debug_handler dhanler;
	uint32	rcomId;
	
}TRDP_CONFIG;

typedef  TRDP_CONFIG * PTRDP_CONFIG;
int trdp_init(PTRDP_CONFIG conf);

#ifdef __cplusplus
}
#endif

#endif

