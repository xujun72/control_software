/********************************************************************

	Copyright (C) 2012  mwt 

*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "udp_socket.h"
#include "trdp_pd.h"
//#include "queue.h"
#include "bsd_queue.h"

#define MCAST_ADDR   "225.0.0.4"
#define MCAST_ADDR_COUPLE_INFO   "224.1.1.1"
#define MCAST_ADDR_TV "224.1.1.1"

#define BCAST_ADDR	 "255.255.255.255"

#define MULTI_COUPLE_TRANS_PORT	8007
#define MULTI_SEND_DISP_PORT	8008
#define MULTI_COUPLE_OUT_PORT	8001
#define MULTI_COUPLE_INFO_PORT	8004


#define SING_RECV_GEOMETRY_PORT	11001
#define SING_RECV_INSPECTION_PORT	11002
#define SING_SEND_INSPECTION_PORT	11002
#define SING_SEND_TCMS_INFO_PORT	11000



#define ETH_FILE  "/tmp/eth_file"
char *MultiAddress="225.0.0.4";
char *MultiAddressCoupleInfo="224.1.1.1";
char LoLoaclIpAddr[20]={"127.0.0.1"};
char Eth1LoaclIpAddr[20];
char Eth0LoaclIpAddr[20];
//////out net ---eth0;   in net ---eth1
char* OutNetIp[4] = {"10.0.0.11",	//前锟斤拷1锟斤拷ip
					"10.0.0.12",	//前锟斤拷4锟斤拷ip
					"10.0.0.13",	//锟斤拷1锟斤拷ip
					"10.0.0.14"};	//锟斤拷4锟斤拷ip
char* PisIpAddress[2] = {"192.168.5.11",//head coach ip
						"192.168.5.14"};//tail coach ip
char* GwIpAddress = {"192.168.11.14"};//change ip for gw						
//static pthread_rwlock_t RecvInfoFlag = PTHREAD_RWLOCK_INITIALIZER;
char RecvInfoFlag=0;
UDP_PROTOCOL_INFO LastUdpInspectionInfo,CurUdpInspectionInfo;
PIC_PATH_FROM_TDS PicPathFromIspection;


unsigned int HeartBeatTime = 0;
int system_do_command(const char *cmd)
{
	int ret = -1;
	//int ret1=0;
	//int ret2=0;
	ret = system(cmd);
	/*
	printf("%s cmd return value:%02x\n", __func__,ret);
	ret1 = WIFEXITED(ret);
	printf("%s WIFEXITED return value:%02x\n", __func__,ret1);
	ret2 = WEXITSTATUS(ret);
	printf("%s WEXITSTATUS return value:%02x\n", __func__,ret2);
	*/
	if (-1 != ret)
	{
		if (WIFEXITED(ret))
		{
			if (0 == WEXITSTATUS(ret))
			{
				ret = 0;
			}else
				ret = -1;
		}else
			ret = -1;
	}

	return ret;
}

int SetOutNetIpAddress(char* addr)
{
	char cmd[128] = {0};	
	sprintf(cmd,"ifconfig ens33 %s netmask 255.255.0.0\n",addr);
	printf("set ens33 ip address:%s\n",cmd);
	if (system_do_command(cmd))
	{
		return -1;
	}
}

int ChangeInNetIpAddress()
{
	char cmd[128] = {0};	
	sprintf(cmd,"ifconfig ens33 %s netmask 255.255.0.0\n",GwIpAddress);
	printf("change ens33 ip address:%s\n",cmd);
	if (system_do_command(cmd))
	{
		return -1;
	}
}

/*****************************************
delete iptables for nat
******************************************
*/
void DeleteIptables(void)
{
	char cmd[128] = {0};
	int i,ret;
	sprintf(cmd,"iptables -t nat -D PREROUTING 1");
	for(i=0;i<10;i++)
	{
		ret = system_do_command(cmd);
		printf("%s,ret:%d,i=%d\n", __func__,ret,i);
		if(ret < 0)
			break;
	}
}
/*****************************************
set iptables for nat
******************************************
*/
void SetIptables(char * out_net_addr)
{	
	char cmd[128] = {0};
	int i,index;
	memset(cmd,0,sizeof(cmd));
	for(i=0; i<4; i++)
	{
		if(memcmp(out_net_addr,OutNetIp[i],strlen(OutNetIp[i]))==0)
		{
			index=i;
			break;
		}
	}
	DeleteIptables();
	if((i==0)||(i==2))//11,13
	{
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p tcp -m tcp --dport 8003 -j DNAT --to-destination %s:8003",out_net_addr,PisIpAddress[1]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p tcp -m tcp --dport 8004 -j DNAT --to-destination %s:8004",out_net_addr,PisIpAddress[1]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p udp -m udp --dport 8005 -j DNAT --to-destination %s:8005",out_net_addr,PisIpAddress[1]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);
	}
	else
	{
		
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p tcp -m tcp --dport 8003 -j DNAT --to-destination %s:8003",out_net_addr,PisIpAddress[0]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p tcp -m tcp --dport 8004 -j DNAT --to-destination %s:8004",out_net_addr,PisIpAddress[0]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);		
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p udp -m udp --dport 8005 -j DNAT --to-destination %s:8005",out_net_addr,PisIpAddress[0]);
		printf("set iptables cmd:%s\n",cmd);
		system_do_command(cmd);
	}
	/*
	snprintf(cmd,sizeof(cmd),"iptables -t nat -A PREROUTING -d %s -p udp -m udp --dport 8005 -j DNAT --to-destination %s:8005",out_net_addr,PisIpAddress[0]);
	printf("set iptables cmd:%s\n",cmd);
	system_do_command(cmd);
	*/
}

