#ifndef F78312DE_F3E2_4C3B_9AB8_1C1CC02F2249
#define F78312DE_F3E2_4C3B_9AB8_1C1CC02F2249

#include "app.h"

typedef enum
{
    APP_MQTT_MSG_TYPE_EXIT = 0, /// 退出MQTT任务

    APP_MQTT_MSG_TYPE_DATA,/// MQTT数据
}app_mqtt_type_t;

typedef struct
{
    app_mqtt_type_t msg_type;
    int8_t id;
    int8_t qos;
    char *topic;
    char *message;
    uint32_t len;
} app_mqtt_msg_t;

/**
* @brief 
* 
* @return int 
 */
int app_mqtt_init();

/**
* @brief 
* 
* @param id 
* @param topic 
* @param qos 
* @param retain 
* @param message 
* @return int 
 */
int app_mqtt_set_will(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message);


/**
* @brief 
* 
* @param id 
* @param usr 
* @param pwd 
* @param ClientID 
* @param DestAddr 
* @param DestPort 
* @param CLeanSession 
* @param keepalive 
* @param UseTls 
* @return int 
 */
int app_mqtt_open(UINT8 id, INT8 *usr, INT8 *pwd, INT8 *ClientID, INT8 *DestAddr, UINT16 DestPort, UINT8 CLeanSession, UINT16 keepalive, UINT8 UseTls);

/**
* @brief 
* 
* @param id 
* @return int 
 */
int app_mqtt_is_connectted(UINT8 id);

/**
* @brief 
* 
* @param id 
* @param topic 
* @param qos 
* @param retain 
* @param message 
* @param msglen 
* @return int 
 */
int app_mqtt_pub(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message, UINT16 msglen);

/**
* @brief 
* 
* @param id 
* @param topic 
* @param qos 
* @return int 
 */
int app_mqtt_sub(UINT8 id, INT8 *topic, UINT8 qos);

/**
* @brief 
* 
* @param id 
* @param topic 
* @return int 
 */
int app_mqtt_unsub(UINT8 id, INT8 *topic);

/**
* @brief 
* 
* @param msg 
* @param timeout 
* @return int 
 */
int app_mqtt_read_msg(app_mqtt_msg_t *msg, uint32_t timeout);

/**
* @brief 
* 
* @param msg 
* @return int 
 */
int app_mqtt_free_msg(app_mqtt_msg_t *msg);

/**
* @brief 
* 
* @param id 
* @return int 
 */
int app_mqtt_close(int id);

/**
* @brief 
* 
 */
int app_mqtt_start_demo();


int app_mqtt_stop_demo();


int app_mqtt_demo();

#endif /* F78312DE_F3E2_4C3B_9AB8_1C1CC02F2249 */
