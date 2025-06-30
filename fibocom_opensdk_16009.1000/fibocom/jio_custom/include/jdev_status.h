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

/*!	\file jdev_status.h
	\brief This file contains api for getting the ignition, mains status, internal status, and network status. 
*/

#ifndef _JDEV_STATUS_H_
#define _JDEV_STATUS_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdlib.h>
#include <stdint.h>

/*! \brief The function is used to reboot the system.
 
  	\return			Return 0 on sucess,
       				-1 If reboot fail,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_DevStatus_reboot();


#if defined(__cplusplus)
}
#endif
#endif
