/*****************************************************************
*   Copyright (C), 2019, Xian Fibocom Wireless Inc
*   		       All rights reserved.                  
*   FileName    :fibo_lwm2m.h
*   Author      :taric.wang
*   Created     :2019-12-05
*   Description :fibo_lwm2m.h
*****************************************************************/
#pragma once
#ifndef _FIBO_LWM2M_H_
#define _FIBO_LWM2M_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "lwm2m_api.h"


#include "lwm2mclient.h"
#include "liblwm2m.h"
#include "internals.h"

#if(defined WITH_TINYDTLS) 
#include "dtlsconnection.h"
#elif (defined WITH_MBEDDTLS)
#include "mbedconnection.h"
#else
#include "connection.h"
#endif




#include "at_engine.h"
#include "at_command.h"
#include "fibocom_com_api.h"
#define  CONFIG_FIBOCOM_LWM2M
#define  CONFIG_FIBOCOM_LWM2M_ECLOUD


#ifdef CONFIG_FIBOCOM_BASE

#ifdef CONFIG_FIBOCOM_LWM2M

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    int sock;
#if (defined WITH_TINYDTLS) || (defined WITH_MBEDDTLS)
    dtls_connection_t * connList;  
#else
    connection_t * connList;
#endif
    lwm2m_context_t * lwm2mH;
    int addressFamily;
} fibo_client_data_t;




typedef struct
{
	char    *url;      /*1-255*/
	char    remoteIp[FIBOCOM_IP_LEN];
	short   remotePort;      /*1-65535*/
	short   localPort;
	char    *endPoint; /*1-30*/
	uint32  leftTime;  /*20-4294967295*/
	ip_type  ipType;
	char    *psk;
	char    *pskId;    /*8-32*/
	int      pskLen;
	int     sockefd;
	
}fibo_lwm2m;

typedef  struct
{
    char   send_data[FIBOCOM_TCPIP_DATA_LENGTH];
    int    len;
    int    transmode;    
}fibo_ecloud_sendinfo;

void  fibocom_lwm2m_init();
void  fibocom_ecloud_exception_handle(int sock);
void  fibocom_ecloud_normal_data_recv_handle(int sock);


#ifdef CONFIG_FIBOCOM_LWM2M_ECLOUD

#define FIBOCOM_ECLOUD_RX_DATA_LENGTH      512

void atCmdHandleGTLWCTOPEN(atCommand_t *cmd);
void atCmdHandleGTLWCTUPDATE(atCommand_t *cmd);
void atCmdHandleGTLWCTSEND(atCommand_t *cmd);
void atCmdHandleGTLWCTREAD(atCommand_t *cmd);
void atCmdHandleGTLWCTDEL(atCommand_t *cmd);
#endif

#endif
#endif
#ifdef __cplusplus
}
#endif
#endif //_FIBO_LWM2M_H_


