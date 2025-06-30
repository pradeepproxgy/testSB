#ifndef __FIBO_BLE_COMMON_H__
#define __FIBO_BLE_COMMON_H__

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "osi_api.h"
#include "fibo_log.h"
#include "at_command.h"
#include "at_response.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <netdb.h>

#ifdef CONFIG_FIBOCOM_BLE_SUPPORT
#define FIBO_BLE_DATA_LEN   512
#define FIBO_BLE_RANDOM_MAC 1
#define FIBO_BLE_PUBLIC_MAC 0
#define FIBO_BLE_ADV_ON     1
#define FIBO_BLE_ADV_OFF    0
#define FR8008XP_UART_PORT  2
#define FIBO_BLE_TEST_MODE  1
#define FIBO_BLE_READ_MODE  2
#define FIBO_BLE_SET_MODE   3
#define FIBO_BLE_EXE_MODE   4
#define FIBO_BLE_UUID_LEN   64
#define SERVICE_UUID_MODE   0
#define WRITE_UUID_MODE     1
#define RECV_UUID_MODE      2
#define FIBO_BLE_TX_MODE            0
#define FIBO_BLE_RX_MODE            1
#define FIBO_BLE_TXCH_MODE          2
#define FIBO_BLE_RXCH_MODE          3
#define FIBO_BLE_RXDATA_MODE        4
#define FIBO_BLE_TXPATTERN_MODE     5
#define FIBO_BLE_TXPKTLEN_MODE      6
#define FIBO_BLE_TXPATTERN_PN9       9
#define FIBO_BLE_TXPATTERN_PN15      15
#define FIBO_BLE_VERSION            8
#define FIBO_BLE_MAIN_UART          0
#define FIBO_BLUETOOTH_AUTH_TEST    7
#define FIBO_BLE_READ_DATA_LEN      20
#define FIBO_BLE_SHORT_UUID_LEN     2

typedef unsigned char UINT8;
/******************************
message type enum
******************************/
typedef enum
{
    FIBO_BLE_ENABLE,
    FIBO_BLE_GET_VER,
    FIBO_BLE_SET_NAME,
    FIBO_BLE_READ_NAME,
    FIBO_BLE_SET_MAC,
    FIBO_BLE_READ_MAC,
    FIBO_BLE_ADV_PARAM,
    FIBO_BLE_ADV_DATA,
    FIBO_BLE_ADV_ENABLE,
    FIBO_BLE_SCAN_PARAM,
    FIBO_BLE_SCAN_ENABLE,
    FIBO_BLE_SCAN_RESPONSE_DATA,
    FIBO_BLE_CONNECT,
    FIBO_BLE_DIS_CONNECT,
    FIBO_BLE_GET_STAT,
    FIBO_BLE_SEND_DATA,
    FIBO_BLE_SERVICR_UUID,
    FIBO_BLE_PEER_MTU_GET,
    FIBO_BLE_MTU_SET,
    FIBO_BLE_GTSN,
    FIBO_BLE_SLEEP,
    FIBO_BLE_TEST,
    FIBO_BLE_MODEL,
    FIBO_BLE_AUTH_TEST,
    FIBO_BLE_SEND_COMMAND,
    FIBO_BLE_GTSEND_DATA,
    FIBO_BLE_GTUUID,
    FIBO_BLE_READ_DATA,
    FIBO_BLE_CLRINFO,
    FIBO_BLE_AUTH,
    FIBO_BLE_BONDCTRL,
} ble_type;
typedef enum
{
    FIBO_DEVICE_UART,
    FIBO_DEVICE_USB,
    FIBO_DEVICE_IIC,
    FIBO_DEVICE_SPI,
} device_type_e;