int get_eth_num()
{
	uint8_t num = 0;
	FILE *pidfile;
	char buf[512] = "";
	int ret = 0;    
	char cmd[128];
	char tmp[128];
	memset(tmp,0x0,sizeof(tmp));
	memset(cmd,0x0,sizeof(cmd));
	sprintf(cmd, "ifconfig | grep eth > %s",ETH_FILE);
	unlink(ETH_FILE);
	system(cmd);
	pidfile = fopen(ETH_FILE, "r");
	if (pidfile != NULL) {
		if(fgets(buf, 500, pidfile) != NULL)
		{
			uint8_t *head = memchr(buf, 'e', strlen(buf));
			uint8_t *tail = memchr(buf, ' ', strlen(buf));
			if((head != NULL) && (tail != NULL))
			{
				memcpy(tmp,head+3,(tail-head)-3);
				num = atoi(tmp);
			}
		}
		fclose(pidfile);
	}
	return num;
}


int getLocalHostIP(char *ipaddr, int interface)
{
	if(ipaddr == NULL)
		return -1;
    int sockfd;
    struct ifreq req;
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == 0)
	{
        perror ("socket");
        return -1;
    }   

    memset(&req, 0, sizeof(struct ifreq));
    sprintf(req.ifr_name, "enss%d", interface);

    // get current ip addr 
    if (ioctl(sockfd, SIOCGIFADDR, (char*)&req)) 
    {
        perror(req.ifr_name);
	close(sockfd);
        return -1;
    }
    else
    {
        struct in_addr ip_addr;
        char * tmpstr;
        ip_addr.s_addr = *((int*) &req.ifr_addr.sa_data[2]);
        tmpstr = inet_ntoa(ip_addr);
        strcpy(ipaddr, tmpstr);
    }
    close(sockfd);   

    return 0;

}
/* Table of CRC values for high-order byte */
static const unsigned char table_crc_hi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40


};
/* Table of CRC values for low-order byte */
static const unsigned char table_crc_lo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40

};

static unsigned short int crc16(unsigned char *buffer, unsigned short int buffer_length)
{
unsigned char crc_hi = 0xFF; /* high CRC byte initialized */
unsigned char crc_lo = 0xFF; /* low CRC byte initialized */
unsigned int i; /* will index into CRC lookup */
/* pass through message buffer */
while (buffer_length--) {
i = crc_hi ^ *buffer++; /* calculate the CRC */
crc_hi = crc_lo ^ table_crc_hi[i];
crc_lo = table_crc_lo[i];
}
return (crc_hi << 8 | crc_lo);
}



int SendMultiData(char *eth_str,uint16 multi_port,uint8 *sndbuff,uint16 datalen)
{
	 int fd = 0;
	 struct ifreq inter;
	 //char *info = "eth1";
	 
	 if(sndbuff == NULL || datalen <= 0)
	 	return -1;
	 printf("multi send data to:%s,port:%d\n",eth_str,multi_port);
	 strncpy(inter.ifr_name, eth_str, IFNAMSIZ);
	 	
	 struct sockaddr_in Multi_addr;
	 //struct sockaddr_in client_addr;
	 fd=socket(AF_INET,SOCK_DGRAM,0);
	 if(fd<0){
		   perror("socket error");
		   return -1;
	 }
	 //指锟斤拷锟斤拷锟斤拷
	 if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&inter, sizeof(inter)) < 0)
	 {
    	close(fd);
    	return -1;
	 }
	 
	 Multi_addr.sin_family=AF_INET;
	 Multi_addr.sin_port=htons(multi_port);
	 Multi_addr.sin_addr.s_addr=inet_addr(MCAST_ADDR);
	 
	  int size=sendto(fd,sndbuff,datalen,0,(struct sockaddr*)&Multi_addr,sizeof(Multi_addr));
          //printf("send_multidata:sendto,port=%d,len=%d\n",multi_port,size);
	  if(size<0){
	    perror("sendto error");
		close(fd);
		return -1;
	  }
	close(fd);
	return 0;
}



