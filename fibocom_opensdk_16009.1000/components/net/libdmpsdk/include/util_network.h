/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_network.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _UTIL_NETWORK_H_
#define _UTIL_NETWORK_H_

#include "util_types.h"
#include "HAL_OS.h"
#include <string.h>
/********************************* Macro Definition ****************************/
#define net_flow(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define net_dbg(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define net_info(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define net_warn(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define net_err(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define net_crit(...)				do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)

/********************************* Type Definition *****************************/
typedef struct utils_network  utils_network_s, *utils_network_p;

struct utils_network{
    int32_t handle;
    const char *pHostAddress;
    uint16_t port;
    const char *ca_crt;
    uint16_t ca_crt_len;
	uint16_t tlsmod;
    int (*read)(utils_network_p, char *, uint32_t, uint32_t);
    int (*write)(utils_network_p,const char *, uint32_t, uint32_t);
    int (*connect)(utils_network_p);
    int (*disconnect)(utils_network_p);
};
typedef struct {
    void *(*malloc)(uint32_t size);
    void (*free)(void *ptr);
} ssl_hooks_s;


/* State of MQTT client */
typedef enum {
    UNI_MC_STATE_DISCONNECTED ,               /* MQTT in disconnected state */
    UNI_MC_STATE_CONNECTED ,                  /* MQTT in connected state */
    UNI_MC_STATE_INVALID ,                    /* MQTT in invalid state */
    UNI_MC_STATE_INITIALIZED ,                /* MQTT in initializing state */
    UNI_MC_STATE_DISCONNECTED_RECONNECTING ,  /* MQTT in reconnecting state */
    UNI_MC_STATE_CONNECT_BLOCK                /* MQTT in connecting state when using async protocol stack */
} util_mqtt_state_s;
	


/********************************* Variables ***********************************/

/********************************* Function ************************************/

int uni_network_init(utils_network_p Net_pt, const char *host, uint16_t port, const char *root_cert);

#endif

