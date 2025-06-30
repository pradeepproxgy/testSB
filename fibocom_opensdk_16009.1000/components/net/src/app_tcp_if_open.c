/* Copyright (C) 2023 UNISOC Technologies Co., Ltd.("UNISOC").
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

/*---------------------------------------------------------------------------*
 * Include Files *
 *---------------------------------------------------------------------------*/
#include "net_config.h"
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
#ifdef IP_V6
#include "ip6.h"
#endif

#include "socket_types.h"
#include "socket_api.h"
#ifdef DHCP_SERVER
#include "dhcps_api.h"
#endif /* DHCP_SERVER */
#ifdef VOLTE_SUPPORT
#include "tcpip_ps.h"
#endif
#include "sockets.h"
#include "osi_api.h"
#include "netdb.h"
#include "netutils.h"

/*---------------------------------------------------------------------------*
 * MACRO DEFINITION *
 *---------------------------------------------------------------------------*/
#define PUBLIC
#define SRVAPI
#define PROTECTED
#define LOCAL static
#define PRIVATE static
#define SCI_NETIF_DEBUG_LOG

/*---------------------------------------------------------------------------*
 * GLOBAL DEFINITION *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * LOCAL FUNCTION DECLARE *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * EXTERNAL DECLARATION *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * FUNCTION DEFINITIONS *
 *---------------------------------------------------------------------------*/
static uint8_t *I8ToString(uint8_t value)
{
    static uint8_t buff[4];
    uint8_t b, s, g, i = 0;
    b = value / 100;
    s = value % 100 / 10;
    g = value % 100 % 10;
    if (b > 0)
        buff[i++] = '0' + b;
    if (b + s > 0)
        buff[i++] = '0' + s;
    buff[i++] = '0' + g;
    buff[i] = 0;
    return buff;
}

/*****************************************************************************/
// Description: modify socket net ID
// Global resource : none
// Author : Liu Xiangyu
// Note : PARAM1: so (IN) -- socket
// PARAM2: netid (IN) -- net interface ID
// RETURN: if successful, return 0; else -1.
// NOTE: when socket is connectless or not in connect state,
// then netid modification on this socket is permitted,
// else function will return -1 with socket error EINVAL.
/*****************************************************************************/
PUBLIC int sci_sock_modifynetid(TCPIP_SOCKET_T so, TCPIP_NETID_T netid)
{

#ifdef SCI_NETIF_DEBUG_LOG
    OSI_LOGI(0x1000a930, "tcpip sci_sock_modifynetid netid 0x%x", netid);
#endif
    struct ifreq device = {0};
    struct netif *netif = NULL;
    uint8_t nSimId = (netid & 0xff00) >> 8;
    uint8_t nCid = (netid & 0xff);
    netif = getGprsNetIf(nSimId, nCid);

    if (netif != NULL)
    {
        device.ifr_name[0] = netif->name[0];
        device.ifr_name[1] = netif->name[1];
        strcat(device.ifr_name, (const char *)I8ToString(netif->num));

#ifdef SCI_NETIF_DEBUG_LOG
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a931, "tcpip sci_sock_modifynetid ifr_name %s\n", (char *)device.ifr_name);
#endif
        return lwip_setsockopt(so, SOL_SOCKET, SO_BINDTODEVICE, &device, sizeof(struct ifreq));
    }
    return -1;
}
#endif