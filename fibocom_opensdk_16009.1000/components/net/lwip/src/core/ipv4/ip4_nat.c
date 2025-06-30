/* Copyright (C) 2022 RDA Technologies Limited and/or its affiliates("RDA").
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

#include "lwip/opt.h"

#ifdef CONFIG_FIBOCOM_BASE
#include "ip6.h"
#include "netutils.h"
#include "inet_chksum.h"
#include <stdint.h>
#endif

#if IP_NAT

#include "lwip/ip4.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/icmp.h"
#include "lwip/prot/tcp.h"
#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/ip4_nat.h"
#include "osi_log.h"

#include <sys/queue.h>

#ifdef CONFIG_FIBOCOM_BASE
#include <lwip/dns.h>
#include "ffw_rat.h"
#endif

#define LWIP_NAT_TTL_INFINITE (INT_MAX)
#ifdef CONFIG_FIBOCOM_BASE
#define UNUSED(x) (void)(x)
#define LWIP_NAT_DEFAULT_TTL_SECONDS (INT_MAX)
#define LWIP_NAT_TEMP_TTL_SECONDS (180)
#endif
#define LWIP_NAT_DEFAULT_TCP_TTL_SECONDS (120)
#define LWIP_NAT_DEFAULT_UDP_TTL_SECONDS (120)

#define LWIP_NAT_DEFAULT_ICMP_TTL_SECONDS (60)

#define LWIP_NAT_DEFAULT_TCP_PORT_COUNT (1024)
#define LWIP_NAT_DEFAULT_UDP_PORT_COUNT (1024)

#define LWIP_NAT_DEFAULT_TCP_SOURCE_PORT (20000)
#define LWIP_NAT_DEFAULT_UDP_SOURCE_PORT (40000)

#define LWIP_NAT_BACKUP_TCP_PORT_COUNT (60000)
#define LWIP_NAT_BACKUP_UDP_PORT_COUNT (60000)

#define LWIP_NAT_BACKUP_TCP_SOURCE_PORT (1024)
#define LWIP_NAT_BACKUP_UDP_SOURCE_PORT (1024)

#ifdef CONFIG_FIBOCOM_BASE
#define IPNAT_ENTRY_RESET(x) do { \
  (x)->ttl = 0; \
  (x)->state = CLOSED; \
  (x)->ts = osiUpTime(); \
} while(0)
#else
#define IPNAT_ENTRY_RESET(x) \
    do                       \
    {                        \
        (x)->ttl = 0;        \
    } while (0)

#endif

struct ip4_nat_conf
{
    struct ip4_nat_conf *next;
    ip4_nat_entry_t entry;
};

#ifdef CONFIG_FIBOCOM_BASE
typedef struct ip4_nat_entry_common
{
  s32_t           ttl; /* @todo: do we really need this to be signed?? */
  ip4_addr_t       source;
  ip4_addr_t       dest;
  ip4_nat_conf_t   *cfg;
  u64_t             ts;
  enum tcp_state    state;
} ip4_nat_entry_common_t;
#else
typedef struct ip4_nat_entry_common
{
    s32_t ttl; /* @todo: do we really need this to be signed?? */
    ip4_addr_t source;
    ip4_addr_t dest;
    ip4_nat_conf_t *cfg;
} ip4_nat_entry_common_t;
#endif

typedef SLIST_ENTRY(ip4_nat_entries_icmp) ip4_nat_entries_icmp_iter_t;
typedef SLIST_HEAD(ip4_nat_entries_icmp_head, ip4_nat_entries_icmp) ip4_nat_entries_icmp_head_t;
ip4_nat_entries_icmp_head_t *ip4_nat_entries_icmp_header = NULL;

typedef struct ip4_nat_entries_icmp
{
    ip4_nat_entry_common_t common;
    u16_t id;
    u16_t seqno;
    ip4_nat_entries_icmp_iter_t iter;
} ip4_nat_entries_icmp_t;

typedef SLIST_ENTRY(ip4_nat_entries_tcp) ip4_nat_entries_tcp_iter_t;
typedef SLIST_HEAD(ip4_nat_entries_tcp_head, ip4_nat_entries_tcp) ip4_nat_entries_tcp_head_t;
ip4_nat_entries_tcp_head_t *ip4_nat_entries_tcp_header = NULL;

typedef struct ip4_nat_entries_tcp
{
    ip4_nat_entry_common_t common;
    u16_t nport;
    u16_t sport;
    u16_t dport;
    ip4_nat_entries_tcp_iter_t iter;
} ip4_nat_entries_tcp_t;

typedef SLIST_ENTRY(ip4_nat_entries_udp) ip4_nat_entries_udp_iter_t;
typedef SLIST_HEAD(ip4_nat_entries_udp_head, ip4_nat_entries_udp) ip4_nat_entries_udp_head_t;
ip4_nat_entries_udp_head_t *ip4_nat_entries_udp_header = NULL;

typedef struct ip4_nat_entries_udp
{
    ip4_nat_entry_common_t common;
    u16_t nport;
    u16_t sport;
    u16_t dport;
    ip4_nat_entries_udp_iter_t iter;
} ip4_nat_entries_udp_t;

typedef union u_nat_entry {
    ip4_nat_entry_common_t *cmn;
    ip4_nat_entries_tcp_t *tcp;
    ip4_nat_entries_icmp_t *icmp;
    ip4_nat_entries_udp_t *udp;
} nat_entry_t;

#ifdef CONFIG_FIBOCOM_BASE

#define    NAT_ENTRY_PORT_NUMS         1024

//It is designed to avoid port duplication when creating or overwriting TCP routing tables
typedef enum nat_port_status
{
    PORT_UNUSED = 0,            //the port is not used
    PORT_USED                   //the port is used
}nat_port_status;
typedef enum nat_protocol
{
    E_NAT_TCP = 0,      //tcp protocol
    E_NAT_UDP,          //udp protocol
    E_NAT_MAX           //max protocol nums
}nat_protocol;
typedef struct nat_entry_port_table
{
    int8_t status;
    u16_t  nport;
}nat_entry_port_table;
static nat_entry_port_table nat_entry_port_table_t[E_NAT_MAX][NAT_ENTRY_PORT_NUMS];
#endif

static uint32_t sNATCfg = 0;

static ip4_nat_conf_t *ip4_nat_cfg = NULL;

/* ----------------------- Static functions (COMMON) --------------------*/
static void ip4_nat_chksum_adjust(u8_t *chksum, const u8_t *optr, s16_t olen, const u8_t *nptr, s16_t nlen);
static void ip4_nat_cmn_init(ip4_nat_conf_t *nat_config, const struct ip_hdr *iphdr,
                             ip4_nat_entry_common_t *nat_entry);
static ip4_nat_conf_t *ip4_nat_shallnat(const struct ip_hdr *iphdr);
static void ip4_nat_remove_config(ip4_nat_conf_t *cfg);

/* ----------------------- Static functions (DEBUG) ---------------------*/
#if NAT_DEBUG
static void ip4_nat_dbg_dump(const char *msg, const struct ip_hdr *iphdr);
static void ip4_nat_dbg_dump_ip(const ip4_addr_t *addr);
static void ip4_nat_dbg_dump_icmp_nat_entry(const char *msg, const ip4_nat_entries_icmp_t *nat_entry);
static void ip4_nat_dbg_dump_tcp_nat_entry(const char *msg, const ip4_nat_entries_tcp_t *nat_entry);
static void ip4_nat_dbg_dump_udp_nat_entry(const char *msg, const ip4_nat_entries_udp_t *nat_entry);
static void ip4_nat_dbg_dump_init(ip4_nat_conf_t *ip4_nat_cfg_new);
static void ip4_nat_dbg_dump_remove(ip4_nat_conf_t *cur);
#else /* defined(LWIP_DEBUG) && (NAT_DEBUG & LWIP_DBG_ON) */
#define ip4_nat_dbg_dump(msg, iphdr)
#define ip4_nat_dbg_dump_ip(addr)
#define ip4_nat_dbg_dump_icmp_nat_entry(msg, nat_entry)
#define ip4_nat_dbg_dump_tcp_nat_entry(msg, nat_entry)
#define ip4_nat_dbg_dump_udp_nat_entry(msg, nat_entry)
#define ip4_nat_dbg_dump_init(ip4_nat_cfg_new)
#define ip4_nat_dbg_dump_remove(cur)
#endif /* defined(LWIP_DEBUG) && (NAT_DEBUG & LWIP_DBG_ON) */

extern void setNatCfgNv(uint32_t natCfg);

/* ---------------------------- bitmap --------------------------------- */
#define BIT_IN_CHAR 8
#define DIV_ROUND_UP(a, b)  (((a) + (b) - 1) / (b))
typedef struct _bitmap
{
    int size;
    char map[0];
}bitmap_t;

static bitmap_t *ip4_nat_tcp_port_backup_map = NULL;
static bitmap_t *ip4_nat_udp_port_backup_map = NULL;

#if 0
static int bitmap_get(bitmap_t *map, int index)
{
    int byte = 0;
    int bit = 0;
    char tmp = 0;

    if ((NULL == map) || (index >= map->size) || (index < 0))
    {
        return false;
    }

    byte = index / BIT_IN_CHAR;
    bit = index % BIT_IN_CHAR;
    tmp = map->map[byte];

    return ((tmp >> bit) & 0x01);
}
#endif

static void bitmap_set(bitmap_t *map, int index)
{
    int byte = 0;
    int bit = 0;
    char tmp = 0;

    if ((NULL == map) || (index >= map->size) || (index < 0))
    {
        return;
    }

    byte = index / BIT_IN_CHAR;
    bit = index % BIT_IN_CHAR;
    tmp = map->map[byte];

    tmp |= (0x01 << bit);
    map->map[byte] = tmp;

    return;
}

static void bitmap_clean(bitmap_t *map, int index)
{
    int byte = 0;
    int bit = 0;
    char tmp = 0;

    if ((NULL == map) || (index >= map->size) || (index < 0))
    {
        return;
    }

    byte = index / BIT_IN_CHAR;
    bit = index % BIT_IN_CHAR;
    tmp = map->map[byte];

    tmp &= ~(0x01 << bit);
    map->map[byte] = tmp;

    return;
}

static int bitmap_search(bitmap_t *map, int value)
{
    int size = 0;
    int byte = 0;
    int bit = 0;
    char tmp = 0;
    int i = 0;

    if (NULL == map)
    {
        return -1;
    }

    size = map->size;
    for (i = 0; i < size; i++)
    {
        byte = i / BIT_IN_CHAR;
        bit = i % BIT_IN_CHAR;
        tmp = map->map[byte];

        if (((tmp >> bit) & 0x01) == value)
        {
            break;
        }
    }

    if (i >= size)
    {
        i = -1;
    }

    return i;
}

static bitmap_t *bitmap_create(int size)
{
    int buf_len = sizeof(bitmap_t) + DIV_ROUND_UP(size, BIT_IN_CHAR);
    bitmap_t *map = (bitmap_t *)calloc(1, buf_len);
    if (NULL == map)
    {
        return NULL;
    }

    map->size = size;
    return map;
}

static void bitmap_destroy(bitmap_t *map)
{
    if (NULL != map)
    {
        free(map);
    }

    return;
}

/* ----------------------- Static functions (TCP) -----------------------*/
static ip4_nat_entries_tcp_t *ip4_nat_tcp_lookup_incoming(const struct ip_hdr *iphdr, const struct tcp_hdr *tcphdr);
static ip4_nat_entries_tcp_t *ip4_nat_tcp_lookup_outgoing(ip4_nat_conf_t *nat_config,
                                                          const struct ip_hdr *iphdr, const struct tcp_hdr *tcphdr,
                                                          u8_t allocate);

/* ----------------------- Static functions (UDP) -----------------------*/
static ip4_nat_entries_udp_t *ip4_nat_udp_lookup_incoming(const struct ip_hdr *iphdr, const struct udp_hdr *udphdr);
static ip4_nat_entries_udp_t *ip4_nat_udp_lookup_outgoing(ip4_nat_conf_t *nat_config,
                                                          const struct ip_hdr *iphdr, const struct udp_hdr *udphdr,
                                                          u8_t allocate);

static bool ip4_nat_check_occupy_udp_port_config(u16_t port)
{
    bool found = false;
    struct ip4_nat_entries_udp *udp_entry = NULL;
    u16_t port_net = htons(port);

    SLIST_FOREACH(udp_entry, ip4_nat_entries_udp_header, iter)
    {
        if ((udp_entry->nport == port_net)&&
            (udp_entry->sport == port_net)&&
            (0 == udp_entry->dport))
        {
            found = true;
            break;
        }
    }

    return found;
}

static bool ip4_nat_check_occupy_tcp_port_config(u16_t port)
{
    bool found = false;
    struct ip4_nat_entries_tcp *tcp_entry = NULL;
    u16_t port_net = htons(port);

    SLIST_FOREACH(tcp_entry, ip4_nat_entries_tcp_header, iter)
    {
        if ((tcp_entry->nport == port_net)&&
            (tcp_entry->sport == port_net)&&
            (0 == tcp_entry->dport))
        {
            found = true;
            break;
        }
    }

    return found;
}

static u16_t ip4_nat_tcp_port_alloc(ip4_nat_conf_t *nat_config, u16_t src_port)
{
    u16_t port = (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT +
                  nat_config->entry.port_index * LWIP_NAT_DEFAULT_TCP_PORT_COUNT +
                  src_port % LWIP_NAT_DEFAULT_TCP_PORT_COUNT);
    int index = 0;

    if (true == ip4_nat_check_occupy_tcp_port_config(port))
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "nat tcp session port in use: %d", port));
        index = bitmap_search(ip4_nat_tcp_port_backup_map, 0);
        if (-1 == index)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0, "nat tcp port run out.d"));
            osiPanic();
        }
        bitmap_set(ip4_nat_tcp_port_backup_map, index);
        port = index + LWIP_NAT_BACKUP_TCP_SOURCE_PORT;
    }
    LWIP_DEBUGF(NAT_DEBUG, (0, "nat tcp session alloc port: %d", port));

    return port;
}

static void ip4_nat_tcp_port_occupy(u16_t port)
{
    if ((port >= LWIP_NAT_DEFAULT_TCP_SOURCE_PORT) &&
        (port < (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT + LWIP_NAT_DEFAULT_TCP_PORT_COUNT)))
    {
        bitmap_set(ip4_nat_tcp_port_backup_map, port - LWIP_NAT_DEFAULT_TCP_SOURCE_PORT);
    }
}

static void ip4_nat_tcp_port_free(u16_t port)
{
    if ((port >= LWIP_NAT_DEFAULT_TCP_SOURCE_PORT) &&
        (port < (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT + LWIP_NAT_DEFAULT_TCP_PORT_COUNT)))
    {
        bitmap_clean(ip4_nat_tcp_port_backup_map, port - LWIP_NAT_DEFAULT_TCP_SOURCE_PORT);
    }
}

