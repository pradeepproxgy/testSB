/*!	\file	dev_pal_info.h
	\brief	This file contain api for getting the imei, serial number and device platform information. 
*/

#ifndef _DEV_PAL_INFO_H_
#define _DEV_PAL_INFO_H_

/* # defines */
#define JIOT_PAL_DEVPALINFO_SERIAL_LEN				33		/*!< Indicate the max length of device serial number. */
#define JIOT_PAL_DEVPALINFO_IMEI_LEN				20		/*!< Indicate the max length of the device imei number. */
#define JIOT_PAL_DEVPALINFO_MAC_LEN				20
#define JIOT_PAL_DEVPALINFO_PLATFORM_TRACKER_MODEL		1		/*!< Indicate the platform tracker model number. */
#define JIOT_PAL_DEVPALINFO_PLATFORM_FIRMWARE_VERSION		2		/*!< Indicate the platform firmware version. */
#define JIOT_PAL_DEVPALINFO_PLATFORM_HARDWARE_VERSION		3		/*!< Indicate the platform hardware version. */
#define JIOT_PAL_DEVPALINFO_TRACKER_MODEL_MAX_LEN 		32		/*!< Indicate the max length of the tracker model number. */
#define JIOT_PAL_DEVPALINFO_FIRMWARE_VERSION_MAX_LEN 		32		/*!< Indicate the max length of the firmware version. */
#define JIOT_PAL_DEVPALINFO_SOFTWARE_VERSION_MAX_LEN 		64		/*!< Indicate the max length of the software version. */
/* Enums */


/* Structures */


/* External variables */


/* External functions */
/*! \brief This function is used to fetch imei of the device from the platform provided by the vendor.
	
	\param imeiNo		Out parameter which will be filled with imei number.

	\return			Return, 0 on success, 
				-1 on imei send request fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getImei(char *imeiNo);


/*! \brief This function is used to fetch iccid of the device from the platform provided by the vendor.
	
	\param iccid		Out parameter which will be filled with iccid.

	\return			Return, 0 on success, 
				-1 on iccid send request fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getIccid(char *iccid);


/*! \brief This function is used to get APN of the device from the platform provided by the vendor.
	
	\param apn		Out parameter which will be filled with APN.

	\return			Return, 0 on success, 
				-1 on iccid send request fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getApn(char *apn);


/*! \brief This function is used to set the APN of the device from the platform provided by the vendor.
	
	\param apn		In parameter which will be filled with APN.

	\return			Return, 0 on success, 
				-1 on iccid send request fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_setApn(char *apn);

/*! \brief This function is used to fetch imei of the device from the platform provided by the vendor.
	
	\param imeiNo		Out parameter which will be filled with imei number.

	\return			Return, 0 on success, 
				-1 on imei send request fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getPlatformVersion();


/*! \brief This function is used to get serial number of the device.

	\param serialNo		Out parameter used to get device serial number.

  	\return			Returns 0 on success,
				-1 on serial number send request fails.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getSerialNumber(char *serialNo);

/*! \brief This api is used for getting the platform/device information like firmware version, build version.

  	\param platformOption	This parameter indicate the whther we have to get the firmware version or build version.
				PLATFORM_OPT_TRACKER_MODEL = 1, PLATFORM_OPT_FIRMWARE_VERSION = 2, PLATFORM_OPT_HARDWARE_VERSION = 3
	\param outputBuff	Indicate the output parameter.
       	\param buffLength	Indicate the buffer length mximum 8 byte.

	\return			Returns 0 on success,
				-1 on fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getPlatformInfo(int8_t platformOption, char *outputBuff, int8_t buffLength);

/*! \brief The API returns whether the interface is up or down.

	\param isIPV6  		Indicates whether the address is of type IPV6 or not.

  	\return 		Returns 0 in case interface is down and 1 in case interface is up.
*/
extern int8_t jiot_client_PAL_DevPalInfo_isInterfaceUp(int8_t isIPV6);

/*! \brief This API is used to get the cpu information like temperature.

  	\param cpuTemp		Indicate the cpu temperature.

	\return			Returns 0 on sucess,
				-1 on fail.
*/
extern int8_t jiot_client_PAL_DevPalInfo_getCPUInfo(int8_t *cpuTemp);

typedef struct
{
     int sigStrength;                      //!< dBm value.
     int sigIndication;                          //!< RSSI value if data registered in GSM/LTE/CDMA
     int sigQuality;                             //!< RSRQ value if data registered in LTE
     int sigPower;                               //!< RSRP value if data registered in LTE
     int sigNoise;                               //!< SINR value if data registered in LTE
     int earfcn;                                 //!< EARFCN Frequency Used by the device
     int rachMaxPower;                           //!< Maximum power used for the latest RACH transmission to set up the RRC connection resulting either in successful or failed instances
     int bler;                                   //!< Residual BLER at the physical layer
     int dataThroughput;                         //!< Instantaneous Throughput of the data session
} jiot_client_DC_signaldetails_t;


typedef struct
{
     int cellID;                                 //!< Cell ID
     int trackAreaCode;                          //!< Tracking Area Code
     int locationAreaCode;                      //!< Location Area Code
} jiot_client_DC_networkinfodetails_t;

extern int8_t jiot_client_PAL_DevPalInfo_getSigQuality(jiot_client_DC_signaldetails_t *sigQ, jiot_client_DC_networkinfodetails_t *cellInfo);

#endif
