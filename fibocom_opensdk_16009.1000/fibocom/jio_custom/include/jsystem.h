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

/*!	\file jsystem.h
	\brief This file contains the api for setting and getting the time related information of the device in GMT and UTC. 
*/

#ifndef _JSYSTEM_H_
#define _JSYSTEM_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>
#include <unistd.h>
#include <time.h>


/* #defines */

	 
/* Enums. */


/* Structures. */
/*! \brief Indicate the structure of epoch time. */
typedef struct
{
	int8_t sec;					/*!< Indicate the seconds (0-60). */
	int8_t min;					/*!< Indicate the minutes (0-59). */
	int8_t hour;					/*!< Indicate the hours (0-23). */
	int8_t day;					/*!< Indicate the day (1-31). */
	int8_t month;					/*!< Indicate the month (0-11). */
	int16_t year;					/*!< Indicate the year. */
}jiot_client_PAL_System_tm_t;


/* Callback function definition. */


/* External variables. */


/* External functions. */
/*! \brief This function is used to return the epoch time. NOTE: Mandatory API need to be implement across all the platforms.

	\return			Return epoch in second.
*/
extern uint32_t jiot_client_PAL_System_getEpochInSec();

/*! \brief This function returns current time in miliseconds from 1st Jan 2018. NOTE: Mandatory API need to be implement across all the platforms.
	
	\return			Return epoch in millisecond.
*/
extern uint64_t jiot_client_PAL_System_getEpochInMsec();

/*! \brief This function returns the milliseconds(tick) since from board boot. NOTE: Mandatory API need to be implement across all the platforms.
	
	\return			Return milliseconds(tick).
*/
extern uint64_t jiot_client_PAL_System_getSystemTick();

/*! \brief This function is used to convert the epoch in second to readable date time.

	\param currTime		In parameter - Specifies the time to be in readable formate to be converted.
	\param epochInSec	Out parameter - Specifies the epoch time converted.

	\return			Return 0 on sucess, 
				-1 if invalid arguments,
				-2 if fail to convert epoch from datetime,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_System_getEpochFromDateTime(jiot_client_PAL_System_tm_t *currTime, uint32_t *epochInSec);

/*! \brief This function is used to convert the readable date time to epoch time in sec.

	\param currTime		Out parameter - Specifies the readable date time.
	\param epochInSec	In parameter - Specifies the epoch time in sec.

	\return			Return 0 on sucess, 
				-1 if invalid arguments,
				-2 if fail to convert epoch from datetime,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_System_getDateTimeFromEpoch(jiot_client_PAL_System_tm_t *currTime, uint32_t epochInSec);

/*! \brief This api is used to set the date time of the system in epoch.
	
  	\param date		In parameter - Specifies the date which is to be set in the system.

	\return			Return 0 on sucess,
				-1 if invalid arguments,
				-2 if fail to set,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_System_setSystemDateTime(jiot_client_PAL_System_tm_t *date);

/*! \brief This function is used to have system() call.

	\param command		In parameter - Command to be execute in system command prompt.

	\return 		Return 0 on sucess,
				-1 if invalid arguments,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_System_systemCmd(const char *command);

/*! \brief This Api is used to set sleep timer for given duration in milliseconds.

  	\param durationMilliSec	In parameter - Time till which process to be in sleep.

	\return			Return 0 on sucess,
				-1 if invalid arguments,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_System_milliSecSleep(uint32_t durationMilliSec);

/*! \brief This function is used to do exit.
	
  	\param idx		In parameter - Indicate the exit status.

	\return			Return 0 on sucess,
				-1 if invalid arguments,
				-128 if API not supported.
*/
extern void jiot_client_PAL_System_exit(uint8_t idx);


#if defined(__cplusplus)
}
#endif
#endif

