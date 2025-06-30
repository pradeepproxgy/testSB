/*******************************************************************************
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-08-17
* Description	  :  dev_psm_api.h
********************************************************************************/

#ifndef _IOT_EXPORT_DEVCARD_H_
#define _IOT_EXPORT_DEVCARD_H_

#if defined(__cplusplus)
extern "C" {
#endif
/********************************* Include File ********************************/
#include "mqtt_client.h" //ws add

/********************************* Macro Definition ****************************/


/********************************* Type Definition *****************************/

//#include "util_types.h"
//#include "util_defs.h"
typedef enum
{
    PSM_OPEN  = 0,
	DRX_OPEN  = 1,
	EDRX_OPEN = 2,
	PSM_CLOSE = 20,
}PSMShadowMode;
/********************************* Function Prototype Definition ***************/
void UNI_PSMEnable_Reply_handle(void *topicData);//uni_mqtt_event_msg_p msg
void UNI_PSMCommand_Reply_handle(void *topicData);
int UNI_SubPSM_Status(void *client);
int UNI_GetPSM_Status(void *client,char *iccid);
int UNI_Pub_PSMShadow(void *client,char *mode);
int UNI_Sub_PSMShadow(void *client);


#if defined(__cplusplus)
}
#endif
#endif



