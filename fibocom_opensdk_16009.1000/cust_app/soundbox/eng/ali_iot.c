/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "infra_config.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "dev_model_api.h"
#include "cJSON.h"

#include "wrappers.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "sequeue.h"
#include "local_flash.h"
#include "ota_service.h"
#include "led.h"
#include "network_service.h"


#ifdef ATM_ENABLED
    #include "at_api.h"
#endif

enum
{
    
    STATE_DEV_MODEL_WRONG_JSON_FORMAT=-4,
    STATE_SYS_DEPEND_MALLOC=-3,
    STATE_USER_INPUT_NULL_POINTER=-2,
    STATE_SYS_DEPEND_SNPRINTF=-1,
    STATE_USER_INPUT_BASE=0

};

//修改此处设备信息即可
char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1fY6JDOTQd";
/* setup your productSecret !!! */
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "6IRSXmUhpHuHpiNi";
/* setup your deviceName !!! */
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "l610-speaker01";
/* setup your deviceSecret !!! */
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "3b0f8dabac89e6cf43ed7b0839e634eb";


char fb_product_key[IOTX_PRODUCT_KEY_LEN + 1];
/* setup your productSecret !!! */
char fb_product_secret[IOTX_PRODUCT_SECRET_LEN + 1];
/* setup your deviceName !!! */
char fb_device_name[IOTX_DEVICE_NAME_LEN + 1];
/* setup your deviceSecret !!! */
char fb_device_secret[IOTX_DEVICE_SECRET_LEN + 1];


#define EXAMPLE_MASTER_DEVID            (0)
#define EXAMPLE_YIELD_TIMEOUT_MS        (200)


#define EXAMPLE_TRACE(...)                                          \
    do {                                                            \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);     \
        HAL_Printf(__VA_ARGS__);                                    \
        HAL_Printf("\033[0m\r\n");                                  \
    } while (0)
#define EXAMPLE_MASTER_DEVID            (0)
#define EXAMPLE_YIELD_TIMEOUT_MS        (200)

typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;
static user_example_ctx_t g_user_example_ctx;


// void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
// {
//     iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

//     switch (msg->event_type) {
//         case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
//             /* print topic name and topic message */
//             EXAMPLE_TRACE("Message Arrived test:");
//             EXAMPLE_TRACE("Topic  test : %.*s", topic_info->topic_len, topic_info->ptopic);
//             EXAMPLE_TRACE("Payload text : %.*s", topic_info->payload_len, topic_info->payload);
//             EXAMPLE_TRACE("\n");
//             break;
//         default:
//             break;
//     }
// }


// int example_subscribe(void *handle)
// {
//     int res = 0;
//     const char *fmt = "/%s/%s/user/get";
//     char *topic = NULL;
//     int topic_len = 0;

//    // const char *fmt2 = ;
//      //char *topic2 = "/sys/a1ITa8t5h8z/switch_test/thing/service/property/set";
//    // int topic_len2 = 0;

//     topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
//     topic = HAL_Malloc(topic_len);
//     if (topic == NULL) {
//         EXAMPLE_TRACE("memory not enough");
//         return -1;
//     }
//     memset(topic, 0, topic_len);
//     HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);
//     res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
//     if (res < 0) {
//         EXAMPLE_TRACE("subscribe failed");
//         HAL_Free(topic);
//         return -1;
//     }
// //topic_len2=strlen(fmt2);
// //topic2 = HAL_Malloc(topic_len2);

// //memset(topic2, 0, topic_len2);
//   //  HAL_Snprintf(topic, topic_len2, fmt2, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

//     HAL_Free(topic);
//     return 0;
// }

