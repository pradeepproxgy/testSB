/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _IDS_H_
#define _IDS_H_

typedef enum
{
    IDS_P_TCP = 0x80000000,
    IDS_P_UDP = 0x40000000,
    IDS_P_ICMP = 0x20000000,
    IDS_P_RAW = 0x10000000
} IDS_PROTOCOL_E;

typedef enum // 28 in total
{
    // TCP
    TCP_SYN_FLOOD = (0x01 << 0) | IDS_P_TCP,
    TCP_NULL_FLOOD = (0x01 << 1) | IDS_P_TCP,
    TCP_ACK_FLOOD = (0x01 << 2) | IDS_P_TCP,
    TCP_FIN_FLOOD = (0x01 << 3) | IDS_P_TCP,
    TCP_XMAS_FLOOD = (0x01 << 4) | IDS_P_TCP,
    TCP_FLOOD = (0x01 << 5) | IDS_P_TCP,
    TCP_INVALID_PACKET = (0x01 << 6) | IDS_P_TCP,
    TCP_PORT_SCAN = (0x01 << 7) | IDS_P_TCP,

    // UDP
    UDP_FLOOD = (0x01 << 8) | IDS_P_UDP,
    UDP_INVALID_PACKET = (0x01 << 9) | IDS_P_UDP,
    UDP_PORT_SCAN = (0x01 << 10) | IDS_P_UDP,

    // icmp
    ICMP_FLOOD = (0x01 << 11) | IDS_P_ICMP,
    ICMP_INVALID_PACKET = (0x01 << 12) | IDS_P_ICMP,
    ICMP_LARGE = (0x01 << 13) | IDS_P_ICMP,

    // raw
    HOST_SERVICE_SCAN = (0x01 << 14) | IDS_P_RAW
} IDS_EVENT_E;

typedef struct _IDS_FILTER
{
    char *name;
    bool enable;
    char *param;
    uint8_t prority;
    struct _IDS_FILTER *next;
} IDS_FILTER_T;

typedef struct _IDS_EVENT
{
    IDS_EVENT_E EventID;
    char *EventName;
} IDS_EVENT_T;

typedef void (*IDS_event_cb_t)(IDS_EVENT_T *ev, void *ctx);

/**
 * init ids module
 * \return           none
 */
void IDS_init();

/**
 * Disable ids module
 */
void IDS_Disable();

/**
 * Enable ids module
 */
void IDS_Enable();
bool IDS_isEnable();

/**
 * modify the filter
 *
 * \param ptl     IDS_PROTOCOL_E
 * \param name    filter name const string
 * \param enable  filter enable
 * \param param   filter param const string, "rate-above:50;tcp-flag:SYN;"
 * \param prority filter prority (1-255), 1 means highest, 0 is resverd for default prority
 * \return        If success return true, otherwise return false
 */
bool IDS_Filter_Set(IDS_PROTOCOL_E ptl, const char *name, bool enable, const char *param, uint8_t prority);

/**
 * get the filter
 *
 * \param ptl     IDS_PROTOCOL_E
 * \param name    filter name const string, if NULL means get all filter in protocol chain
 * \return        filter struct info
 */
IDS_FILTER_T *IDS_Filter_Get(IDS_PROTOCOL_E ptl, const char *name);

/**
 * register the user callback to ids
 *
 * \param cb      callback func
 * \param ctx     user ctx which set to callback param
 * \param interval   time (ms) interval for IDS callback, eg 1000ms
 * \return        none
 */
void IDS_register_user_callback(IDS_event_cb_t cb, void *ctx, uint32_t interval);

#endif
