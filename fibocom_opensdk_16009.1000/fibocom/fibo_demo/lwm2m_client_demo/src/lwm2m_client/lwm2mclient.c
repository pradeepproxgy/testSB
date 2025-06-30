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
 *    Benjamin Cabé - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Axel Lorente - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Christian Renz - Please refer to git log
 *    Ricky Liu - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>
 Bosch Software Innovations GmbH - Please refer to git log

*/
#include "fibo_opencpu.h"
#include "lwm2mclient.h"
#include "liblwm2m.h"
#ifdef WITH_MBEDTLS
#include "mbedconnection.h"
#else
#include "connection.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PACKET_SIZE 2048
#define DEFAULT_SERVER_IPV6 "[::1]"
#define DEFAULT_SERVER_IPV4 "127.0.0.1"

int g_reboot = 0;
static int g_quit = 0;

#define OBJ_COUNT 4
lwm2m_object_t * objArray[OBJ_COUNT];

// only backup security and server objects
# define BACKUP_OBJECT_COUNT 2
lwm2m_object_t * backupObjectArray[BACKUP_OBJECT_COUNT];

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    int sock;
#ifdef WITH_MBEDTLS
    dtls_connection_t * connList;
    lwm2m_context_t * lwm2mH;
#else
    connection_t * connList;
#endif
    int addressFamily;
} client_data_t;

void handle_sigint(int signum)
{
    g_quit = 2;
}

void handle_value_changed(lwm2m_context_t * lwm2mH,
                          lwm2m_uri_t * uri,
                          const char * value,
                          size_t valueLength)
{
    lwm2m_object_t * object = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, uri->objectId);

    if (NULL != object)
    {
        if (object->writeFunc != NULL)
        {
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                APP_LOG_INFO("Internal allocation failure !\n");
                return;
            }
            dataP->id = uri->resourceId;

#ifndef LWM2M_VERSION_1_0
            if (LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uri))
            {
                lwm2m_data_t *subDataP = lwm2m_data_new(1);
                if (subDataP == NULL)
                {
                    APP_LOG_INFO("Internal allocation failure !\n");
                    lwm2m_data_free(1, dataP);
                    return;
                }
                subDataP->id = uri->resourceInstanceId;
                lwm2m_data_encode_nstring(value, valueLength, subDataP);
                lwm2m_data_encode_instances(subDataP, 1, dataP);
            }
            else
#endif
            {
                lwm2m_data_encode_nstring(value, valueLength, dataP);
            }

            result = object->writeFunc(lwm2mH, uri->instanceId, 1, dataP, object, LWM2M_WRITE_PARTIAL_UPDATE);
            if (COAP_405_METHOD_NOT_ALLOWED == result)
            {
                switch (uri->objectId)
                {
                case LWM2M_DEVICE_OBJECT_ID:
                    result = device_change(dataP, object);
                    break;
                default:
                    break;
                }
            }

            if (COAP_204_CHANGED != result)
            {
                APP_LOG_INFO("Failed to change value!\n");
            }
            else
            {
                APP_LOG_INFO("value changed!\n");
                lwm2m_resource_value_changed(lwm2mH, uri);
            }
            lwm2m_data_free(1, dataP);
            return;
        }
        else
        {
            APP_LOG_INFO("write not supported for specified resource!\n");
        }
        return;
    }
    else
    {
        APP_LOG_INFO("Object not found !\n");
    }
}

