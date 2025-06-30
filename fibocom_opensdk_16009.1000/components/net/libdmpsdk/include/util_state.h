
/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_state.h
********************************************************************************/

/********************************* Include File ********************************/

#ifndef __UTIL_STATE_H__
#define __UTIL_STATE_H__

/********************************* Macro Definition ****************************/
#define STATE_BASE                                   (0x0000)

#define STATE_SUCCESS                                (STATE_BASE - 0x0000)
#define STATE_IMPORT_BASE                   	     (-0x0100)
#define STATE_IMPORT_PK_ILLEGAL						 (STATE_IMPORT_BASE - 0x0001)
#define STATE_IMPORT_PS_ILLEGAL						 (STATE_IMPORT_BASE - 0x0002)
#define STATE_IMPORT_DK_ILLEGAL						 (STATE_IMPORT_BASE - 0x0003)
#define STATE_IMPORT_DS_ILLEGAL						 (STATE_IMPORT_BASE - 0x0004)
#define STATE_HOSTNAME_TOO_LONG						 (STATE_IMPORT_BASE - 0x0005)
#define STATE_USERNAME_TOO_LONG						 (STATE_IMPORT_BASE - 0x0006)
#define STATE_PASSWD_TOO_LONG            			 (STATE_IMPORT_BASE - 0x0007)
#define STATE_CLIENTID_TOO_LONG            			 (STATE_IMPORT_BASE - 0x0008)
#define STATE_SYS_MALLOC_ERROR                       (STATE_IMPORT_BASE - 0x000A)
#define STATE_USER_INPUT_ILLEGAL                     (STATE_IMPORT_BASE - 0x000B)
#define STATE_SYS_MUTEX_CREATE_ERROR              	 (STATE_IMPORT_BASE - 0x000C)
#define STATE_SYS_HALNET_INVALID_HANDLE				 (STATE_IMPORT_BASE - 0x000D)
#define STATE_SYS_HALNET_CLOSE                 		 (STATE_IMPORT_BASE - 0x000E)
#define STATE_MQTT_PROTOCOL_SERIAL_WRITE_ERROR     	 (STATE_IMPORT_BASE - 0x000F)
#define STATE_MQTT_REC_BUFFER_TOO_SHORT     		 (STATE_IMPORT_BASE - 0x0010)
#define STATE_SYS_HALNET_READ_ERROR 				 (STATE_IMPORT_BASE - 0x0011)
#define STATE_MQTT_PACKET_READ_ERROR 				 (STATE_IMPORT_BASE - 0x0012)

#define STATE_MQTT_BASE                             (-0x0200)
#define STATE_MQTT_CONNACK_VERSION_UNACCEPT         (STATE_MQTT_BASE - 0x0001)
#define STATE_MQTT_CONNACK_IDENT_REJECT             (STATE_MQTT_BASE - 0x0002)
#define STATE_MQTT_CONNACK_SERVICE_NA               (STATE_MQTT_BASE - 0x0003)
#define STATE_MQTT_CONNACK_NOT_AUTHORIZED           (STATE_MQTT_BASE - 0x0004)
#define STATE_MQTT_CONNACK_BAD_USERDATA             (STATE_MQTT_BASE - 0x0005)
#define STATE_MQTT_CONNACK_UNKNOWN_ERROR            (STATE_MQTT_BASE - 0x0006)
#define STATE_MQTT_CONN_RETRY_EXCEED_MAX            (STATE_MQTT_BASE - 0x0007)
#define STATE_MQTT_IN_OFFLINE_STATUS                (STATE_MQTT_BASE - 0x0008)
#define STATE_MQTT_RECV_UNKNOWN_PACKET              (STATE_MQTT_BASE - 0x0009)
#define STATE_MQTT_DESERIALIZE_CONNACK_ERROR        (STATE_MQTT_BASE - 0x000A)
#define STATE_MQTT_DESERIALIZE_UNSUBACK_ERROR       (STATE_MQTT_BASE - 0x000B)
#define STATE_MQTT_DESERIALIZE_SUBACK_ERROR         (STATE_MQTT_BASE - 0x000C)
#define STATE_MQTT_DESERIALIZE_PUB_ERROR            (STATE_MQTT_BASE - 0x000D)
#define STATE_MQTT_PUB_QOS_INVALID                  (STATE_MQTT_BASE - 0x000E)
#define STATE_MQTT_SERIALIZE_PUBACK_ERROR           (STATE_MQTT_BASE - 0x000F)
#define STATE_MQTT_SERIALIZE_SUB_ERROR              (STATE_MQTT_BASE - 0x0010)
#define STATE_MQTT_SERIALIZE_UNSUB_ERROR			(STATE_MQTT_BASE - 0x0011)
#define STATE_MQTT_SERIALIZE_PUB_ERROR              (STATE_MQTT_BASE - 0x0012)
#define STATE_MQTT_SERIALIZE_PINGREQ_ERROR          (STATE_MQTT_BASE - 0x0013)
#define STATE_MQTT_HAL_INIT_FAIL                	(STATE_MQTT_BASE - 0x0014)
#define STATE_MQTT_WAIT_RECONN_TIMER                (STATE_MQTT_BASE - 0x0015)
#define STATE_MQTT_ASYNC_STACK_CONN_IN_PROG         (STATE_MQTT_BASE - 0x0016)
#define STATE_MQTT_REG_INVALID_DS					(STATE_MQTT_BASE - 0x0017)
#define STATE_MQTT_REG_INVALID_DK					(STATE_MQTT_BASE - 0x0018)
#define STATE_MQTT_AUTH_CLIENTID_BUF_SHORT          (STATE_MQTT_BASE - 0x0019)
#define STATE_MQTT_AUTH_HOSTNAME_BUF_SHORT			(STATE_MQTT_BASE - 0x001A)
#define STATE_MQTT_AUTH_USERNAME_BUF_SHORT			(STATE_MQTT_BASE - 0x001B)
#define STATE_MQTT_TOPIC_FORMAT_INVALID             (STATE_MQTT_BASE - 0x001C)
#define STATE_MQTT_TOPIC_BUF_TOO_SHORT              (STATE_MQTT_BASE - 0x001D)
#define STATE_MQTT_TX_BUFFER_TOO_SHORT              (STATE_MQTT_BASE - 0x001E)
#define STATE_MQTT_QOS1_REPUB_EXCEED_MAX            (STATE_MQTT_BASE - 0x001F)
#define STATE_MQTT_CONN_USER_INFO                   (STATE_MQTT_BASE - 0x0021)
#define STATE_MQTT_SUB_INFO                         (STATE_MQTT_BASE - 0x0022)
#define STATE_MQTT_PUB_INFO                         (STATE_MQTT_BASE - 0x0023)
#define STATE_MQTT_UNEXP_TOPIC_FORMAT               (STATE_MQTT_BASE - 0x0024)
#define STATE_MQTT_INVALID_LOGINMOD					(STATE_MQTT_BASE - 0x0025)
#define STATE_MQTT_MODULE_LOG_ERROR					(STATE_MQTT_BASE - 0x0026)
#define STATE_MQTT_GET_ICCID_ERROR					(STATE_MQTT_BASE - 0x0027)
#define STATE_MQTT_HOST_ERROR                       (STATE_MQTT_BASE - 0x0028)
#define STATE_MQTT_DESTROY_ERROR					(STATE_MQTT_BASE - 0x0029)




/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/


#endif
