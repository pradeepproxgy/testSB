/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_common.h
********************************************************************************/

/********************************* Include File ********************************/

#ifndef _UTIL_COMMON_H_
#define _UTIL_COMMON_H_

#include "util_macro.h"
/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/
typedef struct {
    char product_key[UNI_PRODUCT_KEY_LEN + 1];
    char product_secret[UNI_PRODUCT_SECRET_LEN + 1];
    char device_key[UNI_DEVICE_KEY_LEN + 1];
    char device_secret[UNI_DEVICE_SECRET_LEN + 1];
    char device_token[UNI_DEVICE_TOKEN_LEN + 1];
	int loginmod;
	int tlsmod;
	int bindmod;
} global_devinfo_s;
/********************************* Variables ***********************************/

/********************************* Function ************************************/





#endif