#ifdef WITH_MBEDTLS
void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
  client_data_t * dataP;
  lwm2m_list_t * instance;
  dtls_connection_t * newConnP = NULL;
  dataP = (client_data_t *)userData;
  lwm2m_object_t  * securityObj = dataP->securityObjP;

  instance = LWM2M_LIST_FIND(dataP->securityObjP->instanceList, secObjInstID);
  if (instance == NULL) return NULL;


  newConnP = connection_create(dataP->connList, dataP->sock, securityObj, instance->id, dataP->lwm2mH, dataP->addressFamily);
  if (newConnP == NULL)
  {
      APP_LOG_INFO("Connection creation failed.\n");
      return NULL;
  }

  dataP->connList = newConnP;
  return (void *)newConnP;
}
#else
void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
    client_data_t * dataP;
    char * uri;
    char * host;
    char * port;
    connection_t * newConnP = NULL;

    dataP = (client_data_t *)userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);

    if (uri == NULL) return NULL;

    // parse uri in the form "coaps://[host]:[port]"
    if (0==strncmp(uri, "coaps://", strlen("coaps://"))) {
        host = uri+strlen("coaps://");
    }
    else if (0==strncmp(uri, "coap://",  strlen("coap://"))) {
        host = uri+strlen("coap://");
    }
    else {
        goto exit;
    }
    port = strrchr(host, ':');
    if (port == NULL) goto exit;
    // remove brackets
    if (host[0] == '[')
    {
        host++;
        if (*(port - 1) == ']')
        {
            *(port - 1) = 0;
        }
        else goto exit;
    }
    // split strings
    *port = 0;
    port++;

    APP_LOG_INFO("Opening connection to server at %s:%s\r\n", host, port);
    newConnP = connection_create(dataP->connList, dataP->sock, host, port, dataP->addressFamily);
    if (newConnP == NULL) {
        APP_LOG_INFO("Connection creation failed.\r\n");
    }
    else {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *)newConnP;
}
#endif

void lwm2m_close_connection(void * sessionH,
                            void * userData)
{
    client_data_t * app_data;
#ifdef WITH_MBEDTLS
    dtls_connection_t * targetP;
#else
    connection_t * targetP;
#endif

    app_data = (client_data_t *)userData;
#ifdef WITH_MBEDTLS
    targetP = (dtls_connection_t *)sessionH;
#else
    targetP = (connection_t *)sessionH;
#endif

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
#ifdef WITH_MBEDTLS
        dtls_connection_t * parentP;
#else
        connection_t * parentP;
#endif

        parentP = app_data->connList;
        while (parentP != NULL && parentP->next != targetP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = targetP->next;
            lwm2m_free(targetP);
        }
    }
}

static void update_battery_level(lwm2m_context_t * context)
{
    static time_t next_change_time = 0;
    time_t tv_sec;

    tv_sec = lwm2m_gettime();
    if (tv_sec < 0) return;

    if (next_change_time < tv_sec)
    {
        char value[15];
        int valueLength;
        lwm2m_uri_t uri;
        int level = rand() % 100;

        if (0 > level) level = -level;
        if (lwm2m_stringToUri("/3/0/9", 6, &uri))
        {
            valueLength = sprintf(value, "%d", level);
            APP_LOG_INFO("New Battery Level: %d\n", level);
            handle_value_changed(context, &uri, value, valueLength);
        }
        level = rand() % 20;
        if (0 > level) level = -level;
        next_change_time = tv_sec + level + 10;
    }
}

#ifdef LWM2M_BOOTSTRAP
static void prv_backup_objects(lwm2m_context_t * context)
{
    uint16_t i;

    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
        backupObjectArray[i] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
        memset(backupObjectArray[i], 0, sizeof(lwm2m_object_t));
    }

    /*
     * Backup content of objects 0 (security) and 1 (server)
     */
    copy_security_object(backupObjectArray[0], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID));
    copy_server_object(backupObjectArray[1], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID));
}

static void prv_restore_objects(lwm2m_context_t * context)
{
    lwm2m_object_t * targetP;

    /*
     * Restore content  of objects 0 (security) and 1 (server)
     */
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID);
    // first delete internal content
    clean_security_object(targetP);
    // then restore previous object
    copy_security_object(targetP, backupObjectArray[0]);

    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID);
    // first delete internal content
    clean_server_object(targetP);
    // then restore previous object
    copy_server_object(targetP, backupObjectArray[1]);

    // restart the old servers
    APP_LOG_INFO("[BOOTSTRAP] ObjectList restored\r\n");
}

