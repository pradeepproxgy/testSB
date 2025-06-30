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
* @file jdc_client_common.h
* @brief API's for capturing common params like Build details, device details, sim details etc.
*/

/*  ===============================END ABOUT THIS FILE=========================================== */

/// \file
#ifndef JDC_CLIENT_COMMON_H_
#define JDC_CLIENT_COMMON_H_

#if defined(__cplusplus)
 extern "C" {
#endif

#include "jdc_client_main.h"

/* ===============================#DEFINE START================================================= */

#define JDC_PLATFORM_VERSION_LENGTH	30
#define JDC_HW_VERSION_LENGTH		20
#define JDC_DEVICE_MODEL_LENGTH		15
#define JDC_IMEI_LENGTH				15
#define JDC_SERIAL_NUM_LENGTH		15

#define JDC_IMSI_LENGTH 15
#define JDC_MCC_LENGTH 	3
#define JDC_MNC_LENGTH 	3

/* ===============================#DEFINE END=================================================== */



/* ===============================ENUM START==================================================== */

/*! \brief This enum lists Storage Memory Types. Internal (flash)  (or) External  (Sd Card).*/
typedef enum
{
	JDC_STORAGE_TYPE_ROM_INTERNAL_E = 0,		//!< Device Memory (Internal/Flash)
	JDC_STORAGE_TYPE_ROM_EXTERNAL_E,			//!< External Memory (SD Card)
	JDC_STORAGE_TYPE_RAM_E 						//!< Device RAM
} jdc_client_storage_type_e;

/* ===============================ENUM END====================================================== */


/* ===============================STRUCTURE START====================================================== */

typedef struct
{
	char platformVersion[JDC_PLATFORM_VERSION_LENGTH];		//!< Device Firmware Version
	char hardwareVersion[JDC_HW_VERSION_LENGTH];			//!< Device Hardware Version
	char model[JDC_DEVICE_MODEL_LENGTH];					//!< Device Model
	char imei[JDC_IMEI_LENGTH + 1];							//!< Device IMEI Number
	char serialNum[JDC_SERIAL_NUM_LENGTH + 1];				//!< Device Serial Number
} jdc_client_device_details_t;
#define deviceDetails_initializer {{'\0'}, {'\0'}, {'\0'}, {'\0'}, {'\0'}} /** @hideinitializer */

typedef struct
{
	char imsi[JDC_IMSI_LENGTH + 1];		//!< IMSI Number if Available
	char mcc[JDC_MCC_LENGTH + 1];		//!< Mobile Country Codes( MCC ) code
	char mnc[JDC_MNC_LENGTH + 1];		//!< Mobile Network Codes(MNC) code
} jdc_client_sim_details_t;
#define simDetails_initializer {{'\0'}, {'\0'}, {'\0'}} /** @hideinitializer */

typedef struct
{
	uint64_t cellID;						//!< Cell ID
	int32_t trackAreaCode;					//!< Tracking Area Code
	int32_t locationAreaCode;				//!< Location Area Code
	int32_t pci;							//!< Physical Cell Identifier
} jdc_client_network_details_t;
#define networkDetails_initializer {0, -1, -1, -1} /** @hideinitializer */

typedef struct
{
	bool gpsFix;					//!< Used to indicate the valid gps fix. (1-Freshly location, 0-Stale location)
	double latitude;				//!< Latitude
	double longitude;				//!< Longitude
	double accuracy;				//!< Accuracy in meters
	double speed;					//!< Speed
	double bearing;					//!< bearing is the compass direction from your current position
	uint64_t timestamp;				//!< UTC time since epoch in ms
} jdc_client_location_data_t;
#define locationInfo_initializer {0, 0.0, 0.0, 0.0, 0.0, 0.0, 0} /** @hideinitializer */

typedef struct
{
	uint32_t total; 				//!< Total memory In KB
	uint32_t used; 					//!< Used memory In KB
	uint16_t usage_in_percentage;	//!< Storage usage in percentage (0-100%)
}jdc_client_storage_details_t;
#define storageDetails_initializer {0, 0, 0} /** @hideinitializer */
/* ===============================STRUCTURE END====================================================== */


/* ====================================== FUNCTION START ======================================= */

/**
 * \brief API to get current time details.
 * \param[out] jdc_client_time_t pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getCurrentTime(jdc_client_time_t *time);



/**
 * \brief This is used to get the device details
 * \param[out] data pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getDeviceDetails(jdc_client_device_details_t *data);



/**
 * \brief This is used to get Network Details like cell id, track area code etc.
 * \param[out] lte_var pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getNetworkDetails(
		jdc_client_network_details_t *lte_var);



/**
 * \brief This method is used to get SIM card details like imsi, phone no etc.
 * \param[out] s_var pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getSIMDetails(jdc_client_sim_details_t *s_var);



/**
 * \brief This is used to get the Current Location from GPS.
 * \param[out] location_data structure pointer has to be passed where GPS location data will be populated.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getCurrentLocation(
		jdc_client_location_data_t *location_data);



/**
 * \brief This API is used to get cpu usage in percent(%).
 * \param void
 * \return CPU usage in percent(%) (or) JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getCPUUsage(void);



/**
 * \brief Get the DC Library version implemented by Vendor for supporting the DC params.
 * \param void
 * \return DC Library version.\n
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 */
char* jdc_client_getLibraryVersion(void);



/**
 * \brief Get the Software/Firmware version running on the device.
 * \param void
 * \return Software/Firmware version.\n
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 */
char* jdc_client_getFirmwareVersion(void);



/**
 * \brief This is about DC status(enabled/disabled). 
 * \If DC app is DISABLED, DC Application will not collect & publish the device data.
 * \This status will be useful to take a decision whether DC Application should collect/publish data.
 * \param NA
 * \return
 * 0 : DC application is Disabled (or)\n
 * 1 : DC application is Enabled (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getPrivacyStatus(void);




/**
 * \brief API to get storage details.
 * \param[in] type must be passed to identify Internal (or) External memory details required.
 * \param[out] mem_details pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getStorageDetails(jdc_client_storage_type_e type, jdc_client_storage_details_t* mem_details);
/* ====================================== FUNCTION END ======================================= */

#if defined(__cplusplus)
}
#endif

#endif /* JDC_CLIENT_COMMON_H_ */

/* ===============================END OF THE FILE=============================================== */
