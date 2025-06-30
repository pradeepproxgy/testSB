/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  common_model.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _COMMON_MODEL_H_
#define _COMMON_MODEL_H_
#include "HAL_OS.h"


/********************************* Macro Definition ****************************/
#define UNI_SDK_VERSION_MAJOR           2
#define UNI_SDK_VERSION_MINOR           0
#define UNI_SDK_VERSION_PATCH           0


/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/


/********************************* Function ************************************/

char *UNI_GenerateMsgId(void);
void UNI_syncTime();
#endif
