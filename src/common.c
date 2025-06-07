/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2025-2035 our Corporation All rights reserved.*         *
*        *************************************************         *
*                                                                  *
* FileName    : common.c                                           *
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


#include "common.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Name  : C O M M O N _ G E T _ L O C A L _ I P _ A D D R
*
* in    : 
*
* out   : 
*
* return: 
*
* Desc  : 
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 common_get_local_ip_addr(void)
{
	return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Name  : C O M M O N _ E X C H A N G E _ L E N
*
* in    : 
*
* out   : 
*
* return: 
*
* Desc  : 
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint16 common_exchange_len(uint16 v)
{
	uint8 high = (v>>8)&0xFF;
	uint8 low = v&0xFF;

	return (uint16)low<<8|(uint16)high;
}


void common_crc(void *v, int len, uint8 *c1, uint8 *c2)
{
	int i = 0;
	uint8 CK_A = 0, CK_B = 0;

	for (i=0; i<len ;i++)
	{
		CK_A = CK_A + *(char *)(v+i);
		CK_B = CK_B+CK_A;
	}

	*c1 = CK_A;
	*c2 = CK_B;
}
