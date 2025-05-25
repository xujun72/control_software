/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                  *
 *        *************************************************         *
 *        *©2025-2035 our Corporation All rights reserved.*         *
 *        *************************************************         *
 *                                                                  *
 * FileName    : main.c                                             *
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

#include "protocol.h"
#include "trdp.h"
#include "trdp_if_light.h"
#include "tau_marshall.h"
#include "vos_utils.h"
#include "common.h"
#include "vos_thread.h"
#include "vos_sock.h"
#include "udp_socket.h"
#include "trdp_pd.h"


#define DATA_MAX 1000

#define PD_COMID 1000
#define PD_COMID_RECV_FROM_TCMS 10002
#define PD_COMID_SEND_TO_TCMS 20151
unsigned int MultiIpRecvFromTcms[4] = {239, 193, 0, 1};
unsigned int MultiIpSendToTcms[4] = {239, 193, 0, 15};

#define PD_COMID_CYCLE 200000 /* in us (1000000 = 1 sec) */

/* We use dynamic memory    */
#define RESERVED_MEMORY 1000000

rpkgToTcms pdReplyToTcms;
pthread_mutex_t mutex;

void dbgOut(
	void *pRefCon,
	TRDP_LOG_T category,
	const CHAR8 *pTime,
	const CHAR8 *pFile,
	UINT16 LineNumber,
	const CHAR8 *pMsgStr)
{
	const char *catStr[] = {"**Error:", "Warning:", "   Info:", "  Debug:"};

	if (category != VOS_LOG_DBG)
	{
		printf("%s %s %s:%d %s",
			   pTime,
			   catStr[category],
			   strrchr(pFile, VOS_DIR_SEP) + 1,
			   LineNumber,
			   pMsgStr);
	}
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Name  : U A R T _ S E N D E R
 *
 * in    :
 *
 * out   : �?
 *
 * return:
 *
 * Desc  :
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// int uart_sender(void *v, int len)
// {
// 	int outlen = 0;

// 	char *pv = common_exchange_trdp_to_sci(v, len);

// 	handleSerialDataSend(serialfd, pv, sizeof(frame)+len+2);

// 	if(pv) free(pv);
// }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Name  : P T H R E A D _ T R D P _ R E C I V E R
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
void *pthread_trdp_reciver(void *v)
{
	printf("pthread_trdp_reciver\n");
	unsigned int ip[4];
	TRDP_APP_SESSION_T appHandle; /*	 Our identifier to the library instance    */
	TRDP_SUB_T subHandle;		  /*	 Our identifier to the publication		   */
	UINT32 comId = PD_COMID_RECV_FROM_TCMS;
	TRDP_ERR_T err;
	TRDP_PD_CONFIG_T pdConfiguration =
		{NULL, NULL, {0u, 64u, 0u}, TRDP_FLAGS_NONE, 1000000u, TRDP_TO_SET_TO_ZERO, 0u};
	TRDP_MEM_CONFIG_T dynamicConfig = {NULL, RESERVED_MEMORY, {0}};
	TRDP_PROCESS_CONFIG_T processConfig = {"Me", "", 0, 0, TRDP_OPTION_NONE};
	UINT32 ownIP = 0u;
	UINT32 dstIP = 0u;
	int rv = 0;

	int ch;
	TRDP_PD_INFO_T myPDInfo;
	UINT32 receivedSize;
	CHAR8 buffer_space[1024];
	TrdpPdFromTcms *pdFromTcms = (TrdpPdFromTcms *)buffer_space;

	/*	  Init the library	*/
	if (tlc_init(NULL, /* no logging	*/
				 NULL,
				 &dynamicConfig) != TRDP_NO_ERR) /* Use application supplied memory	  */
	{
		printf("Initialization error\n");
		return NULL;
	}
	dstIP = (MultiIpRecvFromTcms[0] << 24) | (MultiIpRecvFromTcms[1] << 16) | (MultiIpRecvFromTcms[2] << 8) | MultiIpRecvFromTcms[3];
	/*	  Open a session  */
	if (tlc_openSession(&appHandle,
						ownIP, 0,				/* use default IP address			*/
						NULL,					/* no Marshalling					*/
						&pdConfiguration, NULL, /* system defaults for PD and MD	*/
						&processConfig) != TRDP_NO_ERR)
	{
		printf("Initialization error\n");
		return NULL;
	}

	memset(buffer_space, 0, sizeof(buffer_space));
	err = tlp_subscribe(appHandle,	/*    our application identifier            */
						&subHandle, /*    our subscription identifier           */
						NULL,		/*    user reference                        */
						NULL,		/*    callback functiom                     */
						0u,
						comId,				/*    ComID                                 */
						0,					/*    etbTopoCnt: local consist only        */
						0,					/*    opTopoCnt                             */
						0, 0,				/*    Source IP filter              */
						dstIP,				/*    Default destination    (or MC Group)  */
						0,					/*    TRDP flags                            */
						NULL,				/*    default interface                    */
						PD_COMID_CYCLE * 3, /*    Time out in us                        */
						TRDP_TO_SET_TO_ZERO /*    delete invalid data on timeout        */
	);

#if 0
	err = tlp_subscribe( appHandle, 				/*	  our application identifier			*/
						 &subHandle,				/*	  our subscription identifier			*/
						 NULL,						/*	  user reference						*/
						 NULL,						/*	  callback functiom 					*/
						 0,
						 comId, 					/*	  ComID 								*/
						 0, 						/*	  etbTopoCnt: local consist only		*/
						 0, 						/*	  opTopoCnt 							*/
						 0, 						/*	  Source IP filter						*/
						 dstIP, 					/*	  Default destination	 (or MC Group)	*/
						 0, 						/*	  TRDP flags							*/
						 PD_COMID_CYCLE * 3,		/*	  Time out in us						*/
						 TRDP_TO_SET_TO_ZERO	   /*	 delete invalid data on timeout 	   */
						 );
#endif
	if (err != TRDP_NO_ERR)
	{
		printf("prep pd receive error\n");
		tlc_terminate();
		return NULL;
	}

	while (1)
	{
		TRDP_FDS_T rfds;
		INT32 noDesc;
		TRDP_TIME_T tv = {0, 0};
		const TRDP_TIME_T max_tv = {10, 0};
		const TRDP_TIME_T min_tv = {0, 10000};

		FD_ZERO(&rfds);

		tlc_getInterval(appHandle, &tv, &rfds, &noDesc);

		if (vos_cmpTime(&tv, &max_tv) > 0)
		{
			tv = max_tv;
		}

		if (vos_cmpTime(&tv, &min_tv) < 0)
		{
			tv = min_tv;
		}

		rv = vos_select(noDesc + 1, &rfds, NULL, NULL, &tv);

		err = tlc_process(appHandle, &rfds, &rv);
		if (err != TRDP_NO_ERR)
		{
			printf("tlc_process error %d\n", err);
			break;
		}

		receivedSize = sizeof(buffer_space);
		err = tlp_get(appHandle,
					  subHandle,
					  &myPDInfo,
					  (UINT8 *)buffer_space,
					  &receivedSize);
		if ((TRDP_NO_ERR == err) && (receivedSize > 0))
		{
			printf("\nMessage reveived:\n");
			printf("Type = %c%c, ", myPDInfo.msgType >> 8, myPDInfo.msgType & 0xFF);
			printf("Seq  = %u, ", myPDInfo.seqCount);
			printf("with %d Bytes:\n", receivedSize);

			// uart_sender(buffer_space, receivedSize);
			if (SendTcmsDataToIandG(pdFromTcms, receivedSize))
			{
				printf("send data to IandG error\n");
			}
			else
			{
				printf("send data to IandG success\n");
			}

			// int i = 0;
			// for(; i<receivedSize; i++)
			// {
			// 	printf("%02hhx ", buffer_space[i]);
			// }
			// printf("\n");
		}
	}

	/*
	 *	  We always clean up behind us!
	 */
	tlp_unsubscribe(appHandle, subHandle);
	tlc_closeSession(appHandle);
	tlc_terminate();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Name  : P T H R E A D _ U A R T _ R E C I V E R
 *
 * in    :
 *
 * out   : �?
 *
 * return:
 *
 * Desc  :
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// void *pthread_uart_reciver(void *v)
// {
// 	printf("%s\n", __FUNCTION__);

// 	rpkg rtmp;

// 	for(;;)
// 	{
// 		handleSerialDataRecv(serialfd, &rtmp, common_get_buffer_size);

// 		pthread_mutex_lock(&mutex);
// 		if (rtmp.rlen > 0)
// 		{
// 			memcpy(&r, &rtmp, sizeof(rpkg));
// 		}
// 		pthread_mutex_unlock(&mutex);
// 	}
// }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Name  : P T H R E A D _ T R D P _ S E N D E R
 *
 * in    :
 *
 * out   : �?
 *
 * return:
 *
 * Desc  :
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// unsigned char arrayDestIP[4] = {192, 168, 70, 1}; //
void *pthread_trdp_sender(void *v)
{
	// unsigned int            ip[4];
	INT32 hugeCounter = 0;
	TRDP_APP_SESSION_T appHandle; /*    Our identifier to the library instance    */
	TRDP_PUB_T pubHandle;		  /*    Our identifier to the publication         */
	UINT32 comId = PD_COMID_SEND_TO_TCMS;
	UINT32 cycleTime = 200000;
	TRDP_ERR_T err;
	TRDP_PD_CONFIG_T pdConfiguration =
		{NULL, NULL, {0u, 64u, 0u}, TRDP_FLAGS_NONE, 1000000u, TRDP_TO_SET_TO_ZERO, 0};
	TRDP_MEM_CONFIG_T dynamicConfig = {NULL, 160000u, {0}};
	TRDP_PROCESS_CONFIG_T processConfig = {"Me", "", 0, 0, TRDP_OPTION_BLOCK};
	UINT32 ownIP = 0u;
	int rv = 0;
	UINT32 destIP = 0u;

	/*    Generate some data, that we want to send, when nothing was specified. */
	UINT8 *outputBuffer;
	UINT32 outputBufferSize = 0u;

	UINT8 data[DATA_MAX];
	int ch;

	while (!outputBufferSize)
	{
		outputBufferSize = pdReplyToTcms.rlen;
		outputBuffer = &(pdReplyToTcms.recv);
	}

	destIP = (MultiIpSendToTcms[0] << 24) | (MultiIpSendToTcms[1] << 16) | (MultiIpSendToTcms[2] << 8) | MultiIpSendToTcms[3];

	if (destIP == 0)
	{
		fprintf(stderr, "No destination address given!\n");
		return 1;
	}

	//    if (tlc_init(&dbgOut,                              /* no logging    */
	//                 NULL,
	//                 &dynamicConfig) != TRDP_NO_ERR)    /* Use application supplied memory    */
	//    {
	//        printf("Initialization error\n");
	//        return 1;
	//    }

	if (tlc_openSession(&appHandle,
						ownIP, 0,				/* use default IP address           */
						NULL,					/* no Marshalling                   */
						&pdConfiguration, NULL, /* system defaults for PD and MD    */
						&processConfig) != TRDP_NO_ERR)
	{
		printf("Initialization error\n");
		return 1;
	}
	err = tlp_publish(appHandle,  /*    our application identifier    */
					  &pubHandle, /*    our pulication identifier     */
					  NULL, NULL,
					  0u,
					  comId, /*    ComID to send                 */
					  0,	 /*    local consist only            */
					  0,
					  ownIP,				 /*    default source IP             */
					  destIP,				 /*    where to send to              */
					  cycleTime,			 /*    Cycle time in us              */
					  0,					 /*    not redundant                 */
					  TRDP_FLAGS_NONE,		 /*    Use callback for errors       */
					  NULL,					 /*    default qos and ttl           */
					  (UINT8 *)outputBuffer, /*    initial data                  */
					  outputBufferSize		 /*    data size                     */
	);

#if 0						 
    err = tlp_publish(  appHandle,                  /*    our application identifier    */
                        &pubHandle,                 /*    our pulication identifier     */
                        comId,                      /*    ComID to send                 */
                        0,                          /*    etbTopoCnt = 0 for local consist only     */
                        0,                          /*    opTopoCnt = 0 for non-directinal data     */
                        ownIP,                      /*    default source IP             */
                        destIP,                     /*    where to send to              */
                        cycleTime,                  /*    Cycle time in us              */
                        0,                          /*    not redundant                 */
                        TRDP_FLAGS_NONE,            /*    Use callback for errors       */
                        NULL,                       /*    default qos and ttl           */
                        (UINT8 *)outputBuffer,      /*    initial data                  */
                        outputBufferSize            /*    data size                     */
                        );
#endif

	if (err != TRDP_NO_ERR)
	{
		printf("prep pd error\n");
		tlc_terminate();
		return 1;
	}
	printf("pthread_trdp_sender start!!!!\n");
	while (1)
	{
		TRDP_FDS_T rfds;
		INT32 noDesc;
		TRDP_TIME_T tv;
		const TRDP_TIME_T max_tv = {0, 1000000};
		const TRDP_TIME_T min_tv = {0, 10000};

		FD_ZERO(&rfds);
		/* FD_SET(pd_fd, &rfds); */

		tlc_getInterval(appHandle, &tv, &rfds, &noDesc);

		if (vos_cmpTime(&tv, &max_tv) > 0)
		{
			tv = max_tv;
		}
		else if (vos_cmpTime(&tv, &min_tv) < 0)
		{
			tv = min_tv;
		}

		/*
		   Select() will wait for ready descriptors or time out,
		   what ever comes first.
		 */
		rv = vos_select(noDesc + 1, &rfds, NULL, NULL, &tv);

		tlc_process(appHandle, &rfds, &rv);
		uint16_t len = strlen((char *)outputBuffer);
		printf("outputBuffer len = %d,outputBuffer addr=0x%08x\n", len, outputBuffer);
		if (outputBuffer != NULL && strlen((char *)outputBuffer) == 0)
		{
			sprintf((char *)outputBuffer, "Just a Counter: %08d", hugeCounter++);
			outputBufferSize = (UINT32)strlen((char *)outputBuffer);
		}

		err = tlp_put(appHandle, pubHandle, outputBuffer, outputBufferSize);
		if (err != TRDP_NO_ERR)
		{
			printf("put pd error\n");
			rv = 1;
			break;
		}
	}

	/*
	 *    We always clean up behind us!
	 */
	tlp_unpublish(appHandle, pubHandle);
	tlc_closeSession(appHandle);
	tlc_terminate();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Name  : T R D P _ I N I T
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
int trdp_init(PTRDP_CONFIG conf)
{

	pthread_t t1, t2, t3;
	pthread_mutex_init(&mutex, NULL);

	//	init uart

	pthread_create(&t1, NULL, pthread_trdp_reciver, NULL);
	// pthread_create(&t2, NULL, pthread_uart_reciver, NULL);
	pthread_create(&t3, NULL, pthread_trdp_sender, NULL);
	return 0;
}
