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
* @file jdc_client_device.h
* @brief API's for capturing system, Hardware, Networkinterface related params and events
*/

/*  ===============================END ABOUT THIS FILE=========================================== */

/// \file
#ifndef JDC_CLIENT_DEVICE_H_
#define JDC_CLIENT_DEVICE_H_

#if defined(__cplusplus)
 extern "C" {
#endif

#include "jdc_client_main.h"

/* ===============================#DEFINE START================================================= */
#define JDC_WIFI_BAND_24 	0		// 2.4GHz Band
#define JDC_WIFI_BAND_50 	1		// 5.0GHz Band
#define JDC_WIFI_BAND_DUAL 	2		// Both 2.4GHz and 5.0 GHz bands

#define JDC_MAC_ADDRESS_LENGTH		20
#define JDC_IPV4_ADDRESS_LENGTH		16
#define JDC_IPV6_ADDRESS_LENGTH		40

/* ===============================#DEFINE END=================================================== */



/* ===============================ENUM START==================================================== */

/*! \brief This enum lists the battery health status related enum options*/
typedef enum
{
	JDC_BATTERY_HEALTH_STATUS_INVALID = -1,		//!< Invalid status value if status couldn't be determined
	JDC_BATTERY_HEALTH_STATUS_GOOD = 0,			//!< Battery is healthy
	JDC_BATTERY_HEALTH_STATUS_REPLACE_SOON,		//!< Battery health deterioting and good to replace it.
	JDC_BATTERY_HEALTH_STATUS_CRITICAL,			//!< Battery health critical and needs replacement urgently
	JDC_BATTERY_HEALTH_STATUS_DEAD				//!< Battery is dead and no more holding charge.
} jdc_battery_health_status_e;

/*! \brief This enum lists the battery related info*/
typedef enum
{
	JDC_BATTERY_TEMPERATURE = 1,		//!< Temperature in °C.
	JDC_BATTERY_STATE = 2,				//!< Charging state ( 0 - Discharging, 1 - Charging, -1 - Unknown).
	JDC_BATTERY_LEVEL = 3,				//!< Power level (0-100)%
    JDC_BATTERY_VOLTAGE = 4,			//!< Battery Voltage in milli volts
    JDC_BATTERY_PRESENSE = 5,			//!< Battery available or not  (0 -Not Available, 1- Available)
    JDC_BATTERY_HEALTH = 6 				//!< Battery health 
} jdc_client_battery_e;

/*! \brief This enum lists down various battery related Async events*/
typedef enum
{
	JDC_CB_EVENT_BATTERY_STATUS_CONNECTED = 0,		//!< Event for Internal battery Connected state
	JDC_CB_EVENT_BATTERY_STATUS_DISCONNECTED,		//!< Event for Internal battery Disconnected state
	JDC_CB_EVENT_BATTERY_STATUS_CHARGING,			//!< Event for Internal battery in Charging state
	JDC_CB_EVENT_BATTERY_STATUS_FULLY_CHARGED,		//!< Event for Internal battery for Fully Charged state
	JDC_CB_EVENT_BATTERY_STATUS_DISCHARGING,		//!< Event for Internal battery in Discharging state
	JDC_CB_EVENT_BATTERY_STATUS_FULLY_DISCHARGED,	//!< Event for Internal battery for Fully Discharged state
	JDC_CB_EVENT_BATTERY_LEVEL 						//!< When power level of a device goes below 25%
} jdc_client_battery_notifications_e;


/*! \brief This enum lists down possible WiFi states */
typedef enum
{
	JDC_WIFI_STATE_INVALID = -1,
	JDC_WIFI_STATE_DISABLED,
	JDC_WIFI_STATE_ENABLED
} jdc_client_wifi_state_e;

/*! \brief This enum lists down possible System Alerts*/
typedef enum
{
	JDC_CB_EVENT_SYSTEM_ALERT_INVALID = -1,
	JDC_CB_EVENT_DEEP_SLEEP_MODE_ENABLED = 0,	//!< Deep sleep mode enabled update
	JDC_CB_EVENT_DEEP_SLEEP_MODE_DISABLED,		//!< Deep sleep mode disabled update

	JDC_CB_EVENT_DC_ENABLED,					//!< E.g. Notifying user accepted EULA(End User License Agreement) 
	JDC_CB_EVENT_DC_DISABLED,					/*< E.g. Notifying user rejected EULA(End User License Agreement) 
													Device suppose to wait for >= 20 seconds before device reboot*/
	JDC_CB_EVENT_CONFIG_UPDATE,					//!< Notify for DC config has been updated

	JDC_CB_EVENT_IGNITION_MODE_ON,				//!< Notify when the Ignition mode is turned ON.
	JDC_CB_EVENT_IGNITION_MODE_OFF,				//!< Notify when the Ignition mode is turned OFF

	JDC_CB_EVENT_RTC_WAKEUP,					//!< Notify when RTC wakeup happens.

	JDC_CB_EVENT_WIFI_TOGGLE,					//!< Notify when Wifi Radio is Enabled / Disabled
	JDC_CB_EVENT_FOTA_UPDATE_SUCCESS,			//!< Triggered for Successful FOTA update
	JDC_CB_EVENT_FOTA_UPDATE_FAILED,			//!< Triggered for FOTA update failure

	JDC_CB_EVENT_TEMPERATURE_EXCEPTION,			/*< There will be separate threshold temperature values for device CPU and Battery. \
                                                     Temperature exception event will be tiggered when either the CPU or \
                                                     Battery temperature exceeds the respective threshold value(e.g. it can be 45 0C). \
                                                     Threshold values will be defined by product teams and will be specific to device.*/

	JDC_CB_EVENT_CELLULAR_POWER_SAVE_MODE_ENABLED,	//!< Cellular Power save mode enabled 
	JDC_CB_EVENT_CELLULAR_POWER_SAVE_MODE_DISABLED,		//!< Cellular Power save mode enabled 
	JDC_CB_EVENT_BATTERY_SAVE_MODE_ENABLED,		//!< Battery save mode enabled 
	JDC_CB_EVENT_BATTERY_SAVE_MODE_DISABLED,	//!< Battery save mode disabled 

	JDC_CB_EVENT_DATA_CONNECTION_STATE_CHANGED,	//!< Notify when Data connection state changed(Enabled-1/ Disabled-0)

	JDC_CB_EVENT_AIRPLANE_MODE_CHANGED,			//!< Notify when Airplane Mode changed(Enabled-1/ Disabled-0)

	JDC_CB_EVENT_REBOOT,						//!< Event for capturing Device reboot. It will be trigger on device boot/start.
	JDC_CB_EVENT_FACTORY_RESET,					/*< Triggered for Factory Reset. DC needs a buffer time to complete the required functionality. \
                                                     Buffer time will be prescribed by product ownner. \
                                                     DC doesn't guarantees that required functionality will be completed successfully within prescribed buffer time.*/

    JDC_CB_EVENT_INCOMPATIBLE_CHARGER_CONNECTED, //!< Triggered when incompatible charger connected to device>
    JDC_CB_EVENT_ETHERNET_STATUS_CHANGED         //!< Triggered whenever there is a change in ethernet status
} jdc_client_system_events_e;

/*! \brief This enum lists down Different Network Interface types*/
typedef enum
{
	JDC_NETWORK_INTERFACE_INVALID = -1,
	JDC_NETWORK_INTERFACE_WIFI,
	JDC_NETWORK_INTERFACE_LAN,
	JDC_NETWORK_INTERFACE_CELLULAR,
	JDC_NETWORK_INTERFACE_BLUETOOTH

} jdc_client_network_interface_type_e;

/*! \brief This enum lists down Different system Modes available*/
typedef enum
{
    JDC_SYSTEM_MODE_NOT_SUPPORTED = -1,
    JDC_SYSTEM_MODE_FAILURE = 0,	//!< Failed to get System Mode
    JDC_SYSTEM_MODE_ACTIVE = 1,		//!< System is active and running. Ex: Performance mode in case of JMR (or) Ignition ON in case of JVT.
    JDC_SYSTEM_MODE_IGNITION_OFF,	//!< Ignition mode Off
    JDC_SYSTEM_MODE_DEEP_SLEEP,		//!< Deep sleep mode Enabled.
    JDC_SYSTEM_MODE_CELLULAR_POWER_SAVE,	//!< Cellular power save Enabled.
    JDC_SYSTEM_MODE_BATTERY_SAVE	//!< Battery Save Enabled.

} jdc_client_system_mode_e;

/*! \brief This enum lists down modules having language support*/
typedef enum
{
    JDC_MODULE_TYPE_INPUT_EDITOR_E = 0,	//!< Keyboard
    JDC_MODULE_TYPE_SETTINGS_E			//!< Device Settings
} jdc_client_module_type_e;

/**
 *  Enum to indicate various Ethernet duplex modes
 */
 typedef enum
{
    JDC_DUPLEX_MODE_FULL    = 1,  //!< Represents ethernet full duplex mode */
    JDC_DUPLEX_MODE_HALF    = 2,  //!< Represents ethernet half duplex mode */
    JDC_DUPLEX_MODE_UNKNOWN = 3   //!< Represents unknown duplex mode */
} jdc_client_ethernet_duplex_mode_e;

/*! \brief This enum lists down various Wifi related Events */
typedef enum
{
	JDC_CB_EVENT_WIFI_INVALID = -1,
	JDC_CB_EVENT_WIFI24_NEIGHBOUR_AP_INFO_AVAILABLE = 0x01, //!<The event is triggered to notify Access points visible on the 2.4GHz band
	JDC_CB_EVENT_WIFI50_NEIGHBOUR_AP_INFO_AVAILABLE = 0x02  //!<The event is triggered to notify Access points visible on the 5.0GHz band	
} jdc_client_wifi_event_e;

/* ===============================ENUM END====================================================== */


/* ===============================STRUCTURE START====================================================== */

typedef struct
{
	int16_t data;						/**< Data will contain the value of respective input b_info.\n
				\par Ex: <br>
				Based on jdc_client_battery_e enum, data should contain following values.\n
				JDC_BATTERY_TEMPERATURE : Temperature in °C.\n
				JDC_BATTERY_STATE : Charging state ( 0 - Discharging, 1 - Charging, -1 - Unknown). \n
				JDC_BATTERY_LEVEL : Power level (0-100)% \n
				JDC_BATTERY_VOLTAGE : Battery Voltage in milli volts \n
				JDC_BATTERY_PRESENSE : Battery availablitiy \n
				JDC_BATTERY_HEALTH : Battery health (Refer the enum jdc_battery_health_status_e)
				*/
	jdc_client_battery_e b_info;	//!< This value will passed as input.
} jdc_client_batteryinfo_t;

typedef struct
{
	uint64_t tx_bytes;	//!< Transmit data (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB) 
	uint64_t rx_bytes;	//!< Receive data (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB) 
} jdc_client_transmit_receive_info_t;
#define txrxInfo_initializer {0, 0} /** @hideinitializer */


typedef struct
{
	jdc_client_battery_notifications_e noti;
	void* info;		//!< Will contain additonal info based on type of notification. Ex: valid power level for Battery Low Notification.
}jdc_client_battery_noti_details_t;

typedef struct
{
	int8_t status; 	//!< 1 - Success, 0 - Failure, -1 - Invalid
	jdc_client_time_t time_details;
} jdc_client_fota_update_status_t;

typedef struct
{
	char mac[JDC_MAC_ADDRESS_LENGTH];
	char ipv4_address[JDC_IPV4_ADDRESS_LENGTH];
	char ipv6_address[JDC_IPV6_ADDRESS_LENGTH];
} jdc_client_iface_details_t;
#define ifaceDetails_initializer {{'\0'}, {'\0'}, {'\0'}} /** @hideinitializer */

typedef struct
{
	int8_t band_type;				//!< Indicates Wifi Band either JDC_WIFI_BAND_24(2.GHZ (or) JDC_WIFI_BAND_50(5.0GHZ)
	bool wan_IPV6_availability;		//!< true if Available, false if Not
	uint32_t bytes_txed;			//!< Number of bytes transmitted from Wifi radio interface (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB)
	uint32_t bytes_rxed;			//!< Number of bytes received from Wifi radio interface (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB)
	uint32_t pkts_txed;				//!< Number of Packets transmitted from Wifi radio interface
	uint32_t pkts_rxed;				//!< Number of packets received from Wifi radio interface
	uint32_t errors_txed;			//!< Number of error packets transmitted on Wifi radio interface
	uint32_t errors_rxed;			//!< Number of error packets received on Wifi radio interface
	uint32_t clients_connected;		//!< Number of clients connected to wireless radio
	uint32_t channel_used;			//!< Wifi Channel Used
} jdc_client_wifi_statistics_t;
#define wifiDetails_initializer {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0} /** @hideinitializer */

typedef struct
{
	jdc_client_system_events_e evt;
	void* info;		/**< Additional Info which can contain additional data based on the type of event.\n
						Example \n
						JDC_CB_EVENT_CONFIG_UPDATE  - info can contain DC configuration data \n
						JDC_CB_EVENT_WIFI_TOGGLE - info contains bool value to indicate Wifi Enable/Disable \n
                        JDC_CB_EVENT_ETHERNET_STATUS_CHANGED - info contains the ethernet status @ref jdc_client_ethernet_status_info_t \n
                        JDC_CB_EVENT_FOTA_UPDATE_FAILED - info contains the FOTA failure reason in plain text form
					*/
}jdc_client_system_evt_details_t;
#define systemEventDetails_initializer {JDC_CB_EVENT_SYSTEM_ALERT_INVALID, NULL} /** @hideinitializer */

typedef struct {
    bool linkStatus;                    //!< Represents linkstatus false - down, true - up */
    uint32_t speed;                     //!< Represents speed in Mbps */
    jdc_client_ethernet_duplex_mode_e duplex;  //!< Represents duplex state - full,half*/
} jdc_client_ethernet_status_info_t;
#define ethernetStatusInfo_initializer { false, 0, JDC_DUPLEX_MODE_UNKNOWN } /** @hideinitializer */

typedef struct
{
	jdc_client_wifi_event_e evt;
	void* info;		/**< Ex: \n
						//For neighbouring Wifi AP details info should be in JSON format [{"bssid" : "string","ssid" : "string","ch" : number, "rssi" : number}]
						JDC_CB_EVENT_WIFI24_NEIGHBOUR_AP_INFO_AVAILABLE - List of AP's available under the 2.4GHZ band in the above JSON format.
						JDC_CB_EVENT_WIFI50_NEIGHBOUR_AP_INFO_AVAILABLE - List of AP's available under the 5.0GHZ band in the above JSON format.
						
					*/
}jdc_client_wifi_event_details_t;
#define wifiEventDetails_initializer {JDC_CB_EVENT_WIFI_INVALID, NULL} /** @hideinitializer */

/* ===============================STRUCTURE END====================================================== */


/* ====================================== FUNCTION START ======================================= */

/**
 * \brief This API is used to get device up time in seconds since the last boot.
 * \param void
 * \return device uptime in seconds (or)\n
 * 0 : If data not available (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int64_t jdc_client_getDeviceUptime(void);



/**
 * \brief Function to get deviceNetwork Interface details like MAC, IPV4/IPV6 address.
 * \param[in]  interfaceType- Network Interface type(Wlan, Lan, rmnet etc.)
 * \param[out]  deviceDetails - output data will be populated to this jdc_client_iface_details_t pointer.
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_get_DeviceNetworkInterfaceDetails(jdc_client_network_interface_type_e interfaceType, jdc_client_iface_details_t* deviceDetails);



/**
 * \brief Function to get the current Wi-fi connection status of the device.
 * \param void
 * \return Wi-Fi connection status indicating states as per  jdc_client_wifi_state_e (or)JDC_WIFI_STATE_INVALID(if Api not supported)
 */
jdc_client_wifi_state_e jdc_client_getWifiState(void);



/**
 * \brief Function to get Wi-fi band when WiFi turned ON.
 * \param void
 * \return Wi-Fi band value.\n
 * JDC_WIFI_BAND_24 (or) JDC_WIFI_BAND_50 (or) JDC_WIFI_BAND_DUAL (or) JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getWifiBand(void);



/** 
 * \brief Function to obtain connected clients details on Wifi.
 * \param[in] band_type to identify which WIFI band JDC_WIFI_BAND_24 (2.4GHz) or JDC_WIFI_BAND_50 (5GHz)
 * \return List of connected client details in Array of json string format (or) NULL in case if API not supported.\n
 * Format Expected:  [{"mac": "string", "rssi": INTEGER, "txrate":INTEGER, "max_txrate":INTEGER, "txbytes":BIGINT, "rxbytes":BIGINT, "NxN":"string"}, {...}]\n
 * NOTE: For NBIoT Devices, txbytes/rxbytes Data unit will be in KB and for other devices it will be in MB.
 * 		 txrate/max_txrate will be in Mbps
 * N*N - Number of streams supported by wifi clients. Ex: 1*1, 2*2 etc.\n
 * Ex : [ { "mac" : "12:fc:14:ef:06:58", "rssi" : -73, "txrate" : 78.0, "max_txrate" : 130.0, "txbytes":4356, "rxbytes":67534,"NxN" : "2x2"}]\n
 * <B>Note</B>: Caller should take care of deleting the Connected Client details memory returned.
 */
char* jdc_client_getWifiConnectedClientDetails(uint16_t band_type);



/**
 * \brief Function to obtain WAN statistics from last reboot.
 * \param[in] band_type to identify which WIFI band JDC_WIFI_BAND_24 (2.4GHz) or JDC_WIFI_BAND_50 (5GHz)
 * \param[out] stats to get the details
 * \return
 * JDC_API_SUCCESS in case of success (or)\n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getWifiStatistics(uint16_t band_type, jdc_client_wifi_statistics_t* stats);



/**
 * \brief This API is used to get the number of clients Connected to Wired LAN.
 * \param NA
 * \return Num of clients connected (or)JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getNumberOfLANClientsConnected(void);



/**
 * \brief This API is used to get the LAN Client Traffic Information from last reboot.
 * \param NA
 * \return Traffic information of each connected LAN client in following Format ( Array of Json string):\n
	[{"mac": "string", "txbyte": BIGINT, "rxbyte":BIGINT}]   (or) NULL if API not supported\n
	NOTE: For NBIoT Devices, txbyte/rxbyte Data unit will be in KB and for other devices it will be in MB
 * <B>Note</B>: Caller should take care of deleting the Lan Client Traffic details memory returned.
 */
char* jdc_client_getLANClientTrafficInformation(void);



/**
 * \brief This API is used to  get the Device temperature.
 * \param void
 * \return device temperature in centigrade (or) JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getDeviceTemperature(void);



/**
 * This is used to get the battery value based on input BatteryEnum. Result will be added to data variable which is passing in the structure.
 * \param[in,out] jdc_client_batteryinfo_t - BatteryEnum value must be passed in the structure in order to get the output.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getBatteryStatus(jdc_client_batteryinfo_t *data);



/**
 * \brief Function to check whether the device able to communication with Fota/TR69 server
 * \param void
 * \return
 * 1 : If server is reachable (or) \n
 * 0 : Unreachable (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getFotaServerConnectionStatus(void);



/**
 * \brief This Api is used to register callback for getting Notifications about System Alerts.
 * \param[in] callback function which informs system alerts jdc_client_system_alerts_t.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForSystemAlerts(void (*fn)(jdc_client_system_evt_details_t*));



/**
 * \brief This Api is used to De-register callback  which was registerd for getting System Alerts.
 * \param[in] callback function which was registered for system listeners jdc_client_system_alerts_t.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deregisterForSystemAlerts(void (*fn)(jdc_client_system_evt_details_t*));



/**
 * \brief API to check number of times device got rebooted in last 24Hrs.
 * \param void
 * \return Reboot count in last 24Hrs (or) JDC_API_NOT_SUPPORTED
 */
int16_t jdc_client_getRebootCount(void);



/**
 * \brief This method is used to get Transmit and Receive details like Transmit Bytes, Receive Bytes etc for the Network interface type passed from the last reboot.
 * \param[in] interfaceType - Network interface type for which traffic data is required/
 * \param[out] tx_rxdata pointer must be passed. Output data will added to this struct variable.
 * 				NOTE: For NBIoT Devices Data unit will be in KB and for other devices it will be in MB
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getTransmitReceiveDetails( jdc_client_network_interface_type_e interfaceType,
		jdc_client_transmit_receive_info_t *tx_rxdata);



/**
 * \brief This Api is used to register callback for getting Notifications about Internal Battery status.
 * \param[in] callback function which informs Intenal Battery status jdc_client_battery_noti_details_t.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForBatteryAlerts(void (*fn)(jdc_client_battery_noti_details_t*));



/**
 * \brief This Api is used to De-register callback  which was registerd for getting Internal Battery Notifications.
 * \param[in] callback function to be de-registered.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deregisterForBatteryAlerts(void (*fn)(jdc_client_battery_noti_details_t*));



/**
 * \brief This API is used to get the details about the current System Mode.
 * \param[in] NA
 * \return
 * 	jdc_client_system_mode_e value containing current system mode.\n
 *  JDC_SYSTEM_MODE_FAIURE if api failed to get the requested data (or) \n
 *  JDC_SYSTEM_MODE_NOT_SUPPORTED if API is not supported.
 */
jdc_client_system_mode_e jdc_client_getSystemModeStatus(void);



/** 
 * @deprecated since v4.7
 * \brief This API is used to acquire wakelock to prevent device going into sleep mode.
 * \param void
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_acquireWakeLock(void);



/** 
 * @deprecated since v4.7
 * \brief This API is used to release the wakelock to allow the device to enter into sleep mode.
 * \param void.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_releaseWakeLock(void);



/** 
 * @deprecated since v4.7
 * \brief This API is used to read the DC configuaration bitmask.
 * \param[out] bitmask pointer for reading DC configuaration data.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_read_bitmask(char* bitmask);



/**
 * 
 * \brief To check out whether device is in airplane mode or not.
 * \param void
 * \return
 * 0 : Not in Airplane Mode (or) \n
 * 1 : Airplane mode (or) \n
 * JDC_API_NOT_SUPPORTED  if API is not supported.
 */
int16_t jdc_client_isAirplaneMode(void);



/**
 * \brief Function to get the device last FOTA update status.
 * \param[out] jdc_client_fota_update_status_t pointer must be passed. Output data will added to this struct variable.
 * Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
 int16_t jdc_client_getFotaUpdateStatus(jdc_client_fota_update_status_t* fota_status);



/**
 * \brief This API is used to get Language used by the module (keyboard, settings etc.).
 * \param[in] module_type - Query module to obtain language used
 * \return Language used based on the module type passed( Input editor, settings etc..) (or) NULL if API not supported\n
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 */
char* jdc_client_getLanguageForModule(jdc_client_module_type_e module_type);


/**
 * \brief API to get number of FOTA server connection failures ocurred while firmware update procedure in last 24 hours.
 * \param void
 * \return FOTA Connection Failures count (or) JDC_API_NOT_SUPPORTED
 */
int16_t jdc_client_getFotaConnectionFailuresCount(void);


/**
 * \brief Function to get the Wi-Fi Steering Mode.
 * \param[out]  mode - output data indicates Wifi Steering Mode(1 if Steering is Enabled, 0 otherwise).
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getWifiSteeringMode(int16_t *mode);


/**
 * \brief This Api is used to register a callback for Wifi related events.
 *  		Currently this A is applicable for Neighbouring AP events only.
 * \param[in] frequency (in minutes) to indicate how periodic callbacks should be triggered. 
 * \param[in] callback function which informs about Wifi related  events.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForWifiEvents(uint16_t frequencyInMins, void (*fn)(jdc_client_wifi_event_details_t*));


/**
 * \brief This Api is used to De-register callback which was registered for getting Wifi related events.
 * \param[in] callback function pointer to deregister
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deRegisterForWifiEvents(void (*fn)(jdc_client_wifi_event_details_t*));


/**
 * \brief This API is used to get wifi operating mode info.
 * \param[in] band_type to identify which WIFI band JDC_WIFI_BAND_24 (2.4GHz) or JDC_WIFI_BAND_50 (5.0GHz)
 * \param[out] operating mode info about operating standard(os), auto channel(ac), broadcast ssid of primary Ap(bc_ssid) etc. \n
 * in below format [{"AP<num>" : number, "os" : "string", "bw" : "string", "bc_ssid" : number, "ac" : number}]
 *  Example format: [{"AP1": 1,"os": "n","bw": "40","bc_ssid": 1,ac": 1}]	
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 */
int16_t jdc_client_getWifiOperatingModeInfo(uint16_t band_type, char** info);

/**
 * \brief The Api checks whether device is tampered.
 * SW tampering examples: 
 * 	# Stack Corruption
 *  # Device reboot or continuous hang/freeze.
 *  # Random data corruption.
 * 
 * \param void
 * \return
 * 1 : If Device is Tampered (or) \n
 * 0 : If Device Not Tampered (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_isDeviceTampered(void);

/**
 * \brief API to get the Time taken in seconds to register with celluar network after booting up.
 * \param void
 * \return time taken (in seconds) for Registration (or) JDC_API_NOT_SUPPORTED if API is Not supported.
 */
int32_t jdc_client_getRegistrationTime(void);

/**
 * \brief This API is used to get the reason for last reboot.
 * \param NA
 * \return Reason for reboot (or) NULL if API is not supported. \n
 * Possible values are:\n
 * 		FOTAreboot, Normal, Unknown (in following cases) : \n
 *	1) Reboot due to low battery \n
 *	2) Battery removal \n
 *	3) Kernel panic \n
 *	4) System crash \n
 * <B>Note</B>: Caller should take care of deleting the Reboot Reason details memory returned.
 */
char* jdc_client_getRebootReason(void);


/* ====================================== FUNCTION END ======================================= */

#if defined(__cplusplus)
}
#endif

#endif /* JDC_CLIENT_DEVICE_H_ */

/* ===============================END OF THE FILE=============================================== */
