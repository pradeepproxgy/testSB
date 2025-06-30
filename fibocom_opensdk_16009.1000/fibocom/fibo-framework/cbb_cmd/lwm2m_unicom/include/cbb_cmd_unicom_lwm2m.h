#ifndef CBB_CMD_UINCOM_MQTT_H
#define CBB_CMD_UINCOM_MQTT_H
#include <stdbool.h>
#include <stdint.h>
#include "ffw_list.h"
#include "ffw_mqttc.h"
#include "ffw_taskqueue.h"
#include "ffw_file.h"
#include "ffw_utils.h"
#include "ffw_crypto.h"
#include "ffw_mqttc.h"
#include "ffw_result.h"
#include "ffw_fs.h"
#include "ffw_sslconf.h"
#include "ffw_list.h"
#include "ffw_rat.h"
#include "ffw_buffer.h"


#define UNICOM_MQTT_KEYINFO_PATH CONFIG_FS_SYS_MOUNT_POINT "unicom"
#define UNICOM_MQTT_PRODUCTKEY_PATH UNICOM_MQTT_KEYINFO_PATH "/productkey"
#define UNICOM_MQTT_PRODUCTSERECT_PATH UNICOM_MQTT_KEYINFO_PATH "/prodctserect"
#define UNICOM_MQTT_DEVICEKEY_PATH UNICOM_MQTT_KEYINFO_PATH "/devicekey"
#define UNICOM_MQTT_DEVICESERECT_PATH UNICOM_MQTT_KEYINFO_PATH "/deviceserect"
#define UNICOM_MQTT_TOKEN_PATH UNICOM_MQTT_KEYINFO_PATH "/token"
#define UNICOM_MQTT_AUTO_REG_PERIOD_PATH UNICOM_MQTT_KEYINFO_PATH "/autoregperiod"
#define UNICOM_MQTT_AUTO_REG_STATUS_PATH UNICOM_MQTT_KEYINFO_PATH "/autoreg_status"

#define UNICOM_MQTT_MPRODUCTKEY_PATH UNICOM_MQTT_KEYINFO_PATH "/mproductkey"
#define UNICOM_MQTT_MPRODUCTSERECT_PATH UNICOM_MQTT_KEYINFO_PATH "/mprodctserect"
#define UNICOM_MQTT_MDEVICEKEY_PATH UNICOM_MQTT_KEYINFO_PATH "/mdevicekey"
#define UNICOM_MQTT_MDEVICESERECT_PATH UNICOM_MQTT_KEYINFO_PATH "/mdeviceserect"
#define UNICOM_MQTT_MTOKEN_PATH UNICOM_MQTT_KEYINFO_PATH "/mtoken"

#define UNICOM_MQTT_AUTO_REG_CERT_PATH UNICOM_MQTT_KEYINFO_PATH "/cert_path"

typedef struct mqtt_user_conf_s mqtt_user_conf_t;
typedef struct mqtt_pubasync_context_s mqtt_pubasync_context_t;

#define UNICOM_MQTT_CLIENT_NUM 2

#define MQTT_STRING_SIZE 2048
#define AT_UNICOM_MQTT_AUTH_MODE_MIN (0)
#define AT_UNICOM_MQTT_AUTH_MODE_MAX (3)
#define AT_UNICOM_MQTT_TLS_FLAG_MIN (0)
#define AT_UNICOM_MQTT_TLS_FLAG_MAX (1)
#define AT_UNICOM_MQTT_DATA_MODE_MIN (0)
#define AT_UNICOM_MQTT_DATA_MODE_MAX (2)
#define AT_UNICOM_MQTT_BIND_FLAG_MIN (0)
#define AT_UNICOM_MQTT_BIND_FLAG_MAX (1)
#define AT_UNICOM_MQTT_PORT_MIN (500)
#define AT_UNICOM_MQTT_PORT_MAX (10000)
#define AT_UNICOM_MQTT_TIMEOUT_MIN (200)
#define AT_UNICOM_MQTT_TIMEOUT_MAX (5000)
#define AT_UNICOM_MQTT_KEEPALIVE_MIN (30)
#define AT_UNICOM_MQTT_KEEPALIVE_MAX (5000)

