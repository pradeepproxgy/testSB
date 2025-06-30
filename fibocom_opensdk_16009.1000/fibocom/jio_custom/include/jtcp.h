/**
 * Reliance JioInfocomm (RJIL) Ltd. CONFIDENTIAL
 * Copyright (c) 2020 All rights reserved.
 *
 * This source code is proprietory and owned by RJIL.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by RJIL Ltd.
 * No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without RJIL Ltd prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by RJIL Ltd. in writing.
 */

/*! 	\file jtcp.h
	\brief This file contains the api for setting the getting the data over TCP and also get the TCP configuration data.
*/

#ifndef _JTCP_H_
#define _JTCP_H_


#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>

	 
/* # defines */

	 
/* Enums */


/* Structures */
/*! \brief It stores configuration required for TCP socket communication. */ 
typedef struct
{
	unsigned char url[100];					/*!< URL / IP address of the remote server. It can not be a NULL string. */
	uint16_t port;						/*!< TCP port is the port on which remote server is listening. Valid range 1024 to 65535. */
	uint16_t localPort;					/*!< Local TCP port is the port of local socket to bind on. Valid range 0 or 1024 to 65535. If 0 any random port can be selected automatically and used ONLY if the TCP socket is acting as client. */
	int8_t isSockAsServer;					/*!< Flag to indicate whether the socket is to be used as server or client. */ 
	int8_t isIPV6;						/*!< Flag to indicate whether the communication is via IPV6 or IPV4. Value 1 indicates IPV6 and value 0 indicates IPV4. */
	int8_t isReuseAddr;					/*!< Socket option to reuse the TCP address or not. */
	int8_t isSockNonBlock;					/*!< Options for using the socket blocking or non blocking. */
}jiot_client_PAL_Tcp_config_t;


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief It shall check if the TCP socket is open for communication and if not, it shall open it and establish connection in case socket is acting as client or accept
 				* connection in case socket is acting as server. 
  	
	\param config 		In parameter - Structure for configuration the parmeter for opening the tcp socket. 
 
  	\return			Return >=0 if success to open port else,
				-1 if invalid configuration,
				-2 if socket fail for local port,
				-3 if reuseaddr fail,
				-4 if fail to flush socket buffer,
				-5 if fail to bind,
				-6 if fail to set non-block,
				-7 if fail to listen,
				-128 if API not supported.
*/
extern int32_t jiot_client_PAL_Tcp_open(jiot_client_PAL_Tcp_config_t *config);

/*! \brief API to establish a TCP connection with server. 

	\param handle		In parameter - Indicate the fd which has to be make connection.  
	\param config 		In parameter - Structure for configuration the parmeter for accept the tcp connection. 
 
  	\return			Return 0 on success to make the connection,
				-1 if invalid configuration,
				-2 if IP resolved fail,
				-3 if IP resolved list empty,
				-4 if ASCII to addr fail,
				-5 if connection in progress,
			       	-6 if connection is not already establish,
				-7 if error in making the connection,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Tcp_makeConn(int32_t handle, jiot_client_PAL_Tcp_config_t *config);

/*! \brief API to accept the TCP connection of client. 
  	
	\param handle		In parameter - Indicate the fd which has to be accept the connection.  
	\param config 		In parameter - Structure for configuration the parmeter for accept the tcp connection. 
 
  	\return			Return client handle if success,
				-1 if invalid configuration,
				-2 if aceept error,
				-3 if connection is not already there and network is unreachable
				-4 if fail to set non-block,
				-128 if API not supported.
*/
extern int32_t jiot_client_PAL_Tcp_acceptConn(int32_t handle, jiot_client_PAL_Tcp_config_t *config);

/*! \brief It shall read bytes from the TCP socket. 
  	
 	\param handle		In parameter - Socket identifier of the TCP socket to consider.
	\param buffer 		Out parameter - Copy the bytes read from the TCP socket.
	\param length 		In parameter - It specifies the size of buffer, i.e. maximum number of bytes that can be copied to the buffer.

  	\return 		Returns >= 0 number of bytes that are read and copied in the output parameter buffer,
				-1 if invalid inputs,
				-2 if in case of read fail,
				-3 if in case of read fail and errno is EAGAIN. 
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Tcp_read(int32_t handle, unsigned char *buffer, uint16_t length);

/*! \brief It shall write bytes to the TCP socket. 
  	
 	\param handle 		In parameter - Socket identifier of the TCP socket to consider.
  	\param buffer 		In parameter - Buffer containing the bytes to be written.
	\param length 		In parameter - It specifies the number of bytes in the buffer to be written.

  	\return 		Returns >= 0 number of bytes that are to be write,
				-1 if invalid inputs,
				-2 if in case of write fail,
			       	-3 if in case of write fail and errno is EAGAIN,
				-4 if in case of write fail and errno is EMFILE. 
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Tcp_write(int32_t handle, unsigned char *buffer, uint16_t length);

/*! \brief It shall close the TCP socket if it is valid and is already open. 

	\param handle 		In parameter - Socket identifier of the TCP socket to consider.

	\return 		Returns 0 on success,
				-1 if handle invalid,
				-2 if close fail.
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Tcp_close(int32_t handle);

extern int8_t jiot_client_PAL_Tcp_RAI();

#if defined(__cplusplus)
}
#endif
#endif