static void update_bootstrap_info(lwm2m_client_state_t * previousBootstrapState,
        lwm2m_context_t * context)
{
    if (*previousBootstrapState != context->state)
    {
        *previousBootstrapState = context->state;
        switch(context->state)
        {
            case STATE_BOOTSTRAPPING:
#ifdef LWM2M_WITH_LOGS
                APP_LOG_INFO("[BOOTSTRAP] backup security and server objects\r\n");
#endif
                prv_backup_objects(context);
                break;
            default:
                break;
        }
    }
}

static void close_backup_object()
{
    int i;
    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
    }
}
#endif

#ifdef WITH_MBEDTLS
static int stoupper(int ch) {
    if (ch >= 'a' && ch <= 'z') {
        return ch - ('a' - 'A');
    }
    return ch;
}
#endif

// #define LWM2M_SERVER_URL "111.231.250.105"
#define LWM2M_SERVER_URL "leshan.eclipseprojects.io"
#define LWM2M_SERVER_PORT 5683
// #define LWM2M_PSK "e646ae511ece77aa9118ec118b541abe3aae5774a9ee3e8f9bc35418709ce993"
// #define LWM2M_PSK_ID "352273017386340"
#define LWM2M_PSK 0
#define LWM2M_PSK_ID 0
#define LWM2M_ADDRESS_FAMILY_DEFAULT AF_INET
#define LWM2M_LIFETIME_DEFAULT 60
#define LWM2M_ENDPOINT_NAME "352273017386340_LC"

int app_lwm2m_demo()
{
    client_data_t data;
    int result;
    lwm2m_context_t * lwm2mH = NULL;
    const char * localPort = "56830";
    const char * server = LWM2M_SERVER_URL;
    uint16_t serverPort = LWM2M_SERVER_PORT;
    char * name = LWM2M_ENDPOINT_NAME;
    int lifetime = LWM2M_LIFETIME_DEFAULT;
    int batterylevelchanging = 0;
    time_t reboot_time = 0;
    // int opt;
    bool bootstrapRequested = false;
    // bool serverPortChanged = false;

#ifdef LWM2M_BOOTSTRAP
    lwm2m_client_state_t previousState = STATE_INITIAL;
#endif

    char * pskId = LWM2M_PSK_ID;
#ifdef WITH_MBEDTLS
    char * psk = LWM2M_PSK;
#endif
    uint16_t pskLen = -1;
    char * pskBuffer = NULL;

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = LWM2M_ADDRESS_FAMILY_DEFAULT;

    /*
     *This call an internal function that create an IPV6 socket on the port 5683.
     */
    APP_LOG_INFO("Trying to bind LWM2M Client to port %s\r\n", localPort);
    data.sock = create_socket(localPort, data.addressFamily);
    if (data.sock < 0)
    {
        APP_LOG_INFO("Failed to open socket: %d %s\r\n", errno, strerror(fibo_get_socket_error()));
        return -1;
    }

    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
#ifdef WITH_MBEDTLS
    if (psk != NULL)
    {
        pskLen = strlen(psk) / 2;
        pskBuffer = malloc(pskLen);

        if (NULL == pskBuffer)
        {
            APP_LOG_INFO("Failed to create PSK binary buffer\r\n");
            return -1;
        }
        // Hex string to binary
        char *h = psk;
        char *b = pskBuffer;
        char xlate[] = "0123456789ABCDEF";

        for ( ; *h; h += 2, ++b)
        {
            char *l = strchr(xlate, stoupper(*h));
            char *r = strchr(xlate, stoupper(*(h+1)));

            if (!r || !l)
            {
                APP_LOG_INFO("Failed to parse Pre-Shared-Key HEXSTRING\r\n");
                return -1;
            }

            *b = ((l - xlate) << 4) + (r - xlate);
        }
    }
#endif

    char serverUri[50];
    int serverId = 123;
#ifdef WITH_MBEDTLS
    sprintf (serverUri, "coaps://%s:%d", server, serverPort);
#else
    sprintf (serverUri, "coap://%s:%d", server, serverPort);
#endif
#ifdef LWM2M_BOOTSTRAP
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, bootstrapRequested);
#else
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
#endif
    if (NULL == objArray[0])
    {
        APP_LOG_INFO("Failed to create security object\r\n");
        return -1;
    }
    data.securityObjP = objArray[0];

    objArray[1] = get_server_object(serverId, "U", lifetime, false);
    if (NULL == objArray[1])
    {
        APP_LOG_INFO("Failed to create server object\r\n");
        return -1;
    }

    objArray[2] = get_object_device();
    if (NULL == objArray[2])
    {
        APP_LOG_INFO("Failed to create Device object\r\n");
        return -1;
    }

    objArray[3] = get_object_firmware();
    if (NULL == objArray[3])
    {
        APP_LOG_INFO("Failed to create Firmware object\r\n");
        return -1;
    }

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        APP_LOG_INFO("lwm2m_init() failed\r\n");
        return -1;
    }