int  send_broadcastdata(uint16 multi_port,uint8 *sndbuff,uint16 datalen)
{
	 int fd = 0;
	 
	 if(sndbuff == NULL || datalen <= 0)
	 	return -1;
	 struct sockaddr_in broadcast_addr;
	 //struct sockaddr_in client_addr;
	 fd=socket(AF_INET,SOCK_DGRAM,0);
	 if(fd<0){
		   perror("socket error");
		   return -1;
	 }
	 
	 broadcast_addr.sin_family=AF_INET;
	 broadcast_addr.sin_port=htons(multi_port);
	 broadcast_addr.sin_addr.s_addr=inet_addr(BCAST_ADDR);
	 
	  int size=sendto(fd,sndbuff,datalen,0,(struct sockaddr*)&broadcast_addr,sizeof(broadcast_addr));
          //printf("send_multidata:sendto,port=%d,len=%d\n",multi_port,size);
	  if(size<0){
	    perror("sendto error");
		close(fd);
		return -1;
	  }
	close(fd);
	return 0;
}


int CreateMultiRecv(uint16 multircv_port,uint8 eth_num,uint8* multi_addr)
{
	int fd  = 0;
	int ret = 0;
	int ttl = 10;
	int loop = 0;
	char ip_addr[256];		
	struct ip_mreq mreq;
	
	if(multircv_port <= 0)
		return -1;

	struct sockaddr_in localaddr;
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd < 0){
	   perror("socket error");
	    return -1;
	}
	if(getLocalHostIP(ip_addr,eth_num) != 0)
	{
		perror("ip address error!");
	    return -1;
	}
	//锟斤拷锟节端口革拷锟斤拷
	int opt = 1;
	setsockopt(fd, SOL_SOCKET,SO_REUSEADDR, (const void *) &opt, sizeof(opt));
	localaddr.sin_family=AF_INET;
	localaddr.sin_port=htons(multircv_port);
	localaddr.sin_addr.s_addr=htonl(INADDR_ANY);//inet_addr(ip_addr);//
	
	ret=bind(fd,(struct sockaddr*)&localaddr,sizeof(localaddr));
	if(ret<0){
	  perror("bind error");
	  return -1;
	}

	if(setsockopt(fd,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl))<0)
	{
	    perror("IP_MULTICAST_TTL");
	    return -1;
	}

	if(setsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop))<0)
	{
	  perror("IP_MULTICAST_LOOP");
	  return -1;
	}

	//eth_num  = get_eth_num();
	//printf("eth_num = %d\n",eth_num);
	
	{
	}
	
	printf("eth%02x,mcast_address:%s,ip address:%s\n",eth_num,multi_addr,ip_addr);
	//mreq.imr_multiaddr.s_addr=inet_addr(MCAST_ADDR);//
	mreq.imr_multiaddr.s_addr=inet_addr(multi_addr);//
	mreq.imr_interface.s_addr=inet_addr(ip_addr);//inet_addr("192.168.11.11");//htonl(INADDR_ANY);
	if(setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0)
	{
	  perror("IP_ADD_MEMBERSHIP");
	  return -1;
	}

	return fd;
} 



int  SendSingleData(uint16 port,uint8 *serverip,uint8 *sndbuff,uint16 datalen)
{
	 int fd = 0;
	 int ret = 0;
	 if(sndbuff == NULL || datalen <= 0 || serverip == NULL)
	 	return -1;
	 struct sockaddr_in addr;
	 fd=socket(AF_INET,SOCK_DGRAM,0);
	 if(fd<0){
		   perror("socket error");
		   return -1;
	 }
#if 0
	 struct sockaddr_in localaddr;
	  localaddr.sin_family=AF_INET;
	  localaddr.sin_port=htons(port);
	  localaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	 ret=bind(fd,(struct sockaddr*)&localaddr,sizeof(localaddr));
	if(ret<0){
		  perror("bind error");
		  return -1;
	}
#endif	 
	int reuse = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR|SO_BROADCAST, &reuse, sizeof(reuse));

	 addr.sin_family=AF_INET;
	 addr.sin_port=htons(port);
	 addr.sin_addr.s_addr=inet_addr(serverip);
	 
	  int size=sendto(fd,sndbuff,datalen,0,(struct sockaddr*)&addr,sizeof(addr));
	  if(size<0){
	    perror("sendto error");
		close(fd);
		return -1;
	  }
	close(fd);
	return 0;
}

int SendTcmsDataToIandG(uint8 *sndbuff,uint16 datalen)
{
	return SendSingleData(SING_SEND_TCMS_INFO_PORT,LoLoaclIpAddr,sndbuff,datalen);
}

