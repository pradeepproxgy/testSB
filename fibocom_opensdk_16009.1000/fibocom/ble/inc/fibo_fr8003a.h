#ifndef __FIBO_BLE_FR8008XP_H__
#define __FIBO_BLE_FR8008XP_H__
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
#include "fibo_ble_common.h"

#ifdef CONFIG_FIBOCOM_BLE_SUPPORT
#define FIBO_BLE_PUBLIC_MAC 0
#define FIBO_BLE_RAMDOM_MAC 1
#define FIBO_BLE_POWER_ON   1
#define FIBO_BLE_POWER_OFF  0
#define FIBO_BLE_CONN_STAT    1
#define FIBO_BLE_DISCONN_STAT 0
#define ADV_ENABLE_STATUS       1
#define ADV_DISENABLE_STATUS    0
#define GTSN_STORE_ENABLE   1
#define GTSN_STORE_DISENABLE   0
#define FIBO_BLE_SLEEP_ENABLE   1
#define FIBO_BLE_SLEEP_DISENABLE   0
#define FIBO_BLE_RX_ENABLE      1
#define FIBO_BLE_RX_DISENABLE     0
#define FIBO_BLE_TEMP_BUFFER    (4 * 1024)

enum command_flag_s
{
    AT_COMMON_COMMAND = 0,
    AT_SCAN_COMMAND,                 //AT+SCAN
    AT_GTSCANMAC_COMMAND,            //AT+GTSCANMAC
};

typedef struct ble_info
{
    int connect_stat;
    UINT8 service_uuid[FIBO_BLE_UUID_LEN];
    UINT8 write_uuid[FIBO_BLE_UUID_LEN];
    UINT8 recv_uuid[FIBO_BLE_UUID_LEN];
    UINT8 uuid[FIBO_BLE_DATA_LEN];
    uint8_t adv_status;
    uint16_t mtu_value;
    uint16_t peer_mtu;
    ble_status_e ret_status;
    uint8_t store_flag;
    uint8_t sleep_mode;
    uint8_t test_mode;
    uint8_t test_enable;
    uint8_t rx_value;
    uint8_t res_status;
    int ble_power;
    int ble_sem;
    char data[FIBO_BLE_TEMP_BUFFER];
    bool send_flag;
    int total_len;
    int recv_len;
    int offset_len;
    int adv_mode;
    int command_flag;
    int scan_sem;
    int gtscanmac_sem;
}ble_info_t;

int fibo_device_channle_init(device_type_e type);
int fibo_ble_adv_status(void);
int fibo_ble_power_status(void);
char *fibo_get_ble_version(void);
int fibo_ble_modle(uint8_t *model_info);
void fibo_main_uart_init(void);
void fibo_main_uart_deinit(void);
ble_status_e fibo_ble_power(int enable);
/*
static struct ble_operations fr8008_ble_xp = {
    .enable = fibo_ble_enable,
    .get_version = fibo_ble_get_version,
    .set_name = fibo_ble_set_name,
    .get_name = fibo_ble_get_name,
    .get_address = fibo_ble_get_address,
    .adv_param = fibo_ble_adv_param,
    .adv_data = fibo_ble_adv_data,
    .adv_enable = fibo_ble_adv_enable,
    .scan_param = fibo_ble_scan_param,
    .scan_enable = fibo_ble_scan_enable,
    .scan_response = fibo_ble_scan_response,
    .ble_connect = fibo_ble_connect,
    .ble_disconnect = fibo_ble_discon,
    .get_statue = fibo_ble_get_status,
    .send_data = fibo_ble_data_send,
    .uuid = fibo_ble_uuid,
    .peer_mtu = fibo_ble_peer_mtu,
    .set_mtu = fibo_ble_mtu,
};
*/
#endif
#endif
