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

#include "lwip/opt.h"
#include "lwip/prot/dns.h"
#include "lwip/inet_chksum.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "dns_server.h"

#define MAX_DNS_CACHE_NUM 100
static int g_dnsCacheNum = 0;

/** DNS query message structure.
    No packing needed: only used locally on the stack. */
struct dns_query
{
    /* DNS query record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
    u16_t type;
    u16_t cls;
};
#define SIZEOF_DNS_QUERY 4

/** DNS answer message structure.
    No packing needed: only used locally on the stack. */
struct dns_answer
{
    /* DNS answer record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
    u16_t type;
    u16_t cls;
    u32_t ttl;
    u16_t len;
};
#define SIZEOF_DNS_ANSWER 10

typedef struct dnsServerParam
{
    /*user message*/
    ip_addr_t usrSrcAddr;
    ip_addr_t usrDstAddr;
    u16_t usrSrcPort;
    u16_t usrDstPort;

    /*request message*/
    u16_t tid;
    u8_t reqType;
    struct dns_hdr hdr;
    struct dns_query qry;
    char domain[DNS_MAX_NAME_LENGTH]; // 3www5baidu3com0

    /*server message*/
    ip_addr_t addr;
    int ttl;

    dns_server_found_callback callback;
    void *callbackArg;

} dnsServerParam_t;

/*  3www5baidu3com0 -> www.baidu.com*/
err_t prvDnsChangeDomain(char *src, char *dst)
{
    int n = 0;
    int m = 0;
    size_t len = strlen(src);

    if (len >= DNS_MAX_NAME_LENGTH || 0 == len)
        return ERR_ARG;

    while (*src != '\0')
    {
        n = *src++;
        if (n < 0 || n > len)
        {
            return ERR_ARG;
        }
        /** @see RFC 1035 - 4.1.4. Message compression */
        if ((n & 0xc0) == 0xc0)
        {
            /* Compressed name: cannot be equal since we don't send them */
            return ERR_ARG;
        }

        m += n;
        if (m > len)
            return ERR_ARG;

        strncat(dst, src, n);
        strcat(dst, ".");
        src += n;
    }
    len = strlen(dst);
    dst[len - 1] = 0;

    return ERR_OK;
}

/**
 * Walk through a compact encoded DNS name and return the end of the name.
 *
 * @param p pbuf containing the name
 * @param query_idx start index into p pointing to encoded DNS name in the DNS server response
 * @return index to end of the name
 */
static u16_t
dns_skip_name(struct pbuf *p, u16_t query_idx)
{
    int n;
    u16_t offset = query_idx;

    do
    {
        n = pbuf_try_get_at(p, offset++);
        if (n < 0)
        {
            return 0xFFFF;
        }
        /** @see RFC 1035 - 4.1.4. Message compression */
        if ((n & 0xc0) == 0xc0)
        {
            /* Compressed name: since we only want to skip it (not check it), stop here */
            break;
        }
        else
        {
            /* Not compressed name */
            if (offset + n >= p->tot_len)
            {
                return 0xFFFF;
            }
            offset = (u16_t)(offset + n);
        }
        n = pbuf_try_get_at(p, offset);
        if (n < 0)
        {
            return 0xFFFF;
        }
    } while (n != 0);

    return offset + 1;
}

static struct pbuf *prvBuildDnsErrorAnswer(dnsServerParam_t *arg, char type)
{
    struct dns_hdr hdr;
    if (arg == NULL)
        return NULL;

    uint16_t totalLen = SIZEOF_DNS_HDR + SIZEOF_DNS_QUERY + strlen(arg->domain) + 1;
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, totalLen, PBUF_RAM);
    if (!p)
    {
        OSI_LOGE(0x10009501, "prvBuildDnsErrorAnswer malloc error.\n");
        return NULL;
    }
    u16_t res_idx = 0;

