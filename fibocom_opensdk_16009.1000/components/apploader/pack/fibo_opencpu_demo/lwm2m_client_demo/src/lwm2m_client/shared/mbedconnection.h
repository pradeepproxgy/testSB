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
#ifndef __MBEDCONNECTION_H__
#define __MBEDCONNECTION_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "liblwm2m.h"

#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/timing.h"

#define LWM2M_STANDARD_PORT_STR "5683" // Standard port for LWM2M in string format
#define LWM2M_STANDARD_PORT      5683 // Standard port for LWM2M in integer format
#define LWM2M_DTLS_PORT_STR     "5684" // DTLS port for LWM2M in string format
#define LWM2M_DTLS_PORT          5684 // DTLS port for LWM2M in integer format
#define LWM2M_BSSERVER_PORT_STR "5685" // Bootstrap server port for LWM2M in string format
#define LWM2M_BSSERVER_PORT      5685 // Bootstrap server port for LWM2M in integer format

// After 40 seconds of inactivity, a rehandshake is initiated
#define DTLS_NAT_TIMEOUT 40

typedef struct _dtls_connection_t
{
    struct _dtls_connection_t *  next; // Pointer to the next connection in the list
    int                     sock; // Socket descriptor
    struct sockaddr_in     addr; // IPv4 address
    struct sockaddr_in6     addr6; // IPv6 address
    size_t                  addrLen; // Length of the address
    lwm2m_object_t * securityObj; // Security object
    int securityInstId; // Security instance ID
    lwm2m_context_t * lwm2mH; // LWM2M context
    time_t lastSend; // Last time data was sent to the server (used for NAT timeouts)
    mbedtls_ssl_context* ssl; // SSL context
    mbedtls_net_context* server_fd; // Server file descriptor
    mbedtls_entropy_context* entropy; // Entropy context
    mbedtls_ctr_drbg_context* ctr_drbg; // CTR DRBG context
    mbedtls_ssl_config* conf; // SSL configuration
    mbedtls_x509_crt* cacert; // CA certificate
    mbedtls_x509_crt* clicert; // Client certificate
    mbedtls_pk_context* clikey; // Client key
    int sock_id; // Socket ID
    int issecure; // Flag indicating if the connection is secure
    mbedtls_ssl_session saved_session; // Saved SSL session
    int send_firstData; // Flag indicating if the first data has been sent
} dtls_connection_t;

/**
 * Creates a socket with the specified port and address family.
 * @param portStr The port number in string format.
 * @param ai_family The address family (e.g., AF_INET for IPv4).
 * @return The socket descriptor on success, or -1 on error.
 */
int create_socket(const char * portStr, int ai_family);

/**
 * Finds an existing connection in the list based on the provided address and its length.
 * @param connList The head of the connection list.
 * @param addr The address to match.
 * @param addrLen The length of the address.
 * @return A pointer to the matching connection, or NULL if no match is found.
 */
dtls_connection_t * connection_find(dtls_connection_t * connList, const struct sockaddr_storage * addr, size_t addrLen);

/**
 * Creates a new incoming connection and adds it to the connection list.
 * @param connList The head of the connection list.
 * @param sock The socket descriptor for the new connection.
 * @param addr The address of the peer.
 * @param addrLen The length of the address.
 * @return A pointer to the newly created connection.
 */
dtls_connection_t * connection_new_incoming(dtls_connection_t * connList, int sock, const struct sockaddr * addr, size_t addrLen);

/**
 * Creates a new DTLS connection and adds it to the connection list.
 * 
 * @param connList Pointer to the list of DTLS connections.
 * @param sock Socket descriptor for the connection.
 * @param securityObj Pointer to the security object.
 * @param instanceId Instance ID for the connection.
 * @param lwm2mH Pointer to the LwM2M context.
 * @param addressFamily Address family (e.g., AF_INET).
 * @return Pointer to the created DTLS connection.
 */
dtls_connection_t * connection_create(dtls_connection_t * connList, int sock, lwm2m_object_t * securityObj, int instanceId, lwm2m_context_t * lwm2mH, int addressFamily);

/**
 * Frees a DTLS connection and removes it from the connection list.
 * 
 * @param connList Pointer to the list of DTLS connections.
 */
void connection_free(dtls_connection_t * connList);

/**
 * Sends data over a DTLS connection.
 * 
 * @param connP Pointer to the DTLS connection.
 * @param buffer Pointer to the buffer containing data to send.
 * @param length Length of the data in the buffer.
 * @return Number of bytes sent, or a negative error code.
 */
int connection_send(dtls_connection_t *connP, uint8_t * buffer, size_t length);

/**
 * Handles an incoming packet on a DTLS connection.
 * 
 * @param connP Pointer to the DTLS connection.
 * @param buffer Pointer to the buffer containing the received data.
 * @param length Length of the received data in the buffer.
 * @return Number of bytes processed, or a negative error code.
 */
int connection_handle_packet(dtls_connection_t *connP, uint8_t * buffer, size_t length);

//rehandshake a connection, useful when your NAT timed out and your client has a new IP/PORT
//int connection_rehandshake(dtls_connection_t *connP, bool sendCloseNotify);
#endif // __MBEDCONNECTION_H__