int SendReplyDataToIandG(void)
{
	uint8_t sndbuff[SOCKET_BUFF_LEN];
	int datalen = 0;

	HeartBeatTime++;

	sndbuff[0] = 0xAE;
	sndbuff[1] = 0x03;
	sndbuff[2] = 0x00;
	sndbuff[3] = 0x02;
	sndbuff[4] = (HeartBeatTime >> 8) &0xff;
	sndbuff[5] = HeartBeatTime & 0xff;
	unsigned short int calculated_crc = crc16(sndbuff, 6);
	sndbuff[6] = (calculated_crc >> 8) & 0xff;
	sndbuff[7] = calculated_crc & 0xff;

	return SendSingleData(SING_SEND_INSPECTION_PORT,LoLoaclIpAddr,sndbuff,8);
}

int CreateSingleRecv(uint16 port)
{
	  int fd  = 0;
	  int ret = 0;
	  int ttl = 10;
	  int loop = 1;
		  
	  if(port <= 0)
	  	return -1;	  
	  struct sockaddr_in localaddr;
	  fd=socket(AF_INET,SOCK_DGRAM,0);
	  if(fd < 0){
		   perror("socket error");
		    return -1;
	  } 
	    //锟斤拷锟节端口革拷锟斤拷
	  int opt = 1;
	  setsockopt(fd, SOL_SOCKET,SO_REUSEADDR, (const void *) &opt, sizeof(opt));

	  localaddr.sin_family=AF_INET;
	  localaddr.sin_port=htons(port);
	  localaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	  ret=bind(fd,(struct sockaddr*)&localaddr,sizeof(localaddr));
	  if(ret<0)
	  {
		  perror("bind error");
		  return -1;
	  }
	return fd;
} 


//锟斤拷锟斤拷锟介播锟斤拷锟捷ｏ拷锟斤拷锟截斤拷锟秸碉拷锟斤拷锟捷筹拷锟斤拷
int recv_multiData(int fd,uint8 *buf,uint16 buf_len,uint16 multircv_port)
{
	int i = 0,addr_len,ret = 0;
	if(fd <= 0 || buf == NULL || buf_len <= 0 || multircv_port <= 0)
		return -1;
#if 0
	struct sockaddr_in localaddr;
	localaddr.sin_family=AF_INET;
	localaddr.sin_port=htons(multircv_port);
	localaddr.sin_addr.s_addr=htonl(INADDR_ANY);	
	addr_len = sizeof(local_addr);
	
	for(i = 0;i < RETRY_RECV_TIMES;i++)	
	{
		memset(buf,0x0,buf_len);
		ret = recvfrom(fd, buf, buf_len, 0,(struct sockaddr*)&localaddr,&addr_len);	
		if( ret <= 0)	
		{	
			perror("recvfrom:");	
		}
		else
		{
#if 1
			int j = 0;
			printf("recv %d data:",multircv_port);
			for(j=0;j<ret;j++)
				printf("0x%02x ",buf[j]);
			printf("\n");
#endif
			return ret;
		}
	}
	return -1;	
#else
	struct timeval tv = {0, RETRY_RECV_TIMES*1000};
	fd_set readFd;
	int maxfd = fd;
	FD_ZERO(&readFd);
	FD_SET(fd, &readFd);		
	int iRet = select(maxfd + 1, &readFd, NULL, NULL, &tv);
	if(iRet > 0)
	{
		int nbytes = 0;		
		if(FD_ISSET(fd, &readFd))
		{
			memset(buf,0x0,buf_len);
			nbytes = read(fd, buf, buf_len);
			if(nbytes >= 0)
				return nbytes;
		}
	}
	return -1;
#endif
}


int create_localServer(char *serverpath)
{
	if(serverpath == NULL)
		return -1;
	struct sockaddr_un servAddr;
	int serverFd = 0;
	socklen_t socklen = sizeof(struct sockaddr_un);

	bzero(&servAddr, sizeof(servAddr));
	servAddr.sun_family = AF_UNIX;//AF_UNIX
	strcpy(servAddr.sun_path, serverpath);
	unlink(serverpath);	

	if((serverFd = socket(AF_UNIX, SOCK_DGRAM,0)) < 0)
	{
		perror("server socket error\n");
		return -1;
	}

	if(bind(serverFd, (struct sockaddr*)&servAddr,sizeof(servAddr)) < 0){
		close(serverFd);
		perror("server bind error");
		return -1;
	}
	return serverFd;
}

// static void* MyPingThread(void)
// {
// 	int i=0;
// 	while(1)
// 	{
// 		for(i=0;i<4;i++)
// 		{
// 			if(inet_addr(OutNetIp[i])!=inet_addr(Eth0LoaclIpAddr))
// 			{
// 				if(0==myping(OutNetIp[i]))
// 				{
// 					printf("get net ip address:%s\n",OutNetIp[i]);
// 				}
// 			}
// 		}
// 	}
// }

