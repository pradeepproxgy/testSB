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

/*! 	\file jdev_log.h
	\brief This file contains api for start and stop the remote logging. 
*/

#ifndef _JDEV_LOG_H_
#define _JDEV_LOG_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */


/* # defines */
#define JIOT_AP_LOG_TYPE 0
#define JIOT_CP_LOG_TYPE 1

#define JIOT_DEVLOG_REMOTELOG_PORT  (30103)
#define JIOT_DEVLOG_REMOTELOG_DIR "/sftptest/"
/* Enums */


/* Structures */


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief This function is used to start remote logging where the log files are uploaded from device to FTP server.
 	
  	\param devicePassword	In parameter - This variable is used to provide device specific passoword to identify genuineness of request. As per formula from device RSN.
	\param ftpURL		In parameter - This parameter speficies the new FTP url. If NULL then consider default URL.
	\param ftpUsername	In parameter - This parameter specifies the username for new FTP URL. To be ignored if ftuURL parameter is NULL. If parameter is NULL then firmware should ignore and need to connect without Username.	
	\param ftpPassword	In parameter - This parameter specifies the password for new FTP URL. To be ignored if ftuURL parameter is NULL. If parameter is NULL then firmware should ignore and need to connect without Password.	
	\param funcPtrCB	In parameter - This parameter is to register a callback function to get updates on the remote logging activity. Status field mentions different status to be given through callback function.
       				0 - Success
				1 - FTP server not available
				2 - FTP credentials invalid
				3 - FTP copy failed (low memory etc.)
				4 - Remote logging stopped (automatically or using stop API)	
 
 	\return			Returns 0 on success,
				-1 Already in progress,
				-2 Callback NULL,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_DevLog_startRemoteLog(char *devicePassword, char *ftpURL, char *ftpUsername, char *ftpPassword, void (*funcPtrCB)(uint8_t status));

/*! \brief This function is used to stop remote logging.
 	
 	\return			Return 0 on success,
				-1 Remote logging not in progress,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_DevLog_stopRemoteLog();


#if defined(__cplusplus)
}
#endif
#endif
