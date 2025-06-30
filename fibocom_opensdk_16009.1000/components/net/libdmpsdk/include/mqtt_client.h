/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_adapt.h
********************************************************************************/

#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_

/********************************* Include File ********************************/


#include "util_list.h"
#include "util_network.h"
#include "util_timer.h"
#include "util_log.h"
#include "paho_mqtt_include/MQTTPacket.h"

/********************************* Macro Definition ****************************/
#define UNI_MQTT_CLIENT_VERSION                    (4)
#define UNI_MQTT_FAULT_TIMEOUT                     (5000)
#define UNI_MQTT_TIMEOUT_MIN                       (1000)
#define UNI_MQTT_TIMEOUT_MAX                       (10000)
#define UNI_MQTT_FAULT_KEEPALIVE_INTERVAL          (60)
#define UNI_MQTT_KEEPALIVE_INTERVAL_MIN            (30)
#define UNI_MQTT_KEEPALIVE_INTERVAL_MAX            (1200)
#define UNI_MQTT_RECONNECT_INTERVAL_MIN            (1000)
#define UNI_MQTT_RECONNECT_INTERVAL_MAX            (30000)
#define UNI_MC_KEEPALIVE_NUM_MAX                 (2)


#define RETRY_TIMES_LIMIT                          (10+1)
#define RETRY_INTV_PERIOD                          (20000)
#define MQTT_BUF_REC_EDGE                          (8)
#define MQTT_BUF_SEND_EDGE                         (64)
#define MUTLI_SUBSCIRBE_MAX                        (5)
#define MQTT_TOPIC_MAX_LEN                         (128)
#define UNI_MQTT_TX_MAX_LEN						   (2048)
#define UNI_MQTT_RX_MAX_LEN						   (2048)
#define UNI_MQTT_REPUB_NUM_MAX					   (10)

#define MAX_PACKET_ID 65535        // according to the MQTT specification - do not change!
#define MAX_MESSAGE_HANDLERS 10    // redefinable - how many subscriptions do you want?

//#define mqtt_flow(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define mqtt_dbg(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define mqtt_info(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define mqtt_warn(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define mqtt_err(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define mqtt_crit(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)

#define mqtt_malloc(size)			 HAL_Malloc(size)
#define mqtt_free(ptr)				 {HAL_Free((void *)ptr);ptr = NULL;}
//#define MQTT_TASK         0
typedef void UNIMQTTAsync_Success(void* context);

typedef void UNIMQTTAsync_Fail(void* context);

typedef void UNIMQTTAsync_PostEvent(uint8_t nEventId);
/********************************* Type Definition *****************************/

#define UNIMQTT_ASYNC_STACK         1

/* State of MQTT client */
typedef enum {
    UNI_MQTTC_STATE_INVALID ,
    UNI_MQTTC_STATE_INITIALIZED,
    UNI_MQTTC_STATE_CONNECTED,
    UNI_MQTTC_STATE_DISCONNECTED,
    UNI_MQTTC_STATE_DISCONNECTED_RECONNECTING,
    UNI_MQTTC_STATE_CONNECT_BLOCK
} uni_mqttc_state_s;
typedef enum {
    UNI_MQTT_EVENT_UNDEF ,
    UNI_MQTT_EVENT_DISCONNECT,
    UNI_MQTT_EVENT_RECONNECT,
    UNI_MQTT_EVENT_SUBCRIBE_SUCCESS,
    UNI_MQTT_EVENT_SUBCRIBE_TIMEOUT,
    UNI_MQTT_EVENT_SUBCRIBE_ACK,
    UNI_MQTT_EVENT_UNSUBCRIBE_SUCCESS,
    UNI_MQTT_EVENT_UNSUBCRIBE_TIMEOUT,
    UNI_MQTT_EVENT_UNSUBCRIBE_ACK,
    UNI_MQTT_EVENT_PUBLISH_SUCCESS,
    UNI_MQTT_EVENT_PUBLISH_TIMEOUT,
    UNI_MQTT_EVENT_PUBLISH_ACK,
    UNI_MQTT_EVENT_PUBLISH_RECEIVED,
    UNI_MQTT_EVENT_BUFFER_OVERFLOW
} uni_mqtt_event_type;

typedef enum MQTT_NODE_STATE {
    UNI_MQTTC_NODE_NORMANL ,
    UNI_MQTTC_NODE_INVALID,
} uni_mqttc_node_s;

typedef enum {
    UNI_MQTT_QOS_0,      //最多发送一次
    UNI_MQTT_QOS_1,      //最少发送一次
    UNI_MQTT_QOS_2,      //只被接收一次
    UNI_MQTT_QOS_3_L
} uni_mqtt_qos_s;

typedef enum
{
    BUFFER_OVERFLOW = -2,
    FAILURE = -1,
    SUCCESS = 0,
}MQTTRetCode;
typedef struct
{
    uni_mqtt_qos_s qos;               //消息QOS等级
    unsigned char retained;    //消息是否保留
    unsigned char dup;         //消息重发标志
    unsigned short id;         //消息ID
    void *payload;             //消息内容
    size_t payloadlen;         //消息长度
}MqttMsg_t;
typedef struct
{
    char *topicName;
    int   topiclen;
    char *payload;
    int   payloadlen;
}MqttMsgTopicData2_t;