#define AT_MQTT_TOPIC_MAX_LENGTH   (256)
#define AT_MQTT_MESSAGE_MAX_LENGTH   (1024)
#define UNICOM_MQTT_QOS_MIN (0)
#define UNICOM_MQTT_QOS_MAX (1)
#define AT_MQTT_CLIENT_ID_MIN (1)
#define AT_MQTT_CLIENT_ID_MAX (2)
#define AT_MQTT_CLIENT_MAX_NUM (AT_MQTT_CLIENT_ID_MAX - AT_MQTT_CLIENT_ID_MIN + 1)

#define AT_MQTT_RES_MAX_LEN 512

//unicom regist topic
#define UNICOM_MQTT_SYSTEM "$sys/%s/%s"

#define UNICOM_MQTT_REGIST_SUB "/ext/regist"
#define UNICOM_MQTT_AUTO_REGIST_SUB "/ext/autoregist"
#define UNICOM_MQTT_BATCH_REPLY "/property/batch_reply"
#define UNICOM_MQTT_BATCH "/property/batch"
#define UNICOM_MQTT_MODULE "/module/autoregist"
#define UNICOM_MQTT_MODULE_REPLY "/module/autoregist_reply"

#define UNICOM_MQTT_REGIST_SUB_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_REGIST_SUB
#define UNICOM_MQTT_AUTO_REGIST_SUB_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_AUTO_REGIST_SUB
#define UNICOM_MQTT_BATCH_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BATCH_REPLY
#define UNICOM_MQTT_BATCH_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BATCH
#define UNICOM_MQTT_MODULE_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_MODULE
#define UNICOM_MQTT_MODULE_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_MODULE_REPLY

#define UNICOM_MQTT_BIND_PUB "/deviceCard/pub"
#define UNICOM_MQTT_BIND_UPDATE "/deviceCard/update"
#define UNICOM_MQTT_SHADOW_GET "/deviceShadow/get"
#define UNICOM_MQTT_SHADOW_UPDATE "/deviceShadow/update"
#define UNICOM_MQTT_PSM_GET "/psmEnabled/get"
#define UNICOM_MQTT_BIND_PUB_REPLY "/deviceCard/pub_reply"
#define UNICOM_MQTT_BIND_UPDATE_REPLY "/deviceCard/update_reply"
#define UNICOM_MQTT_SHADOW_GET_REPLY "/deviceShadow/get_reply"
#define UNICOM_MQTT_SHADOW_COMMAND "/deviceShadow/command"
#define UNICOM_MQTT_SHADOW_UPDATE_REPLY "/deviceShadow/update_reply"
#define UNICOM_MQTT_PSM_GET_REPLY "/psmEnabled/get_reply"
#define UNICOM_MQTT_SHADOW_COMMAND_REPLY "/deviceShadow/command_reply"

//unicom upload topic
#define UNICOM_MQTT_BIND_PUB_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BIND_PUB
#define UNICOM_MQTT_BIND_UPDATE_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BIND_UPDATE
#define UNICOM_MQTT_SHADOW_GET_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_GET
#define UNICOM_MQTT_SHADOW_UPDATE_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_UPDATE
#define UNICOM_MQTT_PSM_GET_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_PSM_GET
#define UNICOM_MQTT_SHADOW_COMMAND_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_COMMAND_REPLY


//unicom download topic
#define UNICOM_MQTT_BIND_PUB_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BIND_PUB_REPLY
#define UNICOM_MQTT_BIND_UPDATE_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_BIND_UPDATE_REPLY
#define UNICOM_MQTT_SHADOW_GET_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_GET_REPLY
#define UNICOM_MQTT_SHADOW_COMMAND_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_COMMAND
#define UNICOM_MQTT_SHADOW_UPDATE_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_SHADOW_UPDATE_REPLY
#define UNICOM_MQTT_PSM_GET_REPLY_TOPIC UNICOM_MQTT_SYSTEM UNICOM_MQTT_PSM_GET_REPLY

#define UNICOM_MQTT_TOPIC_BUFFER_LEN 128
#define UNICOM_MQTT_MIN_PAYLOAD_LEN 256
#define UNICOM_MQTT_MIN_BUFFER_LEN 32
#define UNICOM_MQTT_MAX_BUFFER_LEN 128
#define UNICOM_MQTT_AUTO_REG_TIMEOUT 5000
#define UNICOM_MQTT_MESSAGE_TIMEOUT 6000

//unicom code

#define UNICOM_MQTT_SUCCESS                             0
#define UNICOM_MQTT_ERROR                               92