static u16_t ip4_nat_udp_port_alloc(ip4_nat_conf_t *nat_config, int i)
{
    u16_t port = LWIP_NAT_DEFAULT_UDP_SOURCE_PORT +
                 nat_config->entry.port_index * LWIP_NAT_DEFAULT_UDP_PORT_COUNT + i;
    int index = 0;

    if (true == ip4_nat_check_occupy_udp_port_config(port))
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "nat udp session port in use: %d", port));
        index = bitmap_search(ip4_nat_udp_port_backup_map, 0);
        if (-1 == index)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0, "nat udp port run out.d"));
            osiPanic();
        }
        bitmap_set(ip4_nat_udp_port_backup_map, index);
        port = index + LWIP_NAT_BACKUP_UDP_SOURCE_PORT;
    }
    LWIP_DEBUGF(NAT_DEBUG, (0, "nat udp session alloc port: %d", port));

    return port;
}

static void ip4_nat_udp_port_occupy(u16_t port)
{
    if ((port >= LWIP_NAT_DEFAULT_UDP_SOURCE_PORT) &&
        (port < (LWIP_NAT_DEFAULT_UDP_SOURCE_PORT + LWIP_NAT_DEFAULT_UDP_PORT_COUNT)))
    {
        bitmap_set(ip4_nat_udp_port_backup_map, port - LWIP_NAT_DEFAULT_UDP_SOURCE_PORT);
    }
}

static void ip4_nat_udp_port_free(u16_t port)
{
    if ((port >= LWIP_NAT_DEFAULT_UDP_SOURCE_PORT) &&
        (port < (LWIP_NAT_DEFAULT_UDP_SOURCE_PORT + LWIP_NAT_DEFAULT_UDP_PORT_COUNT)))
    {
        bitmap_clean(ip4_nat_udp_port_backup_map, port - LWIP_NAT_DEFAULT_UDP_SOURCE_PORT);
    }
}

#if defined CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
extern uint8_t fibo_get_current_lan_ipaddr_haikang();
#endif

#ifdef CONFIG_FIBOCOM_BASE
static void fibo_init_nat_entry_index()
{
    OSI_PRINTFI("fibo_init_nat_entry_index");
    memset(nat_entry_port_table_t, 0, sizeof(nat_entry_port_table_t));
}

static void fibo_remove_nat_entry(int protocol, u16_t nport)
{
    int index = 0;
    if(E_NAT_TCP == protocol)
    {
        for(index = 0; index < NAT_ENTRY_PORT_NUMS; index++)
        {
            if(nport == nat_entry_port_table_t[E_NAT_TCP][index].nport)
            {
                OSI_PRINTFI("fibo_remove_nat_entry tcp index is %d", index);
                nat_entry_port_table_t[protocol][index].status = PORT_UNUSED;
                nat_entry_port_table_t[protocol][index].nport = 0;
            }
        }
    }
    if(E_NAT_UDP == protocol)
    {
        for(index = 0; index < NAT_ENTRY_PORT_NUMS; index++)
        {
            if(nport == nat_entry_port_table_t[E_NAT_UDP][index].nport)
            {
                OSI_PRINTFI("fibo_remove_nat_entry udp index is %d", index);
                nat_entry_port_table_t[protocol][index].status = PORT_UNUSED;
                nat_entry_port_table_t[protocol][index].nport = 0;
            }
        }
    }
}

static int fibo_get_nat_entry(int protocol)
{
    int index;
    int rest = -1;
    static bool _init = false;
    if(!_init)
    {
        OSI_PRINTFI("fibo_get_nat_entry, init address is %p", &nat_entry_port_table_t);
        fibo_init_nat_entry_index(); //the init is added because of Someone will abnormally modify static storage area data 
        _init = true;
    }
    if(E_NAT_TCP == protocol)
    {
        for(index = 0; index < NAT_ENTRY_PORT_NUMS; index++)
        {
            if(PORT_UNUSED == nat_entry_port_table_t[E_NAT_TCP][index].status)
            {
                OSI_PRINTFI("fibo_get_nat_entry, tcp index %d ", index);
                return index;
            }
        }
    }
    if(E_NAT_UDP == protocol)
    {
        for(index = 0; index < NAT_ENTRY_PORT_NUMS; index++)
        {
            if(PORT_UNUSED == nat_entry_port_table_t[E_NAT_UDP][index].status)
            {
                OSI_PRINTFI("fibo_get_nat_entry, udp index %d ", index);
                return index;
            }
        }
    }
    return rest;
}

static void fibo_set_nat_entry(int protocol, int index, u16_t nport)
{
    nat_entry_port_table_t[protocol][index].status = PORT_USED;
    nat_entry_port_table_t[protocol][index].nport = nport;
}

void fibo_get_backup_dns(ip_addr_t *dns_server_ip)
{
    //To distinguish between domestic and international PLMN values, choose different backup DNS 
    uint8_t operator = ffw_get_curr_operator();
    OSI_PRINTFI("fibo_get_backup_dns ffw_get_curr_operator is %d", operator);
    if(FFW_CHINA_MOBILE == operator || FFW_CHN_CT == operator
        || FFW_CHN_UNICOM == operator || FFW_CHN_GUANGDIAN == operator)
    {
        IP_ADDR4(dns_server_ip, 114, 114, 114, 114);
        OSI_PRINTFI("fibo_get_backup_dns domestic dns");
    }
    else
    {
        IP_ADDR4(dns_server_ip, 8, 8, 8, 8);
        OSI_PRINTFI("fibo_get_backup_dns foreign dns");
    }

}

#endif

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
static ip4_addr_t ip4_nat_dns = {0};
static ip4_addr_t ip4_network_dns = {0};
extern bool fibo_get_portmode_flag();

ip4_addr_t fibo_getipv4dns(    uint8_t nCid,   CFW_SIM_ID nSimID)
{
    static ip_addr_t dns_server_ip = {0};
    const ip_addr_t *dns1 = NULL, *dns2 = NULL, *dns3 = NULL;
    ip4_addr_t ip4_dns = {0};

    dns1 = dns_getserver(nSimID, nCid, 0);
    dns2 = dns_getserver(nSimID, nCid, 1);
    dns3 = dns_getserver(nSimID, nCid, 2);
    if (!ip_addr_isany(dns1)  && IP_IS_V4(dns1))
    {
        OSI_PRINTFI("fibo_getipv4dns use first dns");
        ip_addr_copy(dns_server_ip, *dns1);
    }
    else if (!ip_addr_isany(dns2)  && IP_IS_V4(dns2))
    {
        OSI_PRINTFI("fibo_getipv4dns use secondary dns");
        ip_addr_copy(dns_server_ip, *dns2);
    }
    else if (!ip_addr_isany(dns3)  && IP_IS_V4(dns3))
    {
        OSI_PRINTFI("fibo_getipv4dns use third dns");
        ip_addr_copy(dns_server_ip, *dns3);
    }
    else
    {
        OSI_PRINTFI("fibo_getipv4dns use backup dns");
        fibo_get_backup_dns(&dns_server_ip);
    }
    ip4_dns = *ip_2_ip4(&dns_server_ip);
    OSI_PRINTFI(" fibo_getipv4dns: ip4_nat_dns:%s", ip4addr_ntoa(&ip4_dns));

    return ip4_dns;
}
#endif

static uint16_t sNAT_TCP_TTL = LWIP_NAT_DEFAULT_TCP_TTL_SECONDS;
static uint16_t sNAT_UDP_TTL = LWIP_NAT_DEFAULT_UDP_TTL_SECONDS;
static uint16_t sNAT_ICMP_TTL = LWIP_NAT_DEFAULT_ICMP_TTL_SECONDS;

void set_nat_TTL(uint16_t natTTL, enum NAT_TTL_TYPE type)
{

    switch (type)
    {
        case NAT_TCP_TTL_TYPE:
            sNAT_TCP_TTL = natTTL;
            break;

        case NAT_UDP_TTL_TYPE:
            sNAT_UDP_TTL = natTTL;
            break;

        case NAT_ICMP_TTL_TYPE:
            sNAT_ICMP_TTL = natTTL;
            break;

        default:
            OSI_LOGE(0x1000a4dc, "sNAT_TTL set param not allowed");
            break;
    }
}

uint16_t get_nat_TTL(enum NAT_TTL_TYPE type)
{
    uint16_t ret = 0;
    switch (type)
    {
        case NAT_TCP_TTL_TYPE:
            ret = sNAT_TCP_TTL;
            break;

        case NAT_UDP_TTL_TYPE:
            ret = sNAT_UDP_TTL;
            break;

        case NAT_ICMP_TTL_TYPE:
            ret = sNAT_ICMP_TTL;
            break;

       default:
            OSI_LOGE(0x1000a4dd, "Type not allowed");
            ret = 0;
            break;

    }
    return ret;
}

uint32_t get_nat_cfg(void)
{
    return sNATCfg;
}

void set_nat_enable(uint32_t natCfg)
{
    sNATCfg = natCfg;
    setNatCfgNv(natCfg);                //MTC0390-284
}

uint32_t get_nat_enable(void)
{
    return sNATCfg;
}

bool get_nat_enabled(uint8_t nSimId, uint8_t nCid)
{
    //OSI_LOGI(0x10007c46, "sNATCfg %d\n", sNATCfg);
    if (nCid == 0x0f && sNATCfg != 0)
        return true;
    if (nSimId > 2 || nCid > 8 || nCid < 1)
        return false;
#ifdef CONFIG_FIBOCOM_BASE
    /*Nat_cfg is of type uint322-t, with the high eight bits controlling sim1 and the low eight bits controlling sim0. 
    If only sim0 has NAT enabled, the value is 255. If both sim0 and sim1 have NAT enabled, the value is 65535.
    Due to the previous single card logic processing, in order to avoid unknown issues caused by modifications, 
    both cards here are opened with 255 as NAT*/
    uint8_t off = 0; 
#else
    uint8_t off = 8 * nSimId + nCid - 1;
#endif
    if ((sNATCfg >> off) & 0x1)
    {
        return true;
    }
    return false;
}

ip4_nat_entry_t *add_static_route(ip4_addr_t dest_net, ip4_addr_t dest_netmask, struct netif *lan_netif, struct netif *wan_netif)
{
    if (NULL == lan_netif || NULL == wan_netif)
        return NULL;

    if ((!netif_is_valid(lan_netif)) || (!netif_is_valid(wan_netif)))
        return NULL;

    if (wan_netif->link_mode != NETIF_LINK_MODE_NAT_WAN)
        return NULL;

    if ((lan_netif->link_mode != NETIF_LINK_MODE_NAT_LWIP_LAN) && (lan_netif->link_mode != NETIF_LINK_MODE_NAT_PPP_LAN) && (lan_netif->link_mode != NETIF_LINK_MODE_NAT_NETDEV_LAN))
        return NULL;

    ip4_nat_entry_t *nat_entry = calloc(1, sizeof(ip4_nat_entry_t));
    if (nat_entry != NULL)
    {
        nat_entry->in_if = lan_netif;
        nat_entry->out_if = wan_netif;
        ip4_addr_copy(nat_entry->dest_net, dest_net);
        ip4_addr_copy(nat_entry->dest_netmask, dest_netmask);
        IP4_ADDR(&(nat_entry->source_netmask), 255, 255, 255, 255);
        ip4_nat_add(nat_entry);
    }
    return nat_entry;
}
#if 0     /*search nat_entry through nat_config*/
static ip4_nat_conf_t *
ip4_nat_search_entry(const ip4_nat_entry_t *nat_entry)
{
    ip4_nat_conf_t *nat_config = ip4_nat_cfg;

    for (nat_config = ip4_nat_cfg; nat_config != NULL; nat_config = nat_config->next)
    {
        if ((nat_config->entry.source_net.addr == nat_entry->source_net.addr) &&
            (nat_config->entry.source_netmask.addr == nat_entry->source_netmask.addr) &&
            (nat_config->entry.dest_net.addr == nat_entry->dest_net.addr) &&
            (nat_config->entry.dest_netmask.addr == nat_entry->dest_netmask.addr) &&
            (nat_config->entry.out_if == nat_entry->out_if) &&
            (nat_config->entry.in_if == nat_entry->in_if))
        {
            break;
        }
    }
    return nat_config;
}
#endif
static void
ip4_nat_addport_init(ip4_nat_conf_t *nat_config, struct netif *lan_netif, ip4_nat_entry_common_t *nat_entry)
{
    LWIP_ASSERT("NULL != nat_entry", NULL != nat_entry);
    LWIP_ASSERT("NULL != nat_config", NULL != nat_config);
    LWIP_ASSERT("NULL != iphdr", NULL != lan_netif);
    nat_entry->cfg = nat_config;
    ip4_addr_set(&nat_entry->source, netif_ip4_addr(lan_netif));
}

ip4_nat_entry_t *add_static_tcpport_route(u16_t static_port, struct netif *lan_netif, struct netif *wan_netif)
{
    struct ip4_nat_entries_tcp *tcp_entry;
    nat_entry_t unat_entry;
    unat_entry.cmn = NULL;
    ip4_nat_conf_t *cur = NULL;
    int rep_flag = 0;
    u16_t port_net = htons(static_port);
    if (NULL == lan_netif || NULL == wan_netif)
        return NULL;

    if ((!netif_is_valid(lan_netif)) || (!netif_is_valid(wan_netif)))
        return NULL;

    if (wan_netif->link_mode != NETIF_LINK_MODE_NAT_WAN)
        return NULL;

    if ((lan_netif->link_mode != NETIF_LINK_MODE_NAT_LWIP_LAN) &&
        (lan_netif->link_mode != NETIF_LINK_MODE_NAT_PPP_LAN) &&
        (lan_netif->link_mode != NETIF_LINK_MODE_NAT_NETDEV_LAN))
        return NULL;

    SLIST_FOREACH(tcp_entry, ip4_nat_entries_tcp_header, iter)
    {
        if ((tcp_entry->nport == port_net)&&
            (tcp_entry->sport == port_net)&&
            (0 == tcp_entry->dport))
        {
            LWIP_DEBUGF(NAT_DEBUG, (0, "The tcpport has already been added: %d", static_port));
            rep_flag = -1;
            break;
        }
    }

    if (rep_flag < 0)
        return NULL;

    ip4_nat_entry_t *nat_entry = calloc(1, sizeof(ip4_nat_entry_t));
    if (NULL == nat_entry)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "add_static_tcpport_route: no more memery for nat_entry\n"));
        return NULL;
    }

    nat_entry->in_if = lan_netif;
    nat_entry->out_if = wan_netif;
    ip4_addr_copy(nat_entry->source_net, lan_netif->ip_addr.u_addr.ip4);
    IP4_ADDR(&(nat_entry->source_netmask), 255, 255, 255, 255);
    cur = ip4_nat_add(nat_entry);
    if (NULL == cur)
    {
        free(nat_entry);
        return NULL;
    }

    tcp_entry = (struct ip4_nat_entries_tcp *)calloc(1, sizeof(struct ip4_nat_entries_tcp));
    if (NULL == tcp_entry)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x1000a4de, "add_static_tcpport_route: no more memery for TCP\n"));
        ip4_nat_remove(nat_entry);
        free(nat_entry);
        return NULL;
    }
    LWIP_DEBUGF(NAT_DEBUG, (0x1000a4df, "add_static_tcpport_route tcp calloc: %p\n", tcp_entry));
    SLIST_INSERT_HEAD(ip4_nat_entries_tcp_header, tcp_entry, iter);
    unat_entry.tcp = tcp_entry;
    unat_entry.tcp->nport = port_net;
    unat_entry.tcp->sport = port_net;
    unat_entry.tcp->dport = 0;
    unat_entry.tcp->common.ttl = sNAT_TCP_TTL;
    ip4_nat_addport_init(cur, lan_netif, unat_entry.cmn);
    ip4_nat_dbg_dump_tcp_nat_entry("add_static_tcpport_route: created new nat entry: ",
                                   unat_entry.tcp);

    nat_entry->unat_entry = (void *)unat_entry.tcp;

    ip4_nat_tcp_port_occupy(static_port);
    return nat_entry;
}

