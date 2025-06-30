#ifndef _NETWORK_SERVICE_H_
#define _NETWORK_SERVICE_H_

#include "fibo_opencpu.h"
typedef enum{
    NET_CONNECT_SUCCESS =  0,
    NET_NO_SIM          = -1,
    NET_NO_IMSI         = -2,
    NET_NO_CCID         = -3,
    NET_NO_APN          = -4,
    NET_NO_SIGNAL       = -5,
    NET_NO_REGISTER     = -6,
    NET_NO_ACTIVE       = -7,
}netInitInfo;


enum
{
    NET_DISCONNECT=0,
    NET_CONNECT=1,
}netConnectSta;

typedef struct
{
    char imsi[32];
    char ccid[32];
    char apn[40];
    char ip[80];
    char sim_status;
    char cid_status;
    int rssi;  
    int ber;
    reg_info_t registered_status;
}dev_net_info_t;

extern dev_net_info_t dev_net_info;

int get_net_connect_sta(void);
int network_enter(void);
#endif