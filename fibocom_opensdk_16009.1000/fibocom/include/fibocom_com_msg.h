/*****************************************************************
*   Copyright (C), 2018, Xian Fibocom Wireless Inc
*   		       All rights reserved.                  
*   FileName    :fibocom_com_msg.h
*   Author      : taric.wang
*   Created     :2018-10-17
*   Description :fibocom_com_msg.h
*****************************************************************/
#ifndef _FIBOCOM_COM_MSG_H_
#define _FIBOCOM_COM_MSG_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//#include "system.h"
#include "timers.h"
//#include "hal_rtc.h"
//#include "hal_platform.h"
#include "fibocom_com_api.h"
#include "stdbool.h"

// global macro definition
#define FIBOCOM_TCPIP_CLIENT_NUM 4     //TCPIP client num
#define FIBOCOM_TCPIP_NTP_NUM 1        //NTP num
#define FIBOCOM_TCPIP_SERVER_NUM 1     //server num
#define FIBOCOM_TCPIP_COAPS_NUM 0      //coaps num
#define FIBOCOM_TCPIP_HTTP 0           //http  num
#define FIBOCOM_TCPIP_MQTT 0           //mqtt  num
#define FIBOCOM_TCPIP_MAX_ACCEPT_NUM 2 //max remote connect num

#define FIBOCOM_TOTAL_SOCK_NUM (FIBOCOM_TCPIP_CLIENT_NUM + FIBOCOM_TCPIP_NTP_NUM +   \
                                FIBOCOM_TCPIP_SERVER_NUM + FIBOCOM_TCPIP_COAPS_NUM + \
                                FIBOCOM_TCPIP_HTTP + FIBOCOM_TCPIP_MQTT + FIBOCOM_TCPIP_MAX_ACCEPT_NUM)

#define FIBOCOM_TCPIP_RECV_DATA_LENGTH 1024
#define FIBOCOM_AT_COMMAND_LENGTH 1400
#define FIBOCOM_TCPIP_ADDR_LENGTH 40 //ipv4 || ipv6 every group four number hex number,total eight group
#define FIBOCOM_INVALID_PRAM_VALUE -1
#define FIBOCOM_INVALID_SOCKET_ID -1
#define FIBOCOM_INVALID_VALUE -1
#define FIBOCOM_INVALID_IN16 0xFFFF

#define TASK_STACKSIZE 8 * 1024
#define FIBOCOM_TCPIP_QUEUE_MAX_SIZE 64

typedef void (*task_function)(void *);
typedef SemaphoreHandle_t fib_sem_t;
typedef SemaphoreHandle_t fib_mutex_t;
typedef QueueHandle_t fib_mbox_t;
typedef TaskHandle_t fib_thread_t;
typedef TimerHandle_t fib_timer_t;
//typedef bool                boolean;
// golobal enum definition

typedef enum sock_type
{
    sock_local_client,
    sock_server,
    sock_remote_client,
    sock_type_invalid
} sock_type;

typedef enum sock_client_type
{
    sock_tcpip,
    sock_ntp,
    sock_coap,
    sock_http,
    sock_mqtt,
    sock_client_type_invalid,
} sock_client_type;

typedef enum module_type
{
    module_tcpip_io,
    module_tcpip,
    module_ntp,
    moudle_coap,
    module_http,
    module_mqtt,
    module_lbs,
    module_cfun,
    module_invalid
} module_type;

typedef enum msg_type
{
    sock_normal_req,
    sock_clarity_req,
    sock_recv_msg,
    sock_close_msg,
    cfun_attach_req,
    invalid_msg_type
} msg_type;

typedef enum lbs_msg_type
{
    lbs_msg_req,
    lbs_msg_recv,
    lbs_msg_num
} lbs_msg_type;

typedef enum sock_protocal_type
{
    sock_udp,
    sock_tcp,
    invalid_protocal
} sock_protocal_type;

typedef struct module_msg
{
    module_type module;
    int msgtype;
    void *msg;
} module_msg;

typedef struct sock_info
{
    module_type module;
    bool useflag;
    sock_type socktype;
    sock_client_type clienttype;
    sock_protocal_type protocaltype;
    QueueHandle_t remote_queue;
    int sockid;
    int server_sock;
} sock_info;

typedef struct sock_tcpip_msg
{
    sock_type socktype;
    int32_t sockid;
    int32_t datalen;
    char dataBuff[FIBOCOM_TCPIP_RECV_DATA_LENGTH];
} sock_tcpip_msg;

//extern  QueueHandle_t  g_tcpip_queue;  // TCPIP QUEUE

extern bool fibocom_queue_send(QueueHandle_t queue, void *msg);

#endif //_FIBOCOM_COM_MSG_H_