ip4_nat_entry_t *add_static_udpport_route(u16_t static_port, struct netif *lan_netif, struct netif *wan_netif)
{
    struct ip4_nat_entries_udp *udp_entry;
    nat_entry_t unat_entry;
    unat_entry.cmn = NULL;
    ip4_nat_conf_t *cur = NULL;
    int rep_flag = 0;
    u16_t port_net = htons(static_port);

    if (NULL == lan_netif || NULL == wan_netif)
        return NULL;

    if ((!netif_is_valid(lan_netif)) || (!netif_is_valid(wan_netif)))
        return NULL;

    if (wan_netif->link_mode != NETIF_LINK_MODE_NAT_WAN)
        return NULL;

    if ((lan_netif->link_mode != NETIF_LINK_MODE_NAT_LWIP_LAN) &&
        (lan_netif->link_mode != NETIF_LINK_MODE_NAT_PPP_LAN) &&
        (lan_netif->link_mode != NETIF_LINK_MODE_NAT_NETDEV_LAN))
        return NULL;

    SLIST_FOREACH(udp_entry, ip4_nat_entries_udp_header, iter)
    {
        if ((udp_entry->nport == port_net)&&
            (udp_entry->sport == port_net)&&
            (0 == udp_entry->dport))
        {
            LWIP_DEBUGF(NAT_DEBUG, (0, "The udpport has already been added: %d", static_port));
            rep_flag = -1;
            break;
        }
    }

    if (rep_flag < 0)
        return NULL;

    ip4_nat_entry_t *nat_entry = calloc(1, sizeof(ip4_nat_entry_t));
    if (NULL == nat_entry)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "add_static_udpport_route: no more memery for nat_entry\n"));
        return NULL;
    }

    nat_entry->in_if = lan_netif;
    nat_entry->out_if = wan_netif;
    ip4_addr_copy(nat_entry->source_net, lan_netif->ip_addr.u_addr.ip4);
    IP4_ADDR(&(nat_entry->source_netmask), 255, 255, 255, 255);
    cur = ip4_nat_add(nat_entry);
    if (NULL == cur)
    {
        free(nat_entry);
        return NULL;
    }

    udp_entry = (struct ip4_nat_entries_udp *)calloc(1, sizeof(struct ip4_nat_entries_udp));
    if (NULL == udp_entry)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e0, "add_static_udpport_route: no more memery for UDP\n"));
        ip4_nat_remove(nat_entry);
        free(nat_entry);
        return NULL;
    }

    LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e1, "add_static_udpport_route udp calloc: %p\n", udp_entry));
    SLIST_INSERT_HEAD(ip4_nat_entries_udp_header, udp_entry, iter);
    unat_entry.udp = udp_entry;
    unat_entry.udp->nport = port_net;
    unat_entry.udp->sport = port_net;
    unat_entry.udp->dport = 0;
    unat_entry.udp->common.ttl = sNAT_UDP_TTL;
    ip4_nat_addport_init(cur, lan_netif, unat_entry.cmn);
    ip4_nat_dbg_dump_udp_nat_entry("add_static_udpport_route: created new nat entry: ",
                                   unat_entry.udp);

    nat_entry->unat_entry = (void *)unat_entry.udp;

    ip4_nat_udp_port_occupy(static_port);
    return nat_entry;
}

void remove_static_route(ip4_nat_entry_t *nat_entry)
{
    ip4_nat_remove(nat_entry);
}

void remove_static_port_route(ip4_nat_entry_t *nat_entry)
{
    struct ip4_nat_entries_tcp *tcp_entry = NULL;
    struct ip4_nat_entries_tcp *next_tcp_entry = NULL;
    struct ip4_nat_entries_udp *udp_entry = NULL;
    struct ip4_nat_entries_udp *next_udp_entry = NULL;
    u16_t port_host = 0;
    SLIST_FOREACH_SAFE(tcp_entry, ip4_nat_entries_tcp_header, iter, next_tcp_entry)
    {
        if (tcp_entry == (struct ip4_nat_entries_tcp *)nat_entry->unat_entry)
        {
            port_host = ntohs(tcp_entry->nport);
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e2, "remove_static_port_route tcp remove: %p\n", tcp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e3, "tcp_entry->nport: %d\n", port_host));
            SLIST_REMOVE(ip4_nat_entries_tcp_header, tcp_entry, ip4_nat_entries_tcp, iter);
            ip4_nat_tcp_port_free(port_host);
            free(tcp_entry);
        }
    }
    SLIST_FOREACH_SAFE(udp_entry, ip4_nat_entries_udp_header, iter, next_udp_entry)
    {
        if (udp_entry == (struct ip4_nat_entries_udp *)nat_entry->unat_entry)
        {
            port_host = ntohs(udp_entry->nport);
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e4, "remove_static_port_route udp remove: %p\n", udp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e5, "udp_entry->nport: %d\n", port_host));
            SLIST_REMOVE(ip4_nat_entries_udp_header, udp_entry, ip4_nat_entries_udp, iter);
            ip4_nat_udp_port_free(port_host);
            free(udp_entry);
        }
    }
    ip4_nat_remove(nat_entry);

}

/**
 * Timer callback function that calls ip4_nat_tmr() and reschedules itself.
 *
 * @param arg unused argument
 */
static void
nat_timer(void *arg)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_DEBUGF(TIMERS_DEBUG, (0x10007c47, "tcpip: nat_timer()\n"));

    ip4_nat_tmr();
    sys_timeout(LWIP_NAT_TMR_INTERVAL_SEC * 1000, nat_timer, NULL);
}

/** Initialize this module */
void ip4_nat_init(void)
{
    ip4_nat_entries_icmp_header = (ip4_nat_entries_icmp_head_t *)malloc(sizeof(ip4_nat_entries_icmp_head_t));
    if (NULL == ip4_nat_entries_icmp_header)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x100097f8, "ip4_nat_init: no more memery for ip4_nat_entries_icmp_header\n"));
        return;
    }

    ip4_nat_entries_tcp_header = (ip4_nat_entries_tcp_head_t *)malloc(sizeof(ip4_nat_entries_tcp_head_t));
    if (NULL == ip4_nat_entries_tcp_header)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x100097f9, "ip4_nat_init: no more memery for ip4_nat_entries_tcp_header\n"));
        free(ip4_nat_entries_icmp_header);
        return;
    }

    ip4_nat_entries_udp_header = (ip4_nat_entries_udp_head_t *)malloc(sizeof(ip4_nat_entries_udp_head_t));
    if (NULL == ip4_nat_entries_udp_header)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x100097fa, "ip4_nat_init: no more memery for ip4_nat_entries_udp_header\n"));
        free(ip4_nat_entries_tcp_header);
        free(ip4_nat_entries_icmp_header);
        return;
    }

#ifdef CONFIG_FIBOCOM_BASE
    UNUSED(ip4_nat_udp_port_alloc);
    UNUSED(ip4_nat_tcp_port_alloc);
    UNUSED(bitmap_create);
    UNUSED(bitmap_destroy);
#else
    ip4_nat_tcp_port_backup_map = bitmap_create(LWIP_NAT_BACKUP_TCP_SOURCE_PORT);
    if (NULL == ip4_nat_tcp_port_backup_map)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "ip4_nat_init: no more memery for ip4_nat_tcp_port_backup_map\n"));
        free(ip4_nat_entries_udp_header);
        free(ip4_nat_entries_tcp_header);
        free(ip4_nat_entries_icmp_header);
        return;
    }
    ip4_nat_udp_port_backup_map = bitmap_create(LWIP_NAT_BACKUP_UDP_SOURCE_PORT);
    if (NULL == ip4_nat_udp_port_backup_map)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0, "ip4_nat_init: no more memery for ip4_nat_udp_port_backup_map\n"));
        bitmap_destroy(ip4_nat_tcp_port_backup_map);
        free(ip4_nat_entries_udp_header);
        free(ip4_nat_entries_tcp_header);
        free(ip4_nat_entries_icmp_header);
        return;
    }
#endif

#ifdef CONFIG_FIBOCOM_BASE
        fibo_init_nat_entry_index();
#endif

    SYS_ARCH_DECL_PROTECT(lev);

    SLIST_INIT(ip4_nat_entries_icmp_header);
    SLIST_INIT(ip4_nat_entries_tcp_header);
    SLIST_INIT(ip4_nat_entries_udp_header);

    /* we must lock scheduler to protect following code */
    SYS_ARCH_PROTECT(lev);

    /* add a lwip timer for NAT */
    sys_timeout(LWIP_NAT_TMR_INTERVAL_SEC * 1000, nat_timer, NULL);

    /* un-protect */
    SYS_ARCH_UNPROTECT(lev);
}

/** Allocate a new ip4_nat_conf_t item */
static ip4_nat_conf_t *
ip4_nat_alloc_config(void)
{
    ip4_nat_conf_t *ret = (ip4_nat_conf_t *)mem_malloc(sizeof(ip4_nat_conf_t));
    return ret;
}

/** Free a removed ip4_nat_conf_t item */
static void
ip4_nat_free_config(ip4_nat_conf_t *item)
{
    LWIP_ASSERT("item != NULL", item != NULL);
    mem_free(item);
}

/** Add a new NAT entry
 *
 * @param new_entry pointer to a structure used to initialize the entry
 * @return nat_config if succeeded
 */
ip4_nat_conf_t *ip4_nat_add(const ip4_nat_entry_t *new_entry)
{
    ip4_nat_conf_t *cur = ip4_nat_cfg;
    ip4_nat_conf_t *ip4_nat_cfg_new = ip4_nat_alloc_config();
    LWIP_ASSERT("new_entry != NULL", new_entry != NULL);

    if (ip4_nat_cfg_new != NULL)
    {
        SMEMCPY(&ip4_nat_cfg_new->entry, new_entry, sizeof(ip4_nat_entry_t));
        ip4_nat_cfg_new->entry.port_index = 0;
        ip4_nat_cfg_new->next = NULL;

        ip4_nat_dbg_dump_init(ip4_nat_cfg_new);

        if (ip4_nat_cfg == NULL)
        {
            ip4_nat_cfg = ip4_nat_cfg_new;
        }
        else
        {
            /* @todo: do we really need to enqueue the new entry at the end?? */
            uint16_t port_index = 0;
            if (new_entry->out_if == cur->entry.out_if)
            {
                port_index++;
            }
            while (cur->next != NULL)
            {
                cur = cur->next;
                if (new_entry->out_if == cur->entry.out_if)
                {
                    port_index++;
                }
            }
            LWIP_DEBUGF(NAT_DEBUG, (0x100096ca, "ip4_nat_add: find same out_if port_index: %d\n", port_index));

            ip4_nat_cfg_new->entry.port_index = port_index;
            cur->next = ip4_nat_cfg_new;
        }
    }
    else
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e6, "ip4_nat_add: ERR_MEM\n"));
    }
    return ip4_nat_cfg_new;
}

/** Remove a NAT entry previously added by 'ip4_nat_add()'.
 *
 * @param remove_entry describes the entry to remove
 */
void ip4_nat_remove(const ip4_nat_entry_t *remove_entry)
{
    ip4_nat_conf_t *cur = ip4_nat_cfg;
    ip4_nat_conf_t *next = NULL;
    ip4_nat_conf_t *previous = NULL;

    while (cur != NULL)
    {
        /* Remove the NAT interfaces */
        if ((cur->entry.source_net.addr == remove_entry->source_net.addr) &&
            (cur->entry.source_netmask.addr == remove_entry->source_netmask.addr) &&
            (cur->entry.dest_net.addr == remove_entry->dest_net.addr) &&
            (cur->entry.dest_netmask.addr == remove_entry->dest_netmask.addr) &&
            (cur->entry.out_if == remove_entry->out_if) &&
            (cur->entry.in_if == remove_entry->in_if))
        {
            ip4_nat_dbg_dump_remove(cur);

            ip4_nat_remove_config(cur);
            next = cur->next;
            if (cur == ip4_nat_cfg)
            {
                ip4_nat_cfg = next;
            }
            else
            {
                LWIP_ASSERT("NULL != previous", NULL != previous);
                previous->next = next;
            }
            /* free 'cur' or there will be a memory leak */
            ip4_nat_free_config(cur);
            return;
        }
        else
        {
            previous = cur;
            cur = cur->next;
        }
    }
}

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
/** Reset nat_entries but do not free nat_entry previously added by 'ip4_nat_add()'.
 *
 * @param remove_entry describes the entry to Reset
 */
