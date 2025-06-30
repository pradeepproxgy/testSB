/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Q', 'T', 'T')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "oc_gprs.h"
#include "oc_mqtt_demo.h"
#include "fb_config.h"

static UINT32 g_lock = 0;

/*INT8 client_id[] = "lens_0ptfvqWWCzblt9phGtddgOdO3z3";
//INT8 host[] = "test.mosquitto.org";
INT8 host[] = "broker.hivemq.com";
//INT8 host[] = "54.252.162.121";
UINT16 port = 1883;
UINT8 clear_session = 0;
UINT16 keepalive = 70;
UINT8 UseTls = false;
INT32 timeout = 60 * 1000;
BOOL run = false;
BOOL quit = true;
int connect_res_result = 0;
INT8 *sub_topic = "ind/raj/bhi/sub";
INT8 *pub_topic = "ind/raj/bhi/pub";
UINT8 qos = 0;
BOOL retain = 0;
INT8* message = "From MC661 Sound box device";
*/

INT8 client_id[] = "fidesDevice";
//INT8 host[] = "a3323ojfyuiyrv-ats.iot.ap-northeast-1.amazonaws.com";
INT8 host[] = "54.252.162.121";
UINT16 port = 1883;
UINT8 clear_session = 0;
UINT16 keepalive = 120;  // Increased timeout
UINT8 UseTls = false;  // Enable TLS
INT32 timeout = 60 * 1000;
BOOL run = false;
BOOL quit = true;
int connect_res_result = 0;

// AWS IoT Topics
INT8 *sub_topic = "proxgy/audiocube/fides/devices/sub";  // Subscribe to all device messages
INT8 *pub_topic = "proxgy/audiocube/fides/devices/pub";       // Publish to the correct topic

UINT8 qos = 0;
BOOL retain = 0;
INT8* message = "From Sound box device";

typedef struct
{
    char imsi[32];
    char ccid[32];
    char apn[40];
    char ip[80];
    char sim_status;
    char cid_status;
    int rssi;  
    int ber;
    reg_info_t registered_status;
}dev_net_info_t;

static void mqtt_thread_entry(void *param);

void create_mqtt_connect()
{
    run = false;
    connect_res_result = 0;
    fibo_textTrace("mqttapi thread create start");
    // while (!quit)
    // {
    //     fibo_taskSleep(1000);
    // }
    // fibo_textTrace("mqttapi thread create");
    fibo_thread_create(mqtt_thread_entry, "mqtt-thread", 1024 * 16, NULL, OSI_PRIORITY_NORMAL);
}

// void user_signal_process(GAPP_SIGNAL_ID_T sig, va_list arg)
// {
//     fibo_textTrace("mqttapi RECV SIGNAL:%d", sig);
//     char *topic;
//     int8_t qos;
//     char *message;
//     uint32_t len;
//     int ret = 1;
//     switch (sig)
//     {
//     case GAPP_SIG_CONNECT_RSP:
//         connect_res_result = ret = va_arg(arg, int);
//         fibo_textTrace("mqttapi connect :%s", ret ? "ok" : "fail");
//         fibo_sem_signal(g_lock);
//         break;
//     case GAPP_SIG_CLOSE_RSP:
//         ret = va_arg(arg, int);
//         fibo_textTrace("mqttapi close :%s", ret ? "ok" : "fail");
//         break;
//     case GAPP_SIG_SUB_RSP:
//         ret = va_arg(arg, int);
//         fibo_textTrace("mqttapi sub :%s", ret ? "ok" : "fail");
//         fibo_sem_signal(g_lock);
//         break;
//     case GAPP_SIG_UNSUB_RSP:
//         ret = va_arg(arg, int);
//         fibo_textTrace("mqttapi unsub :%s", ret ? "ok" : "fail");
//         fibo_sem_signal(g_lock);
//         break;
//     case GAPP_SIG_PUB_RSP:
//         ret = va_arg(arg, int);
//         fibo_textTrace("mqttapi pub :%s", ret ? "ok" : "fail");
//         fibo_sem_signal(g_lock);
//         break;
//     case GAPP_SIG_INCOMING_DATA_RSP:
//         topic = va_arg(arg, char *);
//         qos = va_arg(arg, int);
//         message = va_arg(arg, char *);
//         len = va_arg(arg, uint32_t);
//         fibo_textTrace("mqttapi recv message :topic:%s, qos=%d message=%s len=%d", topic, qos, message, len);
//         break;

//     default:
//         break;
//     }
// }

// static FIBO_CALLBACK_T user_callback = {
//     .fibo_signal = user_signal_process};

// static void mqtt_thread_entry(void *param)
// {
//     fibo_textTrace("application thread enter, param 0x%x", param);
//     int ret = 0;
//     int test = 1;
//     fibo_textTrace("mqttapi wait network");
//     reg_info_t reg_info;
//     quit = false;
//     run = true;
//     pdp_profile_t pdp_profile;

//     memset(&pdp_profile, 0, sizeof(pdp_profile_t));
//     pdp_profile.cid = 1;
//     memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));

//     if (g_lock == 0)
//     {
//         g_lock = fibo_sem_new(0);
//     }
//     while (test)
//     {
//         fibo_reg_info_get(&reg_info, 0);
//         fibo_taskSleep(1000);
//         fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
//         if(reg_info.nStatus == 1)
//         {
//             test = 0;
//             fibo_pdp_active(1, &pdp_profile, 0);
//             fibo_taskSleep(1000);
//             fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
//             break;
//         }
//     }

//     fibo_taskSleep(1 * 1000);
//     fibo_textTrace("mqtt [%s-%d]", __FUNCTION__, __LINE__);
//     fibo_mqtt_set("test", "123");
//     //fibo_set_ssl_chkmode(0);