typedef enum{
    FIBO_BLE_SUCESS,
    FIBO_BLE_FAIL,
}ble_status_e;
//2024-09-20, haopanpan, MTC0641-66, Begin. ble must support a maximum length of 26 bytes device name.
#define FIBO_BLE_LOCAL_NAME_LEN 26
//2024-09-20, haopanpan, MTC0641-66, End. 
#define FIBO_BLE_MAC_LEN        18
#define FIBO_BLE_ADV_DATA_LEN   64
#define FIBO_SET_NAME   0
#define FIBO_READ_NAME   1
#define FIBO_ADV_MIN   32
#define FIBO_ADV_MAX   16384
#define FIBO_OwnAddrType_MIN   0
#define FIBO_OwnAddrType_MAX   1
#define FIBO_AdvType_MIN   0
#define FIBO_AdvType_MAX   4
#define FIBO_AdvChannMap_DEFAULT   0x00000007
#define FIBO_AdvFilter_MIN   0
#define FIBO_AdvFilter_MAX   3
#define FIBO_DirectAddrType_MIN   0
#define FIBO_DirectAddrType_MAX   1
#define TRACE_HEAD "SPBLE"
#define FIBO_BLE_AUTH_DEFAULT_PASSKEY "123456"
#define FIBO_BLE_AUTH_DEFAULT_PASSKEY_LEN    6
/******************************
message type struct begin
******************************/

//FIBO_BLE_ENABLE
/*MC660 handle power on/off*/
/*ble power on/off,fibo_ble_common(FIBO_BLE_ENABLE,(void *)input,NULL)*/
typedef struct enable_in
{
    uint8_t msg_type;
    uint8_t enable;
}enable_in_t;
typedef struct enable_out
{
    uint8_t msg_type;
}enable_out_t;

//FIBO_BLE_GET_VER
/*ble version get,fibo_ble_common(FIBO_BLE_GET_VER,NULL,(void *)output)*/
typedef struct ver_in
{
    uint8_t msg_type;           /*Reserved parameters default value 2*/
}ver_in_t;
typedef struct ver_out
{
    //uint8_t msg_type;
    UINT8 version[FIBO_BLE_VERSION];              /*bluetooth stack return version*/
}ver_out_t;

//FIBO_BLE_SET_NAME
typedef struct name_set_in
{
    uint8_t msg_type;
    UINT8 name[FIBO_BLE_LOCAL_NAME_LEN + 1];
}name_set_in_t;
typedef struct name_set_out
{    
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}name_set_out_t;

//FIBO_BLE_READ_NAME
/*ble local name get,fibo_ble_common(FIBO_BLE_READ_NAME,NULL,(void *)output)*/
typedef struct name_read_in
{
    uint8_t msg_type;           /*Reserved parameters,default value 4*/
}name_read_in_t;
typedef struct name_read_out
{
    //uint8_t msg_type;
    UINT8 name[FIBO_BLE_LOCAL_NAME_LEN + 1];                 /*bluetooth return code OK*/
}name_read_out_t;

//FIBO_BLE_READ_MAC
typedef struct mac_read_in
{
    uint8_t msg_type;           /*Reserved parameters,default value 5*/
    uint8_t mac_type;           /*bluetooth address type,0:public,1:random*/
}mac_read_in_t;
typedef struct mac_read_out
{
    //uint8_t msg_type;
    UINT8 address[FIBO_BLE_MAC_LEN];              /*bluetooth address*/
}mac_read_out_t;

//FIBO_BLE_SET_MAC
typedef struct mac_set_in
{
    uint8_t msg_type;
    uint8_t mac_type;           /*bluetooth address type,0:public,1:random*/
    UINT8 address[FIBO_BLE_MAC_LEN];              /*bluetooth address*/
}mac_set_in_t;
typedef struct mac_set_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}mac_set_out_t;

//FIBO_BLE_ADV_PARAM
typedef struct adv_param_in
{
    uint8_t msg_type;
    uint8_t discov_mode;        /*the mode of discover*/
    uint8_t adv_type;           /*the type of adv*/
    uint16_t adv_min;           /*the minimum adv interval,unit is 0.625ms,within rang 20ms-10.24s*/
    uint16_t adv_max;           /*the maxmum adv interval,unit is 0.625ms,within rang 20ms-10.24s*/
    uint8_t adv_filter;         /*the policy of adv filter*/
    uint8_t addr_type;          /*own adv address type*/
    uint8_t adv_map;            /*adv channel map,correspond 37,38,39 channel*/
    UINT8 peer_addr[FIBO_BLE_MAC_LEN];              /*peer address*/
    uint8_t peer_addr_type;     /*peer address type*/
    uint8_t param_count;    /*adv param count*/
}adv_param_in_t;
typedef struct adv_param_out
{
    //uint8_t msg_type;
    UINT8 res[FIBO_BLE_ADV_DATA_LEN];             /*bluetooth return code OK*/
}adv_param_out_t;