void
ip4_nat_remove_nofree(void)
{
    OSI_PRINTFE("ip4_nat_remove_nofree is come in!");


    struct ip4_nat_entries_icmp* icmp_entry = NULL;
    struct ip4_nat_entries_icmp* next_icmp_entry = NULL;
    struct ip4_nat_entries_tcp* tcp_entry = NULL;
    struct ip4_nat_entries_tcp* next_tcp_entry = NULL;
    struct ip4_nat_entries_udp* udp_entry = NULL;
    struct ip4_nat_entries_udp* next_udp_entry = NULL;
    SLIST_FOREACH_SAFE(icmp_entry, ip4_nat_entries_icmp_header, iter, next_icmp_entry)
    {
        LWIP_DEBUGF(NAT_DEBUG, (0x100097fb, "ip4_nat_reset_state icmp remove: %p\n", icmp_entry));
        LWIP_DEBUGF(NAT_DEBUG, (0x100097fc, "icmp->common.ttl: %ld\n", icmp_entry->common.ttl));
        icmp_entry->common.ttl = 0;
        icmp_entry->common.dest.addr = 0;
        icmp_entry->common.source.addr = 0;
        icmp_entry->common.state = CLOSED;
    }
    SLIST_FOREACH_SAFE(tcp_entry, ip4_nat_entries_tcp_header, iter, next_tcp_entry)
    {
        OSI_PRINTFE("ip4_nat_remove_nofree tcp nat_config is come in1");
        OSI_PRINTFE("ip4_nat_remove_nofree tcp(source) is %s",ip4addr_ntoa(&(tcp_entry->common.source)));
        OSI_PRINTFE("ip4_nat_remove_nofree tcp(dest) is %s",ip4addr_ntoa(&(tcp_entry->common.dest)));
        LWIP_DEBUGF(NAT_DEBUG, (0x100097fd, "ip4_nat_reset_state tcp remove: %p\n", tcp_entry));
        LWIP_DEBUGF(NAT_DEBUG, (0x100097fe, "tcp->common.ttl: %ld\n", tcp_entry->common.ttl));
        tcp_entry->common.ttl = 0;
        tcp_entry->common.dest.addr = 0;
        tcp_entry->common.source.addr = 0;
        tcp_entry->common.state = CLOSED;
    }
    SLIST_FOREACH_SAFE(udp_entry, ip4_nat_entries_udp_header, iter, next_udp_entry)
    {
        OSI_PRINTFE("ip4_nat_remove_nofree udp nat_config is come in1");
        OSI_PRINTFE("ip4_nat_remove_nofree udp(source) is %s",ip4addr_ntoa(&(udp_entry->common.source)));
        OSI_PRINTFE("ip4_nat_remove_nofree udp(dest) is %s",ip4addr_ntoa(&(udp_entry->common.dest)));
        LWIP_DEBUGF(NAT_DEBUG, (0x100097ff, "ip4_nat_reset_state udp remove: %p\n", udp_entry));
        LWIP_DEBUGF(NAT_DEBUG, (0x10009800, "udp->common.ttl: %ld\n", udp_entry->common.ttl));
        udp_entry->common.ttl = 0;
        udp_entry->common.dest.addr = 0;
        udp_entry->common.source.addr = 0;
        udp_entry->common.state = CLOSED;
    }
}

#endif


/** Remove a NAT configured entry from List
 *
 * @param cfg NAT entry to remove
 */
static void
ip4_nat_remove_config(ip4_nat_conf_t *cfg)
{
    struct ip4_nat_entries_icmp *icmp_entry = NULL;
    struct ip4_nat_entries_icmp *next_icmp_entry = NULL;
    struct ip4_nat_entries_tcp *tcp_entry = NULL;
    struct ip4_nat_entries_tcp *next_tcp_entry = NULL;
    struct ip4_nat_entries_udp *udp_entry = NULL;
    struct ip4_nat_entries_udp *next_udp_entry = NULL;

    SLIST_FOREACH_SAFE(icmp_entry, ip4_nat_entries_icmp_header, iter, next_icmp_entry)
    {
        if (icmp_entry->common.cfg == cfg)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e7, "ip4_nat_remove_config icmp remove: %p\n", icmp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x100097fc, "icmp->common.ttl: %ld\n", icmp_entry->common.ttl));
            SLIST_REMOVE(ip4_nat_entries_icmp_header, icmp_entry, ip4_nat_entries_icmp, iter);
            free(icmp_entry);
        }
    }
    SLIST_FOREACH_SAFE(tcp_entry, ip4_nat_entries_tcp_header, iter, next_tcp_entry)
    {
        if (tcp_entry->common.cfg == cfg)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e8, "ip4_nat_remove_config tcp remove: %p\n", tcp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x100097fe, "tcp->common.ttl: %ld\n", tcp_entry->common.ttl));
#ifdef CONFIG_FIBOCOM_BASE
            fibo_remove_nat_entry(E_NAT_TCP, tcp_entry->nport);
#endif
            SLIST_REMOVE(ip4_nat_entries_tcp_header, tcp_entry, ip4_nat_entries_tcp, iter);
#ifndef CONFIG_FIBOCOM_BASE
            ip4_nat_tcp_port_free(ntohs(tcp_entry->nport));
#endif
            free(tcp_entry);
        }
    }
    SLIST_FOREACH_SAFE(udp_entry, ip4_nat_entries_udp_header, iter, next_udp_entry)
    {
        if (udp_entry->common.cfg == cfg)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x1000a4e9, "ip4_nat_remove_config udp remove: %p\n", udp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x10009800, "udp->common.ttl: %ld\n", udp_entry->common.ttl));
#ifdef CONFIG_FIBOCOM_BASE
            fibo_remove_nat_entry(E_NAT_UDP, udp_entry->nport);
#endif
            SLIST_REMOVE(ip4_nat_entries_udp_header, udp_entry, ip4_nat_entries_udp, iter);
#ifndef CONFIG_FIBOCOM_BASE
            ip4_nat_udp_port_free(ntohs(udp_entry->nport));
#endif
            free(udp_entry);
        }
    }
}

/** Check if this packet should be routed or should be translated
 *
 * @param iphdr the IP header to check
 * @return - a NAT entry if the packet shall be translated,
 *         - NULL if the packet shall be routed normally
 */
static ip4_nat_conf_t *
ip4_nat_shallnat(const struct ip_hdr *iphdr)
{
    ip4_nat_conf_t *nat_config = ip4_nat_cfg;

    for (nat_config = ip4_nat_cfg; nat_config != NULL; nat_config = nat_config->next)
    {
        if (ip4_addr_netcmp(&(iphdr->dest), &(nat_config->entry.dest_net),
                            &(nat_config->entry.dest_netmask)))
        {
            if (netif_is_valid(nat_config->entry.in_if) && netif_is_valid(nat_config->entry.out_if))
                break;
        }
    }
    if (nat_config != NULL)
        return nat_config;

    for (nat_config = ip4_nat_cfg; nat_config != NULL; nat_config = nat_config->next)
    {
        if (ip4_addr_netcmp(&(iphdr->src), &(nat_config->entry.source_net),
                            &(nat_config->entry.source_netmask)))
        {
            if (netif_is_valid(nat_config->entry.in_if) && netif_is_valid(nat_config->entry.out_if))
                break;
        }
    }
    return nat_config;
}
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
static ip4_nat_conf_t *ip4_nat_shallnat_for_inputpkt(const struct ip_hdr *iphdr)
{
    ip4_nat_conf_t *nat_config = ip4_nat_cfg;
    ip4_addr_t temp_ip;
    uint8_t current_lan_ip = fibo_get_current_lan_ipaddr_haikang();
    if(current_lan_ip == 1)
    {
        ip4addr_aton("192.168.0.100", &temp_ip);
    }
    else if(current_lan_ip == 2)
    {
        ip4addr_aton("192.168.225.100", &temp_ip);
    }
    else if(current_lan_ip == 3)
    {
        ip4addr_aton("192.168.226.100", &temp_ip);
    }
    else if(current_lan_ip == 4)
    {
        ip4addr_aton("192.168.224.100", &temp_ip);
    }

    for (nat_config = ip4_nat_cfg; nat_config != NULL; nat_config = nat_config->next) 
    {
      OSI_PRINTFE("wjy ip4_nat_shallnat_for_inputpkt begin!");
      ip4_nat_dbg_dump_ip(&(nat_config->entry.source_net));
      ip4_nat_dbg_dump_ip(&(nat_config->entry.dest_net));
      OSI_PRINTFE("wjy ip4_nat_shallnat_for_inputpkt compare result is: %d", ip4_addr_cmp(&(nat_config->entry.source_net),&temp_ip));
      if ((ip4_addr_netcmp(&(iphdr->dest), &(nat_config->entry.dest_net),
                       &(nat_config->entry.dest_netmask)) && ip4_addr_cmp(&(nat_config->entry.source_net),&temp_ip)) ||
      ip4_addr_netcmp(&(iphdr->src), &(nat_config->entry.source_net),
                     &(nat_config->entry.source_netmask))) {
        break;
    }
}

  OSI_PRINTFI("ip4_nat_shallnat_for_inputpkt nat_config is %p",nat_config);
  return nat_config;
}
#endif

/** Check if the IP header can be hidden and if the remaining packet
 * is long enough. p->payload is reset to the IP header on return.
 *
 * @param p received packet, p->payload pointing to IP header
 * @param min_size minimum p->tot_len after hiding IP header
 * @return a pointer to the next header (after IP header),
 *         NULL if hiding IP header fails or the packet is too short
 */
static void *
ip4_nat_check_header(struct pbuf *p, u16_t min_size)
{
    void *ret = NULL;
    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    s16_t iphdr_len = IPH_HL(iphdr) * 4;
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c48, "ip4_nat_check_header: iphdr_len %d\n", iphdr_len));
    if (p->len > iphdr_len)
    {
        if (!pbuf_header(p, -iphdr_len))
        {
            if (p->tot_len >= min_size)
            {
                ret = p->payload;
            }
            /* Restore pbuf payload pointer from previous header check. */
            pbuf_header(p, iphdr_len);
        }
    }
    else
    {
        uint16_t offset = 0;
        struct pbuf *q1 = NULL;
        for (q1 = p; q1 != NULL; q1 = q1->next)
        {
            if (offset + q1->len > iphdr_len)
            {
                if (q1->len > min_size)
                {
                    ret = q1->payload + offset - iphdr_len;
                }
                else
                {
                    break;
                }
            }
            offset += q1->len;
        }
    }
    return ret;
}

/** Input processing: check if a received packet belongs to a NAT entry
 * and if so, translated it and send it on.
 *
 * @param p received packet
 * @return 1 if the packet has been consumed (it was a NAT packet),
 *         0 if the packet has not been consumed (no NAT packet)
 */
u8_t ip4_nat_input(struct pbuf *p)
{
    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    struct tcp_hdr *tcphdr;
    struct udp_hdr *udphdr;
    struct icmp_echo_hdr *icmphdr;
    nat_entry_t nat_entry;
    err_t err;
    u8_t consumed = 0;
    //int                   i;
    struct pbuf *q = NULL;
    struct ip4_nat_entries_icmp *icmp_entry;

    nat_entry.cmn = NULL;
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c49, "ip4_nat_input: p %p\n", p));
    ip4_nat_dbg_dump("ip4_nat_in: checking nat for", iphdr);

    switch (IPH_PROTO(iphdr))
    {
    case IP_PROTO_TCP:
        tcphdr = (struct tcp_hdr *)ip4_nat_check_header(p, sizeof(struct tcp_hdr));
        if (tcphdr == NULL)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x10007c4a, "ip4_nat_input: short tcp packet (%" U16_F " bytes) discarded\n", p->tot_len));
        }
        else
        {
            nat_entry.tcp = ip4_nat_tcp_lookup_incoming(iphdr, tcphdr);
            if (nat_entry.tcp != NULL)
            {
                /* Refresh TCP entry */
#ifdef CONFIG_FIBOCOM_BASE
                LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_incoming: nat_entry.tcp->common.state= %d\n", nat_entry.tcp->common.state));
                LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_incoming: tcphdr->_hdrlen_rsvd_flags= %X\n",lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)));
                switch(nat_entry.tcp->common.state){
                case CLOSED:
                if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_SYN)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_input: tcp_SYN will send!\n"));
                    nat_entry.tcp->common.state = SYN_RCVD;
                }
                break;
                case SYN_RCVD:
                if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_ACK)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_input: tcp_ACK will send after net respond SYN_ACK!\n"));
                    nat_entry.tcp->common.ttl = LWIP_NAT_DEFAULT_TTL_SECONDS;
                    nat_entry.tcp->common.state = ESTABLISHED;
                }
                break;
                case ESTABLISHED:
                if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_FIN || lwip_ntohs(tcphdr->_hdrlen_rsvd_flags) &TCP_RST)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_incoming: after 30s remove  ip4_nat_tcp_table\n"));
                    nat_entry.tcp->common.ttl  = 30;
                    nat_entry.tcp->common.state = CLOSED;
                }
                break;
                default:
                break;
                }
                LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_input: nat_entry.tcp->common.ttl= %d, nat_entry.tcp->common.state= %d\n", nat_entry.tcp->common.ttl, nat_entry.tcp->common.state));
#else
                nat_entry.tcp->common.ttl = sNAT_TCP_TTL;
#endif
                tcphdr->dest = nat_entry.tcp->sport;
                /* Adjust TCP checksum for changed destination port */
                ip4_nat_chksum_adjust((u8_t *)&(tcphdr->chksum),
                                      (u8_t *)&(nat_entry.tcp->nport), 2, (u8_t *)&(tcphdr->dest), 2);

                /* Adjust TCP checksum for changing dest IP address */
                ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT /// XLAT
                if (outif_ip4->addr > 0)
                {
                    ip4_nat_chksum_adjust((u8_t *)&(tcphdr->chksum), (u8_t *)&(outif_ip4->addr), 4, (u8_t *)&(nat_entry.cmn->source.addr), 4);
                }
                else
                {
                    ip4_addr_t v4src = {0};
                    ip4_addr_t v4dest = {0};
                    /* let p point to TCP/UDP/ICMP header */
                    pbuf_header(p, -(s16_t)sizeof(struct ip_hdr));
                    tcphdr->chksum = 0;
                    v4src.addr = iphdr->src.addr;
                    v4dest.addr = iphdr->dest.addr;
                    tcphdr->chksum = inet_chksum_pseudo(p, IP_PROTO_TCP, p->tot_len, &v4src, &v4dest);
                    pbuf_header(p, (s16_t)sizeof(struct ip_hdr));
                }
#else
                ip4_nat_chksum_adjust((u8_t *)&(tcphdr->chksum),
                                      (u8_t *)&(outif_ip4->addr), 4,
                                      (u8_t *)&(nat_entry.cmn->source.addr), 4);