#define UNICOM_MQTT_SUB_FREE(p)                                 \
    if(p != NULL)                                               \
    {                                                           \
        unicom_mqtt_sub_t *s = NULL;                            \
        unicom_mqtt_sub_t *subtmp = NULL;                       \
        list_for_each_entry_safe(s, subtmp, &p->list, list)     \
        {                                                       \
            list_del(&s->list);                                 \
            if(s->topic != NULL)                                \
            {                                                   \
                ffw_put_memory(s->topic);                    \
            }                                                   \
            ffw_put_memory(s);                               \
        }                                                       \
        ffw_put_memory(p);                                   \
        p = NULL;                                               \
    }

#define UNICOM_MQTT_PUB_FREE(p)                 \
    if(p != NULL)                               \
    {                                           \
        if(p->topic != NULL)                    \
        {                                       \
            ffw_put_memory(p->topic);        \
        }                                       \
        if(p->message != NULL)                  \
        {                                       \
            ffw_put_memory(p->message);      \
        }                                       \
        ffw_put_memory(p);                   \
        p = NULL;                               \
    }
#define UNICOM_MQTT_BIND_FREE(p)                \
    if(p != NULL)                               \
    {                                           \
        ffw_put_memory(p);                   \
    }
#define UNICOM_MQTT_SHADOW_FREE(p)              \
    if(p != NULL)                               \
    {                                           \
        ffw_put_memory(p);                   \
    }
#define UNICOM_MQTT_PSM_FREE(p)                 \
    if(p != NULL)                               \
    {                                           \
        ffw_put_memory(p);                   \
    }
#define UNICOM_MQTT_SHADOW_GET_FREE(p)          \
    if(p != NULL)                               \
    {                                           \
        if(p->property != NULL)                 \
        {                                       \
            ffw_put_memory(p->property);     \
        }                                       \
        ffw_put_memory(p);                   \
    }
#define UNICOM_MQTT_CONNECT_INFO_FREE(p)         \
    if(p != NULL)                                \
    {                                            \
        if(p->device_key != NULL)                \
        {                                        \
            ffw_put_memory(p->device_key);    \
        }                                        \
        if(p->device_serect != NULL)             \
        {                                        \
            ffw_put_memory(p->device_serect); \
        }                                        \
        if(p->product_key != NULL)               \
        {                                        \
            ffw_put_memory(p->product_key);   \
        }                                        \
        if(p->product_serect != NULL)            \
        {                                        \
            ffw_put_memory(p->product_serect);\
        }                                        \
        if(p->hostname != NULL)                  \
        {                                        \
            ffw_put_memory(p->hostname);      \
        }                                        \
        if(p->cert_path != NULL)                  \
        {                                        \
            ffw_put_memory(p->cert_path);      \
        }                                        \
        ffw_put_memory(p);                    \
    }

#define UNICOM_MQTT_COPY_STR(d,s)                                 \
    if(s != NULL)                                                 \
    {                                                             \
        if(d != NULL)                                             \
        {                                                         \
            ffw_put_memory(d);                                 \
        }                                                         \
        d = (char *)ffw_get_memory(strlen(s) + 1);             \
        if(d != NULL)                                             \
        {                                                         \
            strncpy(d, s, strlen(s));                             \
        }                                                         \
    }

#define UNICOM_MQTT_COPY_FILE_STR(d, s)                                     \
    ({                                                                      \
        if (d != NULL)                                                      \
        {                                                                   \
            ffw_put_memory(d);                                              \
        }                                                                   \
        int file_size = 0;                                                  \
        if (ffw_file_read_all(s, (void **)&d, &file_size) != FFW_R_SUCCESS) \
        {                                                                   \
            FFW_LOG_ERROR("read file error,path:%s", s);                    \
        }                                                                   \
    })