/**
 * @brief 上报属性PowerSwitch_1到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性PowerSwitch_1的值，数据类型uint8_t
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_PowerSwitch_1(uint32_t devid, uint8_t value);



/**
 * @brief 上报属性DeviceID到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性DeviceID的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_DeviceID(uint32_t devid, char* value);

/**
 * @brief 上报属性BatteryPercentage到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性BatteryPercentage的值，数据类型double
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_BatteryPercentage(uint32_t devid, double value);

/**
 * @brief 上报属性PlayTTSText到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性PlayTTSText的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_PlayTTSText(uint32_t devid, char* value);


/**
 * @brief 上报事件alarmEvent到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 事件alarmEvent的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_event_alarmEvent(uint32_t devid, char* value);


/**
 * @brief 上报属性PowerSwitch_1到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性PowerSwitch_1的值，数据类型uint8_t
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_PowerSwitch_1(uint32_t devid, uint8_t value)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char property_payload[64] = {0};

    res = HAL_Snprintf(property_payload, sizeof(property_payload), "{\"PowerSwitch_1\": %d}", value);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                            (uint8_t *)property_payload, strlen(property_payload));
    return res;
}

/**
 * @brief 上报属性DeviceID到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性DeviceID的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_DeviceID(uint32_t devid, char* value)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char *property_payload = NULL;
    uint32_t property_payload_len = 0;

    if (value == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    property_payload_len = strlen("DeviceID") + strlen(value) + 10;
    property_payload = HAL_Malloc(property_payload_len);
    if (property_payload == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(property_payload, 0, property_payload_len);

    res = HAL_Snprintf(property_payload, property_payload_len, "{\"DeviceID\": %s}", value);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                            (uint8_t *)property_payload, strlen(property_payload));
    HAL_Free(property_payload);
    return res;
}

/**
 * @brief 上报属性BatteryPercentage到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性BatteryPercentage的值，数据类型double
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_BatteryPercentage(uint32_t devid, double value)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char property_payload[384] = {0};

    EXAMPLE_TRACE("\r\n\r\n--------BatteryPercentage\r\n");
    res = HAL_Snprintf(property_payload, sizeof(property_payload), "{\"BatteryPercentage\": %f}", value);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                            (uint8_t *)property_payload, strlen(property_payload));
    return res;
}

/**
 * @brief 上报属性PlayTTSText到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性PlayTTSText的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_PlayTTSText(uint32_t devid, char* value)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char *property_payload = NULL;
    uint32_t property_payload_len = 0;
     EXAMPLE_TRACE("property_PlayTTSText entered");
    if (value == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    property_payload_len = strlen("PlayTTSText") + strlen(value) + 10;
    property_payload = HAL_Malloc(property_payload_len);
    if (property_payload == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(property_payload, 0, property_payload_len);

    res = HAL_Snprintf(property_payload, property_payload_len, "{\"PlayTTSText\": %s}", value);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                            (uint8_t *)property_payload, strlen(property_payload));
    HAL_Free(property_payload);
    return res;
}

/**
 * @brief 上报属性soft_Version到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 属性soft_Version的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_property_soft_Version(uint32_t devid, char* version)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char *property_payload = NULL;
    uint32_t property_payload_len = 0;

    if (version == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    property_payload_len = strlen("soft_Version") + strlen(version) + 10;
    property_payload = HAL_Malloc(property_payload_len);
    if (property_payload == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(property_payload, 0, property_payload_len);

    res = HAL_Snprintf(property_payload, property_payload_len, "{\"soft_Version\": %s}", version);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                            (uint8_t *)property_payload, strlen(property_payload));
    HAL_Free(property_payload);
    return res;
}



/**
 * @brief 上报事件alarmEvent到云端
 * @param devid 由调用IOT_Linkkit_Open返回的设备标示符
 * @param value 事件alarmEvent的值，数据类型char*
 * @return 消息id:(>=1), 上报失败: <0
 */