typedef struct
{
    unsigned char rc;
    unsigned char sessionPresent;
}MQTTConnackData;	

typedef struct
{
    uni_mqtt_qos_s grantedQoS;
}MQTTSubackData;

typedef void (*MqttMsgHandler)(void *);

typedef struct
{
	const char *topicFilter;
	MqttMsgHandler fp;
}MqttMsgHandlers_t;

typedef struct
{
    unsigned int next_packetid;
    unsigned int command_timeout_ms;
    size_t buf_size;
    size_t readbuf_size;
    unsigned char *buf;
    unsigned char *readbuf;
    unsigned int keepAliveInterval;    //unit: s
    char ping_outstanding;
    int isconnected;
    int cleansession;
	MqttMsgHandlers_t messageHandlers[MAX_MESSAGE_HANDLERS];
	MqttMsgHandler defaultMessageHandler;
    utils_network_s *ipstack;
    Timer last_sent, last_received;
	void          *common_lock; 
#if defined(MQTT_TASK)
     osiMutex_t *mutex;
    //Thread thread;
#endif
     UNIMQTTAsync_PostEvent *UNIPOSTEvent;
     char topicfiler[256];
      char topicsave[256];
     uint8_t       qos;
     MqttMsgHandler UNIMQTTMessageHandler;
     int suback_isarrived;

} MQTTClient;
typedef struct
{
    char *topicName;
    int   topiclen;
    char *payload;
    int   payloadlen;
}UNI_MQTT_TopicData;

typedef struct {
    uni_mqtt_event_type  event_type;
    void *msg;
} uni_mqtt_event_msg_s, *uni_mqtt_event_msg_p;


typedef struct node_t
{
    int   qos;
    char TopicStr[256];
    struct node_t *next;

} linknode_t, *linklist_t;

typedef void (*recieve_process_func_p)(void *topicData);//uni_mqtt_event_msg_p msg

/* The structure of MQTT event handle */
typedef struct {
    recieve_process_func_p     fun_p;
    void                               *pcontext;
} uni_process_handle_s, *uni_process_handle_p;

typedef struct {
    const char                 *host;   
    uint16_t                    port;  
    uint16_t                    tls_mod;
    const char                 *root_cert;

    const char                 *client_id;  
    const char                 *username;    
    const char                 *password;              
    uint8_t                     clean_session;              
    uint32_t                    keepalve_interval;   
    uint32_t                    time_out; 

    uint8_t                    *recvBuf;
    uint8_t                    *sendBuf;
    uint32_t                    read_buf_size;          
    uint32_t                    write_buf_size;  
    UNIMQTTAsync_Success *UNISuccess;
    UNIMQTTAsync_Fail *UNIFail;
} mqtt_client_param, *mqtt_client_param_pt;

typedef struct {
    uintptr_t package_id;
    uint8_t ack_type;
    recieve_process_func_p sub_state_cb;
    struct dlist_head linked_list;
} mqtt_sub_sync_node_s;


/* topic information */
//typedef struct {
//    uint16_t        package_id;
//    uint8_t         qos;
//    uint8_t         dup;
//    uint8_t         retain;
//    uint16_t        topic_len;
//    uint32_t        payload_len;
//    const char     *ptopic;
//    const char     *payload;
//}uni_mqtt_topic_s, *uni_mqtt_topic_p;
typedef enum {
    UNI_MQTTC_CONN_ACCEPTED,
    UNI_MQTTC_CONN_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION,
    UNI_MQTTC_CONN_REFUSED_IDENTIFIER_REJECTED,
    UNI_MQTTC_CONN_REFUSED_SERVER_UNAVAILABLE,
    UNI_MQTTC_CONN_REFUSED_BAD_USERDATA,
    UNI_MQTTC_CONN_REFUSED_NOT_AUTHORIZED
} iotx_mc_connect_ack_code_t;
typedef enum {
    UNI_TOPIC_NAME_TYPE,
    UNI_TOPIC_FILTER_TYPE
} uni_mqttc_topic_type_s;


typedef struct  {
    uni_mqttc_topic_type_s topic_type;
    uni_process_handle_s handle;
    const char *topic_filter;
    struct dlist_head linked_list;

} uni_mqttc_topic_handle_s;


typedef struct {
    int authmod;
    int loginmod;
    int tlsmod;
    int datamode;
    int bindmod;
    int timeout;
    int keepalive;
    UNIMQTTAsync_Success *UNISuccess;
    UNIMQTTAsync_Fail *UNIFail;
} global_mqttsetinfo_s;

/********************************* Variables ***********************************/

/********************************* Function Prototype Definition ***************/

void *MqttClientConnect(mqtt_client_param *param);
int MqttClientSubscribe(void *handle, const char *topicFilter, uni_mqtt_qos_s qos, MqttMsgHandler msgHandler);
int MqttClientPublish(void *handle, const char *topicName, MqttMsg_t *message);
int MqttClientUnsubscribe(void *handle, const char *topicFilter);
int MqttClientReconnect(void *handle, mqtt_client_param *param);
int MqttClientYield(void *handle, int timeout_ms);
int MqttClientDisconnect(void **handle);
util_mqtt_state_s MqttClientGetstate(MQTTClient *handle);
void MqttClientSetstate(MQTTClient *handle, util_mqtt_state_s mcState);



#endif