typedef enum
{
    UNICOM_SUCCESS = 0,
    UNICOM_NOT_RECEIVE_REPLY = 92001,
    UNICOM_PSM_HARDWARE_ENABLE_FAILURE,
    UNICOM_DMP_FORBID_PSM,
    UNICOM_PSM_BINDCARD_FAILURE,
    UNICOM_PSM_UPDATE_OF_BINDCARD_FAILURE, //92005
    UNICOM_NO_THIS_PORPERTY,
    UNICOM_FORMAT_ERROR,
    UNICOM_OPERATING_OF_KEY_FAILURE,
    UNICOM_NOT_FIND_LOCAL_CERT,
    UNICOM_IMPORT_CERT_FAILURE, //92010
    UNICOM_NOT_FIND_LOCATION_CERT,
    UNICOM_AUTHENTICATION_INFORMATION_ERROR,
    UNICOM_DEFINE_ACCESS,
    UNICOM_NETWORK_ANOMALY,
    UNICOM_NO_AUTHENTICATION_INFORMATION_OF_MODULE, //92015
    UNICOM_NO_ACCOUNT_OF_MODULE,
    UNICOM_LOGIN_FAILURE_FOR_BINDONG,
    UNICOM_EXECUTE_FAILURE,
    UNICOM_OTHER_ERROR,
    UNICOM_DISCOONNECT_FAILURE, //92020
    UNICOM_SUBSCRIBER_FAILURE,
    UNICOM_PUBLISH_FALIURE,
    UNICOM_CONNECT_FAILURE,
    UNICOM_COMMAND_PARAMETER_ERROR,
    UNICOM_COMMAND_EXECUTION, //92025
    UNICOM_PSM_ENABLE,
    UNICOM_RECONNECT_SUCCESS = 99000
}unicom_mqtt_result;

#define UNICOM_MQTT_VALUE_INVALID (0)
#define UNICOM_MQTT_VALUE_FALSE  (1 << 0)
#define UNICOM_MQTT_VALUE_TRUE   (1 << 1)
#define UNICOM_MQTT_VALUE_NULL   (1 << 2)
#define UNICOM_MQTT_VALUE_NUMBER (1 << 3)
#define UNICOM_MQTT_VALUE_STRING (1 << 4)

typedef struct unicom_mqtt_sub unicom_mqtt_sub_t;
typedef void (*unicom_mqtt_sub_print_callback)(void *engine, char *topic, int32_t qos);
typedef void (*unicom_mqtt_event_callback)(int event, void *param);

typedef enum
{
    UNICOM_MQTT_EVENT_CONNECT_SUCCESS, //0
    UNICOM_MQTT_EVENT_CONNECT_FAILED,
    UNICOM_MQTT_EVENT_DISCONNECT_SUCCESS,
    UNICOM_MQTT_EVENT_DISCONNECT_FAILED,
    UNICOM_MQTT_EVENT_RE_DISCONNECT,
    UNICOM_MQTT_EVENT_RE_CONNECTE, //5
    UNICOM_MQTT_EVENT_SUB_SUCCESS,
    UNICOM_MQTT_EVENT_SUB_FAILED,
    UNICOM_MQTT_EVENT_UNSUB_SUCCESS,
    UNICOM_MQTT_EVENT_UNSUB_FAILED,
    UNICOM_MQTT_EVENT_PUB_SUCCESS, //10
    UNICOM_MQTT_EVENT_PUB_FAILED,
    UNICOM_MQTT_EVENT_RECV_MESSAGE,
    UNICOM_MQTT_EVENT_BIND_SUB_RESULT,
    UNICOM_MQTT_EVENT_BIND_RESULT,
    UNICOM_MQTT_EVENT_SHADOW_SUB_RESULT, //15
    UNICOM_MQTT_EVENT_SHADOW_GET_RESULT,
    UNICOM_MQTT_EVENT_SHADOW_PUB_RESULT,
    UNICOM_MQTT_EVENT_PSM_SET_RESULT
}unicom_mqtt_event;

typedef enum
{
    UNICOM_MQTT_AUTOREG_NULL = -3,
    UNICOM_MQTT_AUTOREG_EVERY_POWERON = -2,
    UNICOM_MQTT_AUTOREG_ONE_POWERON = -1,
    UNICOM_MQTT_AUTOREG_CLOSE = 0
}unicom_mqtt_autoreg_period;

typedef enum
{
    UNICOM_MQTT_AUTOREG_IDLE = 0,
    UNICOM_MQTT_AUTOREG_SUCCESS
}unicom_mqtt_autoreg_status;

typedef enum
{
    UNICOM_MQTT_CLIENT_AT = 0,
    UNICOM_MQTT_CLIENT_AUTO_REG
}unicom_mqtt_client;

typedef enum
{
    UNICOM_MQTT_SUB_CANCEL = 0,
    UNICOM_MQTT_SUB_CONFIRM
}unicom_mqtt_sub_flag;