static void* UdpMultiThreadRxEth0(void)
{
	int serverfd;
	char eth0_locla_ip[255];
	int eth_num = 0;
	char buf[SOCKET_BUFF_LEN];
	char last_buf[SOCKET_BUFF_LEN],cur_buf[SOCKET_BUFF_LEN];
	int len = 0;
	int i = 0;
	getLocalHostIP(eth0_locla_ip,eth_num);
	//printf("eth0 local ip address:%s\n",eth0_locla_ip);
	serverfd = CreateMultiRecv(MULTI_COUPLE_OUT_PORT,eth_num,MultiAddress);
	if(serverfd <= 0)
	{
		printf("eth0 bind %d port fail!\n",MULTI_COUPLE_OUT_PORT);
		return;
	}
	printf("out net eth0 bind %d port seccess!\n",MULTI_COUPLE_OUT_PORT);
	
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);
	memset(last_buf,0x0,sizeof(last_buf));
	memset(cur_buf,0x0,sizeof(cur_buf));
	while(1)
	{
			memset(buf,0x0,sizeof(buf));
			len = recvfrom(serverfd, buf, sizeof(buf), 0,(struct sockaddr*)&clientaddr,&addr_len);
			if( len <= 0)	
			{	
				perror("recvfrom eth0:\n");		
			}
			else
			{	
				
				if((clientaddr.sin_addr.s_addr==inet_addr(Eth0LoaclIpAddr))
					||(clientaddr.sin_addr.s_addr==inet_addr(Eth1LoaclIpAddr)))
				{
					//printf("get data from myself\n");
				}
				else
				{
					//send_multidata(8001,buf,len);
					SendMultiData("eth1",MULTI_SEND_DISP_PORT,buf,len);
					/*
					memcpy(cur_buf,buf,len);
					if(0!=memcmp(last_buf,cur_buf,len))
					{
						printf("client source address:%s\n",inet_ntoa(clientaddr.sin_addr));
						printf("RECV DATA eth0,port:%d\n",MULTI_COUPLE_OUT_PORT);
						
						for(i = 0;i<len;i++)
							printf("%02x ",cur_buf[i]);
						printf(" len:%02x\n",len);
						
						memcpy(last_buf,cur_buf,len);
						//if((cur_buf[6]==0xff)||(cur_buf[7]==0xff))
						{
							SendMultiData("eth1",MULTI_SEND_DISP_PORT,cur_buf,len);
						}
					}
					*/
				}
			}
	}
}