    /* fill dns header */
    memset(&hdr, 0, SIZEOF_DNS_HDR);
    hdr.id = lwip_htons(arg->tid);   //2Byte
    hdr.flags1 = DNS_FLAG1_RESPONSE; //0x80    //1Byte
    hdr.flags2 = type;               //0x03    //1Byte
    hdr.numquestions = PP_HTONS(1);  //2Byte
    hdr.numanswers = PP_HTONS(0);    //2Byte
    hdr.numauthrr = PP_HTONS(0);     //2Byte
    hdr.numextrarr = PP_HTONS(0);    //2Byte
    pbuf_take(p, &hdr, SIZEOF_DNS_HDR);
    res_idx += SIZEOF_DNS_HDR;

    /* copy domain */
    pbuf_take_at(p, &(arg->domain), strlen(arg->domain) + 1, res_idx);
    res_idx += strlen(arg->domain) + 1;

    /* copy query */
    pbuf_take_at(p, &(arg->qry), SIZEOF_DNS_QUERY, res_idx);

    /* UDP header */
    pbuf_header(p, UDP_HLEN);
    struct udp_hdr *udphdr = (struct udp_hdr *)p->payload;
    udphdr->src = arg->usrDstPort;
    udphdr->dest = arg->usrSrcPort;
    udphdr->len = lwip_htons(p->tot_len);
    udphdr->chksum = 0;
    uint16_t udpchksum = ip_chksum_pseudo(p, IP_PROTO_UDP, p->tot_len, &(arg->usrDstAddr), &(arg->usrSrcAddr));
    if (udpchksum == 0x0000)
    {
        udpchksum = 0xffff;
    }
    udphdr->chksum = udpchksum;

    /*IP header*/
    pbuf_header(p, IP_HLEN);
    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    IPH_TTL_SET(iphdr, 255);
    IPH_PROTO_SET(iphdr, IP_PROTO_UDP);

    ip4_addr_copy(iphdr->dest, *ip_2_ip4(&(arg->usrSrcAddr)));
    ip4_addr_copy(iphdr->src, *ip_2_ip4(&(arg->usrDstAddr)));

    u16_t ip_hlen = IP_HLEN;
    IPH_VHL_SET(iphdr, 4, ip_hlen / 4);
    IPH_TOS_SET(iphdr, 0);
    IPH_LEN_SET(iphdr, lwip_htons(p->tot_len));
    IPH_OFFSET_SET(iphdr, 0);
    IPH_ID_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, ip_hlen));

    return p;
}

struct pbuf *prvBuildDnsAnswerPackage(dnsServerParam_t *arg)
{
    struct dns_hdr hdr;
    struct dns_answer ans;
    u16_t res_idx = 0;
    uint16_t domain_pos = 0;

    if (arg == NULL)
        return NULL;