typedef enum
{
    UNICOM_MQTT_SUB_FAILED = 0,
    UNICOM_MQTT_SUB_SUCCESS
}unicom_mqtt_sub_status;

typedef enum
{
    UNICOM_MQTT_SUB_SHADOW_COMMAND = 0,
    UNICOM_MQTT_SUB_SHADOW_GET_REPLY,
    UNICOM_MQTT_SUB_SHADOW_UPADTE_REPLY
}unicom_mqtt_shadow_type;

typedef enum
{
    UNICOM_MQTT_SUB_BIND_PUB_REPLY = 0,
    UNICOM_MQTT_SUB_BIND_UPADTE_REPLY
}unicom_mqtt_bind_type;

typedef enum
{
    UNICOM_MQTT_PSM_DISABLE = 0,
    UNICOM_MQTT_PSM_ENABLE
}unicom_mqtt_psm_state;

typedef enum
{
    UNICOM_MQTT_SUB_STANDARD = 0,
    UNICOM_MQTT_SUB_BIND,
    UNICOM_MQTT_SUB_SHADOW,
    UNICOM_MQTT_SUB_PSM
}unicom_mqtt_sub_cmd_type;

typedef enum
{
    UNICOM_MQTT_PUB_STANDARD = 0,
    UNICOM_MQTT_PUB_BIND,
    UNICOM_MQTT_PUB_SHADOW,
    UNICOM_MQTT_PUB_PSM
}unicom_mqtt_pub_cmd_type;

typedef enum
{
    UNICOM_MQTT_DISCONNECTED = 0,
    UNICOM_MQTT_CONNECTED,
    UNICOM_MQTT_RECONNECTED
}unicom_mqtt_connect_flag;

typedef enum
{
    UNICOM_MQTT_AUTH_OTOS_ONLINE = -2,  /* 一型一密免预注册模式上线     */
    UNICOM_MQTT_AUTH_OMAOS = 0,         /* 一机一密模式                */
    UNICOM_MQTT_AUTH_OTOSR,             /* 一型一密预注册模式           */
    UNICOM_MQTT_AUTH_OTOS,              /* 一型一密免予注册模式         */
    UNICOM_MQTT_AUTH_OMOOS              /* 一模一密模式                */
}unicom_mqtt_auth_mode;

typedef struct {
    int8_t auth_mode;
    int8_t conn_mode;
    bool tls_flag;
    bool data_mode;
    char *product_key;
    char *device_key;
    char *product_serect;
    char *device_serect;
    bool bind_flag;
    char *hostname;
    uint16_t port;
    uint16_t keep_alive;
    uint16_t timeout;
    char *username;
    char *clientid;
    char *password;
    char *cert_path;
} unicom_mqtt_connect_t;

typedef struct 
{
    int32_t sub_flag;
    int32_t type;
    int32_t qos;
}unicom_mqtt_shadow_t;

typedef struct 
{
    char *property;
    int32_t qos;
    char *code;
    char *value;
    int32_t value_type;
}unicom_mqtt_shadow_get_t;

typedef struct 
{
    int32_t sub_flag;
    int32_t type;
    int32_t qos;
}unicom_mqtt_bind_t;

typedef struct 
{
    char *topic;
    int32_t qos;
    char *message;
}unicom_mqtt_pub_t;

typedef struct 
{
    char *topic;
    int topic_len;
    uint8_t qos;
    uint8_t *payload;
    uint32_t payload_len;
}unicom_mqtt_message_t;

struct unicom_mqtt_sub
{
    uint8_t status;
    char *topic;
    int32_t qos;
    struct list_head list;
};

typedef struct 
{
    int32_t state;
    int32_t qos;
    int32_t pubstate;
    char *code;
}unicom_mqtt_psm_t;

typedef struct 
{
    int32_t size;
    char path[UNICOM_MQTT_MAX_BUFFER_LEN + 1];
}unicom_mqtt_cert_t;

typedef struct 
{
    //连接次数
    int32_t num;

    //定时器
    void *reconnect_timer;
}unicom_mqtt_reconnect_t;


