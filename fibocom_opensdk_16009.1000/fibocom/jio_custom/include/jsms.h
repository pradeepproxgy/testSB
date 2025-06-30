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

/*!	\file jsms.h
  	\brief This file contains the api for getting and sending the sms related information.
*/

#ifndef _JSMS_H_
#define _JSMS_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>

	 
/* # defines */
#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#endif

#define JIOT_PAL_SMS_SUCCESS	0		/*!< Return code: No error. Indicates successful operation of SMS function. */
#define JIOT_PAL_SMS_FAILURE	-1		/*!< Return code: A generic error code indicating the failure of an SMS operation. */

 
/* Enums. */

	 
/* Structures. */
/*! \brief Structure of the SMS arrival timing information in IST format. */
typedef struct
{
	int8_t sec;					/*!< Indicate the seconds (0-60). */
	int8_t min;					/*!< Indicate the minutes (0-59). */
	int8_t hour;					/*!< Indicate the hours (0-23). */
	int8_t day;					/*!< Indicate the day (1-31). */
	int8_t month;					/*!< Indicate the month (0-11). */
	int16_t year;					/*!< Indicate the year. */
}jiot_client_PAL_Sms_time_t;

/*! \brief Structure for the received sms message. */
typedef struct
{
	char *message;					/*!< SMS message sent from lower layer. */
	uint16_t msglen;				/*!< SMS message length in bytes. */
	char *phoneNum;					/*!< SMS Originating source. */
	uint32_t phoneNumLen;				/*!< SMS Originating source length in bytes. */
	jiot_client_PAL_Sms_time_t time;		/*!< SMS message arrival time in IST format. */
}jiot_client_PAL_Sms_msg_t;

/* Callback function definition. */
typedef void (*jiot_client_PAL_Sms_cb)(jiot_client_PAL_Sms_msg_t data);		/*!< Function pointer of the Callback for SMS recieved from lower layer, Client applications can set this callback using jiot_client_PAL_Sms_registerCb() API.
1. jiot_client_PAL_Sms_msg_t data:  indicate the data of the recieved sms.
*/


/* External variables. */


/* External functions. */
/*! \brief This function registers SMS callback from lower layers.

	\param callback		In parameter - jiot_client_PAL_Sms_cb registered from the application, which gets triggered upon arrival of SMS message.
	
	\return			Return JIOT_PAL_SMS_SUCCESS if successful or JIOT_PAL_SMS_FAILURE on failure,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Sms_registerCb(jiot_client_PAL_Sms_cb callback);

/*! \brief This function sends sms to the destination number.
	
	\param destNum		In parameter - Destination number to which the sms needs to be sent.
	\param message		In parameter - SMS message which needs to be sent.
	\param length		In parameter - Length of the SMS message in bytes.
	
	return			Return JIOT_PAL_SMS_SUCCESS if successful or JIOT_PAL_SMS_FAILURE on failure,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Sms_send(char *destNum, char* message, uint16_t length);


#ifdef __cplusplus
}
#endif
#endif