#ifdef WITH_MBEDTLS
    data.lwm2mH = lwm2mH;
#endif
    /*
     * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
     * the number of objects we will be passing through and the objects array
     */
    result = lwm2m_configure(lwm2mH, name, NULL, NULL, OBJ_COUNT, objArray);
    if (result != 0)
    {
        APP_LOG_INFO("lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }

    // signal(SIGINT, handle_sigint);

    /**
     * Initialize value changed callback.
     */
    init_value_change(lwm2mH);

    APP_LOG_INFO("LWM2M Client \"%s\" started on port %s\r\n", name, localPort);
    APP_LOG_INFO("> "); fflush(stdout);
    /*
     * We now enter in a while loop that will handle the communications from the server
     */
    while (0 == g_quit)
    {
        struct timeval tv;
        fd_set readfds;

        if (g_reboot)
        {
            time_t tv_sec;

            tv_sec = lwm2m_gettime();

            if (0 == reboot_time)
            {
                reboot_time = tv_sec + 5;
            }
            if (reboot_time < tv_sec)
            {
                /*
                 * Message should normally be lost with reboot ...
                 */
                APP_LOG_INFO("reboot time expired, rebooting ...");
                system_reboot();
            }
            else
            {
                tv.tv_sec = reboot_time - tv_sec;
            }
        }
        else if (batterylevelchanging)
        {
            update_battery_level(lwm2mH);
            tv.tv_sec = 5;
        }
        else
        {
            tv.tv_sec = 60;
        }
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(data.sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        /*
         * This function does two things:
         *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
         *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
         *    (eg. retransmission) and the time between the next operation
         */
        result = lwm2m_step(lwm2mH, &(tv.tv_sec));
        APP_LOG_INFO(" -> State: ");
        switch (lwm2mH->state)
        {
        case STATE_INITIAL:
            APP_LOG_INFO("STATE_INITIAL\r\n");
            break;
        case STATE_BOOTSTRAP_REQUIRED:
            APP_LOG_INFO("STATE_BOOTSTRAP_REQUIRED\r\n");
            break;
        case STATE_BOOTSTRAPPING:
            APP_LOG_INFO("STATE_BOOTSTRAPPING\r\n");
            break;
        case STATE_REGISTER_REQUIRED:
            APP_LOG_INFO("STATE_REGISTER_REQUIRED\r\n");
            break;
        case STATE_REGISTERING:
            APP_LOG_INFO("STATE_REGISTERING\r\n");
            break;
        case STATE_READY:
            APP_LOG_INFO("STATE_READY\r\n");
            break;
        default:
            APP_LOG_INFO("Unknown...\r\n");
            break;
        }
        if (result != 0)
        {
            APP_LOG_INFO("lwm2m_step() failed: 0x%X\r\n", result);
            if(previousState == STATE_BOOTSTRAPPING)
            {
#ifdef LWM2M_WITH_LOGS
                APP_LOG_INFO("[BOOTSTRAP] restore security and server objects\r\n");
#endif
                prv_restore_objects(lwm2mH);
                lwm2mH->state = STATE_INITIAL;
            }
            else return -1;
        }
#ifdef LWM2M_BOOTSTRAP
        update_bootstrap_info(&previousState, lwm2mH);
#endif
        /*
         * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
         * with the precedent function)
         */
        result = fibo_sock_select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

        if (result < 0)
        {
            if (errno != EINTR)
            {
              APP_LOG_INFO("Error in fibo_sock_select(): %d %s\r\n", fibo_get_socket_error(), strerror(fibo_get_socket_error()));
            }
        }
        else if (result > 0)
        {
            uint8_t buffer[MAX_PACKET_SIZE];
            ssize_t numBytes;

            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(data.sock, &readfds))
            {
                struct sockaddr_storage addr;
                uint32_t addrLen;

                addrLen = sizeof(addr);

                /*
                 * We retrieve the data received
                 */
                if (pskBuffer != NULL || pskId != NULL)
                {
                    numBytes = fibo_sock_recvfrom_v2(data.sock, buffer, MAX_PACKET_SIZE, MSG_PEEK, (struct sockaddr *)&addr, &addrLen);
                }
                else
                {
                    numBytes = fibo_sock_recvfrom_v2(data.sock, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);
                }

                if (0 > numBytes)
                {
                    APP_LOG_INFO("Error in recvfrom(): %d %s\r\n", fibo_get_socket_error(), strerror(fibo_get_socket_error()));
                }
                else if (numBytes >= MAX_PACKET_SIZE) 
                {
                    APP_LOG_INFO("Received packet >= MAX_PACKET_SIZE\r\n");
                } 
                else if (0 < numBytes)
                {
                    char s[INET6_ADDRSTRLEN];
                    in_port_t port = 0;

#ifdef WITH_MBEDTLS
                    dtls_connection_t * connP;
#else
                    connection_t * connP;
#endif
                    if (AF_INET == addr.ss_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == addr.ss_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin6_port;
                    }
                    APP_LOG_INFO("%zd bytes received from [%s]:%hu\r\n", numBytes, s, ntohs(port));

                    /*
                     * Display it in the STDERR
                     */
                    // output_buffer(stderr, buffer, (size_t)numBytes, 0);

                    // connP = connection_find(data.connList, data.sock);
                    connP = connection_find(data.connList, &addr, addrLen);
                    if (connP != NULL)
                    {
                        /*
                         * Let liblwm2m respond to the query depending on the context
                         */
#ifdef WITH_MBEDTLS
                        int result = connection_handle_packet(connP, buffer, numBytes);
                        if (0 != result)
                        {
                             APP_LOG_INFO("error handling message %d\n",result);
                        }
#else
                        lwm2m_handle_packet(lwm2mH, buffer, (size_t)numBytes, connP);
#endif
                        // conn_s_updateRxStatistic(objArray[7], numBytes, false);
                    }
                    else
                    {
                        APP_LOG_INFO("received bytes ignored!\r\n");
                    }
                }
            }
        }
    }

    /*
     * Finally when the loop is left smoothly - asked by user in the command line interface - we unregister our client from it
     */
    if (g_quit == 1)
    {
#ifdef WITH_MBEDTLS
        free(pskBuffer);
#endif

#ifdef LWM2M_BOOTSTRAP
        close_backup_object();
#endif
        lwm2m_close(lwm2mH);
    }
    fibo_sock_close_v2(data.sock);
    connection_free(data.connList);

    clean_security_object(objArray[0]);
    lwm2m_free(objArray[0]);
    clean_server_object(objArray[1]);
    lwm2m_free(objArray[1]);
    free_object_device(objArray[2]);
    free_object_firmware(objArray[3]);

#ifdef MEMORY_TRACE
    if (g_quit == 1)
    {
        trace_print(0, 1);
    }
#endif

    return 0;
}