#endif
                consumed = 1;
            }
        }
        break;

    case IP_PROTO_UDP:
        udphdr = (struct udp_hdr *)ip4_nat_check_header(p, sizeof(struct udp_hdr));
        if (udphdr == NULL)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x100097a9, "ip4_nat_input: short udp packet (%hu bytes) discarded\n",
                                    p->tot_len));
        }
        else
        {
            nat_entry.udp = ip4_nat_udp_lookup_incoming(iphdr, udphdr);
            if (nat_entry.udp != NULL)
            {
                /* Refresh UDP entry */
#if defined CONFIG_FIBOCOM_BASE
                nat_entry.udp->common.ttl = LWIP_NAT_DEFAULT_TTL_SECONDS;
#else
                nat_entry.udp->common.ttl = sNAT_UDP_TTL;
#endif
                udphdr->dest = nat_entry.udp->sport;
                if (udphdr->chksum != 0)
                {
                    /* Adjust UDP checksum for changed destination port */
                    ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                                          (u8_t *)&(nat_entry.udp->nport), 2, (u8_t *)&(udphdr->dest), 2);
                    /* Adjust UDP checksum for changing dest IP address */
                    ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
                    ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                                          (u8_t *)&(outif_ip4->addr), 4,
                                          (u8_t *)&(nat_entry.cmn->source.addr), 4);
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
                    /*Because DNS is fixed, in order to distinguish UDP packets of external dial-up and other services,
                    NAT matching lookup is performed in the routing table, 
                    and then comparison and check code calculation are performed*/
                    OSI_PRINTFE("wjy input begin");
                    ip4_addr_t  *dns_fix = calloc(1, sizeof(ip4_addr_t));
                    IP4_ADDR(dns_fix, ip4_addr1(&(iphdr->src)), ip4_addr2(&(iphdr->src)), ip4_addr3(&(iphdr->src)), ip4_addr4(&(iphdr->src)));
                    OSI_PRINTFE("wjy input network DNS is :");
                    ip4_nat_dbg_dump_ip(&(ip4_network_dns));
                    OSI_PRINTFE("wjy input iphdr src DNS is :");
                    ip4_nat_dbg_dump_ip(dns_fix);
                    if(ip4_addr_cmp(dns_fix, &ip4_network_dns))
                    {
                        OSI_PRINTFE("wjy input matchdnsflag is true");
                        ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                          (u8_t *)&(iphdr->src.addr), 4, (u8_t *)&(ip4_nat_dns.addr), 4);

                        ip4_nat_chksum_adjust((u8_t *) & IPH_CHKSUM(iphdr),
                            (u8_t *) & (iphdr->src.addr), 4, (u8_t *) & ip4_nat_dns.addr, 4);
                        ip4_addr_copy(iphdr->src, ip4_nat_dns);
                    }
                    free(dns_fix);
#endif
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT /// XLAT
                    OSI_PRINTFI("xlat ip4addr %x", outif_ip4->addr);
                    if (outif_ip4->addr == 0)
                    {
                      ip4_addr_t v4src = {0};
                      ip4_addr_t v4dest = {0};
                      /* let p point to TCP/UDP/ICMP header */
                      pbuf_header(p, -(s16_t)sizeof(struct ip_hdr));
                      udphdr->chksum = 0;
                      v4src.addr = iphdr->src.addr;
                      v4dest.addr = iphdr->dest.addr;
                      udphdr->chksum = inet_chksum_pseudo(p, IP_PROTO_UDP, p->tot_len, &v4src, &v4dest);
                      pbuf_header(p, (s16_t)sizeof(struct ip_hdr));
                    }
#endif //CONFIG_FIBOCOM_BASE XLAT
                }
                consumed = 1;
            }
        }
        break;

    case IP_PROTO_ICMP:
        icmphdr = (struct icmp_echo_hdr *)ip4_nat_check_header(p, sizeof(struct icmp_echo_hdr));
        if (icmphdr == NULL)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x100097aa, "ip4_nat_out: short icmp echo reply packet (%hu bytes) discarded\n",
                                    p->tot_len));
        }
        else
        {
            if (ICMP_ER == ICMPH_TYPE(icmphdr))
            {
                SLIST_FOREACH(icmp_entry, ip4_nat_entries_icmp_header, iter)
                {
                    nat_entry.icmp = icmp_entry;
                    if ((nat_entry.icmp->common.ttl) &&
                        (iphdr->src.addr == nat_entry.icmp->common.dest.addr) &&
                        (nat_entry.icmp->id == icmphdr->id) &&
                        (nat_entry.icmp->seqno == icmphdr->seqno))
                    {
                        ip4_nat_dbg_dump_icmp_nat_entry("found existing nat entry: ", nat_entry.icmp);
                        consumed = 1;
                        IPNAT_ENTRY_RESET(nat_entry.cmn);
                        break;
                    }
                }
            }
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
            if (ICMPH_TYPE(icmphdr) == ICMP_ECHO)
            {
                ip4_nat_conf_t *nat_config;
                nat_config = ip4_nat_shallnat_for_inputpkt(iphdr);
                OSI_PRINTFI("ip4_nat_input: nat_config is %p",nat_config);
                /*DUMP  if nat_config is null,result is 0 directly*/
                if (NULL == nat_config)
                {
                    return 0;
                }
                icmp_entry = (struct ip4_nat_entries_icmp*)calloc(1, sizeof(struct ip4_nat_entries_icmp));
                if (NULL == icmp_entry)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x10009804, "ip4_nat_input: no more memery for ICMP\n"));
                    return 0;
                }
                LWIP_DEBUGF(NAT_DEBUG, (0x10009805, "ip4_nat_input icmp calloc: %p\n", icmp_entry));
                SLIST_INSERT_HEAD(ip4_nat_entries_icmp_header, icmp_entry, iter);
                nat_entry.icmp = icmp_entry;
                /* Reset ttl*/
                if(nat_config != NULL)
                {
                    nat_entry.cmn->cfg = nat_config;
                    ip4_addr_set(&nat_entry.cmn->dest, &nat_config->entry.dest_net);
                    ip4_addr_set(&nat_entry.cmn->source, &nat_config->entry.source_net);
                    OSI_PRINTFI("nat_entry->cmn->dest is 0x%lx",nat_entry.cmn->dest);
                    OSI_PRINTFI("nat_entry->cmn->source is 0x%lx",nat_entry.cmn->source);
                    nat_entry.icmp->common.ttl = LWIP_NAT_DEFAULT_ICMP_TTL_SECONDS;
                }
                nat_entry.icmp->id = icmphdr->id;
                nat_entry.icmp->seqno = icmphdr->seqno;
                consumed = 1;
                ip4_nat_dbg_dump_icmp_nat_entry(" ip4_nat_input: created new NAT entry ", nat_entry.icmp);
            }
#endif
        }
        break;

    default:
        break;
    }

    if (consumed)
    {
        /* packet consumed, send it out on in_if */
        struct netif *in_if;
        q = p;
        in_if = nat_entry.cmn->cfg->entry.in_if;
        if (netif_is_valid(in_if) == false)
            return 0;
        iphdr->dest.addr = nat_entry.cmn->source.addr;
        ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
        ip4_nat_chksum_adjust((u8_t *)&IPH_CHKSUM(iphdr),
                              (u8_t *)&(outif_ip4->addr), 4,
                              (u8_t *)&(iphdr->dest.addr), 4);

        ip4_nat_dbg_dump("ip4_nat_input: packet back to source after nat: ", iphdr);
        LWIP_DEBUGF(NAT_DEBUG, (0x10007c4e, "ip4_nat_input: sending packet on interface ("));
        ip4_nat_dbg_dump_ip(ip_2_ip4(&(in_if->ip_addr)));
        LWIP_DEBUGF(NAT_DEBUG, (0x10007c4f, ")\n"));
        LWIP_DEBUGF(NAT_DEBUG, (0x10007c50, "ip4_nat_input: q %p\n", q));
#if 0
    uint8_t *pData = malloc(q->tot_len);
    uint16_t offset = 0;
    struct pbuf *q1 = NULL;
    for (q1 = q; q1!= NULL; q1 = q1->next)
    {
        memcpy(&pData[offset], q1->payload, q1->len);
        offset += q1->len;
    }

    sys_arch_dump(pData, q->tot_len);
    free(pData);
#endif
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT /// XLAT
        if (outif_ip4->addr == 0)
        {
            (iphdr)->_chksum = 0;
            IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, IP_HLEN));
        }
#endif
        err = in_if->input(q, in_if);
        if (err != ERR_OK)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x100097ab, "ip4_nat_input: failed to send rewritten packet. link layer returned %d\n",
                                    err));
        }
    }
    return consumed;
}

/** Check if one NAT entry timed out */
static int
ip4_nat_check_timeout(ip4_nat_entry_common_t *nat_entry)
{
    if (nat_entry->ttl > 0)
    {
        if (nat_entry->ttl != LWIP_NAT_TTL_INFINITE)
        {
            /* this is not a 'no-timeout' entry */
            if (nat_entry->ttl > LWIP_NAT_TMR_INTERVAL_SEC)
            {
                nat_entry->ttl -= LWIP_NAT_TMR_INTERVAL_SEC;
                return 0;
            }
            else
            {
                nat_entry->ttl = 0;
                nat_entry->dest.addr = 0;
                nat_entry->source.addr = 0;
#ifdef CONFIG_FIBOCOM_BASE
                nat_entry->state = CLOSED;
#endif
                return 1;
            }
        }
#ifdef CONFIG_FIBOCOM_BASE
    else
    {
        return 0;
    }
#endif
    }
    return 1;
}

/** The NAT timer function, to be called at an interval of
 * LWIP_NAT_TMR_INTERVAL_SEC seconds.
 */
void ip4_nat_tmr(void)
{
    int rc;

#ifdef CONFIG_FIBOCOM_BASE
    int count_tcp = 0;
    int count_udp = 0;
#endif
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c51, "ip4_nat_tmr: removing old entries\n"));

    struct ip4_nat_entries_icmp *icmp_entry = NULL;
    struct ip4_nat_entries_icmp *next_icmp_entry = NULL;
    struct ip4_nat_entries_tcp *tcp_entry = NULL;
    struct ip4_nat_entries_tcp *next_tcp_entry = NULL;
    struct ip4_nat_entries_udp *udp_entry = NULL;
    struct ip4_nat_entries_udp *next_udp_entry = NULL;

    SLIST_FOREACH_SAFE(icmp_entry, ip4_nat_entries_icmp_header, iter, next_icmp_entry)
    {
        rc = ip4_nat_check_timeout((ip4_nat_entry_common_t *)icmp_entry);
        if (rc == 1)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x10009801, "ip4_nat_tmr icmp remove: %p\n", icmp_entry));
            LWIP_DEBUGF(NAT_DEBUG, (0x100097fc, "icmp->common.ttl: %ld\n", icmp_entry->common.ttl));
            SLIST_REMOVE(ip4_nat_entries_icmp_header, icmp_entry, ip4_nat_entries_icmp, iter);
            free(icmp_entry);
            icmp_entry = NULL;
        }
    }
    SLIST_FOREACH_SAFE(tcp_entry, ip4_nat_entries_tcp_header, iter, next_tcp_entry)
    {
#ifdef CONFIG_FIBOCOM_BASE
        ++count_tcp;
        if(tcp_entry->common.ttl >0)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tmr tcp table index = %d  ttl %d\n",count_tcp, tcp_entry->common.ttl));
        }
#endif
        if(0 != tcp_entry->dport)
        {
            rc = ip4_nat_check_timeout((ip4_nat_entry_common_t *)tcp_entry);
            if (rc == 1)
            {
                LWIP_DEBUGF(NAT_DEBUG, (0x10009802, "ip4_nat_tmr tcp remove: %p\n", tcp_entry));
                LWIP_DEBUGF(NAT_DEBUG, (0x100097fe, "tcp->common.ttl: %ld\n", tcp_entry->common.ttl));
#ifdef CONFIG_FIBOCOM_BASE
                fibo_remove_nat_entry(E_NAT_TCP, tcp_entry->nport);
#endif
                SLIST_REMOVE(ip4_nat_entries_tcp_header, tcp_entry, ip4_nat_entries_tcp, iter);
#ifndef CONFIG_FIBOCOM_BASE
                ip4_nat_tcp_port_free(ntohs(tcp_entry->nport));
#endif
                free(tcp_entry);
            }
        }
    }
    SLIST_FOREACH_SAFE(udp_entry, ip4_nat_entries_udp_header, iter, next_udp_entry)
    {
#ifdef CONFIG_FIBOCOM_BASE
        ++count_udp;
        if(udp_entry->common.ttl >0)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tmr udp table index = %d  ttl %d\n",count_udp, udp_entry->common.ttl));
        }
#endif
        if(0 != udp_entry->dport)
        {
            rc = ip4_nat_check_timeout((ip4_nat_entry_common_t *)udp_entry);
            if (rc == 1)
            {
                LWIP_DEBUGF(NAT_DEBUG, (0x10009803, "ip4_nat_tmr udp remove: %p\n", udp_entry));
                LWIP_DEBUGF(NAT_DEBUG, (0x10009800, "udp->common.ttl: %ld\n", udp_entry->common.ttl));
#ifdef CONFIG_FIBOCOM_BASE
                fibo_remove_nat_entry(E_NAT_UDP, udp_entry->nport);
#endif
                SLIST_REMOVE(ip4_nat_entries_udp_header, udp_entry, ip4_nat_entries_udp, iter);
#ifndef CONFIG_FIBOCOM_BASE
                ip4_nat_udp_port_free(ntohs(udp_entry->nport));
#endif
                free(udp_entry);
            }
        }
    }
}

/** Check if we want to perform NAT with this packet. If so, send it out on
 * the correct interface.
 *
 * @param p the packet to test/send
 * @return 1: the packet has been sent using NAT,
 *         0: the packet did not belong to a NAT entry
 */
u8_t ip4_nat_out(struct pbuf *p)
{
    u8_t sent = 0;
    err_t err;
    struct ip_hdr *iphdr = p->payload;
    struct icmp_echo_hdr *icmphdr;
    struct tcp_hdr *tcphdr;
    struct udp_hdr *udphdr;
    ip4_nat_conf_t *nat_config;
    nat_entry_t nat_entry;
    //int             i;
    struct ip4_nat_entries_icmp *icmp_entry;

    nat_entry.cmn = NULL;
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
    bool  matchdnsflag = false;
#endif

    ip4_nat_dbg_dump("ip4_nat_out: checking nat for", iphdr);

    /* Check if this packet should be routed or should be translated */
    nat_config = ip4_nat_shallnat(iphdr);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c53, "ip4_nat_out: ip4_nat_shallnat %p\n", nat_config));
    if (nat_config != NULL)
    {
        if (nat_config->entry.out_if == NULL)
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x10007c54, "ip4_nat_out: no external interface for nat table entry\n"));
        }
        else
        {
            switch (IPH_PROTO(iphdr))
            {
            case IP_PROTO_TCP:
                tcphdr = (struct tcp_hdr *)ip4_nat_check_header(p, sizeof(struct tcp_hdr));
                if (tcphdr == NULL)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x100097ac, "ip4_nat_out: short tcp packet (0x0, %hu bytes) discarded\n", p->tot_len));
                }
                else
                {
                    nat_entry.tcp = ip4_nat_tcp_lookup_outgoing(nat_config, iphdr, tcphdr, 1);
                    if (nat_entry.tcp != NULL)
                    {
                        /* Reset ttl*/
#ifdef CONFIG_FIBOCOM_BASE
                        LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_outgoing: nat_entry.tcp->common.state= %d\n", nat_entry.tcp->common.state));
                        LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_outgoing: tcphdr->_hdrlen_rsvd_flags= %X\n",lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)));
                        switch(nat_entry.tcp->common.state){
                        case CLOSED:
                        if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_SYN)
                        {
                            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_out: tcp_SYN will send!\n"));
                            nat_entry.tcp->common.state = SYN_SENT;
                        }
                        break;
                        case SYN_SENT:
                        if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_ACK)
                        {
                            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_out: tcp_ACK will send after net respond SYN_ACK!\n"));
                            nat_entry.tcp->common.ttl = LWIP_NAT_DEFAULT_TTL_SECONDS;
                            nat_entry.tcp->common.state = ESTABLISHED;
                        }
                        break;
                        case ESTABLISHED:
                        if (lwip_ntohs(tcphdr->_hdrlen_rsvd_flags)&TCP_FIN || lwip_ntohs(tcphdr->_hdrlen_rsvd_flags) &TCP_RST)
                        {
                            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_outgoing: after 30s remove  ip4_nat_tcp_table\n"));
                            nat_entry.tcp->common.ttl  = 30;
                            nat_entry.tcp->common.state = CLOSED;
                        }
                        break;
                        default:
                        break;
                        }
                        LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_out: nat_entry.tcp->common.ttl= %d, nat_entry.tcp->common.state= %d\n", nat_entry.tcp->common.ttl, nat_entry.tcp->common.state));
