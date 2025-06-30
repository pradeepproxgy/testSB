/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_macro.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _UTIL_MACRO_H_
#define _UTIL_MACRO_H_
#include "util_types.h"
/********************************* Macro Definition ****************************/

#define UNI_INIT_ZERO           		0
#define UNI_INIT_TIMEOUT            	5000
#define UNI_INIT_KEEPLIVE    			60000       //unit:ms
//#define UNI_INIT_RECONNUM      	 		1
#define UNI_INIT_RECONNTIME      	 	1000 		//Reconnect interval, unit:ms
#define UNI_BUF_SIZE					1000
#define UNI_PRODUCT_KEY_LEN            (20)
#define UNI_PRODUCT_SECRET_LEN         (32)
#define UNI_DEVICE_KEY_LEN             (32)
#define UNI_DEVICE_SECRET_LEN          (64)
#define UNI_DEVICE_TOKEN_LEN           (64)
#define UNI_AUTH_INFO_LEN              (64)
#define UNI_PROB_MSG_LEN			   (100)
#define UNI_FIREWARE_VER_LEN		   (32)
#define UNI_HARDWARE_VER_LEN	 	   (32)
#define UNI_MANUFACTURE_INFO_LEN	   (32)
#define UNI_APN_INFO_LEN			   (99)
#define UNI_CELLID_LEN				   (32)
#define UNI_RSSI_INFO_LEN			   (8)
#define UNI_IMEI_INFO_LEN			   (16)
#define UNI_IMSI_INFO_LEN			   (16)
#define UNI_ICCID_LEN				   (32)
#define UNI_MODULEINFO_LEN             (32)
#define MQTT_AREA_NUM 				   (2)
#define UNI_DOMAIN_MAX_LEN             (64)
#define UNI_HOSTNAME_MAXLEN  (64)
#define UNI_CLIENT_ID_MAXLEN (200)
#define UNI_USERNAME_MAXLEN  (64)
#define UNI_PASSWORD_MAXLEN  (65)
#define UNI_PASS_HEX_MAXLEN  (32)
#define UNI_PASS_SOURCE_MAXLEN  (65)
#define UNI_CERT_FILENAME_LEN   (128)

#define MQTT_PORT     1883
#define MQTT_TLS_PORT 8883
#define WAIT_CONN_ACK_MAX (10000)//#define WAIT_CONNACK_MAX (10)
#define UNI_TRUE            (1)
#define UNI_FALSE           (0)
#define UNI_MQTT_PACKET_ID_MAX                   (65535)
#define UNI_TOPIC_MAXLEN                (100 + UNI_PRODUCT_KEY_LEN + UNI_DEVICE_KEY_LEN)

#ifndef _IN_
#define _IN_
#endif

#ifndef _OU_
#define _OU_
#endif
/********************************* Type Definition *****************************/
typedef enum UNI_RETURN_CODES {
    RETURN_SUCCESS = 0,
    RETURN_FAILURE = -1,
    INPUT_INVALUE_ERROR = -2,
    MQTT_PUBLISH_ACK_PACKET_ERROR = -19,
	MQTT_PUBLISH_PACKET_ERROR = -21,
	MQTT_CONNECT_BLOCK = -44,
	ERROR_SHADOW_INVALID_STATE = -2000,
	ERROR_SHADOW_WAIT_LIST_OVERFLOW = -2001,
	ERROR_SHADOW_ATTR_EXIST = -2002,
	ERROR_SHADOW_ATTR_NO_EXIST = -2003,       /**< NO such attribute */
	ERROR_SHADOW_NO_ATTRIBUTE = -2004,
}uni_err;

typedef struct {
    char hostname[UNI_HOSTNAME_MAXLEN+1];
    uint16_t port;
    char clientid[UNI_CLIENT_ID_MAXLEN+1];
    char username[UNI_USERNAME_MAXLEN+1];
    char password[UNI_PASSWORD_MAXLEN+1];
} uni_auth_mqtt_s;

typedef enum {
    UNI_DMP_AREA_0,   /* DMP ADRESS */
    UNI_DMP_AREA_1,  /* DMP ADRESS-BAK */
    UNI_DMP_CUSTOM_AREA,     /* CUSTOM SETTING */
    UNI_DMP_AREA_MAX
} uni_mqtt_domain_type;

typedef struct {
    char address[UNI_HOSTNAME_MAXLEN];
    int port;
} uni_domain_item_s;

/********************************* Variables ***********************************/

extern uni_domain_item_s g_mqtt_address_port[MQTT_AREA_NUM];

/********************************* Function ************************************/

#endif

