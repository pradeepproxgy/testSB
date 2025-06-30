/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  dev_firstreg_api.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _FIRSTREG_API_H_
#define _FIRSTREG_API_H_

#include "util_types.h"
#include "util_macro.h"
#include "util_common.h"
#include "mqtt_client_api.h"


/********************************* Macro Definition ****************************/
#define MQTT_FIRSTREG_TIMEOUT_MS (10 * 1000)
#define MQTT_DYNREG_SUBNUM 30
//#define dynreg_dbg(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define dynreg_info(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
//#define dynreg_err(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)


/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/

void _mqtt_firstreg_topic_handle(void *topicData);
void _mqtt_freereg_topic_handle(void *topicData);
int32_t UNI_First_Regist( global_devinfo_s *dev_meta);

#endif