#else
                        nat_entry.tcp->common.ttl = sNAT_TCP_TTL;
#endif
                        /* Adjust TCP checksum for changing source port */
                        tcphdr->src = nat_entry.tcp->nport;
                        ip4_nat_chksum_adjust((u8_t *)&(tcphdr->chksum),
                                              (u8_t *)&(nat_entry.tcp->sport), 2, (u8_t *)&(tcphdr->src), 2);
                        /* Adjust TCP checksum for changing source IP address */
                        ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
                        ip4_nat_chksum_adjust((u8_t *)&(tcphdr->chksum),
                                              (u8_t *)&(nat_entry.cmn->source.addr), 4,
                                              (u8_t *)&(outif_ip4->addr), 4);
                    }
                }
                break;

            case IP_PROTO_UDP:
                udphdr = (struct udp_hdr *)ip4_nat_check_header(p, sizeof(struct udp_hdr));
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
                ip4_addr_t  *dns_fix = calloc(1, sizeof(ip4_addr_t));
                IP4_ADDR(dns_fix, ip4_addr1(&(iphdr->src)), ip4_addr2(&(iphdr->src)), ip4_addr3(&(iphdr->src)), 1);
                OSI_PRINTFE("wjy out iphdr DNS is: %s", ip4addr_ntoa(dns_fix));
                ip4_nat_dbg_dump_ip(dns_fix);
                if(ip4_addr_cmp(&(iphdr->dest), dns_fix))
                {
                    OSI_PRINTFE("ip4_nat_out matchdnsflag is true");
                    matchdnsflag = true;
                }
                else
                {
                    OSI_PRINTFE("[%s,%d] HAIKANG_DNS: ip4_nat_out matchdnsflag is not true", __FUNCTION__, __LINE__);
                }
                free(dns_fix);
#endif
                if (udphdr == NULL)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x100097ad, "ip4_nat_out: short udp packet (%hu bytes) discarded\n", p->tot_len));
                }
                else
                {
                    nat_entry.udp = ip4_nat_udp_lookup_outgoing(nat_config, iphdr, udphdr, 1);
                    if (nat_entry.udp != NULL)
                    {
                        /* Reset ttl*/
#if defined CONFIG_FIBOCOM_BASE
                        nat_entry.udp->common.ttl = LWIP_NAT_DEFAULT_TTL_SECONDS;
#else
                        nat_entry.udp->common.ttl = sNAT_UDP_TTL;
#endif
                        udphdr->src = nat_entry.udp->nport;
                        if (udphdr->chksum != 0)
                        {
                            /* Adjust UDP checksum for changing source port */
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
                            /*add there wjy,Fixed DNS*/
                            ip4_addr_t ip4_dns = {0};
                            /*
                            The DNS address is three fixed addresses(192.168.225.1; 192.168.226.1; 192.168.0.1),
                            and the DNS address and IP need to correspond(ex: IP: 192.168.225.100; DNS: 192.168.225.1)
                            */
                            if(matchdnsflag)
                            {
                                ip4_dns = fibo_getipv4dns(1, 0);
                                if(ip4_dns.addr == 0)
                                    OSI_PRINTFE(" ip4_nat_out matchdnsflag ip4_dns get failed!");
                                else
                                {
                                    ip4_addr_copy(ip4_nat_dns,nat_entry.udp->common.dest);
                                    ip4_addr_copy(nat_entry.udp->common.dest, ip4_dns);
                                    ip4_addr_copy(ip4_network_dns, ip4_dns);
                                    OSI_PRINTFI("[%s,%d] HAIKANG_DNS: ip4_nat_dns:%s, ip4_network_dns:%s", __FUNCTION__, __LINE__, ip4addr_ntoa(&ip4_nat_dns), ip4addr_ntoa(&ip4_network_dns));
                                }
                            }
                            /*add end wjy*/
#endif

                            ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                                                  (u8_t *)&(nat_entry.udp->sport), 2, (u8_t *)&(udphdr->src), 2);
                            /* Adjust UDP checksum for changing source IP address */
                            ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
                            ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                                                  (u8_t *)&(nat_entry.cmn->source.addr), 4,
                                                  (u8_t *)&(outif_ip4->addr), 4);

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
                            if(matchdnsflag)
                            {
                                if(ip4_dns.addr != 0)
                                {
                                    ip4_nat_chksum_adjust((u8_t *)&(udphdr->chksum),
                                        (u8_t *) & (iphdr->dest.addr), 4, (u8_t *)&(ip4_dns.addr), 4);
                                    ip4_nat_chksum_adjust((u8_t *) & IPH_CHKSUM(iphdr),
                                        (u8_t *) & (iphdr->dest.addr), 4, (u8_t *) & (ip4_dns.addr), 4);
                                    ip4_addr_copy(iphdr->dest, ip4_dns);
                                }
                            }
                            matchdnsflag = false;
#endif
                        }
                    }
                }
                break;

            case IP_PROTO_ICMP:
            {
                icmphdr = (struct icmp_echo_hdr *)ip4_nat_check_header(p, sizeof(struct icmp_echo_hdr));
                if (icmphdr == NULL)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x100097ae, "ip4_nat_out: short icmp echo packet (%hu bytes) discarded\n", p->tot_len));
                }
                else
                {
                    if (ICMPH_TYPE(icmphdr) == ICMP_ECHO)
                    {
                        icmp_entry = (struct ip4_nat_entries_icmp *)calloc(1, sizeof(struct ip4_nat_entries_icmp));
                        if (NULL == icmp_entry)
                        {
                            LWIP_DEBUGF(NAT_DEBUG, (0x10009804, "ip4_nat_out: no more memery for ICMP\n"));
                            return 0;
                        }
                        LWIP_DEBUGF(NAT_DEBUG, (0x10009805, "ip4_nat_out icmp calloc: %p\n", icmp_entry));
                        SLIST_INSERT_HEAD(ip4_nat_entries_icmp_header, icmp_entry, iter);
                        nat_entry.icmp = icmp_entry;
                        /* Reset ttl*/
                        nat_entry.icmp->common.ttl = sNAT_ICMP_TTL;
                        ip4_nat_cmn_init(nat_config, iphdr, nat_entry.cmn);
                        nat_entry.icmp->id = icmphdr->id;
                        nat_entry.icmp->seqno = icmphdr->seqno;
                        ip4_nat_dbg_dump_icmp_nat_entry(" ip4_nat_out: created new NAT entry ", nat_entry.icmp);
                    }
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
                    if (ICMPH_TYPE(icmphdr) == ICMP_ER)
                    {
                        SLIST_FOREACH(icmp_entry, ip4_nat_entries_icmp_header, iter)
                        {
                            nat_entry.icmp = icmp_entry;
                            if ((nat_entry.icmp->common.ttl) &&
                                (iphdr->src.addr == nat_entry.icmp->common.dest.addr) &&
                                (nat_entry.icmp->id == icmphdr->id) &&
                                (nat_entry.icmp->seqno == icmphdr->seqno))
                            {
                                ip4_nat_dbg_dump_icmp_nat_entry("found existing nat entry: ", nat_entry.icmp);
                                nat_entry.cmn->cfg = nat_config;
                                IPNAT_ENTRY_RESET(nat_entry.cmn);
                                break;
                            }
                        }
                    }
#endif
                }
            }
            break;
            default:
                break;
            }

            if (nat_entry.cmn != NULL)
            {
                struct netif *out_if = nat_entry.cmn->cfg->entry.out_if;
                if (netif_is_valid(out_if) == false)
                    return 0;
                /* Exchange the IP source address with the address of the interface
        * where the packet will be sent.
        */
                /* @todo: check nat_config->entry.out_if agains nat_entry.cmn->cfg->entry.out_if */
                ip4_addr_t *outif_ip4 = ip_2_ip4(&(nat_entry.cmn->cfg->entry.out_if->ip_addr));
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT //XLAT
                if (outif_ip4->addr == 0)
                {
                    return 0;
                }
#endif
                iphdr->src.addr = outif_ip4->addr;
                ip4_nat_chksum_adjust((u8_t *)&IPH_CHKSUM(iphdr),
                                      (u8_t *)&(nat_entry.cmn->source.addr), 4, (u8_t *)&iphdr->src.addr, 4);

                ip4_nat_dbg_dump("ip4_nat_out: rewritten packet", iphdr);
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c56, "ip4_nat_out: sending packet on interface %c%c%hu(", out_if->name[0], out_if->name[1], (u16_t)out_if->num));
                ip4_nat_dbg_dump_ip(outif_ip4);
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c4f, ")\n"));
                ip4_addr_t dest_ip4;
                ip4_addr_copy(dest_ip4, iphdr->dest);
                err = out_if->output(out_if, p, &dest_ip4);
                if (err != ERR_OK)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x100097af, "ip4_nat_out: failed to send rewritten packet. link layer returned %d\n", err));
                }
                else
                {
                    sent = 1;
                }
            }
        }
    }

    return sent;
}

/** Initialize common parts of a NAT entry
 *
 * @param nat_config NAT config entry
 * @param iphdr IP header from which to initialize the entry
 * @param nat_entry entry to initialize
 */
static void
ip4_nat_cmn_init(ip4_nat_conf_t *nat_config, const struct ip_hdr *iphdr, ip4_nat_entry_common_t *nat_entry)
{
    LWIP_ASSERT("NULL != nat_entry", NULL != nat_entry);
    LWIP_ASSERT("NULL != nat_config", NULL != nat_config);
    LWIP_ASSERT("NULL != iphdr", NULL != iphdr);
    nat_entry->cfg = nat_config;
    ip4_addr_set(&nat_entry->dest, &iphdr->dest);
    ip4_addr_set(&nat_entry->source, &iphdr->src);
#ifdef CONFIG_FIBOCOM_BASE
    nat_entry->ttl = LWIP_NAT_TEMP_TTL_SECONDS;
    nat_entry->state = CLOSED;
    nat_entry->ts = osiUpTime();
#endif
}

/**
 * This function checks for incoming packets if we already have a NAT entry.
 * If yes a pointer to the NAT entry is returned. Otherwise NULL.
 *
 * @param nat_config NAT configuration.
 * @param iphdr The IP header.
 * @param udphdr The UDP header.
 * @return A pointer to an existing NAT entry or
 *         NULL if none is found.
 */
static ip4_nat_entries_udp_t *
ip4_nat_udp_lookup_incoming(const struct ip_hdr *iphdr, const struct udp_hdr *udphdr)
{
#ifdef CONFIG_FIBOCOM_BASE
    int nat_entry_count = 0;
#else
    int i = 0;
#endif
    ip4_nat_entries_udp_t *nat_entry = NULL;
#ifdef CONFIG_FIBOCOM_BASE
    struct ip4_nat_entries_udp* udp_entry = NULL;
#else
    struct ip4_nat_entries_udp *udp_entry;
#endif

#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
    struct ip4_nat_entries_udp*oldest_entry = NULL;
    int oldest_entry_num = 0;
    int last_free = 0;
#endif
    SLIST_FOREACH(udp_entry, ip4_nat_entries_udp_header, iter)
    {
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
        if(oldest_entry == NULL)
        {
            oldest_entry = udp_entry;
            oldest_entry_num = nat_entry_count;
        }
        if (udp_entry->common.ts < oldest_entry->common.ts)
        {
            oldest_entry = udp_entry;
            oldest_entry_num = nat_entry_count;
        }
#endif
        if (udp_entry->common.ttl)
        {
            if (((0 == udp_entry->common.dest.addr ) || (iphdr->src.addr == udp_entry->common.dest.addr)) &&
                ((0 == udp_entry->dport) || (udphdr->src == udp_entry->dport)) &&
                (udphdr->dest == udp_entry->nport))
            {
                nat_entry = udp_entry;
#ifdef CONFIG_FIBOCOM_BASE
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c57, "ip4_nat_udp_lookup_incoming: nat_entry_count %d\n", nat_entry_count));
#else
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c57, "ip4_nat_udp_lookup_incoming: i %d\n", i));
#endif
                ip4_nat_dbg_dump_udp_nat_entry("ip4_nat_udp_lookup_incoming: found existing nat entry: ",
                                               nat_entry);

#ifdef CONFIG_FIBOCOM_BASE
                udp_entry->common.ts = osiUpTime();
#endif
                break;
            }
#ifdef CONFIG_FIBOCOM_BASE
            nat_entry_count++;
#endif
        }
    }