int app_post_event_alarmEvent(uint32_t devid, char* value)
{
    int32_t res = STATE_USER_INPUT_BASE;
    char *event_id = "alarmEvent";
    char *event_payload = NULL;
    uint32_t event_payload_len = 0;

    if (value == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
   
    
    event_payload_len = strlen("alarmType") + strlen(value) + 10;
    event_payload = HAL_Malloc(event_payload_len);
    if (event_payload == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(event_payload, 0, event_payload_len);

    HAL_Snprintf(event_payload, event_payload_len, "{\"alarmType\": %s}", value);
    if (res < 0) {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    res = IOT_Linkkit_TriggerEvent(EXAMPLE_MASTER_DEVID, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
    HAL_Free(event_payload);
    return res;
}


/**
 * @brief 解析所有属性设置的值
 * @param request 指向属性设置请求payload的指针
 * @param request_len 属性设置请求的payload长度
 * @return 解析成功: 0, 解析失败: <0
 */
int32_t app_parse_property(const char *request, uint32_t request_len)
{
    cJSON *powerswitch_1 = NULL;
    cJSON *playttstext = NULL;
    uint8_t rx_len=0; 
    cJSON *money_decimal_1 = NULL;
    cJSON *money_1 = NULL;
    cJSON *type_1= NULL;
    cJSON *params_1= NULL;
    //char  integer_str[20]="0";
    //char  decimals_str[10]="0";
    char  momey_str[100]={0};
    char  momey_hex_str[100]={0};
    int decimals_i=0;
    int integer_i=0;
    // int momey_i=0;
    int   pay_type=0;
    char zfb_buf[]="e694afe4bb98e5ae9de588b0e8b4a6";//支付宝到帐
    char wx_buf[]="e5beaee4bfa1e588b0e8b4a6";//微信到帐
    char  RMB_BUF[]="e58583";//元
    char  tts_buff[TTS_MAX_DATA];

    cJSON *req = cJSON_Parse(request);
    if (req == NULL || !cJSON_IsObject(req)) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }

    powerswitch_1 = cJSON_GetObjectItem(req, "PowerSwitch_1");
    if (powerswitch_1 != NULL && cJSON_IsNumber(powerswitch_1)) 
    {
        /* process property PowerSwitch_1 here */
        EXAMPLE_TRACE("property id: PowerSwitch_1, value: %d", powerswitch_1->valueint);
    }

    playttstext = cJSON_GetObjectItem(req, "PlayTTSText");
    if (playttstext != NULL && cJSON_IsString(playttstext)) 
    {
        EXAMPLE_TRACE("property id playttstext: %s",playttstext->valuestring);
       
         cJSON *req_1 = cJSON_Parse(playttstext->valuestring);
        
         params_1 = cJSON_GetObjectItem(req_1, "params");

        if(params_1 != NULL&&cJSON_IsObject(params_1))
        {
            money_1 = cJSON_GetObjectItem(params_1, "money");
            if (money_1 != NULL && cJSON_IsNumber(money_1)) 
            {
                EXAMPLE_TRACE("property id money_1: %d",money_1->valueint);
            //dm_log_info(" money: %d\r\n",money_1->valueint);
                integer_i=money_1->valueint;
            }

            money_decimal_1 = cJSON_GetObjectItem(params_1, "money_decimal");
            if (money_decimal_1 != NULL && cJSON_IsNumber(money_decimal_1)) 
            {
                /* process property PowerSwitch_1 here */
                decimals_i=money_decimal_1->valueint;
                // decimals_i=decimals_f/10;
                EXAMPLE_TRACE("property id decimals_i: %d",decimals_i);
            }

            type_1 = cJSON_GetObjectItem(params_1, "type");
            if (type_1 != NULL && cJSON_IsNumber(type_1)) 
            {
                pay_type = type_1->valueint;
                EXAMPLE_TRACE("property id type_1: %d",pay_type);
            }

            PLAT_LOG("houy debug integer_i = %d, decimals_i = %d", integer_i, decimals_i);
            // momey_f=integer_f+decimals_f;
            sprintf(momey_str,"%d.%d",integer_i,decimals_i);
            int i=0;
            for(i=0;i<strlen(momey_str);i++)
            {
                sprintf(momey_hex_str+i*2,"%02x",momey_str[i]);
            }
            EXAMPLE_TRACE("property id momey_str[%s] momey_hex_str[%s]\r\n",momey_str,momey_hex_str);


           

            memset(tts_buff,0,sizeof(tts_buff));
            if(pay_type==0)//支付宝
            {
                rx_len=strlen(zfb_buf)+strlen(momey_hex_str)+strlen(RMB_BUF)+1;
                HAL_Snprintf(tts_buff,rx_len,"%s%s%s",zfb_buf,momey_hex_str,RMB_BUF);
            }
            else if(pay_type==1)//微信
            {
                rx_len=strlen(wx_buf)+strlen(momey_hex_str)+strlen(RMB_BUF)+1;
                HAL_Snprintf(tts_buff,rx_len,"%s%s%s",wx_buf,momey_hex_str,RMB_BUF);
            }
            else
            {
                rx_len=strlen(momey_hex_str)+strlen(RMB_BUF)+1;
                HAL_Snprintf(tts_buff,rx_len,"%s%s",momey_hex_str,RMB_BUF);
            }
            //将json结构占用的数据空间释放
             cJSON_Delete(req_1);

            if(rx_len<=TTS_MAX_DATA)
            {
                if(QUEUE_IS_FULL==push_tts_play_list(tts_buff))
                {
                    EXAMPLE_TRACE("InQueue failed\r\n");
                }
            }
            else
            {
                EXAMPLE_TRACE("Receive data to long max:%d Byte rx_len:%d\r\n",TTS_MAX_DATA,rx_len);
            }

        }
    }

    cJSON_Delete(req);
    return 0;
}

/** 建连成功事件回调 */
static int user_connected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Connected");
    g_user_example_ctx.cloud_connected = 1;

    led_only_blue_on();

    push_audio_play_list("NET_CONNECTED_CN.mp3");

    return 0;
}

/** 断开连接事件回调 */
static int user_disconnected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Disconnected");
    g_user_example_ctx.cloud_connected = 0;

    if(get_net_connect_sta() == NET_DISCONNECT)
    {
        led_only_blue_set(5,5);
    }
    else
    {
        led_only_blue_set(1,1);
    }

    push_audio_play_list("NET_DISCONNECTED_CN.mp3");

    return 0;
}

/* 设备初始化成功事件回调 */
static int user_initialized(const int devid)
{
    EXAMPLE_TRACE("Device Initialized");
    g_user_example_ctx.master_initialized = 1;

    return 0;
}

/** 事件回调：接收到云端回复属性上报应答 **/
static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    EXAMPLE_TRACE("Message Post Reply Received, Message ID: %d, Code: %d, Reply: %.*s", msgid, code,
                  reply_len,
                  (reply == NULL) ? ("NULL") : (reply));
    return 0;
}