    uint16_t ipaddrLen = 0;
    if (arg->reqType == IPADDR_TYPE_V4)
    {
        ipaddrLen = sizeof(ip4_addr_t);
    }
    else
    {
        ipaddrLen = 16;
    }
    uint16_t totalLen = SIZEOF_DNS_HDR + SIZEOF_DNS_QUERY + strlen(arg->domain) +
                        3 + SIZEOF_DNS_ANSWER + ipaddrLen;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, totalLen, PBUF_RAM);
    if (!p)
    {
        OSI_LOGE(0x10009502, "prvBuildDnsAnswerPackage malloc error.\n");
        return NULL;
    }

    /* fill dns header */
    memset(&hdr, 0, SIZEOF_DNS_HDR);
    hdr.id = lwip_htons(arg->tid); //2Byte
    if (arg->hdr.flags1 & DNS_FLAG1_RD)
    {
        hdr.flags1 = DNS_FLAG1_RD | DNS_FLAG1_RESPONSE;
        hdr.flags2 = DNS_FLAG2_RA | DNS_FLAG2_ERR_NONE;
    }
    else
    {
        hdr.flags1 = DNS_FLAG1_RESPONSE;
        hdr.flags2 = DNS_FLAG2_ERR_NONE; //1Byte
    }
    hdr.numquestions = PP_HTONS(1); //2Byte
    hdr.numanswers = PP_HTONS(1);   //2Byte
    hdr.numauthrr = PP_HTONS(0);    //2Byte
    hdr.numextrarr = PP_HTONS(0);   //2Byte
    pbuf_take(p, &hdr, SIZEOF_DNS_HDR);
    res_idx += SIZEOF_DNS_HDR;

    /* copy domain */
    pbuf_take_at(p, &(arg->domain), strlen(arg->domain) + 1, res_idx);
    res_idx += strlen(arg->domain) + 1;

    /* copy query */
    pbuf_take_at(p, &(arg->qry), SIZEOF_DNS_QUERY, res_idx);
    res_idx += SIZEOF_DNS_QUERY;

    /* fill answers */
    domain_pos = lwip_htons(0xC00C);
    pbuf_take_at(p, &domain_pos, 2, res_idx);
    res_idx += 2;

    memset(&ans, 0, sizeof(struct dns_answer));
    ans.cls = PP_HTONS(DNS_RRCLASS_IN);
    if (arg->ttl < 1)
        arg->ttl = 1;
    ans.ttl = PP_HTONL(arg->ttl);
    if (arg->reqType == IPADDR_TYPE_V4)
    {
        ans.len = PP_HTONS(sizeof(ip4_addr_t));
        ans.type = PP_HTONS(DNS_RRTYPE_A);
    }
    else
    {
        ans.len = PP_HTONS(16 /*sizeof(ip6_addr_t)*/);
        ans.type = PP_HTONS(DNS_RRTYPE_AAAA);
    }
    pbuf_take_at(p, &ans, SIZEOF_DNS_ANSWER, res_idx);
    res_idx += SIZEOF_DNS_ANSWER;

    /* copy ipaddr */
    if (arg->reqType == IPADDR_TYPE_V4)
        pbuf_take_at(p, &(arg->addr.u_addr.ip4), sizeof(ip4_addr_t), res_idx);
    else
        pbuf_take_at(p, &(arg->addr.u_addr.ip6), 16 /*sizeof(ip6_addr_t)*/, res_idx);

    /* UDP header */
    pbuf_header(p, UDP_HLEN);
    struct udp_hdr *udphdr = (struct udp_hdr *)p->payload;
    udphdr->src = arg->usrDstPort;
    udphdr->dest = arg->usrSrcPort;
    udphdr->len = lwip_htons(p->tot_len);
    udphdr->chksum = 0;
    uint16_t udpchksum = ip_chksum_pseudo(p, IP_PROTO_UDP, p->tot_len, &(arg->usrDstAddr), &(arg->usrSrcAddr));
    if (udpchksum == 0x0000)
    {
        udpchksum = 0xffff;
    }
    udphdr->chksum = udpchksum;

    /*IP header*/
    pbuf_header(p, IP_HLEN);
    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    IPH_TTL_SET(iphdr, 255);
    IPH_PROTO_SET(iphdr, IP_PROTO_UDP);
    ip4_addr_copy(iphdr->dest, *ip_2_ip4(&(arg->usrSrcAddr)));
    ip4_addr_copy(iphdr->src, *ip_2_ip4(&(arg->usrDstAddr)));

    u16_t ip_hlen = IP_HLEN;
    IPH_VHL_SET(iphdr, 4, ip_hlen / 4);
    IPH_TOS_SET(iphdr, 0);
    IPH_LEN_SET(iphdr, lwip_htons(p->tot_len));
    IPH_OFFSET_SET(iphdr, 0);
    IPH_ID_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, 0);
    IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, ip_hlen));

    return p;
}

/*  
    DNS PACKAGE
    2Byte         2Byte  
    TID           Flags
    Questions     Answer RRS
    Authority RRS Additional RRS   ___dns header
    Queries
    Answers
    Authority nameservers
    Additional records             ___RRs 


    Dns_hdr: Flags
    QR opcode  AA  TC  RD  RA  (zero) rcode
     1    4     1   1   1   1    3      4
 
    QR:     0 query 1 answer
    opcode: 0 stand query 1 reverse qeury 2 server state query
 
 */