//FIBO_BLE_ADV_DATA
typedef struct adv_data_in
{
    uint8_t msg_type;
    char adv_data[FIBO_BLE_ADV_DATA_LEN];
    uint8_t len;
}adv_data_in_t;
typedef struct adv_data_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}adv_data_out_t;

//FIBO_BLE_ADV_ENABLE
typedef struct adv_enable_in
{
    uint8_t msg_type;
    uint8_t enable;             /*bluetooth enable adv0:adv off;1: adv on*/
}adv_enable_in_t;
typedef struct adv_enable_out
{
    uint8_t msg_type;
    uint8_t advstatus;                  /*bluetooth return code OK*/
}adv_enable_out_t;

//FIBO_BLE_SCAN_PARAM
typedef struct scan_param_in
{
    uint8_t msg_type;
    uint8_t scan_type;          /*the type of scan,0:passive,1:active*/
    uint8_t filter_policy;      /*the filter policy of scan*/
    uint16_t scan_window;       /*the window of scan,unit is 0.625ms,within rang 2.5ms-10.24s*/
    uint16_t scan_interval;     /*the interval of scan,unit is 0.625ms,within rang 2.5ms-10.24s*/
}scan_param_in_t;
typedef struct scan_param_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}scan_param_out_t;

//FIBO_BLE_SCAN_ENABLE
typedef struct scan_enable_in
{
    uint8_t msg_type;
    uint8_t enable;             /*bluetooth enable scan 0:scan off;1: scan on*/
}scan_enable_in_t;
typedef struct scan_enable_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}scan_enable_out_t;
    
//FIBO_BLE_SCAN_RESPONSE_DATA
typedef struct scan_response_data_in
{
    uint8_t msg_type;
    char response_data[64];
    uint8_t len;
}scan_response_data_in_t;
typedef struct scan_response_data_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}scan_response_data_out_t;

//FIBO_BLE_CONNECT
typedef struct connect_in
{
    uint8_t msg_type;
    char bytes[6];
    uint8_t addr_type;
}connect_in_t;
typedef struct connect_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}connect_out_t;

//FIBO_BLE_DIS_CONNECT
typedef struct disconnect_in
{
    uint8_t msg_type;
    uint8_t index;
}disconnect_in_t;

typedef struct disconnect_out
{
    uint8_t msg_type;
}disconnect_out_t;

//FIBO_BLE_GET_STAT
/*ble version get,fibo_ble_common(FIBO_BLE_GET_STAT,NULL,(void *)output)*/
typedef struct status_in
{
    uint8_t msg_type;           /*msg_type parameters default value 15*/
    //uint8_t status;
}status_in_t;
typedef struct status_out
{
    uint8_t msg_type;
    uint8_t status;               /*bluetooth stack return version*/
}status_out_t;

//FIBO_BLE_SEND_DATA
typedef struct send_data_in
{
    uint8_t msg_type;
    UINT8 data[FIBO_BLE_DATA_LEN];
    uint16_t len;
}send_data_in_t;
typedef struct send_data_out
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}send_data_out_t;

//FIBO_BLE_SERVICR_UUID
typedef struct uuid_in
{
    uint8_t msg_type;
    uint8_t uuid_mode;
    UINT8 service_uuid[FIBO_BLE_UUID_LEN];
    UINT8 recv_uuid[FIBO_BLE_UUID_LEN];
    UINT8 write_uuid[FIBO_BLE_UUID_LEN];
}uuid_in_t;
typedef struct uuid_out
{
    uint8_t msg_type;
    UINT8 service_uuid[FIBO_BLE_UUID_LEN];
    UINT8 recv_uuid[FIBO_BLE_UUID_LEN];
    UINT8 write_uuid[FIBO_BLE_UUID_LEN];
}uuid_out_t;

