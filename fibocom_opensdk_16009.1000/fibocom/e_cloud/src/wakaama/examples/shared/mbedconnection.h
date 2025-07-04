/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Simon Bernard - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#ifndef DTLS_CONNECTION_H_
#define DTLS_CONNECTION_H_

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>

#include "liblwm2m.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/timing.h"
#include "at_engine.h"
#endif

#define LWM2M_STANDARD_PORT_STR "5683"
#define LWM2M_STANDARD_PORT      5683
#define LWM2M_DTLS_PORT_STR     "5684"
#define LWM2M_DTLS_PORT          5684
#define LWM2M_BSSERVER_PORT_STR "5685"
#define LWM2M_BSSERVER_PORT      5685

// after 40sec of inactivity we rehandshake
#define DTLS_NAT_TIMEOUT 40

typedef struct _dtls_connection_t
{
    struct _dtls_connection_t *  next;
    int                     sock;
     struct sockaddr_in     addr;
#ifdef LWIP_IPV6_ON
    struct sockaddr_in6     addr6;
#endif
    size_t                  addrLen;
    lwm2m_object_t * securityObj;
    int securityInstId;
    lwm2m_context_t * lwm2mH;
    time_t lastSend; // last time a data was sent to the server (used for NAT timeouts)
    mbedtls_ssl_context ssl;
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    int issecure;
} dtls_connection_t;

int ecloud_create_socket(const char *portStr, int ai_family,uint8_t nCid,uint8_t nSim);
dtls_connection_t * ecloud_connection_find(dtls_connection_t * connList, const struct sockaddr_storage * addr, size_t addrLen);
dtls_connection_t * ecloud_connection_new_incoming(dtls_connection_t * connList, int sock, const struct sockaddr * addr, size_t addrLen);
dtls_connection_t * ecloud_connection_create(dtls_connection_t * connList, int sock, lwm2m_object_t * securityObj, int instanceId, lwm2m_context_t * lwm2mH, int addressFamily);

void ecloud_connection_free(dtls_connection_t * connList);

int ecloud_connection_send(dtls_connection_t *connP, uint8_t * buffer, size_t length);
int ecloud_connection_handle_packet(dtls_connection_t *connP, uint8_t * buffer, size_t length);

//rehandshake a connection, useful when your NAT timed out and your client has a new IP/PORT
//int connection_rehandshake(dtls_connection_t *connP, bool sendCloseNotify);