static void* UdpMultiThreadRxEth0TV(void)
{
	int serverfd;
	char eth0_locla_ip[255];
	int eth_num = 0;
	char buf[SOCKET_BUFF_LEN];
	char last_buf[SOCKET_BUFF_LEN],cur_buf[SOCKET_BUFF_LEN];
	int len = 0;
	int i = 0;
	getLocalHostIP(eth0_locla_ip,eth_num);
	//printf("eth0 local ip address:%s\n",eth0_locla_ip);
	serverfd = CreateMultiRecv(6666,eth_num,MCAST_ADDR_TV);
	if(serverfd <= 0)
	{
		printf("eth0 bind %d port fail!\n",6403);
		return;
	}
	printf("out net eth0 bind %d port seccess!\n",6403);
	
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);
	memset(last_buf,0x0,sizeof(last_buf));
	memset(cur_buf,0x0,sizeof(cur_buf));
	while(1)
	{
			memset(buf,0x0,sizeof(buf));
			len = recvfrom(serverfd, buf, sizeof(buf), 0,(struct sockaddr*)&clientaddr,&addr_len);
			if( len <= 0)	
			{	
				perror("recvfrom eth0:\n");		
			}
			else
			{	
				
				if((clientaddr.sin_addr.s_addr==inet_addr(Eth0LoaclIpAddr))
					||(clientaddr.sin_addr.s_addr==inet_addr(Eth1LoaclIpAddr)))
				{
					//printf("get data from myself\n");
				}
				else
				{
					//send_multidata(8001,buf,len);
					//SendMultiData("eth1",6403,buf,len);
					/*
					memcpy(cur_buf,buf,len);
					if(0!=memcmp(last_buf,cur_buf,len))
					{
						printf("client source address:%s\n",inet_ntoa(clientaddr.sin_addr));
						printf("RECV DATA eth0,port:%d\n",MULTI_COUPLE_OUT_PORT);
						
						for(i = 0;i<len;i++)
							printf("%02x ",cur_buf[i]);
						printf(" len:%02x\n",len);
						
						memcpy(last_buf,cur_buf,len);
						//if((cur_buf[6]==0xff)||(cur_buf[7]==0xff))
						{
							SendMultiData("eth1",MULTI_SEND_DISP_PORT,cur_buf,len);
						}
					}
					*/
				}
			}
	}
}
int ParseUdpInspectionInfo(uint8 *buffer, uint16 len)
{

	INFO_FROM_TDS info_from_tds;
	uint16 lenth;
	uint16 crc;
	printf("ParseUdpInspectionInfo len:%d\n",len);
	// 解析数据为 UDP_PROTOCOL_INFO 结构体
    UDP_PROTOCOL_INFO *udp_info = (UDP_PROTOCOL_INFO *)buffer;

    // 计算接收到的数据的 CRC 值
	// lenth = sizeof(udp_info->crc);
	// printf("lenth: 0x%02x\n",lenth);
    uint16 calculated_crc = crc16(buffer, len - sizeof(udp_info->crc));
	udp_info->crc = buffer[len - sizeof(udp_info->crc)] << 8 | buffer[len - sizeof(udp_info->crc) + 1];
	// 比较计算出的 CRC 值与数据包中的 CRC 值
    if (calculated_crc != udp_info->crc) {
        printf("CRC check failed. Calculated: 0x%04x, Received: 0x%04x\n", calculated_crc, ntohs(udp_info->crc));
        return -1; // 校验失败
    }
	printf("CRC check passed. Command: 0x%02x\n", udp_info->cmd);

	if(udp_info->cmd == UDP_CMD_CAR_INFO)
	{
		// for(i = 0;i<len;i++)
		// 	printf("%02x ",udp_info->data[i]);
		// printf(" len:%02x\n",len);
		memcpy(&info_from_tds,udp_info->data,sizeof(info_from_tds));
		pdReplyToTcms.recv.tds_active = info_from_tds.tds_active;
		pdReplyToTcms.recv.tds_device_inspection = info_from_tds.tds_device_inspection;
		memcpy(&pdReplyToTcms.recv.tds_IusTrackImg1Abnormal,&info_from_tds.tds_IusTrackImg1Abnormal,8);
		//pdReplyToTcms.recv.tds_IusTrackImg1Abnormal = info_from_tds.tds_IusTrackImg1Abnormal;
		pdReplyToTcms.rlen = sizeof(pdReplyToTcms.recv);
		SendReplyDataToIandG();
	}
	//get pic path
	else if(udp_info->cmd == UDP_CMD_CAR_PIC_PATH)
	{
		if(info_from_tds.tds_pic_deal_finish)
		{
			PicPathFromIspection.len = strlen(udp_info->data);
			strcpy(PicPathFromIspection.tds_pic_path[0],udp_info->data);
			PicPathFromIspection.newdata_flag = 1;
		}
		else
		{
			//没有发送完
		}
		SendReplyDataToIandG();
	}
	//reply to geometry info
	else if(udp_info->cmd == UDP_CMD_CAR_REPLY)
	{
		printf("get cmd:%02x\n",udp_info->cmd);
	}
	else
	{
		printf("get cmd:%02x\n",udp_info->cmd);
	}
}

void udp_clear_newdataflag()
{
	PicPathFromIspection.newdata_flag = 0;
	memset(&PicPathFromIspection,0x00,sizeof(PIC_PATH_FROM_TDS));
}



static void* UdpThreadTdsRecv(void)
{
	int serverfd;
	char eth1_locla_ip[255];	
	int len = 0;
	int i = 0;
	int eth_num = 1;
	char addr=0;
	
	//getLocalHostIP(eth1_locla_ip,eth_num);
	//printf("eth1 local ip address:%s\n",eth1_locla_ip);
	serverfd = CreateSingleRecv(SING_RECV_INSPECTION_PORT);
	if(serverfd <= 0)
	{
		printf("eth1 bind %d port fail!\n",SING_RECV_GEOMETRY_PORT);
		return;
	}
	printf("eth1 bind %d port seccess!\n",SING_RECV_GEOMETRY_PORT);
	uint8 buf[SOCKET_BUFF_LEN];	
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);
	while(1)
	{
			memset(buf,0x0,sizeof(buf));
			len = recvfrom(serverfd, buf, sizeof(buf), 0,(struct sockaddr*)&clientaddr,&addr_len);
			if( len <= 0)	
			{	
				perror("recvfrom eth1:\n");		
			}
			else
			{	
				
				// if((clientaddr.sin_addr.s_addr==inet_addr(Eth0LoaclIpAddr))
				// 	||(clientaddr.sin_addr.s_addr==inet_addr(Eth1LoaclIpAddr)))
				// {
				// 	printf("get data from myself\n");
				// }
				// else
				{
					for(i = 0;i<len;i++)
						printf("%02x ",buf[i]);
					printf(" len:%02x\n",len);
					
					#if 1
					//memcpy(&CurUdpInspectionInfo,buf,len);					
					// if(0 != memcmp(&LastUdpInspectionInfo,&CurUdpInspectionInfo,len))
					// {
					// 	memcpy(&LastUdpInspectionInfo,&CurUdpInspectionInfo,len);							
					// }
					//get info send to tcms
					ParseUdpInspectionInfo(buf,len);
					
					
					//reply to geometry info
				}
					#endif
				
			}
	}
}