//FIBO_BLE_PEER_MTU_GET
/*ble peer mtu get,fibo_ble_common(FIBO_BLE_PEER_MTU_GET,NULL,(void *)output)*/
typedef struct peer_mtu_in
{
    uint8_t msg_type;           /*msg_type parameters default value 18*/
    //uint8_t peer_mtu;
}peer_mtu_in_t;
typedef struct peer_mtu_out
{
    uint8_t msg_type;
    uint16_t mtu;                  /*bluetooth device peer mtu value*/
}peer_mtu_out_t;

//FIBO_BLE_MTU_SET
typedef struct mtu_in
{
    uint8_t msg_type;
    uint16_t mtu;
}mtu_in_t;
typedef struct mtu_out
{
    //uint8_t msg_type;
    uint16_t mtu;                  /*bluetooth return code OK*/
}mtu_out_t;

typedef struct msg_type_in
{
    uint8_t type;
}msg_type_in_t;

typedef struct gtsn_in
{
    uint8_t msg_type;
    UINT8 address[FIBO_BLE_MAC_LEN];

}gtsn_in_t;
typedef struct gtsn_out
{
    //uint8_t msg_type;
    UINT8 addr[FIBO_BLE_MAC_LEN];
}gtsn_out_t;

typedef struct sleep_in
{
    uint8_t msg_type;
    uint8_t sleep_mode;
}sleep_in_t;

typedef struct sleep_out
{
    uint8_t msg_type;
    uint8_t sleep_mode;
}sleep_out_t;

typedef struct bt_test_in
{
    uint8_t msg_type;
    uint8_t test_mode;
    uint16_t test_value;
}bt_test_in_t;
typedef struct bt_test_out
{
    uint8_t msg_type;
    uint8_t test_mode;
    uint8_t test_value;
}bt_test_out_t;

typedef struct model_in
{
    uint8_t msg_type;
}model_in_t;
typedef struct model_out
{
    UINT8 model[24];
}model_out_t;

//FIBO_BLE_AUTH_TEST
typedef struct auth_test_in
{
    uint8_t msg_type;
    uint8_t test_mode;
}auth_test_in_t;
typedef struct auth_test_out
{
    uint8_t msg_type;
    uint8_t test_mode;
}auth_test_out_t;
/******************************
message type struct end
******************************/
typedef struct recv_res
{
    uint8_t msg_type;
    char *res;                  /*bluetooth return code OK*/
}recv_res_t;

typedef struct send_command_in
{
    UINT8 data[FIBO_BLE_DATA_LEN];
    uint16_t len;
    uint16_t timeout;
}send_command_in_t;

typedef struct gtsend_in
{
    uint8_t msg_type;
    uint8_t index;
    UINT8 data[FIBO_BLE_DATA_LEN];
    uint16_t len;
}gtsend_in_t;

typedef struct gtread_in
{
   uint8_t msg_type;
   UINT8 data[FIBO_BLE_READ_DATA_LEN];
   uint16_t len;
}gtread_in_t;

typedef struct gtuuid_in
{
    uint8_t msg_type;
    uint8_t index;
    UINT8 uuid[5];
    UINT8 attr;
}gtuuid_in_t;

typedef struct auth_in
{
    uint8_t msg_type;
    uint8_t onoff;
    char conn_key[FIBO_BLE_AUTH_DEFAULT_PASSKEY_LEN + 1];
}auth_in_t;

typedef struct auth_out
{
    uint8_t onoff;
    char conn_key[FIBO_BLE_AUTH_DEFAULT_PASSKEY_LEN + 1];
}auth_out_t;

int fibo_ble_common(ble_type type,void *input, void *output);