#if defined CONFIG_FIBOCOM_DNAT_ENABLE
    //add UDP of DNAT,Add the routing table for server to module transmission
    //Function ip4_nat_out is working module to server(upstream),and Function ip4_nat_input is working for server to module(downstream)
    if (nat_entry == NULL)
    {
        ip4_nat_conf_t *nat_config = ip4_nat_shallnat_for_inputpkt(iphdr);
        if (nat_config != NULL)
        {
          if(nat_entry_count >= LWIP_NAT_DEFAULT_UDP_PORT_COUNT)
          {
              nat_entry = oldest_entry;
              last_free = oldest_entry_num;
          }
          else
          {
              last_free = nat_entry_count;
              udp_entry = (struct ip4_nat_entries_udp*)calloc(1, sizeof(struct ip4_nat_entries_udp));
              if (NULL == udp_entry)
              {
                  LWIP_DEBUGF(NAT_DEBUG, (0x10009808, "ip4_nat_input: no more memery for UDP\n"));
                  return NULL;
              }
              LWIP_DEBUGF(NAT_DEBUG, (0x10009809, "ip4_nat_input udp calloc: %p\n", udp_entry));
              SLIST_INSERT_HEAD(ip4_nat_entries_udp_header, udp_entry, iter);
              nat_entry = udp_entry;
          }
          nat_entry->nport = udphdr->dest;//htons((u16_t) (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT + last_free));
          nat_entry->sport = udphdr->dest;
          nat_entry->dport = udphdr->src;
          nat_entry->common.cfg= nat_config;
          ip4_addr_set(&nat_entry->common.dest, &iphdr->src);
          ip4_addr_set(&nat_entry->common.source, &nat_config->entry.source_net);
          nat_entry->common.ttl = LWIP_NAT_DEFAULT_UDP_TTL_SECONDS;

          ip4_nat_dbg_dump_udp_nat_entry("ip4_nat_udp_lookup_incoming: created new nat entry: ",
                                    nat_entry);
          OSI_PRINTFI("ip4_nat_udp_lookup_incoming nat_entry source is 0x%lx",nat_entry->common.source);
          OSI_PRINTFI("ip4_nat_udp_lookup_incoming nat_entry dest is 0x%lx",nat_entry->common.dest);
          OSI_PRINTFI("ip4_nat_udp_lookup_incoming nat_entry nport is %d",htons(nat_entry->nport));
          OSI_PRINTFI("ip4_nat_udp_lookup_incoming nat_entry dport is %d",htons(nat_entry->dport));
          OSI_PRINTFI("ip4_nat_udp_lookup_incoming: created udp table idx=%d!!!", last_free);

        }
        else
        {
          LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_udp_lookup_incoming: no more NAT entries available\n"));
        }

    }
#endif
    return nat_entry;
}

/**
 * This function checks if we already have a NAT entry for this UDP connection.
 * If yes the a pointer to this NAT entry is returned.
 *
 * @param iphdr The IP header.
 * @param udphdr The UDP header.
 * @param allocate If no existing NAT entry is found and this flag is true
 *        a NAT entry is allocated.
 */
static ip4_nat_entries_udp_t *
ip4_nat_udp_lookup_outgoing(ip4_nat_conf_t *nat_config, const struct ip_hdr *iphdr,
                            const struct udp_hdr *udphdr, u8_t allocate)
{
#ifdef CONFIG_FIBOCOM_BASE
    int nat_entry_count = 0;
#else
    int i = 0;
#endif
    nat_entry_t nat_entry;

    nat_entry.cmn = NULL;

#ifdef CONFIG_FIBOCOM_BASE
    struct ip4_nat_entries_udp*oldest_entry = NULL;
    int last_free = 0;
    struct ip4_nat_entries_udp* udp_entry = NULL;
#else
    struct ip4_nat_entries_udp *udp_entry;
#endif

    SLIST_FOREACH(udp_entry, ip4_nat_entries_udp_header, iter)
    {
#ifdef CONFIG_FIBOCOM_BASE
        if(oldest_entry == NULL)
        {
          oldest_entry = udp_entry;
        }
        if (udp_entry->common.ts < oldest_entry->common.ts)
        {
          oldest_entry = udp_entry;
        }
#else
        i++;
#endif
        if (udp_entry->common.ttl)
        {
            if ((iphdr->src.addr == udp_entry->common.source.addr) &&
                (udphdr->src == udp_entry->sport) &&
                ((0 == udp_entry->dport) ||
                ((iphdr->dest.addr == udp_entry->common.dest.addr) &&
                (udphdr->dest == udp_entry->dport))))
            {
                nat_entry.udp = udp_entry;
#ifdef CONFIG_FIBOCOM_BASE
                OSI_PRINTFI("ip4_nat_udp_lookup_outgoing: nat_entry_count %d, noprt %d, soprt %d, doprt %d",
                 nat_entry_count, ntohs(udp_entry->nport), ntohs(udp_entry->sport), ntohs(udp_entry->dport));
#else
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c58, "ip4_nat_udp_lookup_outgoing: i %d\n", i));
#endif
                ip4_nat_dbg_dump_udp_nat_entry("ip4_nat_udp_lookup_outgoing: found existing nat entry: ",
                                               nat_entry.udp);

#ifdef CONFIG_FIBOCOM_BASE
                udp_entry->common.ts = osiUpTime();
#endif
                break;
            }
#ifdef CONFIG_FIBOCOM_BASE
            nat_entry_count++;
#endif
        }
    }

    if (nat_entry.cmn == NULL)
    {
        if (allocate)
        {
#ifdef CONFIG_FIBOCOM_BASE
            if(nat_entry_count >= LWIP_NAT_DEFAULT_UDP_PORT_COUNT)
            {
                nat_entry.udp = oldest_entry;
                OSI_PRINTFI("ip4_nat_udp_lookup_outgoing: use oldest_entry ");
            }
            else
            {
                last_free = fibo_get_nat_entry(E_NAT_UDP);
                if(last_free < 0)
                {
                    OSI_PRINTFI("ip4_nat_udp_lookup_outgoing: cannot get available index ");
                    return NULL;
                }
#endif
            udp_entry = (struct ip4_nat_entries_udp *)calloc(1, sizeof(struct ip4_nat_entries_udp));
            if (NULL == udp_entry)
            {
                LWIP_DEBUGF(NAT_DEBUG, (0x10009806, "ip4_nat_out: no more memery for UDP\n"));
                return NULL;
            }
            LWIP_DEBUGF(NAT_DEBUG, (0x10009807, "ip4_nat_out udp calloc: %p\n", udp_entry));
            SLIST_INSERT_HEAD(ip4_nat_entries_udp_header, udp_entry, iter);

            nat_entry.udp = udp_entry;
#ifdef CONFIG_FIBOCOM_BASE
            }
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
            OSI_PRINTFE("ip4_nat_udp_lookup_outgoing fibo_get_portmode_flag is %d", fibo_get_portmode_flag());
            if(fibo_get_portmode_flag())
            {
                OSI_PRINTFE("ip4_nat_udp_lookup_outgoing source port use udphdr->src");
                nat_entry.udp->nport = udphdr->src;
            }
            else
            {
                OSI_PRINTFE("ip4_nat_udp_lookup_outgoing modify source port");
#endif
            //MTC0588-307,Only modify the nport when creating, and after exceeding 1024, overwrite the routing table without modifying the nport
            if(0 == nat_entry.udp->nport)
            {
                OSI_PRINTFE("ip4_nat_udp_lookup_outgoing nport is 0 and need to set");
                nat_entry.udp->nport = htons((u16_t) (LWIP_NAT_DEFAULT_UDP_SOURCE_PORT + nat_config->entry.port_index * LWIP_NAT_DEFAULT_UDP_PORT_COUNT + last_free));
                fibo_set_nat_entry(E_NAT_UDP, last_free, nat_entry.udp->nport);
            }
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
            }
#endif
#else
            nat_entry.udp->nport = htons(ip4_nat_udp_port_alloc(nat_config, i));
#endif
            nat_entry.udp->sport = udphdr->src;
            nat_entry.udp->dport = udphdr->dest;
            nat_entry.udp->common.ttl = sNAT_UDP_TTL;
            ip4_nat_cmn_init(nat_config, iphdr, nat_entry.cmn);

            ip4_nat_dbg_dump_udp_nat_entry("ip4_nat_udp_lookup_outgoing: created new nat entry: ",
                                           nat_entry.udp);
#ifdef CONFIG_FIBOCOM_BASE
          OSI_PRINTFI("ip4_nat_udp_lookup_outgoing: create udp table  %d",ntohs(nat_entry.udp->nport));
#endif
        }
    }
    return nat_entry.udp;
}

/**
 * This function checks for incoming packets if we already have a NAT entry.
 * If yes a pointer to the NAT entry is returned. Otherwise NULL.
 *
 * @param nat_config NAT configuration.
 * @param iphdr The IP header.
 * @param tcphdr The TCP header.
 * @return A pointer to an existing NAT entry or NULL if none is found.
 */
static ip4_nat_entries_tcp_t *
ip4_nat_tcp_lookup_incoming(const struct ip_hdr *iphdr, const struct tcp_hdr *tcphdr)
{
#ifdef CONFIG_FIBOCOM_BASE
    int nat_entry_count = 0;
#else
    int i = 0;
#endif
    ip4_nat_entries_tcp_t *nat_entry = NULL;

#ifdef CONFIG_FIBOCOM_BASE
  struct ip4_nat_entries_tcp* tcp_entry = NULL;
#else
    struct ip4_nat_entries_tcp *tcp_entry;
#endif
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
    /*
    oldest_entry: The oldest nat_entry of all nat_entry, use it if all nat_entry are established
    oldest_noest_entry: The oldest nat_entry of all not established nat_entry
    */
    struct ip4_nat_entries_tcp*oldest_entry = NULL;
    struct ip4_nat_entries_tcp*oldest_noest_entry = NULL;
    int oldest_entry_num = 0;
    int oldest_noest_entry_num = 0;
    int last_free = 0;
#endif

    SLIST_FOREACH(tcp_entry, ip4_nat_entries_tcp_header, iter)
    {
#ifdef CONFIG_FIBOCOM_DNAT_ENABLE
        if(tcp_entry->common.state != ESTABLISHED)
        {
            if(oldest_noest_entry == NULL)
            {
                oldest_noest_entry = tcp_entry;
                oldest_noest_entry_num = nat_entry_count;
            }
            if(tcp_entry->common.ts < oldest_noest_entry->common.ts)
            {
                oldest_noest_entry = tcp_entry;
                oldest_noest_entry_num = nat_entry_count;
            }
        }
        if(oldest_entry == NULL)
        {
            oldest_entry = tcp_entry;
            oldest_entry_num = nat_entry_count;
        }
        if(tcp_entry->common.ts < oldest_entry->common.ts)
        {
            oldest_entry = tcp_entry;
            oldest_entry_num = nat_entry_count;
        }
#endif
        if (tcp_entry->common.ttl)
        {
            if (((0 == tcp_entry->common.dest.addr) || (iphdr->src.addr == tcp_entry->common.dest.addr)) &&
                ((0 == tcp_entry->dport) || (tcphdr->src == tcp_entry->dport)) &&
                (tcphdr->dest == tcp_entry->nport))
            {
                nat_entry = tcp_entry;
#ifdef CONFIG_FIBOCOM_BASE
        LWIP_DEBUGF(NAT_DEBUG, (0x10007c5a, "ip4_nat_tcp_lookup_incoming: i %d\n", nat_entry_count));
#else
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c5a, "ip4_nat_tcp_lookup_incoming: i %d\n", i));
#endif
                ip4_nat_dbg_dump_tcp_nat_entry("ip4_nat_tcp_lookup_incoming: found existing nat entry: ",
                                               nat_entry);

#ifdef CONFIG_FIBOCOM_BASE
                tcp_entry->common.ts = osiUpTime();
#endif
                break;
            }
#ifdef CONFIG_FIBOCOM_BASE
            nat_entry_count++;
#endif
        }
    }

#if defined CONFIG_FIBOCOM_DNAT_ENABLE
    //add TCP of DNAT,Add the routing table for server to module transmission
    //Function ip4_nat_out is working module to server(upstream),and Function ip4_nat_input is working for server to module(downstream)
    if (nat_entry == NULL)
    {
        ip4_nat_conf_t *nat_config = ip4_nat_shallnat_for_inputpkt(iphdr);
        if (nat_config != NULL)
        {
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming: nat_config != NULL");
            if(nat_entry_count >= LWIP_NAT_DEFAULT_TCP_PORT_COUNT)
            {
                if(oldest_noest_entry == NULL)
                {
                    nat_entry = oldest_entry;
                    last_free = oldest_entry_num;
                    OSI_PRINTFI("ip4_nat_tcp_lookup_incoming: all natentry are established, and have to use oldest_entry");
                }
                else
                {
                    nat_entry = oldest_noest_entry;
                    last_free = oldest_noest_entry_num;
                    OSI_PRINTFI("ip4_nat_tcp_lookup_incoming: use oldest_noestablish");
                }
            }
            else
            {
                last_free = nat_entry_count;
                tcp_entry = (struct ip4_nat_entries_tcp*)calloc(1, sizeof(struct ip4_nat_entries_tcp));
                if (NULL == tcp_entry)
                {
                    LWIP_DEBUGF(NAT_DEBUG, (0x10009808, "ip4_nat_input: no more memery for TCP\n"));
                    return NULL;
                }
                LWIP_DEBUGF(NAT_DEBUG, (0x10009809, "ip4_nat_input tcp calloc: %p\n", tcp_entry));
                SLIST_INSERT_HEAD(ip4_nat_entries_tcp_header, tcp_entry, iter);
                nat_entry = tcp_entry;
            }

            nat_entry->nport = tcphdr->dest;//htons((u16_t) (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT + last_free));
            nat_entry->sport = tcphdr->dest;
            nat_entry->dport = tcphdr->src;
            nat_entry->common.cfg= nat_config;
            ip4_addr_set(&nat_entry->common.dest, &iphdr->src);
            ip4_addr_set(&nat_entry->common.source, &nat_config->entry.source_net);
            nat_entry->common.ttl = LWIP_NAT_DEFAULT_TTL_SECONDS;

            ip4_nat_dbg_dump_tcp_nat_entry("ip4_nat_tcp_lookup_incoming: created new nat entry: ",
                                      nat_entry);
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming nat_entry source is 0x%lx",nat_entry->common.source);
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming nat_entry dest is 0x%lx",nat_entry->common.dest);
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming nat_entry nport is %d",htons(nat_entry->nport));
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming nat_entry dport is %d",htons(nat_entry->dport));
            OSI_PRINTFI("ip4_nat_tcp_lookup_incoming create tcp table,%d",last_free);

        }
        else
        {
            LWIP_DEBUGF(NAT_DEBUG, (0x0, "ip4_nat_tcp_lookup_incoming: no more NAT entries available\n"));
        }

    }
#endif

    return nat_entry;
}

/**
 * This function checks if we already have a NAT entry for this TCP connection.
 * If yes the a pointer to this NAT entry is returned.
 *
 * @param iphdr The IP header.
 * @param tcphdr The TCP header.
 * @param allocate If no existing NAT entry is found and this flag is true
 *   a NAT entry is allocated.
 */
