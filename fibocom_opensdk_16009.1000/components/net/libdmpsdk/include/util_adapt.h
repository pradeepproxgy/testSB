/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_adapt.h
********************************************************************************/

/********************************* Include File ********************************/

#ifndef _UTIL_ADAPT_H_
#define _UTIL_ADAPT_H_
#include "HAL_OS.h"

/********************************* Macro Definition ****************************/	
/* DMP PLATFORM TOPIC LIST */
/* list had used */
//动态注册
#define UNI_TOPIC_SUB_REGIST     	    "$sys/%s/%s/ext/regist"
#define UNI_TOPIC_SUB_AUTOREGIST	    "$sys/%s/%s/ext/autoregist"
	
//属性上报
#define UNI_TOPIC_PUB_PROPERTY 	        "$sys/%s/%s/property/pub"
#define UNI_TOPIC_SUB_PUB_REPLY         "$sys/%s/%s/property/pub_reply"
#define UNI_TOPIC_PUB_BATCH_PROPERTY	"$sys/%s/%s/property/batch"
#define UNI_TOPIC_SUB_BATCH_REPLY       "$sys/%s/%s/property/batch_reply"
	
//事件上报
#define UNI_TOPIC_PUB_EVENT 	        "$sys/%s/%s/event/pub"
#define UNI_TOPIC_SUB_EVENT_REPLY       "$sys/%s/%s/event/pub_reply"
#define UNI_TOPIC_PUB_BATCH_EVENT 	    "$sys/%s/%s/event/batch"
#define UNI_TOPIC_SUB_BATCH_EVENT_REPLY "$sys/%s/%s/event/batch_reply"
	
//服务调用
#define UNI_TOPIC_SUB_ASYNCSER 	        "$sys/%s/%s/service/pub"
#define UNI_TOPIC_PUB_ASYNCSER_REPLY    "$sys/%s/%s/service/pub_reply"
#define UNI_TOPIC_SUB_SYNCSER 	        "$sys/%s/%s/sync/pub"
#define UNI_TOPIC_PUB_SYNCSER_REPLY     "$sys/%s/%s/sync/pub_reply"
	
//物网绑定或更新
#define UNI_TOPIC_SUB_DEVICECARD_REPLY   "$sys/%s/%s/deviceCard/pub_reply"
#define UNI_TOPIC_PUB_DEVICECARD         "$sys/%s/%s/deviceCard/pub"
#define UNI_TOPIC_SUB_DEVICECARD_UPREPLY "$sys/%s/%s/deviceCard/update_reply"
#define UNI_TOPIC_PUB_DEVICECARD_UPATE   "$sys/%s/%s/deviceCard/update"
	
//PSM功能
#define UNI_TOPIC_SUB_PSMGETSTATUS_REPLY "$sys/%s/%s/psmEnabled/get_reply"
#define UNI_TOPIC_PUB_PSMGETSTATUS       "$sys/%s/%s/psmEnabled/get"
	
//设备影子
#define UNI_TOPIC_SUB_SHADOW_COMMAND     "$sys/%s/%s/deviceShadow/command"
#define UNI_TOPIC_SUB_SHADOW_UPREPLY     "$sys/%s/%s/deviceShadow/update_reply"
#define UNI_TOPIC_SUB_SHADOW_GETREPLY    "$sys/%s/%s/deviceShadow/get_reply"
#define UNI_TOPIC_PUB_SHADOW_CMDREPLY    "$sys/%s/%s/deviceShadow/command_reply"
#define UNI_TOPIC_PUB_SHADOW_UPDATE	     "$sys/%s/%s/deviceShadow/update"
#define UNI_TOPIC_PUB_SHADOW_GET         "$sys/%s/%s/deviceShadow/get"

//一模一密
#define UNI_TOPIC_SUB_TERMINALREG_REPLY	 "$sys/%s/%s/module/autoregist_reply"
#define UNI_TOPIC_SUB_TERMINALREG	     "$sys/%s/%s/module/autoregist"

#define UNI_EVENT_DEBUG(fmt, ...)  \
    do { \
        HAL_Printf("%s(%03d) :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

/********************************* Type Definition *****************************/

typedef enum {
	UNI_PARAM_SET_PRODUCT_KEY,        
    UNI_PARAM_GET_PRODUCT_KEY,         
    UNI_PARAM_SET_PRODUCT_SECRET,     
    UNI_PARAM_GET_PRODUCT_SECRET,     
    UNI_PARAM_SET_DEVICE_KEY,        
    UNI_PARAM_GET_DEVICE_KEY,         
    UNI_PARAM_SET_DEVICE_SECRET,     
    UNI_PARAM_GET_DEVICE_SECRET,       
    UNI_PARAM_SET_DEVICE_TOKEN,     
    UNI_PARAM_GET_DEVICE_TOKEN, 
    UNI_PARAM_SET_LOGIN_MODE,   
    UNI_PARAM_GET_LOGIN_MODE,   
    UNI_PARAM_SET_TLS_MODE,   
    UNI_PARAM_GET_TLS_MODE,
    UNI_PARAM_SET_BIND_MODE,   
    UNI_PARAM_GET_BIND_MODE,
    UNI_PARAM_SET_FIRST_REGISTER,   
    UNI_PARAM_GET_FIRST_REGISTER,
	UNI_PARAM_SET_HOSTNAME,
	UNI_PARAM_GET_HOSTNAME,
	UNI_PARAM_SET_PORT,
	UNI_PARAM_GET_PORT,
    UNI_PARAM_SET_DOMAIN,
    UNI_PARAM_GET_DOMAIN,
    UNI_PARAM_SET_TIMEOUT,
    UNI_PARAM_GET_TIMEOUT, 
    UNI_PARAM_SET_KEEPALIVE_INTERVAL,
    UNI_PARAM_GET_KEEPALIVE_INTERVAL,
    UNI_PARAM_SET_RECONN_TIME,
    UNI_PARAM_GET_RECONN_TIME
} uni_dev_param_index;
	
typedef enum {
	UNI_LOGIN_MQTT_DEVICE_MOD,        
    UNI_LOGIN_MQTT_TYPE_REG_MOD,       
    UNI_LOGIN_MQTT_TYPE_NOREG_MOD,
    UNI_LOGIN_MQTT_TYPE_MODULE_MOD,
	UNI_LOGIN_HTTP_DEVICE_MOD,        
    UNI_LOGIN_HTTP_TYPE_REG_MOD,       
    UNI_LOGIN_HTTP_TYPE_NOREG_MOD  
} uni_login_mode;
	
typedef enum {
	UNI_LOGIN_NO_TLS_MOD,        
	UNI_LOGIN_TLS_MOD
} uni_tls_mode;
typedef enum
{
	ZEROSTAGE = 0,
	BUDSTAGE = 1,
    REGSTAGE  = 2,
	CONSTAGE  = 3,
}CycleStage;
/********************************* Variables ***********************************/

/********************************* Function Prototype Definition ***************/	
int UNI_Param(int  option, void *data);




#endif  
