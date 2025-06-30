/* Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
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

#include "cfw.h"
#include "osi_api.h"
#include "osi_log.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4.h"
#include "lwip/ip6_addr.h"
#include "lwip/prot/icmp6.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/nd6.h"
#include "lwip/inet_chksum.h"
#include "lwip/dns.h"
#include "drv_ether.h"
#include <machine/endian.h>
#if LWIP_IPV6
extern u8_t buffer_reply[200];
extern uint16_t Dhcp6_info_reply_dataprepare(uint8_t *pULDataBuf, uint8_t *pDLDataBuf, struct netif *netif);
extern err_t ppp_netif_output_ip6(struct netif *netif, struct pbuf *pb, const ip6_addr_t *ipaddr);
bool isDhcp6infoReqPackage(struct pbuf *pb)
{
    OSI_LOGI(0x10007ad0, "isDhcp6infoReqPackage");
    if (pb->tot_len <= 0)
        return false;

    //bool isIPpackage = false;
    //bool isUDP = false;
    uint8_t *ipData = pb->payload;
    OSI_LOGI(0x10007ad1, "isDhcp6infoReqPackage 2");
    if (ipData[0] == 0x60) //V6
    {
        OSI_LOGI(0x10007ad2, "is ip v6");
    }
    else
        return false;

    if (ipData[6] == 0x11) //UDP
    {
        OSI_LOGI(0x10007558, "isDhcpPackage cookie isUDP");
        //isUDP = true;
    }
    OSI_LOGI(0x10007ad3, "isDhcp6infoReqPackage 3");
    uint8_t *udpData = ipData + 40;
    bool isUDPPortOK = false;
    if (udpData[0] == 0x02 && udpData[1] == 0x22 && udpData[2] == 0x02 && udpData[3] == 0x23) //port 546->547
    {
        isUDPPortOK = true;
        OSI_LOGI(0x1000755a, "isDhcpPackage isUDPPortOK");
    }
    OSI_LOGI(0x10007ad4, "isDhcp6infoReqPackage 4");
    if (!isUDPPortOK)
        return false;
    OSI_LOGI(0x10007ad5, "isDhcp6infoReqPackage 5");
    if (udpData[8] == 0x0b) // DHCP info request
        return true;
    return false;
}
void pppDhcp6_Info_req_reply(struct netif *netif, struct pbuf *pb)
{
    struct ip6_hdr *ip6hdr = NULL;

    uint16_t dataLen = 0;
    uint8_t *buf_reply = buffer_reply;

    dataLen = Dhcp6_info_reply_dataprepare(pb->payload, buf_reply, netif);
    if (dataLen == 0)
        return;

    struct pbuf *p = (struct pbuf *)pbuf_alloc(PBUF_RAW, dataLen, PBUF_POOL);
    if (p == NULL)
        return;

    memcpy(p->payload, buf_reply, dataLen);
    uint8_t *dhcp6_reply = p->payload;

    //udp checksum
    struct pbuf *pbuf, *q;
    int totalLen = dataLen - 40;
    uint8_t *pData = dhcp6_reply;
    int offset = 40;
    pbuf = (struct pbuf *)pbuf_alloc(PBUF_RAW, totalLen, PBUF_POOL);
    uint8_t *upd_hdr = (pData + 40);
    u16_t chksum = 0;

    if (pbuf != NULL)
    {
        if (totalLen > pbuf->len)
        {
            for (q = pbuf; totalLen > q->len; q = q->next)
            {
                memcpy(q->payload, &(pData[offset]), q->len);
                totalLen -= q->len;
                offset += q->len;
            }
            if (totalLen != 0)
            {
                memcpy(q->payload, &(pData[offset]), totalLen);
            }
        }
        else
        {
            memcpy(pbuf->payload, &(pData[offset]), totalLen);
        }
        ip6hdr = p->payload;
        ip6_addr_t src_ip = {0};
        ip6_addr_copy_from_packed(src_ip, ip6hdr->src);
        ip6_addr_t dst_ip = {0};
        ip6_addr_copy_from_packed(dst_ip, ip6hdr->dest);

        chksum = ip6_chksum_pseudo(pbuf, IP6_NEXTH_UDP, pbuf->len, &src_ip, &dst_ip);
        memcpy(((u8_t *)upd_hdr) + 6, &chksum, sizeof(u16_t));
        OSI_LOGI(0x10007ad7, "udp reply chksum 0x%x", chksum);
        pbuf_free(pbuf);
    }

    ppp_netif_output_ip6(netif, p, NULL);
    pbuf_free(p);
    //sys_arch_dump(RS_reply, sizeof(router_advertisement));
    sys_arch_dump(dhcp6_reply, p->tot_len);
}
bool isRAPackage(struct pbuf *pb)
{
    bool isV6 = false;
    bool isICMP6 = false;

    if (pb->tot_len <= 0)
        return false;

    uint8_t *ipData = pb->payload;
    if (ipData[0] == 0x60) //V6
    {
        isV6 = true;
    }
    if (ipData[6] == 0x3a)
    {
        isICMP6 = true;
    }
    if (!isV6)
        return false;
    if (!isICMP6)
        return false;

    struct icmp6_hdr *icmp_pkg = (struct icmp6_hdr *)(ipData + 40);
    if (icmp_pkg->type == ICMP6_TYPE_RA)
    {
        OSI_LOGI(0x10007ad8, "RA package catched %d", icmp_pkg->type);
        return true;
    }
    return false;
}

void RA_reply(struct pbuf *pb)
{
    uint8_t *RS_reply = pb->payload;
    struct ip6_hdr *ip6_pkg_out = (struct ip6_hdr *)(RS_reply);
    struct pbuf *p, *q;
    int totalLen = pb->tot_len - 40;
    uint8_t *pData = RS_reply;
    int offset = 40;

    p = (struct pbuf *)pbuf_alloc(PBUF_RAW, totalLen, PBUF_POOL);
    if (p == NULL)
    {
        return;
    }
    struct ra_header *ra_hdr;
    ra_hdr = (struct ra_header *)(pData + 40);

    if (ra_hdr->flags == 0x40)
    {
        OSI_LOGI(0x1000950f, "RA package catched ra_hdr->flags = %d", ra_hdr->flags);
        pbuf_free(p);
        return;
    }

    ra_hdr->chksum = 0;
    ra_hdr->flags = 0x40;

    if (p != NULL)
    {
        if (totalLen > p->len)
        {
            for (q = p; totalLen > q->len; q = q->next)
            {
                memcpy(q->payload, &(pData[offset]), q->len);
                totalLen -= q->len;
                offset += q->len;
            }
            if (totalLen != 0)
            {
                memcpy(q->payload, &(pData[offset]), totalLen);
            }
        }
        else
        {
            memcpy(p->payload, &(pData[offset]), totalLen);
        }

        ip6_addr_t src_ip = {0};
        ip6_addr_copy_from_packed(src_ip, ip6_pkg_out->src);
        ip6_addr_t dst_ip = {0};
        ip6_addr_copy_from_packed(dst_ip, ip6_pkg_out->dest);

        ra_hdr->chksum = ip6_chksum_pseudo(p, IP6_NEXTH_ICMP6, p->len, &src_ip, &dst_ip);
        OSI_LOGI(0x100075a8, "RA chksum 0x%x", ra_hdr->chksum);
        pbuf_free(p);
    }
    sys_arch_dump(RS_reply, pb->tot_len);
}
#endif