typedef struct
{
    ffw_mqttc_t *client;                 //mqtt client
    uint32_t msgid;                         //unicom app message id, unique for platform message
    uint8_t sub_cmd_type;                   //current sub type sub/bind/shadow?
    uint8_t pub_cmd_type;                   //current pub type sub/bind/shadow?
    unicom_mqtt_sub_t *sub;                 //sub message info
    unicom_mqtt_pub_t *pub;                 //pub message info
    unicom_mqtt_bind_t *bind;               //bind message info
    unicom_mqtt_shadow_t *shadow;           //shadow message info
    unicom_mqtt_shadow_get_t *shadow_get;   //shadow message info
    unicom_mqtt_psm_t *psm;                 //psm message info
    int32_t connect_state;                  //connect state
    unicom_mqtt_connect_t *connect_info;    //connect info
    char *token;                            //token
    void *autoreg_timer;                    //auto reg timer
    unicom_mqtt_event_callback event_cb;    //event callback
    void *event_arg;                        //event arg
    char* cert_path;                        //cert path
    int16_t cert_size;                      //cert size
    int8_t clientid;                        //unicom mqtt client id
}unicom_mqtt_t;

/* internal function */
/**
 * @brief 根据ID获取mqtt结构体参数
 * 
 * @param id 
 * @return unicom_mqtt_t* 
 */
unicom_mqtt_t *unicom_mqtt_client_handle_get(int id);

/**
 * @brief 获取消息ID，获取后ID会累加
 * 
 * @param unimqtt 
 * @return uint32_t 
 */
uint32_t unicom_mqtt_client_message_id(unicom_mqtt_t *unimqtt);

/**
 * @brief 释放雁飞mqtt结构体参数
 * 
 * @param unimqtt 
 * @return int32_t 
 */
int32_t unicom_mqtt_client_handle_free(unicom_mqtt_t *unimqtt);

/**
 * @brief 输入参数整理，将mqtt_connect参数拷贝至雁飞结构体参数里
 * 
 * @param unimqtt       雁飞mqtt结构
 * @param mqtt_connect  连接参数
 * @param id            客户端ID
 * @return int32_t 
 */
int32_t unicom_mqtt_connect_prepare(unicom_mqtt_t *unimqtt, unicom_mqtt_connect_t *mqtt_connect, int id);

/**
 * @brief 雁飞mqtt，password加密信息获取
 * 
 * @param key       加密KEY
 * @param data      明文数据
 * @param password  密文密码
 * @return int32_t 
 */
int32_t unicom_mqtt_password_get(char *key, char *data, ffw_buffer_t *password);



/* AT api */

/**
 * @brief 模组账户信息设置
 * 
 * @param product_key 
 * @param product_serect 
 * @param device_key 
 * @param device_serect 
 * @return int32_t 
 */
int32_t unicom_mqtt_keyinfom(const char * product_key, const char * product_serect, const char * device_key, const char *device_serect);

/**
 * @brief 读取模组账户信息
 * 
 * @param p_k 
 * @param p_s 
 * @param d_k 
 * @param d_s 
 * @return int32_t 
 */
int32_t unicom_mqtt_read_keyinfom(char** p_k, char** p_s, char** d_k, char** d_s);

/**
 * @brief 删除模组账户信息
 * 
 * @param type 
 * @return int32_t 
 */
int32_t unicom_mqtt_delkeyinfom(int type);

/**
 * @brief 终端账户信息设置
 * 
 * @param product_key 
 * @param product_serect 
 * @param device_key 
 * @param device_serect 
 * @return int32_t 
 */
int32_t unicom_mqtt_keyinfo(const char * product_key, const char * product_serect, const char * device_key, const char *device_serect);

/**
 * @brief 读取终端账户信息
 * 
 * @param p_k 
 * @param p_s 
 * @param d_k 
 * @param d_s 
 * @return int32_t 
 */
int32_t unicom_mqtt_read_keyinfo(char** p_k, char** p_s, char** d_k, char** d_s);

/**
 * @brief 删除终端账户信息
 * 
 * @param type 
 * @return int32_t 
 */
int32_t unicom_mqtt_delkeyinfo(int type);

/**
 * @brief 证书配置
 * 
 * @param path 
 * @param size 
 * @return int32_t 
 */
int32_t unicom_mqtt_certinfo(const char* path, int16_t size);

/**
 * @brief 查询证书地址
 * 
 * @param path 
 * @return int32_t 
 */
int32_t unicom_mqtt_sh_certinfo(const char* path);

