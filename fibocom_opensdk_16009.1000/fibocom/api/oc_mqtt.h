#ifndef OPENCPU_API_OC_MQTT
#define OPENCPU_API_OC_MQTT
#include <stdbool.h>
#include <stdint.h>

#include "fibo_opencpu_comm.h"

#define MQTT_QOS0 (0)
#define MQTT_QOS1 (1)
#define MQTT_QOS2 (2)

/**
* @brief 
* 
* @param psk_id 
* @param psk_key 
* @return UINT32 
 */
UINT32 fibo_mqtt_set_psk(const INT8 *psk_id, const INT8 *psk_key);

/**
* @brief 
* 
* @param ver 
* @return UINT32 
 */
UINT32 fibo_mqtt_set_tls_ver(INT32 ver);

/**
* @brief 
* 
* @param psk_id 
* @param psk_key 
* @return UINT32 
 */
UINT32 fibi_mqtt_set_psk(const INT8 *psk_id, const INT8 *psk_key);

/**
* @brief 
* 
* @param ver 
* @return UINT32 
 */
UINT32 fibi_mqtt_set_tls_ver(INT32 ver);

/**
 * @brief 
 * 
 * @param usr 
 * @param pwd 
 * @return INT32 
 */
INT32 fibo_mqtt_set(INT8 *usr, INT8 *pwd);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @param retain 
 * @param message 
 * @return INT32 
 */
INT32 fibo_mqtt_will(INT8 *topic, UINT8 qos, BOOL retain, INT8 *message);

/**
 * @brief 
 * 
 * @param ClientID 
 * @param DestAddr 
 * @param DestPort 
 * @param CLeanSession 
 * @param keepalive 
 * @param UseTls 
 * @return INT32 
 */
INT32 fibo_mqtt_connect(INT8 *ClientID, INT8 *DestAddr, UINT16 DestPort, UINT8 CLeanSession, UINT16 keepalive, UINT8 UseTls);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @param retain 
 * @param message 
 * @param msglen 
 * @return INT32 
 */
INT32 fibo_mqtt_pub(INT8 *topic, UINT8 qos, BOOL retain, INT8 *message, UINT16 msglen);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @return INT32 
 */
INT32 fibo_mqtt_sub(INT8 *topic, UINT8 qos);

/**
 * @brief 
 * 
 * @param topic 
 * @return INT32 
 */
INT32 fibo_mqtt_unsub(INT8 *topic);

/**
 * @brief 
 * 
 * @return INT32 
 */
INT32 fibo_mqtt_close(void);

/**
 * @brief 
 * 
 * @param usr 
 * @param pwd 
 * @return INT32 
 */
INT32 fibo_mqtt_set1(UINT8 id, INT8 *usr, INT8 *pwd);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @param retain 
 * @param message 
 * @return INT32 
 */
INT32 fibo_mqtt_will1(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message);

/**
 * @brief 
 * 
 * @param ClientID 
 * @param DestAddr 
 * @param DestPort 
 * @param CLeanSession 
 * @param keepalive 
 * @param UseTls 
 * @return INT32 
 */
INT32 fibo_mqtt_connect1(UINT8 id, INT8 *ClientID, INT8 *DestAddr, UINT16 DestPort, UINT8 CLeanSession, UINT16 keepalive, UINT8 UseTls);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @param retain 
 * @param message 
 * @param msglen 
 * @return INT32 
 */
INT32 fibo_mqtt_pub1(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message, UINT16 msglen);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @return INT32 
 */
INT32 fibo_mqtt_sub1(UINT8 id, INT8 *topic, UINT8 qos);

/**
 * @brief 
 * 
 * @param topic 
 * @return INT32 
 */
INT32 fibo_mqtt_unsub1(UINT8 id, INT8 *topic);

/**
 * @brief 
 * 
 * @return INT32 
 */
INT32 fibo_mqtt_close1(UINT8 id);

/**
 * @brief 
 * 
 * @param ClientID 
 * @param DestAddr 
 * @param DestPort 
 * @param CLeanSession 
 * @param keepalive 
 * @param UseTls 
 * @param timeout 
 * @return INT32 
 */
INT32 fibo_mqtt_connect_v2(INT8 *ClientID, INT8 *DestAddr, UINT16 DestPort, UINT8 CLeanSession, UINT16 keepalive, UINT8 UseTls, int32_t timeout);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @param retain 
 * @param message 
 * @param msglen 
 * @return INT32 
 */
INT32 fibo_mqtt_pub_v2(INT8 *topic, UINT8 qos, BOOL retain, INT8 *message, UINT16 msglen, uint16_t *pacid);

/**
 * @brief 
 * 
 * @param topic 
 * @param qos 
 * @return INT32 
 */
INT32 fibo_mqtt_sub_v2(INT8 *topic, UINT8 qos, uint16_t *pacid);

/**
 * @brief 
 * 
 * @param topic 
 * @return INT32 
 */
INT32 fibo_mqtt_unsub_v2(INT8 *topic, uint16_t *pacid);


#endif /* OPENCPU_API_OC_MQTT */

