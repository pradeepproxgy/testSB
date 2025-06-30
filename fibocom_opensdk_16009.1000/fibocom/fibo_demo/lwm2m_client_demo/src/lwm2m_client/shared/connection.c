/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Pascal Rieux - Please refer to git log
 *
 *******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "connection.h"
#include "app_lwm2m.h"
#include "app_pdp.h"


static int get_port(struct sockaddr *x)
{
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   }
   else if (x->sa_family == AF_INET6) {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   }
   else {
       APP_LOG_INFO("non IPV4 or IPV6 address\n");
       return  -1;
   }
}

static int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y)
{
    int portX = get_port(x);
    int portY = get_port(y);

    // if the port is invalid of different
    if (portX == -1 || portX != portY)
    {
        return 0;
    }

    // IPV4?
    if (x->sa_family == AF_INET)
    {
        // is V4?
        if (y->sa_family == AF_INET)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == ((struct sockaddr_in *)y)->sin_addr.s_addr;
        } else {
            return 0;
        }
    }
    else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6) {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr, 16) == 0;
    }
    else {
        // unknown address type
        APP_LOG_INFO("non IPV4 or IPV6 address\n");
        return 0;
    }
}

int create_socket(const char * portStr, int addressFamily)
{
    int s = -1;
    int ret = 0;
    fibo_addrinfo_t *local_addr = NULL;
    char *localipv4 = NULL;
    char *localipv6 = NULL;
    int port_nr = atoi(portStr);
    if ((port_nr <= 0) || (port_nr > 0xffff)) {
        APP_LOG_INFO("port range error!");
        return -1;
    }

    ret = app_pdp_get_ipaddress(CID_DEFAULT, &localipv4, &localipv6);
    if (ret != 0)
    {
        APP_LOG_INFO("get cid %d local address fail", CID_DEFAULT);
        return -1;
    }

    if(addressFamily == AF_INET)
    {
        fibo_addrinfo_para localparas = {
            .host_name = localipv4,
            .host_port = 0,
            .ip_type = addressFamily,
            .dnsServerIp1 = NULL,
            .dnsServerIp2= NULL,
            .peer_addr = &local_addr,
            .timeout = 0
        };
        ret = fibo_get_addrinfo(&localparas);
        if (ret != 0){
            APP_LOG_INFO("get local addr(%s) fail %d", localipv4, ret);
            return -1;
        }
    }
    else if(addressFamily == AF_INET6)
    {
        fibo_addrinfo_para localparas = {
            .host_name = localipv6,
            .host_port = 0,
            .ip_type = addressFamily,
            .dnsServerIp1 = NULL,
            .dnsServerIp2= NULL,
            .peer_addr = &local_addr,
            .timeout = 0
        };
        ret = fibo_get_addrinfo(&localparas);
        if (ret != 0){
            APP_LOG_INFO("get local addr(%s) fail %d", localipv6, ret);
            return -1;
        }
    }
    else
    {
        return -1;
    }

    s = fibo_sock_socket(addressFamily, SOCK_DGRAM, IPPROTO_UDP);
    if (s >= 0)
    {
        APP_LOG_INFO("socket get success!");
        if(-1 == fibo_sock_bind_v2(s, (struct sockaddr *)local_addr->ai_addr, local_addr->ai_addrlen))
        {
            APP_LOG_INFO("bind error!");
            fibo_sock_close_v2(s);
            s = -1;
        }
    }

    if(local_addr != NULL)
    {
        fibo_sock_freeaddrinfo(local_addr);
    }
    return s;
}

connection_t * connection_find(connection_t * connList,
                               struct sockaddr_storage * addr,
                               size_t addrLen)
{
    connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {
        if (sockaddr_cmp((struct sockaddr *)(&connP->addr), (struct sockaddr *)addr))
        {
            return connP;
        }
        connP = connP->next;
    }

    return connP;
}



connection_t * connection_new_incoming(connection_t * connList,
                                       int sock,
                                       struct sockaddr * addr,
                                       size_t addrLen)
{
    connection_t * connP;

    connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP != NULL)
    {
        connP->sock = sock;
        memcpy(&(connP->addr), addr, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;
        APP_LOG_INFO("sock %x connP->sock %x", sock, connP->sock);
    }

    return connP;
}

connection_t * connection_create(connection_t * connList,
                                 int sock,
                                 char * host,
                                 char * port,
                                 int addressFamily)
{
    int ret = 0;
    int s;
    struct sockaddr *sa = NULL;
    socklen_t sl = 0;
    connection_t * connP = NULL;
    fibo_addrinfo_t *servinfo = NULL;
    fibo_addrinfo_t *p = NULL;
    uint16_t port_s = atoi(port);
    fibo_addrinfo_para paras = {host, port_s, addressFamily, NULL, NULL, &servinfo, 0};
    ret = fibo_get_addrinfo(&paras);
    if (ret != 0)
    {
        APP_LOG_INFO("get dns addr(%s) fail %d", host, ret);
        return NULL;
    }

    if(servinfo == NULL)
    {
        APP_LOG_INFO("servinfo is NULL!");
        return NULL;
    }

    s = -1;
    for (p = servinfo; p != NULL && s == -1; p = p->ai_next)
    {
        s = fibo_sock_socket(p->ai_family, SOCK_DGRAM, IPPROTO_UDP);
        if (s >= 0)
        {
            sa = p->ai_addr;
            sl = p->ai_addrlen;
            ret = fibo_sock_connect_v2(s, p->ai_addr, p->ai_addrlen);
            if (ret != 0 || fibo_get_socket_error() == EINPROGRESS)
            {
                if (fibo_get_socket_error() != EINPROGRESS)
                {
                    APP_LOG_INFO("connect failed:%s,%d", strerror(fibo_get_socket_error()), fibo_get_socket_error());
                    fibo_sock_close_v2(s);
                    s = -1;
                }
            }
        }
    }
    if (s >= 0)
    {
        connP = connection_new_incoming(connList, sock, sa, sl);
        fibo_sock_close_v2(s);
    }
    if (NULL != servinfo) {
        fibo_sock_freeaddrinfo(servinfo);
    }

    return connP;
}

void connection_free(connection_t * connList)
{
    while (connList != NULL)
    {
        connection_t * nextP;

        nextP = connList->next;
        lwm2m_free(connList);

        connList = nextP;
    }
}

int connection_send(connection_t *connP,
                    uint8_t * buffer,
                    size_t length)
{
    int nbSent;
    size_t offset;

#ifdef LWM2M_WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connP->addr.sin6_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&connP->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    else if (AF_INET6 == connP->addr.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&connP->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    } else {
        return -1;
    }

    APP_LOG_INFO("Sending %zu bytes to [%s]:%hu\r\n", length, s, ntohs(port));

    // output_buffer(stderr, buffer, length, 0);
#endif

    offset = 0;
    while (offset != length)
    {
        nbSent = fibo_sock_sendto(connP->sock, buffer + offset, length - offset, 0, (struct sockaddr *)&(connP->addr), connP->addrLen);
        if (nbSent == -1) return -1;
        offset += nbSent;
    }
    return 0;
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    connection_t * connP = (connection_t*) sessionH;

    (void)userdata; /* unused */

    if (connP == NULL)
    {
        APP_LOG_INFO("#> failed sending %zu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        APP_LOG_INFO("#> failed sending %zu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    (void)userData; /* unused */

    return (session1 == session2);
}