static void* UdpThreadTldsRecv(void)
{
	int serverfd;
	char eth1_locla_ip[255];	
	int len = 0;
	int i = 0;
	int eth_num = 1;
	char addr=0;
	
	//getLocalHostIP(eth1_locla_ip,eth_num);
	//printf("eth1 local ip address:%s\n",eth1_locla_ip);
	serverfd = CreateSingleRecv(SING_RECV_GEOMETRY_PORT);
	if(serverfd <= 0)
	{
		printf("eth1 bind %d port fail!\n",SING_RECV_GEOMETRY_PORT);
		return;
	}
	printf("eth1 bind %d port seccess!\n",SING_RECV_GEOMETRY_PORT);
	uint8 buf[SOCKET_BUFF_LEN];	
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);
	while(1)
	{
			memset(buf,0x0,sizeof(buf));
			len = recvfrom(serverfd, buf, sizeof(buf), 0,(struct sockaddr*)&clientaddr,&addr_len);
			if( len <= 0)	
			{	
				perror("recvfrom eth1:\n");		
			}
			else
			{	
				
				if((clientaddr.sin_addr.s_addr==inet_addr(Eth0LoaclIpAddr))
					||(clientaddr.sin_addr.s_addr==inet_addr(Eth1LoaclIpAddr)))
				{
					printf("get data from myself\n");
				}
				else
				{					
					for(i = 0;i<len;i++)
						printf("%02x ",buf[i]);
					printf(" len:%02x\n",len);
					
					#if 1
					memcpy(&CurUdpInspectionInfo,buf,len);
					//get info send to tcms
					//get pic path
					//reply to geometry info
					if(0 != memcmp(&LastUdpInspectionInfo,&CurUdpInspectionInfo,len))
					{
						memcpy(&LastUdpInspectionInfo,&CurUdpInspectionInfo,len);	
						
					}
					#endif
				}
			}
	}
}


pthread_mutex_t mutex;

int udp_init(void)
{
	int i=0;
	int fd_eth0;
	//int len = 0;
	int eth_num = 0;
	char addr=0;
	UDP_PROTOCOL_INFO LastUdpInspectionInfo,CurUdpInspectionInfo;
	char eth0_buf[SOCKET_BUFF_LEN];	
	char last_eth0_buf[SOCKET_BUFF_LEN];	
	char eth0_len;
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);

	// SetEth1Ipaddress();
	getLocalHostIP(Eth0LoaclIpAddr,0);
	getLocalHostIP(Eth1LoaclIpAddr,1);
	//SetIptables(Eth0LoaclIpAddr);
	printf("eth0 local ip address:%s\n",Eth0LoaclIpAddr);
	printf("eth1 local ip address:%s\n",Eth1LoaclIpAddr);
	
	memset(&LastUdpInspectionInfo,0,sizeof(UDP_PROTOCOL_INFO));
	memset(&CurUdpInspectionInfo,0,sizeof(UDP_PROTOCOL_INFO));
	pthread_t t1,t2;
	pthread_mutex_init(&mutex, NULL);
	// pthread_create(&t1, NULL, (void*)MyPingThread, NULL);
	pthread_create(&t1, NULL, (void*)UdpThreadTdsRecv, NULL);	//巡检系统
	pthread_create(&t2, NULL, (void*)UdpThreadTldsRecv, NULL);//几何系统
	

}
// 定义链表节点的数据结构
typedef struct pic_path_node {
    char tds_pic_path[256]; // 存储图片路径
    SLIST_ENTRY(pic_path_node) field; // 链表指针
} pic_path_node_t;

// 定义链表头
typedef SLIST_HEAD(pic_path_list, pic_path_node) pic_path_list_t;

int test_queue(void) {
    // 创建链表头并初始化
    pic_path_list_t head;
    SLIST_INIT(&head);

    // 创建第一个节点并插入链表
    pic_path_node_t *node1 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));
    strcpy(node1->tds_pic_path, "/opt/test/id1/tds/1.jpg");
    SLIST_INSERT_HEAD(&head, node1, field);

    // 创建第二个节点并插入链表
    pic_path_node_t *node2 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));
    strcpy(node2->tds_pic_path, "/opt/test/id1/tds/2.jpg");
    SLIST_INSERT_HEAD(&head, node2, field);

    // 遍历链表并打印节点内容
    printf("链表内容:\n");
    pic_path_node_t *tmp;
    SLIST_FOREACH(tmp, &head, field) {
        printf("%s\n", tmp->tds_pic_path);
    }

    // 创建第三个节点并插入到node2之后
    pic_path_node_t *node3 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));
    strcpy(node3->tds_pic_path, "/opt/test/id1/tds/3.jpg");
    SLIST_INSERT_AFTER(node2, node3, field);

    // 再次遍历链表并打印节点内容
    printf("\n插入新节点后链表内容:\n");
    SLIST_FOREACH(tmp, &head, field) {
        printf("%s\n", tmp->tds_pic_path);
    }

    // 删除node2节点
    printf("\n删除节点node2后链表内容:\n");
    SLIST_REMOVE(&head, node2, pic_path_node, field);
    free(node2);
    SLIST_FOREACH(tmp, &head, field) {
        printf("%s\n", tmp->tds_pic_path);
    }

    // 销毁链表并释放内存
    while (!SLIST_EMPTY(&head)) {
        tmp = SLIST_FIRST(&head);
        SLIST_REMOVE_HEAD(&head, field);
        free(tmp);
    }

    return 0;
}
// int test_queue(void)
// {
// 	/*?创建链表头节点并初始化?*/
// 	pic_path_list_t *head = (pic_path_list_t *)malloc(sizeof(pic_path_list_t));
// 	SLIST_INIT(head);
// 	/*?头插法插入一个节点node1?*/