/**
 * @brief 删除证书
 * 
 * @param path 
 * @return int32_t 
 */
int32_t unicom_mqtt_del_certinfo(const char* path);

/**
 * @brief 断开雁飞MQTT连接
 * 
 * @param id 客户端ID
 * @return int32_t 
 */
int32_t unicom_mqtt_disconnect(int id);

/**
 * @brief 查询雁飞MQTT连接状态
 * 
 * @param id 客户端ID
 * @param connect_state 连接状态
 * @return int32_t 
 */
int32_t unicom_mqtt_status(int id, int32_t *connect_state);

/**
 * @brief 连接雁飞MQTT
 * 
 * @param mqtt_connect 
 * @param id 客户端ID
 * @param cid 指定CID
 * @param cb 客户端回调参数
 * @return int32_t 
 */
int32_t unicom_mqtt_connect(unicom_mqtt_connect_t *mqtt_connect, int id, int cid,unicom_mqtt_event_callback cb);

/**
 * @brief 连接状态获取
 * 
 * @param id 
 * @param mqtt_connect 
 * @return int32_t 
 */
int32_t unicom_mqtt_connect_read(int id, unicom_mqtt_connect_t **mqtt_connect);

/**
 * @brief 订阅消息
 * 
 * @param id 
 * @param topic 
 * @param sub_flag 
 * @param qos 
 * @param cmd_type 
 * @return int32_t 
 */
int32_t unicom_mqtt_sub(int id, const char *topic, int32_t sub_flag, int32_t qos, uint8_t cmd_type, uint16* pacid);

/**
 * @brief 获取当前订阅信息结果
 * 
 * @param id 客户端ID
 * @param engine AT引擎
 * @param callback 订阅结果回调
 * @return int32_t 
 */
int32_t unicom_mqtt_sub_result(int id, void *engine, unicom_mqtt_sub_print_callback callback);

/**
 * @brief 发布消息
 * 
 * @param id 
 * @param topic 
 * @param qos 
 * @param message 
 * @return int32_t 
 */
int32_t unicom_mqtt_pub(int id, const char *topic, int32_t qos, const char *message);

/**
 * @brief 获取最后一次发布消息结果
 * 
 * @param id 
 * @param topic 
 * @param qos 
 * @param message 
 * @return int32_t 
 */
int32_t unicom_mqtt_pub_result(int id, const char **topic, int32_t *qos, const char **message);

/**
 * @brief 绑定模式订阅
 * 
 * @param id 
 * @param sub_flag 
 * @param type 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_bind_sub(int id, int32_t sub_flag, int32_t type, int32_t qos);

/**
 * @brief 绑定消息结果获取
 * 
 * @param id 
 * @param type 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_bind_sub_result(int id, int32_t *type, int32_t *qos);

/**
 * @brief 雁飞绑定功能
 * 
 * @param id 
 * @param type 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_bind(int id, int32_t type, int32_t qos);

/**
 * @brief 影子消息订阅
 * 
 * @param id 
 * @param sub_flag 
 * @param type 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_shadow_sub(int id, int32_t sub_flag, int32_t type, int32_t qos, uint16* pacid);

/**
 * @brief 最后一次影子消息订阅结果获取
 * 
 * @param id 
 * @param type 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_shadow_sub_result(int id, int32_t *type, int32_t *qos);

/**
 * @brief 获取指定内容影子消息
 * 
 * @param id 
 * @param property 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_shadow_get(int id, const char *property, int32_t qos);

/**
 * @brief 提交指定内容影子消息
 * 
 * @param id 
 * @param property 
 * @param value 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_shadow_pub(int id, const char *property, const char *value, int32_t qos);

/**
 * @brief psm状态上报
 * 
 * @param id 
 * @param state 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_psm_pub(int id, int32_t state, int32_t qos);

/**
 * @brief psm消息获取并设置
 * 
 * @param id 
 * @param state 
 * @param qos 
 * @return int32_t 
 */
int32_t unicom_mqtt_psm_set(int id, int32_t state, int32_t qos);

/**
 * @brief psm消息结果获取
 * 
 * @param id 
 * @param state 
 * @return int32_t 
 */
int32_t unicom_mqtt_psm_set_result(int id, int32_t *state);

/**
 * @brief 自注册任务
 * 
 * @return int32_t 
 */
int32_t unicom_mqtt_auto_register(void);
#endif