/** 事件回调：接收到云端回复的事件上报应答 **/
static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    EXAMPLE_TRACE("Trigger Event Reply Received, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s",
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

/** 事件回调：接收到云端下发的属性设置 **/
static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;

    EXAMPLE_TRACE("Property Set Received, Request: %s", request);
    app_parse_property(request, request_len);

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    EXAMPLE_TRACE("Post Property return: %d", res);

    return 0;
}

/** 事件回调：接收到云端下发的服务请求 **/
static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    int add_result = 0;
    cJSON *root = NULL, *item_number_a = NULL, *item_number_b = NULL;
    const char *response_fmt = "{\"Result\": %d}";

    EXAMPLE_TRACE("Service Request Received, Service ID: %.*s, Payload: %s", serviceid_len, serviceid, request);

    /* Parse Root */
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    if (strlen("Operation_Service") == serviceid_len && memcmp("Operation_Service", serviceid, serviceid_len) == 0) {
        /* Parse NumberA */
        item_number_a = cJSON_GetObjectItem(root, "NumberA");
        if (item_number_a == NULL || !cJSON_IsNumber(item_number_a)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberA = %d", item_number_a->valueint);

        /* Parse NumberB */
        item_number_b = cJSON_GetObjectItem(root, "NumberB");
        if (item_number_b == NULL || !cJSON_IsNumber(item_number_b)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberB = %d", item_number_b->valueint);

        add_result = item_number_a->valueint + item_number_b->valueint;

        /* Send Service Response To Cloud */
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = (char *)HAL_Malloc(*response_len);
        if (*response == NULL) {
            EXAMPLE_TRACE("Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        HAL_Snprintf(*response, *response_len, response_fmt, add_result);
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    return 0;
}

/** 事件回调：接收到云端回复的时间戳 **/
static int user_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

/** FOTA事件回调处理 **/
static int user_fota_event_handler(int type, const char *version)
{
    char buffer[128] = {0};
    int buffer_length = 128;
    int ret =0;

    /* 0 - new firmware exist, query the new firmware */
    if (type == 0) {
        EXAMPLE_TRACE("New Firmware Version: %s", version);
        push_audio_play_list("OTA_UPGRADING.mp3");
        HAL_SleepMs(1000*3);
        IOT_Linkkit_Query(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_FOTA_DATA, (unsigned char *)buffer, buffer_length);
        if(ret == 0)
            {
                ota_service_enter();
            }
    }

    return 0;
}

/** 事件回调：接收到云端错误信息 **/
static int user_cloud_error_handler(const int code, const char *data, const char *detail)
{
    EXAMPLE_TRACE("code =%d ,data=%s, detail=%s", code, data, detail);
    return 0;
}

/** 事件回调：通过动态注册获取到DeviceSecret **/
static int dynreg_device_secret(const char *device_secret)
{
    EXAMPLE_TRACE("device secret: %s", device_secret);
    return 0;
}

// /** 事件回调: SDK内部运行状态打印 **/
static int user_sdk_state_dump(int ev, const char *msg)
{
    EXAMPLE_TRACE("received state event, -0x%04x(%s)\n", -ev, msg);
    return 0;
}



/**
 * @brief main函数
 */


void ali_iot_loop(void)
{
    int res = 0;
    int cnt = 0;
    iotx_linkkit_dev_meta_info_t master_meta_info;
    int dynamic_register = 0, post_reply_need = 0;
    memset(&g_user_example_ctx, 0, sizeof(user_example_ctx_t));

 
#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0) {
        EXAMPLE_TRACE("IOT_ATM_Init failed!\n");
        return -1;
    }
#endif

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
if(fb_get_dev_params(fb_product_key,fb_product_secret,fb_device_name,fb_device_secret)==0)//获取设备三元组
{
    memcpy(master_meta_info.product_key, fb_product_key, strlen(fb_product_key));
    memcpy(master_meta_info.product_secret, fb_product_secret, strlen(fb_product_secret));
    memcpy(master_meta_info.device_name, fb_device_name, strlen(fb_device_name));
    memcpy(master_meta_info.device_secret, fb_device_secret, strlen(fb_device_secret));
}
 else
{
    memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(master_meta_info.device_name, g_device_name, strlen(g_device_name));
    memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));
}

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* 注册回调函数 */
    IOT_RegisterCallback(ITE_STATE_EVERYTHING, user_sdk_state_dump);
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_FOTA, user_fota_event_handler);
    IOT_RegisterCallback(ITE_CLOUD_ERROR, user_cloud_error_handler);
    IOT_RegisterCallback(ITE_DYNREG_DEVICE_SECRET, dynreg_device_secret);


    /* 选择上线方式（是否使用动态注册） */
    dynamic_register = 1;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* 是否需用属性、事件上报(设置)应答 */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_reply_need);

    do {
        g_user_example_ctx.master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
        if (g_user_example_ctx.master_devid >= 0) {
            break;
        }
        EXAMPLE_TRACE("IOT_Linkkit_Open failed! retry after %d ms\n", 2000);
        HAL_SleepMs(1000);//2S
    } while (1);

    do {
        res = IOT_Linkkit_Connect(g_user_example_ctx.master_devid);
        if (res >= 0) {
            break;
        }

        EXAMPLE_TRACE("IOT_Linkkit_Connect failed! retry after %d ms\n", 5000);
        HAL_SleepMs(1000);//5S
    } while (1);

    {
        app_post_property_PowerSwitch_1(EXAMPLE_MASTER_DEVID, 0);
        app_post_property_DeviceID(EXAMPLE_MASTER_DEVID,"123");
        app_post_property_BatteryPercentage(EXAMPLE_MASTER_DEVID, 100);
        app_post_property_PlayTTSText(EXAMPLE_MASTER_DEVID, "\"hello 123\"");
        app_post_event_alarmEvent(EXAMPLE_MASTER_DEVID, "\"test warning\"");
        app_post_property_soft_Version(EXAMPLE_MASTER_DEVID, APP_VERSION);

    }
    while (1) {
        IOT_Linkkit_Yield(EXAMPLE_YIELD_TIMEOUT_MS);

        // /* Post Proprety Example */
         if ((cnt % 20) == 0) {
        //     app_post_property_PowerSwitch_1(EXAMPLE_MASTER_DEVID, 0);
        //     app_post_property_DeviceID(EXAMPLE_MASTER_DEVID, "\"\"");
                app_post_property_BatteryPercentage(EXAMPLE_MASTER_DEVID, cnt%100);
        //     app_post_property_PlayTTSText(EXAMPLE_MASTER_DEVID, "\"hello 123\"");
        //     app_post_event_alarmEvent(EXAMPLE_MASTER_DEVID, "\"test warning\"");

        }
        cnt++;
        HAL_SleepMs(1000);
    }

    IOT_Linkkit_Close(g_user_example_ctx.master_devid);
    IOT_SetLogLevel(IOT_LOG_NONE);

    return ;
}

static void ali_iot_task(void *param)
{
    while(1)
    {
        ali_iot_loop();
        fibo_taskSleep(1000);
    }
    fibo_thread_delete();
}

int ali_iot_enter(void)
{
    return  fibo_thread_create(ali_iot_task,"_ali_iot_task", 1024*8*4, NULL, OSI_PRIORITY_NORMAL);
}