// 	pic_path_node_t *node1 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));

// 	strcpy((char *)node1->data.tds_pic_path, "/opt/test/id1/tds/1.jpg");
// 	node1->data.len = strlen(node1->data.tds_pic_path);
// 	SLIST_INSERT_HEAD(head, node1, field);

// 	/*?头插法插入一个节点node2?*/
// 	pic_path_node_t *node2 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));
// 	strcpy((char *)node2->data.tds_pic_path, "/opt/test/id1/tds/2.jpg");
// 	node2->data.len = strlen(node2->data.tds_pic_path);
// 	SLIST_INSERT_HEAD(head, node2, field);

// 	/*?遍历打印当前链表节点?*/
// 	printf("list:\n");
// 	pic_path_node_t *tmp_elm;
// 	SLIST_FOREACH(tmp_elm, head, field)
// 	{
// 		printf("%s ", tmp_elm->data.tds_pic_path);
// 	}
// 	printf("\n");
	
// 	/*?尾插法插入一个节点node3?*/
// 	printf("insert node3:\n");
// 	pic_path_node_t *node3 = (pic_path_node_t *)malloc(sizeof(pic_path_node_t));
// 	strcpy((char *)node3->data.tds_pic_path, "/opt/test/id1/tds/3.jpg");
// 	node3->data.len = strlen(node3->data.tds_pic_path);
// 	SLIST_INSERT_AFTER(node2, node3, field);
// 	SLIST_FOREACH(tmp_elm, head, field)
// 	{
// 		printf("%s ", tmp_elm->data.tds_pic_path);
// 	}
// 	printf("\n");
	
// 	/*?删除node2?*/
// 	printf("delete node2:\n");
// 	SLIST_REMOVE(head, node2, node, field);
// 	free(node2);
// 	node2 = NULL;
// 	SLIST_FOREACH(tmp_elm, head, field)
// 	{
// 		printf("%s ", tmp_elm->data.tds_pic_path);
// 	}
// 	printf("\n");
	
// 	/*?销毁链表?*/
// 	while (!SLIST_EMPTY(head))
// 	{
// 		pic_path_node_t *p = SLIST_FIRST(head);
// 		SLIST_REMOVE_HEAD(head, field);
// 		free(p);
// 		p = NULL;
// 	}
// 	free(head);
// 	head = NULL;

// 	return 0;
// }

#if 0
int main()
{
	char *multi_addr="225.0.0.4";
	int serverfd;
	int eth_num = 1;
	printf("-----net card start------- !\n");
	//serverfd = create_multircv(8001);
	getLocalHostIP(Eth1LoaclIpAddr,eth_num);
	printf("eth1 local ip address:%s\n",Eth1LoaclIpAddr);
	serverfd = CreateMultiRecv(8001,1,multi_addr);
	if(serverfd <= 0)
	{
		printf("main bind 8001 port fail!\n");
		return -1;
	}
	char buf[1024];	
	struct sockaddr_in clientaddr;
	int addr_len = sizeof(clientaddr);
	int len = 0;
	int i = 0;
	printf("-----initial success------- !\n");
	while(1)
	{
			memset(buf,0x0,sizeof(buf));
			//len = recv_multiData(serverfd,buf,1024,8001);
			len = recvfrom(serverfd, buf, sizeof(buf), 0,(struct sockaddr*)&clientaddr,&addr_len);
			if( len <= 0)	
			{	
				perror("recvfrom:");	
		
			}
			else
			{	
				printf("RECV DATA:");
				for(i = 0;i<len;i++)
					printf("%02x ",buf[i]);
				printf("\n");
				printf("client source address:%s\n",inet_ntoa(clientaddr.sin_addr));
				
				if(clientaddr.sin_addr.s_addr==inet_addr("192.168.11.11"))
				{
					printf("555555555555\n");
				}
				else
				{
					printf("6666666666666\n");
					send_multidata(8001,buf,len);
				}
			}
			printf("-----receive!!!%0x2------- !\n",len);
	}
}
#endif