//     do
//     {
//         ret = fibo_mqtt_connect_v2(client_id, host, port, clear_session, keepalive, UseTls, timeout);
//         fibo_taskSleep(2000);
//         fibo_textTrace("mqttapi connect ret = %d", ret);
//     } while (ret < 0);

//     fibo_textTrace("mqttapi connect finish");
//     //fibo_sem_wait(g_lock);
//     fibo_textTrace("mqttapi start sub topic");
//     if (connect_res_result)
//     {
//         unsigned short pacid = 0;
//         ret = fibo_mqtt_sub_v2(sub_topic, qos, &pacid);
//         if (ret < 0)
//         {
//             fibo_textTrace("mqttapi sub failed");
//         }
//         ret = fibo_mqtt_pub_v2(pub_topic, qos, retain, message, strlen(message), &pacid);
//         if (ret < 0)
//         {
//             fibo_textTrace("mqttapi sub failed");
//         }
//     }
//     else
//     {
//         run = false;
//         fibo_textTrace("mqttapi connect res fail");
//     }

//     while (1)
//     {
//         fibo_textTrace("mqttapi running");
//         fibo_taskSleep(2 * 1000);
//     }
//     quit = true;
//     fibo_sem_wait(g_lock);
//     fibo_textTrace("mqttapi thread exit");
//     fibo_thread_delete();
// }

static void mqtt_thread_entry(void *param)
{
	fibo_taskSleep(2000);
    fibo_textTrace("mqttapi application thread enter, param 0x%x", param);
    push_index(33);
    int ret = 0;
    int test = 1;
	fibo_taskSleep(2000);
//	DAC_CTRL_ON();
    fibo_textTrace("mqttapi wait network");
    reg_info_t reg_info;
    quit = false;
    run = true;
    // pdp_profile_t pdp_profile;

    // memset(&pdp_profile, 0, sizeof(pdp_profile_t));
    // pdp_profile.cid = 1;
    // memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));
    // UINT8 ip;
	fibo_taskSleep(2000);
    if (g_lock == 0)
    {
        g_lock = fibo_sem_new(0);
    }
    dev_net_info_t dev_net_info;
    bool is_connect=false;
	    fibo_textTrace("mqttapi before while");
	    pdp_profile_t pdp_profile;
	    char *pdp_type = "ipv4v6";
    while(!is_connect)
    {
        fibo_taskSleep(1000);
        ret = fibo_PDPStatus(1,(UINT8*)dev_net_info.ip,(UINT8*)&dev_net_info.cid_status,0);
                  fibo_textTrace(" GSM network ret = %d ip = %d status = %d", ret, dev_net_info.ip, dev_net_info.cid_status);
        if((ret == 0)&&(dev_net_info.ip!=NULL)&&(dev_net_info.cid_status==1)){
            is_connect=true;
        }    
    }
    // while (test)
    // {
    //     fibo_getRegInfo(&reg_info, 0);
    //     fibo_taskSleep(1000);
    //     fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
    //     if(reg_info.nStatus == 1)
    //     {
    //         test = 0;
    //         fibo_PDPActive(1,NULL,0,NULL,NULL,0, &pdp_profile, 0,&ip);
    //         fibo_taskSleep(1000);
    //         fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
    //         break;
    //     }
    // }

    fibo_taskSleep(1 * 1000);
    fibo_textTrace("mqtt [%s-%d]", __FUNCTION__, __LINE__);
    //fibo_mqtt_set("admin", "1234");
    //fibo_set_ssl_chkmode(0);
	fibo_taskSleep(2000);
	fibo_textTrace("mqttapi next to connect with mqtt server");
	fibo_taskSleep(2000);
    do
    {
	fibo_textTrace("mqttapi starting mqtt connction");
        ret = fibo_mqtt_connect(client_id, host, port, 1, keepalive, UseTls);
	connect_res_result=1;
	fibo_textTrace("mqttapi connection established");
        fibo_taskSleep(2000);
        fibo_textTrace("mqttapi connect ret = %d", ret);
    } while (ret < 0);
	fibo_taskSleep(2000);
    fibo_textTrace("mqttapi connect established");
    //fibo_sem_wait(g_lock);
    fibo_taskSleep(2000);
    if (connect_res_result) //initial used connect_res_result
    {
        unsigned short pacid = 0;
	fibo_textTrace("mqttapi start sub topic");
        ret = fibo_mqtt_sub(sub_topic, qos);
	fibo_textTrace("fibo_mqtt_sub = %d",ret);
        if (ret < 0)
        {
            fibo_textTrace("mqttapi sub failed");
        }
	fibo_taskSleep(3000);
	fibo_textTrace("mqttapi start publishing topic");
        ret = fibo_mqtt_pub(pub_topic, qos, retain,message,strlen(message));
	fibo_textTrace("fibo_mqtt_pub = %d",ret);
        if (ret < 0)
        {
            fibo_textTrace("mqttapi pub failed");
        }
	fibo_textTrace("mqttapi start Publish topic successfully");
    }
    else
    {
        run = false;
        fibo_textTrace("mqttapi connect res fail");
    }
	fibo_taskSleep(2000);
	fibo_textTrace("mqttapi entering While loop");
	
    while (1)
    {
        fibo_textTrace("mqttapi running");
        fibo_taskSleep(2 * 1000);
    }
    quit = true;
    fibo_sem_wait(g_lock);
    fibo_textTrace("mqttapi thread exit");
    fibo_thread_delete();
}
// void *appimg_enter(void *param)
// {
//     fibo_textTrace("application image enter, param 0x%x", param);
//     create_mqtt_connect();
//     return &user_callback;
// }

// void appimg_exit(void)
// {
//     fibo_textTrace("application image exit");
// }