dnsServerParam_t *prvParseDnsReqPackage(struct pbuf *p)
{
    u16_t txid;
    u16_t res_idx;
    struct dns_hdr hdr;
    struct dns_query qry;
    u16_t nquestions;
    dnsServerParam_t *arg = NULL;

    int len = 0;
    char host[DNS_MAX_NAME_LENGTH] = {0};

    if (p->tot_len < (SIZEOF_DNS_HDR + SIZEOF_DNS_QUERY))
    {
        LWIP_DEBUGF(DNS_DEBUG, (0x10007918, "dns_recv: pbuf too small\n"));
        goto memerr;
    }

    if (pbuf_copy_partial(p, &hdr, SIZEOF_DNS_HDR, 0) == SIZEOF_DNS_HDR)
    {
        txid = lwip_ntohs(hdr.id);
        nquestions = lwip_ntohs(hdr.numquestions);

        /* Check for correct query */
        if (DNS_FLAG1_RD != hdr.flags1)
        {
            LWIP_DEBUGF(DNS_DEBUG, (0x1000980e, "dns_parseRequst: not a standard request.\n"));
            goto memerr;
        }

        if (nquestions != 1)
        {
            LWIP_DEBUGF(DNS_DEBUG, (0x1000980f, "dns_parseRequst: response not match to query\n"));
            goto memerr;
        }

        res_idx = dns_skip_name(p, SIZEOF_DNS_HDR);
        if (res_idx == 0xFFFF)
        {
            LWIP_DEBUGF(DNS_DEBUG, (0x10009810, "dns_parseRequst: host name error\n"));
            goto memerr;
        }

        len = res_idx - SIZEOF_DNS_HDR - 1;
        if (pbuf_copy_partial(p, &host, len, SIZEOF_DNS_HDR) != len)
        {
            goto memerr;
        }

        /* check if "question" part matches the request */
        if (pbuf_copy_partial(p, &qry, SIZEOF_DNS_QUERY, res_idx) != SIZEOF_DNS_QUERY)
        {
            goto memerr; /* ignore this packet */
        }

        if ((qry.cls != PP_HTONS(DNS_RRCLASS_IN)) ||
            (qry.type != PP_HTONS(DNS_RRTYPE_AAAA) && qry.type != PP_HTONS(DNS_RRTYPE_A)))
        {
            LWIP_DEBUGF(DNS_DEBUG, (0x1000791a, "dns_recv: response not match to query\n"));
            goto memerr; /* ignore this packet */
        }

        // add to param
        arg = (dnsServerParam_t *)malloc(sizeof(dnsServerParam_t));
        if (NULL == arg)
        {
            goto memerr;
        }

        memset(arg, 0, sizeof(dnsServerParam_t));
        arg->tid = txid;
        memcpy(&(arg->hdr), &hdr, SIZEOF_DNS_HDR);
        memcpy(&(arg->qry), &qry, SIZEOF_DNS_QUERY);
        strcpy(arg->domain, host);
        if (qry.type == PP_HTONS(DNS_RRTYPE_AAAA))
            arg->reqType = LWIP_DNS_ADDRTYPE_IPV6;
        else
            arg->reqType = LWIP_DNS_ADDRTYPE_IPV4;

        return arg;
    }

memerr:
    /* deallocate memory and return */
    OSI_LOGE(0x10009503, "prvParseDnsReqPackage error.");
    return NULL;
}

