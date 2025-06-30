/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  tuple_trans.h
********************************************************************************/

/********************************* Include File ********************************/


#ifndef _TUPLE_TRANS_H_
#define _TUPLE_TRANS_H_

#include "util_types.h"
#include "util_macro.h"
#include "util_common.h"
/********************************* Macro Definition ****************************/	

/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/


/********************************* Function Prototype Definition ***************/	

int UNI_FUNC_SET_PRODUCT_KEY(void *data);
int UNI_FUNC_GET_PRODUCT_KEY(void *data);
int UNI_FUNC_SET_PRODUCT_SECRET(void *data);
int UNI_FUNC_GET_PRODUCT_SECRET(void *data);
int UNI_FUNC_SET_DEVICE_KEY(void *data);
int UNI_FUNC_GET_DEVICE_KEY(void *data);
int UNI_FUNC_SET_DEVICE_SECRET(void *data);
int UNI_FUNC_GET_DEVICE_SECRET(void *data);
int UNI_FUNC_SET_DEVICE_TOKEN(void *data);
int UNI_FUNC_GET_DEVICE_TOKEN(void *data);
int UNI_FUNC_SET_TLS_MODE(void *data);
int UNI_FUNC_GET_TLS_MODE(void *data);
int UNI_FUNC_SET_LOGIN_MODE(void *data);
int UNI_FUNC_GET_LOGIN_MODE(void *data);
int UNI_FUNC_SET_BIND_MODE(void *data);
int UNI_FUNC_GET_BIND_MODE(void *data);
int UNI_FUNC_SET_MQTT_DOMAIN(void *data);
int UNI_FUNC_GET_MQTT_DOMAIN(void *data);
int UNI_FUNC_SET_FIRST_REGIST(void *data);
int UNI_FUNC_GET_FIRST_REGIST(void *data);
int UNI_FUNC_SET_HOSTNAME(void *data);
int UNI_FUNC_GET_HOSTNAME(void *data);
int UNI_FUNC_SET_PORT(void *data);
int UNI_FUNC_GET_PORT(void *data);
int UNI_FUNC_SET_TIMEOUT(void *data);
int UNI_FUNC_GET_TIMEOUT(void *data);
int UNI_FUNC_SET_KEEPALIVE_INTERVAL(void *data);
int UNI_FUNC_GET_KEEPALIVE_INTERVAL(void *data);
int UNI_FUNC_SET_RECONN_TIME(void *data);
int UNI_FUNC_GET_RECONN_TIME(void *data);

#endif



