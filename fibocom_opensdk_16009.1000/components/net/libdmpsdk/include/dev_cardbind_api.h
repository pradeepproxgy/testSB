
/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  dev_cardbind_api.h
********************************************************************************/
#ifndef _DEVCARDBIND_H_
#define _DEVCARDBIND_H_

#if defined(__cplusplus)
extern "C" {
#endif
/********************************* Include File ********************************/

//#include "util_types.h"
//#include "util_defs.h"

#include "mqtt_client.h" //ws add
/********************************* Macro Definition ****************************/

/********************************* Variables ***********************************/

/********************************* Function Prototype Definition ***************/

void UNI_MQTT_Devcard_Reply_handle(void *topicData);
int UNI_MQTT_DevCardbindRepSub(void *handle,uni_mqtt_qos_s qos);
int UNI_MQTT_DevCardbindUpSub(void *handle,uni_mqtt_qos_s qos);
int UNI_MQTT_DevCardbindReport(void *handle,uni_mqtt_qos_s qos);
int UNI_MQTT_DevCardbindUpdate(void *handle,uni_mqtt_qos_s qos);

/*ws add*/
int UNI_MQTT_DevCardbindRepUnSub(void *handle);
int UNI_MQTT_DevCardbindUpUnSub(void *handle);

#if defined(__cplusplus)
}
#endif
#endif



