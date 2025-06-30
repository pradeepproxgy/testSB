/* ===============================START OF FILE====================================================
*
* Jio Platforms Limited.
* CONFIDENTIAL
* __________________
*
*  Copyright (C) 2021 Jio Platforms Limited -
*
*  ALL RIGHTS RESERVED.
*
* NOTICE:  All information including computer software along with source code and associated
* documentation contained herein is, and remains the property of Jio Platforms Limited. The
* intellectual and technical concepts contained herein are proprietary to Jio Platforms Limited.
* and are protected by copyright law or as trade secret under confidentiality obligations.
* Dissemination, storage, transmission or reproduction of this information in any part or full
* is strictly forbidden unless prior written permission along with agreement for any usage right
* is obtained from Jio Platforms Limited.
================================================================================================ */

/* ===============================ABOUT THIS FILE=============================================== */

/**
*
* @file jdc_client_application.h
* @brief API's for capturing application related params, crashes, events etc.
*/

/*  ===============================END ABOUT THIS FILE=========================================== */

/// \file
#ifndef JDC_CLIENT_APPLICATION_H_
#define JDC_CLIENT_APPLICATION_H_

#if defined(__cplusplus)
 extern "C" {
#endif

#include "jdc_client_main.h"
/* ===============================#DEFINE START================================================= */


/* ===============================#DEFINE END=================================================== */



/* ===============================ENUM START==================================================== */
/*! \brief This enum lists down various Application related events*/
typedef enum
{
	JDC_CB_EVENT_APP_OPEN = 0,			//!< Event for Application Open
	JDC_CB_EVENT_APP_CLOSE,				//!< Event for Application Close
	JDC_CB_EVENT_APP_CRASH,				//!< Event for Application Crash
	JDC_CB_EVENT_APP_NOT_RESPONDING,	//!< Event for Application Not responding
	JDC_CB_EVENT_APP_UPDATED			//!< Event for Application getting updated
} jdc_client_application_events_e;
/* ===============================ENUM END====================================================== */


/* ===============================STRUCTURE START====================================================== */
typedef struct
{
	jdc_client_application_events_e evt;
	void* info;		/**< Will contain additonal info based on type of notification. 
						Ex: \n
						JDC_CB_EVENT_APP_OPEN - Application name and version in Json string format. Ex: [{"name":"Application1","appVersion":"1.2"}]
						JDC_CB_EVENT_APP_CLOSE - Application name and version in Json string format. Ex: [{"name":"Application1","appVersion":"1.2"}]
						JDC_CB_EVENT_APP_CRASH - Below is free form json sample for the crash reason. Actual json content details will be decided by implementor.
            				Ex: {"app://clock.gaiamobile.org/manifest.webapp": [{"time": 1596568084306,"usageTimeSecs": 43,"crashedReason": "MemoryFault","appVersion": "2.2"}]}
                            Respective product team should decide the specific applications for which crash will be reported
            			JDC_CB_EVENT_APP_NOT_RESPONDING - Application Name
            			JDC_CB_EVENT_APP_UPDATED - Application name and updated version in Json string format. Ex: [{"name":"Application1","appVersion":"1.3"}]                  

					*/
}jdc_client_application_evt_details_t;
/* ===============================STRUCTURE END====================================================== */


/* ====================================== FUNCTION START ======================================= */

/**
 * \brief Get the application SW crash reason(e.g. low memory, memory corruption, stack overflow). 
 * \Its a readable text format having software crash information like exception details, error code etc.
 * Respective product team should decide the specific applications for which crash will be reported
 * \param NA
 * \return Crash reason (or) NULL if API not supported.\n
 * Below is free form json sample for the crash reason. Actual json content details will be decided by implementor.
 * Ex: {"app://clock.gaiamobile.org/manifest.webapp": [{"time": 1596568084306,"usageTimeSecs": 43,"crashedReason": "MemoryFault","appVersion": "2.2"}]}
 * <B>Note</B>: Caller should take care of deleting the Crash reason details memory returned.
 */
char* jdc_client_getCrashReason(void);



/**
 * \brief This Api is used to register callback for getting Notifications about Application events like App open, close, Crash, not responsing etc.
 * \param[in] callback function which informs Application events jdc_client_application_evt_details_t.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForApplicationEvents(void (*fn)(jdc_client_application_evt_details_t*));



/**
 * \brief This Api is used to De-register callback  which was registerd for getting Application Events.
 * \param[in] callback function which informs Intenal Battery status jdc_client_application_evt_details_t.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deregisterForApplicationEvents(void (*fn)(jdc_client_application_evt_details_t*));



/**
 * \brief Get number of Application (system applications including DC) crashes occured in last 24Hrs.
 * \param  NA
 * \return  Application Crash count occured in last 24Hrs (or) JDC_API_NOT_SUPPORTED 
 */
int16_t jdc_client_getAppCrashCount(void);
/* ====================================== FUNCTION END ======================================= */

#if defined(__cplusplus)
}
#endif

#endif /* JDC_CLIENT_APPLICATION_H_ */

/* ===============================END OF THE FILE=============================================== */