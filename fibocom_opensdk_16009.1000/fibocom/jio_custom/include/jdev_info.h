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

/*!	\file jdev_info.h
	\brief This file contains api for getting the device storage path and backup path. 
*/

#ifndef _JDEV_INFO_H_
#define _JDEV_INFO_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* # defines */

	 
/* Enums. */


/* Structures. */


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief This function is used to get the device storage path.

	\param storagePath	Out parameter - Parameter in which we get the storage path.

	\return 		Returns 0 on success,
				-1 if fail to get path,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_DevInfo_getStoragePath(char *storagePath);


/*! \brief This function is used to get the device backup path. Path which cannot be removed after the SCOMO(app push).
	
	\param backupPath	Out parameter - Parameter in which we get the backup path.

	\return 		Returns 0 on success,
				-1 if fail to get path,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_DevInfo_getBackupPath(char *backupPath);


#if defined(__cplusplus)
}
#endif
#endif