void dnsServerCallback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    if (!callback_arg || !name)
        return;

    OSI_LOGXI(OSI_LOGPAR_SI, 0x10009811, "dnsServerCallback name:%s, ipaddr:%d\n", name, (NULL != ipaddr));

    struct pbuf *answer = NULL;
    const struct dns_table_entry *entry = NULL;
    dnsServerParam_t *arg = (dnsServerParam_t *)callback_arg;

    char domain[DNS_MAX_NAME_LENGTH] = {0};
    prvDnsChangeDomain(arg->domain, domain);

    if (NULL == ipaddr)
    {
        answer = prvBuildDnsErrorAnswer(arg, DNS_FLAG2_ERR_NAME);
    }
    else
    {
        entry = dns_get_table(domain, arg->reqType);
        if (NULL == entry)
        {
            OSI_LOGE(0x10009504, "dnsServerCallback get table error.");
            answer = prvBuildDnsErrorAnswer(arg, DNS_FLAG2_ERR_NAME);
        }
        else
        {
            arg->ttl = entry->ttl;
            ip_addr_copy(arg->addr, *ipaddr);
            answer = prvBuildDnsAnswerPackage(arg);
        }
    }

    if (answer)
    {
        if (arg->callback != NULL)
        {
            arg->callback(answer, arg->callbackArg);
        }
        pbuf_free(answer);
    }

    g_dnsCacheNum--;
    if (arg)
        free(arg);

    return;
}

err_t dnsServer_gethostbyname(struct pbuf *p, dns_server_found_callback callback, void *callbackarg)
{
    err_t iRet = 0;
    if (g_dnsCacheNum++ > MAX_DNS_CACHE_NUM)
    {
        OSI_LOGE(0x10009505, "dnsServer_gethostbyname overflow %d\n", g_dnsCacheNum);
        ///return ERR_CACHE_NUM;
    }

    dnsServerParam_t *arg = NULL;
    struct pbuf *pAnswer = NULL;
    char domain[DNS_MAX_NAME_LENGTH] = {0};

    struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
    uint16_t iphdr_hlen;
    /* obtain IP header length in number of 32-bit words */
    iphdr_hlen = IPH_HL(iphdr);
    /* calculate IP header length in bytes */
    iphdr_hlen *= 4;
    ip_addr_t srcAddr, dstAddr;
    ip_addr_copy_from_ip4(srcAddr, iphdr->src);
    ip_addr_copy_from_ip4(dstAddr, iphdr->dest);
    pbuf_header(p, -(int16_t)iphdr_hlen); //remove IP header

    struct udp_hdr *udphdr = (struct udp_hdr *)p->payload;
    uint16_t srcPort = udphdr->src;
    uint16_t dstPort = udphdr->dest;
    pbuf_header(p, -UDP_HLEN); //remove UDP header

    arg = prvParseDnsReqPackage(p);
    if (NULL == arg)
    {
        iRet = ERR_DNS_PACKAGE;
        goto fail;
    }
    arg->callback = callback;
    arg->callbackArg = callbackarg;
    memcpy(&(arg->usrSrcAddr), &srcAddr, sizeof(ip_addr_t));
    memcpy(&(arg->usrDstAddr), &dstAddr, sizeof(ip_addr_t));
    arg->usrSrcPort = srcPort;
    arg->usrDstPort = dstPort;
    iRet = prvDnsChangeDomain(arg->domain, domain);
    if (ERR_OK != iRet)
    {
        iRet = ERR_DOMAIN;
        goto fail;
    }
    OSI_LOGXI(OSI_LOGPAR_SI, 0x10009812, "dnsServer_gethostbyname  domain=%s reqType=%d", domain, arg->reqType);
    iRet = dns_gethostbyname_addrtype(domain, &(arg->addr), dnsServerCallback, (void *)arg, arg->reqType);
    if (ERR_LOCAL_OK == iRet)
    {
        //hit in cache, send answer back.
        dnsServerCallback(domain, &(arg->addr), arg);
    }
    else if (ERR_INPROGRESS != iRet)
    {
        //error
        goto fail;
    }

    //ERR_INPROGRESS, Wait for answer package.
    return iRet;

fail:
    OSI_LOGE(0x10009506, "dnsServer_gethostbyname goto fail ret=%d\n", iRet);
    pAnswer = prvBuildDnsErrorAnswer(arg, 0x01);
    callback(pAnswer, callbackarg);
    if (arg)
    {
        free(arg);
        g_dnsCacheNum--;
    }
    if (pAnswer)
    {
        pbuf_free(pAnswer);
    }

    return iRet;
}
