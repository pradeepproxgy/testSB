#ifndef __FIBO_WIFISCAN_H__
#define __FIBO_WIFISCAN_H__


#define AP_MAC_ADDR_LEN 20
#define AP_SSID_LEN 20
#define FIBO_DEFAULT_AP_SSID "TP-LINK"
#define WIFI_MAX_COUNT 30

typedef struct{
    char ap_mac[AP_MAC_ADDR_LEN];
    int8_t  ap_rssi;
    char ap_name[AP_SSID_LEN];
}wifi_scan_info_t;

wifi_scan_info_t g_wifi_info[WIFI_MAX_COUNT];
int g_wifi_count = 0;

#endif
