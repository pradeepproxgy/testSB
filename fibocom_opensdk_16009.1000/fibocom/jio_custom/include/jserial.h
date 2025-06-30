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

/*! 	\file jserial.h
	\brief This file contains the api for getting the data on serial, write the data on serial, and configure the serial. 
*/

#ifndef _JSERIAL_H_
#define _JSERIAL_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>


/* # defines */

	 
/* Enums */
/*! \brief Baud rates supported by system for serial communication, used for configuring serial port. */ 
typedef enum
{
	JIOT_PAL_SERIAL_BAUD_RATE_300 = 300,		/*!< For selecting communication speed as 300. */
	JIOT_PAL_SERIAL_BAUD_RATE_1200 = 1200,		/*!< For selecting communication speed as 1200. */
	JIOT_PAL_SERIAL_BAUD_RATE_2400 = 2400,		/*!< For selecting communication speed as 2400. */
	JIOT_PAL_SERIAL_BAUD_RATE_4800 = 4800,		/*!< For selecting communication speed as 4800. */
	JIOT_PAL_SERIAL_BAUD_RATE_9600 = 9600,		/*!< For selecting communication speed as 9600. */
	JIOT_PAL_SERIAL_BAUD_RATE_19200 = 19200, 	/*!< For selecting communication speed as 19200. */
	JIOT_PAL_SERIAL_BAUD_RATE_38400 = 38400,	/*!< For selecting communication speed as 38400. */
	JIOT_PAL_SERIAL_BAUD_RATE_57600 = 57600,	/*!< For selecting communication speed as 57600. */
	JIOT_PAL_SERIAL_BAUD_RATE_115200 = 115200,	/*!< For selecting communication speed as 115200. */
	JIOT_PAL_SERIAL_BAUD_RATE_14400 = 14400,	/*!< For selecting communication speed as 14400. */
}jiot_client_PAL_Serial_baudRate_t;

/*! \brief Stop bits supported by system for serial communication, used for configuring serial port. */ 
typedef enum
{
	JIOT_PAL_SERIAL_STOP_BITS_1 = 1,		/*!< For select 1 stop bit. */
	JIOT_PAL_SERIAL_STOP_BITS_2 = 2,		/*!< For select 2 stop bits. */
}jiot_client_PAL_Serial_stopBits_t;

/*! \brief Parity bits supported by system for serial communication, used for configuring serial port. */ 
typedef enum
{
	JIOT_PAL_SERIAL_PARITY_ODD = 1,			/*!< For select odd parity(PARODD). */
	JIOT_PAL_SERIAL_PARITY_EVEN = 2,		/*!< For select even parity(~PARODD). */
	JIOT_PAL_SERIAL_PARITY_NONE = 0,		/*!< For none parity. */
}jiot_client_PAL_Serial_parityBit_t;

/*! \brief Data bits supported by system for serial communication, used for configuring serial port. */ 
typedef enum
{
	JIOT_PAL_SERIAL_DATA_BITS_5 = 5,		/*!< For select 5 data bits. */
	JIOT_PAL_SERIAL_DATA_BITS_6 = 6,		/*!< For select 6 data bits. */
	JIOT_PAL_SERIAL_DATA_BITS_7 = 7,		/*!< For select 7 data bits. */
	JIOT_PAL_SERIAL_DATA_BITS_8 = 8,		/*!< For select 8 data bits. */
}jiot_client_PAL_Serial_dataBits_t;


/* Structures */
/*! \brief It stores configuration required for serial communication. */
typedef struct 
{
	uint8_t portId;			/*!< Serial port to use. This should be mapped to array in device file. */
	int32_t baudRate;		/*!< Baud rate at which device will communicate over serial. */
	uint8_t parityBit;		/*!< Parity bit used to communicate over serial. */
	uint8_t stopBit;		/*!< Stop bits used to communicate over serial. */
	uint8_t dataBit;		/*!< Data bits used to communicate over serial. */
}jiot_client_PAL_Serial_config_t;


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief This function is used to open the serial port. Port need to be opened before calling any other function in this file from outside.
  	
	\param config	 	In parameter - Indicate the structure pointer for getting the port id, data bit, stop bit, and parity bit.
 
	\return			Returns, handle on success else on error it returns,
				-1 if invalid configuration, i.e. baudrate, paritybit, stopbit, databit. 
				-2 if fail to open port,
				-3 if fail to get terminal attribute,
				-4 if fail to set new attributes,
				-5 if ioctl error,
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Serial_open(jiot_client_PAL_Serial_config_t *config);

/*! \brief It shall read bytes from the serial port.

	\param handle	 	In parameter - Port identifier of the serial port to consider.
	\param buffer	 	Out parameter - Copy the bytes read from the serial port.
	\param length 		In parameter - It specifies the size of buffer, i.e. maximum number of bytes that can be copied to the buffer.

	\return 		Returns, >= 0 i.e. number of bytes that are read and copied in the output parameter buffer,
				-1 if invalid input, 
				-2 if read fail.
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Serial_read(int16_t handle, unsigned char *buffer, uint16_t length);

/*! \brief It shall write bytes to the serial port.

	\param handle	 	In parameter - Port identifier of the serial port to consider.
	\param buffer	 	In parameter - Buffer containing the bytes to be written.
	\param length 		In parameter - It specifies the number of bytes in the buffer to be written.

	\return 		Returns, >= 0 i.e. number of bytes that are writen,
				-1 if invalid input, 
				-2 if write fail.
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Serial_write(int16_t handle, unsigned char *buffer, uint16_t length);

/*! \brief It shall close the serial port if it is valid and is already open.
	
	\param handle	 	In parameter - Serial port on which operation is to be carried out.
	
	\return 		Returns 0 on successfully close,
				-1 on invalid input,
				-2 on fail to close.
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Serial_close(int16_t handle);


#if defined(__cplusplus)
}
#endif
#endif

