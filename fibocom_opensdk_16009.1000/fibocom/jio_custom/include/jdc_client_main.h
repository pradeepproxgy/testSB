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

/**

    \brief  This document list down the list of DataCollector API's required for collecting DC items.

*/

/* ===============================ABOUT THIS FILE=============================================== */

/**
*
* @file jdc_client_main.h
* @brief This file containts Common across all other API.h files) data-structures for DC.
*/

/*  ===============================END ABOUT THIS FILE=========================================== */


/// \file
#ifndef JDC_CLIENT_MAIN_H_
#define JDC_CLIENT_MAIN_H_

#if defined(__cplusplus)
 extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

/* ===============================#DEFINE START================================================= */

#define JDC_API_NOT_SUPPORTED -1     //Indicate API Not supported
#define JDC_API_FAILURE 0            //Indicate API failed to get the requested data
#define JDC_API_SUCCESS 1            //Indicate API execution was success

#define JDC_API_VERSION "4.11"		 //Indicate Current DC API Version
/* ===============================#DEFINE END=================================================== */



/* ===============================ENUM START==================================================== */


/* ===============================ENUM END====================================================== */


/* ===============================STRUCTURE START====================================================== */


typedef struct
{
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t mins;
    uint8_t secs;
    uint16_t year;
    uint16_t milliSecs;
} jdc_client_time_t;
#define time_initializer {0, 0, 0, 0, 0, 0, 0} /** @hideinitializer */

/* ===============================STRUCTURE END====================================================== */



/* ====================================== FUNCTION START ======================================= */

/* ====================================== FUNCTION END ======================================= */

#if defined(__cplusplus)
}
#endif

#endif /* JDC_CLIENT_MAIN_H_ */

/* ===============================END OF THE FILE=============================================== */
