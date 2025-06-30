#ifndef FFW_WIFISCAN
#define FFW_WIFISCAN
#include <stdint.h>  // for int32_t
#include "ffw_result.h"
#include "ffw_rat.h"


#define FFW_AP_MAC_ADDR_LEN 65
#define FFW_AP_SSID_LEN 65
#define FFW_AP_MAX_NUM 30

typedef struct
{
    char ap_mac[FFW_AP_MAC_ADDR_LEN];
    int8_t ap_rssi;
    char ap_name[FFW_AP_SSID_LEN];
}ffw_ap_info_t;

typedef struct
{
    ffw_ap_info_t ap_info[FFW_AP_MAX_NUM];
    int8_t ap_num;
}ffw_wifi_info_t;

typedef struct ffw_wifi_context_s
{
    uint8_t simid;
    void *arg;
    uint16_t uti;
    ffw_wifi_info_t wifi;
    int result;
    bool pending;
    void *sem;
    void *lock;
    void *timer;
    bool init;

   ffw_wifiinfo_callback_t cb;
} ffw_wifi_context_t;

typedef void (*ffw_wifiscan_callback_t)(int result, ffw_wifi_info_t *wifi, void *arg);

int ffw_wifi_scan_start(void);
int ffw_wifi_scan_end(ffw_wifi_info_t ap_info, ffw_result_t result);
int ffw_get_ap_info(int simid, int32_t timeout, ffw_wifi_info_t *wifi_info);

#endif /* FFW_WIFISCAN */