static ip4_nat_entries_tcp_t *
ip4_nat_tcp_lookup_outgoing(ip4_nat_conf_t *nat_config, const struct ip_hdr *iphdr,
                            const struct tcp_hdr *tcphdr, u8_t allocate)
{
#ifdef CONFIG_FIBOCOM_BASE
    int nat_entry_count = 0;
#else
    int i = 0;
#endif
    nat_entry_t nat_entry;

    nat_entry.cmn = NULL;

#ifdef CONFIG_FIBOCOM_BASE
    struct ip4_nat_entries_tcp* tcp_entry = NULL;
      /*
      oldest_entry: The oldest nat_entry of all nat_entry, use it if all nat_entry are established
      oldest_noest_entry: The oldest nat_entry of all not established nat_entry
    */
    struct ip4_nat_entries_tcp*oldest_entry = NULL;
    struct ip4_nat_entries_tcp*oldest_noest_entry = NULL;
    int last_free = 0;
#else
    struct ip4_nat_entries_tcp *tcp_entry;
#endif

    SLIST_FOREACH(tcp_entry, ip4_nat_entries_tcp_header, iter)
    {
#ifdef CONFIG_FIBOCOM_BASE
        if(tcp_entry->common.state != ESTABLISHED)
        {
            if(oldest_noest_entry == NULL)
            {
                oldest_noest_entry = tcp_entry;
            }
            if(tcp_entry->common.ts < oldest_noest_entry->common.ts)
            {
                oldest_noest_entry = tcp_entry;
            }
        }
        if(oldest_entry == NULL)
        {
            oldest_entry = tcp_entry;
        }
        if(tcp_entry->common.ts < oldest_entry->common.ts)
        {
            oldest_entry = tcp_entry;
        }
#else
        i++;
#endif
        if (tcp_entry->common.ttl)
        {
            if ((iphdr->src.addr == tcp_entry->common.source.addr) &&
                (tcphdr->src == tcp_entry->sport) &&
                ((0 == tcp_entry->dport) ||
                ((iphdr->dest.addr == tcp_entry->common.dest.addr) &&
                (tcphdr->dest == tcp_entry->dport))))
            {
                nat_entry.tcp = tcp_entry;
#ifdef CONFIG_FIBOCOM_BASE
                OSI_PRINTFI("ip4_nat_tcp_lookup_outgoing: nat_entry_count %d, noprt %d, soprt %d, doprt %d",
                nat_entry_count, ntohs(tcp_entry->nport), ntohs(tcp_entry->sport), ntohs(tcp_entry->dport));
#else
                LWIP_DEBUGF(NAT_DEBUG, (0x10007c5b, "ip4_nat_tcp_lookup_outgoing: i %d\n", i));
#endif
                ip4_nat_dbg_dump_tcp_nat_entry("ip4_nat_tcp_lookup_outgoing: found existing nat entry: ",
                                               nat_entry.tcp);

#ifdef CONFIG_FIBOCOM_BASE
                tcp_entry->common.ts = osiUpTime();
#endif
                break;
            }
#ifdef CONFIG_FIBOCOM_BASE
            nat_entry_count++;
#endif
        }
    }

    if (nat_entry.cmn == NULL)
    {
        if (allocate)
        {
#ifdef CONFIG_FIBOCOM_BASE
            if(nat_entry_count >= LWIP_NAT_DEFAULT_TCP_PORT_COUNT)
            {
                if(oldest_noest_entry == NULL)
                {
                    nat_entry.tcp = oldest_entry;
                    OSI_PRINTFI("ip4_nat_tcp_lookup_outgoing: all natentry are established, and have to use oldest_entry");
                }
                else
                {
                    nat_entry.tcp = oldest_noest_entry;
                    OSI_PRINTFI("ip4_nat_tcp_lookup_outgoing: use oldest_noestablish ");
                }
            }
            else
            {
            last_free = fibo_get_nat_entry(E_NAT_TCP);
            if(last_free < 0)
            {
                OSI_PRINTFI("ip4_nat_tcp_lookup_outgoing: cannot get available index ");
                return NULL;
            }
#endif
            tcp_entry = (struct ip4_nat_entries_tcp *)calloc(1, sizeof(struct ip4_nat_entries_tcp));
            if (NULL == tcp_entry)
            {
                LWIP_DEBUGF(NAT_DEBUG, (0x10009808, "ip4_nat_out: no more memery for TCP\n"));
                return NULL;
            }
            LWIP_DEBUGF(NAT_DEBUG, (0x10009809, "ip4_nat_out tcp calloc: %p\n", tcp_entry));
            SLIST_INSERT_HEAD(ip4_nat_entries_tcp_header, tcp_entry, iter);
            nat_entry.tcp = tcp_entry;
#ifdef CONFIG_FIBOCOM_BASE
            }
#endif
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
            OSI_PRINTFE("ip4_nat_tcp_lookup_outgoing fibo_get_portmode_flag is %d", fibo_get_portmode_flag());
            if(fibo_get_portmode_flag())
            {
                OSI_PRINTFE("ip4_nat_tcp_lookup_outgoing source port use tcphdr->src");
                nat_entry.tcp->nport = tcphdr->src;
            }
            else
            {
                OSI_PRINTFE("ip4_nat_tcp_lookup_outgoing modify source port");
#endif
#ifdef CONFIG_FIBOCOM_BASE
            //MTC0588-307,Only modify the nport when creating, and after exceeding 1024, overwrite the routing table without modifying the nport
            if(0 == nat_entry.tcp->nport)
            {
                OSI_PRINTFE("ip4_nat_tcp_lookup_outgoing nport is 0 and need to set");
                nat_entry.tcp->nport = htons((u16_t) (LWIP_NAT_DEFAULT_TCP_SOURCE_PORT + nat_config->entry.port_index * LWIP_NAT_DEFAULT_TCP_PORT_COUNT + last_free));
                fibo_set_nat_entry(E_NAT_TCP, last_free, nat_entry.tcp->nport);
            }
#else
            u16_t src_port = ntohs(tcphdr->src);
            nat_entry.tcp->nport = htons(ip4_nat_tcp_port_alloc(nat_config, src_port));
#endif

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
            }
#endif
            nat_entry.tcp->sport = tcphdr->src;
            nat_entry.tcp->dport = tcphdr->dest;
            nat_entry.tcp->common.ttl = sNAT_TCP_TTL;
            ip4_nat_cmn_init(nat_config, iphdr, nat_entry.cmn);

            ip4_nat_dbg_dump_tcp_nat_entry("ip4_nat_tcp_lookup_outgoing: created new nat entry: ",
                                           nat_entry.tcp);
#ifdef CONFIG_FIBOCOM_BASE
            OSI_PRINTFI("ip4_nat_tcp_lookup_outgoing create tcp table  %d",ntohs(nat_entry.tcp->nport));
#endif
        }
    }
    return nat_entry.tcp;
}

/** Adjusts the checksum of a NAT'ed packet without having to completely recalculate it
 * @todo: verify this works for little- and big-endian
 *
 * @param chksum points to the chksum in the packet
 * @param optr points to the old data in the packet
 * @param olen length of old data
 * @param nptr points to the new data in the packet
 * @param nlen length of new data
 */
static void
ip4_nat_chksum_adjust(u8_t *chksum, const u8_t *optr, s16_t olen, const u8_t *nptr, s16_t nlen)
{
    s32_t x = 0;
    s32_t oldval = 0;
    s32_t newval = 0;

    LWIP_ASSERT("NULL != chksum", NULL != chksum);
    LWIP_ASSERT("NULL != optr", NULL != optr);
    LWIP_ASSERT("NULL != nptr", NULL != nptr);
#if NAT_DEBUG
    LWIP_DEBUGF(NAT_DEBUG, (0x100096cb, "ip4_nat_chksum_adjust: chksum=%p, optr=%p, olen=%hu, nptr=%p, nlen=%hu\n",
                            chksum, optr, olen, nptr, nlen));
#endif
    x = chksum[0] * 256 + chksum[1];
    x = ~x & 0xFFFF;
    while (olen)
    {
        oldval = optr[0] * 256 + optr[1];
        optr += 2;
        x -= oldval & 0xffff;
        if (x <= 0)
        {
            x--;
            x &= 0xffff;
        }
        olen -= 2;
    }
    while (nlen)
    {
        newval = nptr[0] * 256 + nptr[1];
        nptr += 2;
        x += newval & 0xffff;
        if (x & 0x10000)
        {
            x++;
            x &= 0xffff;
        }
        nlen -= 2;
    }
    x = ~x & 0xFFFF;
    chksum[0] = x / 256;
    chksum[1] = x & 0xff;
#if NAT_DEBUG
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c5e, "ip4_nat_chksum_adjust: chksum = 0x%x\n", *((u16_t *)chksum)));
#endif
}

#if NAT_DEBUG
/**
 * This function dumps an IP address
 *
 * @param addr IP address
 */
static void
ip4_nat_dbg_dump_ip(const ip4_addr_t *addr)
{
    LWIP_ASSERT("NULL != addr", NULL != addr);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c5f, "%hu.%hu.%hu.%hu",
                            ip4_addr1(addr), ip4_addr2(addr), ip4_addr3(addr), ip4_addr4(addr)));
}

/**
 * This function dumps an IP header
 *
 * @param msg a message to print
 * @param iphdr IP header
 */
static void
ip4_nat_dbg_dump(const char *msg, const struct ip_hdr *iphdr)
{
    LWIP_ASSERT("NULL != msg", NULL != msg);
    LWIP_ASSERT("NULL != iphdr", NULL != iphdr);
    OSI_LOGXI(OSI_LOGPAR_S, 0x100096cc, "%s: IP: (", msg);
    ip4_addr_t src_ip4;
    ip4_addr_copy(src_ip4, iphdr->src);
    ip4_nat_dbg_dump_ip(&src_ip4);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_addr_t dest_ip4;
    ip4_addr_copy(dest_ip4, iphdr->dest);
    ip4_nat_dbg_dump_ip(&dest_ip4);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c61, " id=%hu, chksum=%hu)\n",
                            ntohs(IPH_ID(iphdr)), ntohs(IPH_CHKSUM(iphdr))));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c62, "PROTO %d", IPH_PROTO(iphdr)));
}

/**
 * This function dumps an ICMP echo reply/recho request nat entry.
 *
 * @param msg a message to print
 * @param nat_entry the ICMP NAT entry to print
 */
static void
ip4_nat_dbg_dump_icmp_nat_entry(const char *msg, const ip4_nat_entries_icmp_t *nat_entry)
{
    LWIP_ASSERT("NULL != msg", NULL != msg);
    LWIP_ASSERT("NULL != nat_entry", NULL != nat_entry);
    LWIP_ASSERT("NULL != nat_entry->common.cfg", NULL != nat_entry->common.cfg);
    LWIP_ASSERT("NULL != nat_entry->common.cfg->entry.out_if",
                NULL != nat_entry->common.cfg->entry.out_if);
    OSI_LOGXI(OSI_LOGPAR_S, 0x0800015f, "%s", msg);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c63, "ICMP : ("));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.source));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c64, " id=%hu", ntohs(nat_entry->id)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c65, ", seq=%hu", ntohs(nat_entry->seqno)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c66, ") mapped at ("));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(nat_entry->common.cfg->entry.out_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c64, " id=%hu", ntohs(nat_entry->id)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c65, ", seq=%hu", ntohs(nat_entry->seqno)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c4f, ")\n"));
}

/**
 * This function dumps an TCP nat entry.
 *
 * @param msg a message to print
 * @param nat_entry the TCP NAT entry to print
 */
static void
ip4_nat_dbg_dump_tcp_nat_entry(const char *msg, const ip4_nat_entries_tcp_t *nat_entry)
{
    LWIP_ASSERT("NULL != msg", NULL != msg);
    LWIP_ASSERT("NULL != nat_entry", NULL != nat_entry);
    LWIP_ASSERT("NULL != nat_entry->common.cfg", NULL != nat_entry->common.cfg);
    LWIP_ASSERT("NULL != nat_entry->common.cfg->entry.out_if",
                NULL != nat_entry->common.cfg->entry.out_if);
    OSI_LOGXI(OSI_LOGPAR_S, 0x0800015f, "%s", msg);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c67, "TCP : ("));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.source));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->sport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->dport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c66, ") mapped at ("));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(nat_entry->common.cfg->entry.out_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->nport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->dport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c4f, ")\n"));
}

/**
 * This function dumps a UDP NAT entry.
 *
 * @param msg a message to print
 * @param nat_entry the UDP NAT entry to print
 */
static void
ip4_nat_dbg_dump_udp_nat_entry(const char *msg, const ip4_nat_entries_udp_t *nat_entry)
{
    LWIP_ASSERT("NULL != msg", NULL != msg);
    LWIP_ASSERT("NULL != nat_entry", NULL != nat_entry);
    LWIP_ASSERT("NULL != nat_entry->common.cfg", NULL != nat_entry->common.cfg);
    LWIP_ASSERT("NULL != nat_entry->common.cfg->entry.out_if",
                NULL != nat_entry->common.cfg->entry.out_if);
    OSI_LOGXI(OSI_LOGPAR_S, 0x0800015f, "%s", msg);
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c69, "UDP : ("));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.source));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->sport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->dport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c66, ") mapped at ("));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(nat_entry->common.cfg->entry.out_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->nport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(nat_entry->common.dest));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c68, ":%hu", ntohs(nat_entry->dport)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c4f, ")\n"));
}

/** Prints some info when creating a new NAT entry */
static void
ip4_nat_dbg_dump_init(ip4_nat_conf_t *ip4_nat_cfg_new)
{
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6a, "ip4_nat_init: added new NAT interface\n"));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6b, "ip4_nat_init:   "));
    ip4_nat_dbg_dump_ip(&(ip4_nat_cfg_new->entry.source_net));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6c, "/"));
    ip4_nat_dbg_dump_ip(&(ip4_nat_cfg_new->entry.source_netmask));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6d, "@"));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(ip4_nat_cfg_new->entry.in_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(ip4_nat_cfg_new->entry.dest_net));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6c, "/"));
    ip4_nat_dbg_dump_ip(&(ip4_nat_cfg_new->entry.dest_netmask));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6d, "@"));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(ip4_nat_cfg_new->entry.out_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x08000161, "\n"));
}

/** Prints some info when removing a NAT entry */
static void
ip4_nat_dbg_dump_remove(ip4_nat_conf_t *cur)
{
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6e, "ip4_nat_remove: removing existing NAT interface\n"));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6f, "ip4_nat_remove:   "));
    ip4_nat_dbg_dump_ip(&(cur->entry.source_net));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6c, "/"));
    ip4_nat_dbg_dump_ip(&(cur->entry.source_netmask));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6d, "@"));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(cur->entry.in_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c60, " --> "));
    ip4_nat_dbg_dump_ip(&(cur->entry.dest_net));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6c, "/"));
    ip4_nat_dbg_dump_ip(&(cur->entry.dest_netmask));
    LWIP_DEBUGF(NAT_DEBUG, (0x10007c6d, "@"));
    ip4_nat_dbg_dump_ip(ip_2_ip4(&(cur->entry.out_if->ip_addr)));
    LWIP_DEBUGF(NAT_DEBUG, (0x08000161, "\n"));
}
#endif /* NAT_DEBUG */

#endif /* IP_NAT */
