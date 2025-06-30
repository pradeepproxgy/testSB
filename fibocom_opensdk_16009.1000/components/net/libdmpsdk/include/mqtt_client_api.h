/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  mqtt_client_api.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _MQTT_CLIENT_API_H_
#define _MQTT_CLIENT_API_H_
#include "util_types.h"
#include "mqtt_client.h"

/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/
extern void *UNI_MQTT_Init(mqtt_client_param *mqtt_params);
extern int UNI_MQTT_Connect(void *client);
extern int UNI_MQTT_CONN(global_mqttsetinfo_s *unimqttinfo);
extern const char *UNI_MQTT_GetCertInfo(void);
extern void UNI_MQTT_ReleaseCertInfo(const char *cert);
extern void *UNI_MQTT_Create(void);
extern int UNI_MQTT_CheckState(void *client);
extern int UNI_MQTT_Subscribe(void *client, const char *topic,
                              uni_mqtt_qos_s qos,
                              recieve_process_func_p reply_handle_func,
                              void *pcontext);
extern int UNI_MQTT_Unsubscribe(void *client, const char *topic);
extern int UNI_MQTT_Publish(void *client, const char *topic, int qos,
                            void *data, int len);
extern int UNI_MQTT_KeepAlive(void *client, int timeout_ms);
extern int UNI_MQTT_Destroy(void **client);
extern void UNI_SubAckProcess(void *topicData);

#endif
