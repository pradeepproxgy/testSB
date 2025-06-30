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
 *
 *******************************************************************************/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "fibo_opencpu.h"
#include <stdio.h>
#include <unistd.h>
#include <liblwm2m.h>

#define LWM2M_STANDARD_PORT_STR "5683" // Standard port for LWM2M in string format.
#define LWM2M_STANDARD_PORT      5683 // Standard port for LWM2M in integer format.
#define LWM2M_DTLS_PORT_STR     "5684" // DTLS port for LWM2M in string format.
#define LWM2M_DTLS_PORT          5684 // DTLS port for LWM2M in integer format.
#define LWM2M_BSSERVER_PORT_STR "5685" // BSSERVER port for LWM2M in string format.
#define LWM2M_BSSERVER_PORT      5685 // BSSERVER port for LWM2M in integer format.

typedef struct _connection_t
{
    struct _connection_t *  next; // Pointer to the next connection in the list.
    int                     sock; // Socket descriptor.
    struct sockaddr_in6     addr; // Address of the connection.
    size_t                  addrLen; // Length of the address.
} connection_t;

/**
 * @brief Creates a socket with the specified port and address family.
 *
 * @param portStr The port number as a string.
 * @param ai_family The address family (e.g., AF_INET6).
 * @return Returns the socket descriptor on success, or -1 on error.
 */
int create_socket(const char * portStr, int ai_family);

/**
 * @brief Finds a connection in the list based on the address.
 *
 * @param connList The list of connections to search through.
 * @param addr The address to match against the connections.
 * @param addrLen The length of the address.
 * @return Returns a pointer to the matching connection, or NULL if not found.
 */
connection_t * connection_find(connection_t * connList, struct sockaddr_storage * addr, size_t addrLen);

/**
 * @brief Creates a new incoming connection.
 *
 * @param connList The list of connections to add the new connection to.
 * @param sock The socket descriptor for the new connection.
 * @param addr The address of the new connection.
 * @param addrLen The length of the address.
 * @return Returns a pointer to the new connection.
 */
connection_t * connection_new_incoming(connection_t * connList, int sock, struct sockaddr * addr, size_t addrLen);

/**
 * @brief Creates a new connection.
 *
 * @param connList The list of connections to add the new connection to.
 * @param sock The socket descriptor for the new connection.
 * @param host The hostname or IP address for the new connection.
 * @param port The port number for the new connection.
 * @param addressFamily The address family (e.g., AF_INET6).
 * @return Returns a pointer to the new connection.
 */
connection_t * connection_create(connection_t * connList, int sock, char * host, char * port, int addressFamily);

/**
 * @brief Frees the list of connections.
 *
 * @param connList The list of connections to free.
 */
void connection_free(connection_t * connList);

/**
 * @brief Sends data over a connection.
 *
 * @param connP The connection over which to send data.
 * @param buffer The buffer containing the data to send.
 * @param length The length of the data to send.
 * @return Returns the number of bytes sent on success, or -1 on error.
 */
int connection_send(connection_t *connP, uint8_t * buffer, size_t length);

#endif
