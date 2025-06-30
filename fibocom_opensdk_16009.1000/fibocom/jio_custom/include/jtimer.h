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

/*!	\file jtimer.h
	\brief This file contains the api for setting and getting the timer. 
*/

#ifndef _JTIMER_H_
#define _JTIMER_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* # defines */


/* Enums */


/* Structures */


/* Callback function definition. */
typedef void (* jiot_client_PAL_Timer_cb)(void * param);			/* This Callback needs to be called when timer value expire.
1. void * param:  indicate the parameter which has passed while timer created.
*/


/* External variables */


/* External functions */
/*! \brief This api is used for creating a new per-process interval timer.

	\param timerId		Out parameter - Timer ID which will be returned by the platform . 
	\param cb 		In parameter - Callback which needs to be called on timer expiry.
	\param param 		In parameter - Pointer containing the attributes for the callback.

  	\return			Return 0 on success,
				-1 on fail to register / provide NULL callback,
				-2 on fail to creat timer,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Timer_create(uint32_t *timerId, jiot_client_PAL_Timer_cb cb, void *param);

/*! \brief This api is used to start the timer.

	\param timerId 		In parameter - Timer ID which is passed by the platform.
	\param interval 	In parameter - Timer interval in millisec.
	\param reload 		In parameter - Auto reload , 1 = true / 0 = false.

  	\return			Return 0 on success,
				-1 on invalid timerId,
				-2 on invalid interval,
				-3 on invalid reload value,
				-4 on fail to start timer,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Timer_start(uint32_t timerId, uint32_t interval, int8_t reload);

/*! \brief This api is used to stop the timer.

	\param timerId 		In parameter - Timer ID which is passed by the platform.

  	\return			Return 0 on success,
				-1 on invalid timerId,
				-2 on fail to stop timer,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Timer_stop(uint32_t timerId);

/*! \brief This api is used to delete the timer.

	\param timerId 		In parameter - Timer ID which is passed by the platform. 

  	\return			Return 0 on success,
				-1 on invalid timerId,
				-2 on fail to delete timer,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Timer_delete(uint32_t timerId);


#if defined(__cplusplus)
}
#endif
#endif