ble_status_e fibo_ble_enable(ble_type type,void *input, void *output);
ble_status_e fibo_ble_get_version(ble_type type,void *input, void *output);
ble_status_e fibo_ble_set_name(ble_type type,void *input, void *output);
ble_status_e fibo_ble_get_name(ble_type type,void *input, void *output);
ble_status_e fibo_ble_get_address(ble_type type,void *input, void *output);
ble_status_e fibo_ble_adv_param(ble_type type,void *input, void *output);
ble_status_e fibo_ble_adv_data(ble_type type,void *input, void *output);
ble_status_e fibo_ble_adv_enable(ble_type type,void *input, void *output);
ble_status_e fibo_ble_scan_param(ble_type type,void *input, void *output);
ble_status_e fibo_ble_scan_enable(ble_type type,void *input, void *output);
ble_status_e fibo_ble_scan_response(ble_type type,void *input, void *output);
ble_status_e fibo_ble_connect(ble_type type,void *input, void *output);
ble_status_e fibo_ble_discon(ble_type type,void *input, void *output);
ble_status_e fibo_ble_get_status(ble_type type,void *input, void *output);
ble_status_e fibo_ble_data_send(ble_type type,void *input, void *output);
ble_status_e fibo_ble_uuid(ble_type type,void *input, void *output);
ble_status_e fibo_ble_peer_mtu(ble_type type,void *input, void *output);
ble_status_e fibo_ble_mtu(ble_type type,void *input, void *output);
ble_status_e fibo_ble_gtsn(ble_type type,void *input, void *output);
ble_status_e fibo_ble_sleep(ble_type type,void *input, void *output);
ble_status_e fibo_ble_test(ble_type type,void *input, void *output);
ble_status_e fibo_ble_auth_test(ble_type type,void *input, void *output);
ble_status_e fibo_ble_send_command(ble_type type,void *input, void *output);
ble_status_e fibo_ble_gtsend(ble_type type, void *input, void *output);
ble_status_e fibo_ble_gtuuid(ble_type type, void *input, void *output);
ble_status_e fibo_ble_read_data(ble_type type, void *input, void *output);
ble_status_e fibo_ble_clrinfo(ble_type type, void *input, void *output);
ble_status_e fibo_ble_auth(ble_type type, void *input, void *output);
ble_status_e fibo_ble_bondctrl(ble_type type, void *input, void *output);


struct ble_operations{
    ble_status_e (*enable) (ble_type type,void *input, void *output);
    ble_status_e (*get_version) (ble_type type,void *input, void *output);
    ble_status_e (*set_name) (ble_type type,void *input, void *output);
    ble_status_e (*get_name) (ble_type type,void *input, void *output);
    ble_status_e (*get_address) (ble_type type,void *input, void *output);
    ble_status_e (*adv_param) (ble_type type,void *input, void *output);
    ble_status_e (*adv_data) (ble_type type,void *input, void *output);
    ble_status_e (*adv_enable) (ble_type type,void *input, void *output);
    ble_status_e (*scan_param) (ble_type type,void *input, void *output);
    ble_status_e (*scan_enable) (ble_type type,void *input, void *output);
    ble_status_e (*scan_response) (ble_type type,void *input, void *output);
    ble_status_e (*ble_connect) (ble_type type,void *input, void *output);
    ble_status_e (*ble_disconnect) (ble_type type,void *input, void *output);
    ble_status_e (*get_statue) (ble_type type,void *input, void *output);
    ble_status_e (*send_data) (ble_type type,void *input, void *output);
    ble_status_e (*uuid) (ble_type type,void *input, void *output);
    ble_status_e (*peer_mtu) (ble_type type,void *input, void *output);
    ble_status_e (*set_mtu) (ble_type type,void *input, void *output);
    ble_status_e (*gtsn) (ble_type type,void *input, void *output);
    ble_status_e (*sleep) (ble_type type,void *input, void *output);
    ble_status_e (*bt_test) (ble_type type,void *input, void *output);
    ble_status_e (*ble_test) (ble_type type,void *input, void *output);
    ble_status_e (*ble_send_command) (ble_type type,void *input, void *output);
    ble_status_e (*gtsend_data) (ble_type type,void *input, void *output);
    ble_status_e (*gtuuid) (ble_type type,void *input, void *output);
    ble_status_e (*gtread_data) (ble_type type,void *input, void *output);
    ble_status_e (*clrinfo) (ble_type type,void *input, void *output);
    ble_status_e (*ble_auth) (ble_type type,void *input, void *output);
    ble_status_e (*ble_bondctrl) (ble_type type,void *input, void *output);
};
#endif
#endif
