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
* @file jdc_client_network.h
* @brief API's for capturing network, signal related events and params for LTE and NR.
*/

/*  ===============================END ABOUT THIS FILE=========================================== */

/// \file
#ifndef JDC_CLIENT_NETWORK_H_
#define JDC_CLIENT_NETWORK_H_

#if defined(__cplusplus)
 extern "C" {
#endif

#include "jdc_client_main.h"

/* ===============================#DEFINE START================================================= */
#define JDC_EDRX_LENGTH			10
#define JDC_PTW_LENGTH			10
#define JDC_ATTACHTYPE_LENGTH	10
#define JDC_PSM_LENGTH			10
#define JDC_MAX_CHANNEL_LENGTH 10
/* ===============================#DEFINE END=================================================== */



/* ===============================ENUM START==================================================== */

/*! \brief This enum lists down the Data service states of the network*/
typedef enum
{
	JDC_DATA_STATE_NOT_SUPPORTED = -2,
	JDC_DATA_STATE_INVALID = -1,
	JDC_DATA_OUT_OF_SERVICE = 0,
	JDC_DATA_IN_SERVICE,
	JDC_DATA_IN_SERVICE_EMERGENCY

} jdc_client_data_service_state_e;

/*! \brief This enum lists down the various 4G Network async events*/
typedef enum
{
	JDC_CB_EVENT_OUTOFSERVICE = 0x0001,	//!<Service loss indication to higher layers
	JDC_CB_EVENT_FULLSERVICE = 0x0002, //!<	Device recovers to service from Loss of Service or Limited Service
	JDC_CB_EVENT_ATTACHFAILURE = 0x0003, //!<Device received attach failure from Network
	JDC_CB_EVENT_RLFAILURE = 0x0004, //!<Device declared Radio Link Failure
	JDC_CB_EVENT_RACHFAILURE = 0x0005, //!<	Rach failures during RRC Connection Request
	JDC_CB_EVENT_DATASTALL = 0x0006, //!< Deprecated IGNORE it, not in use. Data Session paused; if RLC state variables do not change for 5 secs then the event can be triggered or if the received packet number is increasing or not in RLC PDU stats.
	//JDC_CB_EVENT_MQTTCONFAIL = 0x0007, //!<	MQTT Connection Failure
	JDC_CB_EVENT_DATASESSION_END = 0x0008, //!<Data Session Ended
	JDC_CB_EVENT_TAC_UPDATE = 0x0009, //!<Tracking Area Update
	JDC_CB_EVENT_LOW_RADIO_SIGNAL = 0x000A, //!<The event is triggered when any of the radio signals: RSRP, RSRQ & SINR reaches min-threshold value \n
									//!<RSRP- If the (currentRSRP < -110 dBm) on app start then the NE16 will be triggered \n
									//!<If the (currentRSRP < -110 dBm) AND (currentRSRP - previousRSRP) is < -10dbm the NE16 will be triggered and reference(previousValue) value will be updated for next NE16 event \n
									//!<If the (currentRSRP > -110 dBm) AND (currentRSRP - previousRSRP) is > +10dbm the reference(previousValue) value will be updated for next NE16 event. \n
									//!<RSRQ- If RSRQ report -15dB first event will trigger and then every -2dB change from previous RSRQ. \n
									//!<SINR- If SINR report -8dB first event will trigger and then every -3dB change from previous SINR.
	JDC_CB_EVENT_RRC_CONN_RELEASE = 0x000B, //!<	RRC Connection is released
	JDC_CB_EVENT_RRC_CONN_FAILURE = 0x000C, //!< Failure in establishing the RRC connection (during RRC Connection Setup, RRC Reconfiguration and RRC Reestablishment)
	JDC_CB_EVENT_INTRA_FREQ_HANDOVER = 0x000D, //!<	Intra frequency handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_INTER_FREQ_HANDOVER = 0x000E, //!<	Inter frequency handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_INTER_BAND_HANDOVER = 0x000F, //!<	Inter band handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_CELL_RESELECTION = 0x0010, //!<  cell relection procedure is completed in both successful and failure scenarios

	JDC_CB_EVENT_ATTACH_SUCCESS =  0x0011, //!< Successful attach of device with the network
	JDC_CB_EVENT_NONRECOVERABLE_DATASTALL = 0x0012 //!< Deprecated IGNORE it, not in use. The event is triggered when a data session stalls  and does not resume; if RLC state variables do not change for 30 secs then the event can be triggered

} jdc_client_radio_notifications_e;

/*! \brief This enum lists down the various 5G Network async events*/
typedef enum
{
	JDC_CB_EVENT_NR_OUTOFSERVICE = 0x0001,	//!<Service loss indication to higher layers
	JDC_CB_EVENT_NR_FULLSERVICE = 0x0002, //!<	Device recovers to service from Loss of Service or Limited Service


	JDC_CB_EVENT_NR_RLFAILURE = 0x0003, //!<Device declared Radio Link Failure
	JDC_CB_EVENT_NR_RACHFAILURE = 0x0004, //!<	Rach failures during RRC Connection Request
	//JDC_CB_EVENT_MQTTCONFAIL = 0x0007, //!<	MQTT Connection Failure
	JDC_CB_EVENT_NR_LOW_RADIO_SIGNAL = 0x0005,  //!<The event is triggered when any of the NR radio signals: NR-RSRP, NR-RSRQ & NR-SINR reaches min-threshold value
									//!NR_RSRP- <If the (currentRSRP < -110 dBm) on app start then the NE16 will be triggered \n
									//!<If the (currentRSRP < -110 dBm) AND (currentRSRP - previousRSRP) is < -10dbm the NE16 will be triggered and reference(previousValue) value will be updated for next NE16 event \n
									//!<If the (currentRSRP > -110 dBm) AND (currentRSRP - previousRSRP) is > +10dbm the reference(previousValue) value will be updated for next NE16 event.
									//!<NR-RSRQ- If RSRQ report -20dB first event will trigger and then every -3dB change from previous RSRQ. \n
									//!<NR-SINR- If SINR report -10dB first event will trigger and then every -3dB change from previous SINR.  
	JDC_CB_EVENT_NR_RRC_CONN_RELEASE = 0x0006, //!<	RRC Connection is released
	JDC_CB_EVENT_NR_RRC_CONN_FAILURE = 0x0007, //!< Failure in establishing the RRC connection (during RRC Connection Setup, RRC Reconfiguration and RRC Reestablishment)
	JDC_CB_EVENT_NR_INTRA_FREQ_HANDOVER = 0x0008, //!<	Intra frequency handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_NR_INTER_FREQ_HANDOVER = 0x0009, //!<	Inter frequency handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_NR_INTER_BAND_HANDOVER = 0x000A, //!<	Inter band handover procedure is completed; both in successful and failure cases
	JDC_CB_EVENT_NR_CELL_RESELECTION = 0x000B, //!<  cell relection procedure is completed in both successful and failure scenarios

	JDC_CB_EVENT_NR_DATASTALL = 0x000C, //!< Deprecated IGNORE it, not in use. Data Session paused. if RLC state variables do not change for 5 secs then the event can be triggered or if the received packet number is increasing or not in RLC PDU stats.
	JDC_CB_EVENT_NR_DATASESSION_END = 0x000D, //!<Data Session Ended. No Active DRB (data radio bearer)

	JDC_CB_EVENT_NR_REGISTRATION_FAILURE = 0x000E, //!< Triggered when REGISTRATION failure happens
	JDC_CB_EVENT_NR_PDU_SESSION_ESTABLISHMENT_FAILURE = 0x000F, //!<  Triggered  when a PDU SESSION ESTABLISHMENT failure happens
	JDC_CB_EVENT_NR_REGISTRATION_SUCCESS = 0x0010, //!<  Triggered after successful Registration of device with the network

	//FR2 events
	JDC_CB_EVENT_NR_FR2_BEAM_FAILURE_DETECTION = 0x0020, //!< Triggered when there is a Beam failure detected.
	JDC_CB_EVENT_NR_FR2_BEAM_FAILURE_RECOVERY	 //!< Triggered when there is a Beam recovery detected.
	
} jdc_client_NR_notifications_e;

/*! \brief This enum lists down additonal info about low radio signal which are applicable for JDC_CB_EVENT_LOW_RADIO_SIGNAL and JDC_CB_EVENT_NR_LOW_RADIO_SIGNAL events*/
typedef enum
{
	JDC_SIGNAL_TYPE_LOW_RSRP = 0x0001,
	JDC_SIGNAL_TYPE_LOW_RSRQ = 0x0002,
	JDC_SIGNAL_TYPE_LOW_SINR = 0x0003

} jdc_client_low_radio_signal_type_e;

/*! \brief This enum lists down Additonal info about Attach Failures which are applicable for JDC_CB_EVENT_ATTACHFAILURE event*/
typedef enum
{
	JDC_CLIENT_SUCCESS = 0x00,
	JDC_CLIENT_IMSI_UNKNOWN = 0x02,
	JDC_CLIENT_ILLEGAL_UE,
	JDC_CLIENT_ILLEGAL_ME = 0x06,
	JDC_CLIENT_EPS_SERVICES_NOT_ALLOWED = 0x07,
	JDC_CLIENT_EPS_NON_EPS_SERVICES_NOT_ALLOWED = 0x08,
	JDC_CLIENT_PLMN_NOT_ALLOWED = 0x0B,
	JDC_CLIENT_TA_NOT_ALLOWED,
	JDC_CLIENT_ROAMING_NOT_ALLOWED_TA,
	JDC_CLIENT_EPS_SERVICES_NOT_ALLOWED_PLMN,
	JDC_CLIENT_NO_SUITABLE_CELLS_TA,
	JDC_CLIENT_MSC_TEMP_NOT_REACHABLE,
	JDC_CLIENT_NETWORK_FAILURE,
	JDC_CLIENT_CS_DOMAIN_NOT_AVAILABLE,
	JDC_CLIENT_CONGESTION = 0x16,
	JDC_CLIENT_SERVICEOPTION_NOT_AUTHORIZED_PLMN = 0x23,
	JDC_CLIENT_NO_EPS_BEARER_CTX_ACTIVATED = 0x28,
	JDC_CLIENT_SERVICE_NETWORK_FAILURE = 0x2A,
	/* PROTOCOL ERRORS */
	JDC_CLIENT_INVALID_MANDATORY_INFO = 0x60,
	JDC_CLIENT_IE_NON_EXIST_NOT_IMPL = 0x63,
	JDC_CLIENT_CONDITIONAL_IE_ERROR,
	JDC_CLIENT_PROTOCOL_ERROR_UNSPEC,

	JDC_CLIENT_ATTACH_FAIL
} jdc_client_attach_failure_responses_e;

/*! \brief This enum lists down various Sim identifcation status*/
typedef enum
{	
	JDC_SIM_STATUS_INVALID = -1,
	JDC_SIM_NOSIM,
	JDC_SIM_BLOCKED,
	JDC_SIM_STATUS_WORKING,
	JDC_SIM_NOT_PROVISIONED,
	JDC_SIM_UNKNOWN = 10
} jdc_client_sim_identification_status_e;

/*! \brief This enum lists down data Connection status values*/
typedef enum
{
	JDC_DATA_STATUS_INVALID = -1,
	JDC_DATA_STATUS_DISCONNECTED = 0,
	JDC_DATA_STATUS_CONNECTED = 1
} jdc_client_data_connection_status_e;

/*! \brief This enum lists down SIP registration status values*/
typedef enum
{
	JDC_SIP_STATUS_NOT_SUPPORTED_E = -1,
	JDC_SIP_STATUS_UNKNOWN_E = 0,
	JDC_SIP_STATUS_NOT_REGISTERED_E,
	JDC_SIP_STATUS_REGISTERING_E,
	JDC_SIP_STATUS_AUTH_CHALLENGE_E,
	JDC_SIP_STATUS_AUTH_REGISTER_E,
	JDC_SIP_STATUS_REGISTERED_E,
	JDC_SIP_STATUS_NOTIFIED_E
} jdc_client_sip_registration_status_e;

/*! \brief This enum lists down the various Telephony async events*/
typedef enum
{
	//Call Related
	JDC_CB_NOTI_CALL_TRIGGER_E = 0x0001,	//!<The event is triggerred when a call is initiated.Notes: This should cover every Call Request to Modem i.e. RIL to Modem.
	JDC_CB_NOTI_CALL_ATTEMPTFAILED_E, 		//!<The event is triggered when a call attempt fails. Notes: This should cover all the call failures if it's not connected.
	JDC_CB_NOTI_CALL_ESTABLISHED_E, 		//!<The event is triggered when a call is successfully established.
	JDC_CB_NOTI_CALL_DISCONNECT_E, 		//!<The event is triggerred when a call is terminated. It covers both successful and failure scenarios. It will indicate only successful call terminations.Notes: This should cover every graceful Call Disconnect be it triggered by MO or MT side.
	JDC_CB_NOTI_CALL_DROP_E, 				//!<The event is triggered when a call drops. The event will capture both RAN induced and IMS induced causes.Notes: This should cover abnormal call drop.
	JDC_CB_NOTI_CALL_MUTING_E, 			//!<The event is triggered when consecutive RTP packet loss is seen for more than 2 secs.

	//SMS Related
	JDC_CB_NOTI_SMS_SENT_E,				//!<The event is triggered when a SMS is sent
	JDC_CB_NOTI_SMS_RECEIVED_E,			//!<The event is triggered when a SMS is received

	//Volte Related
	JDC_CB_NOTI_VOLTE_REGISTRATION_E,		//!<The event is triggered when VoLTE registration is complete either in success or failure.Notes: This should basically cover the status of IMS REG procedure.
	JDC_CB_NOTI_VOLTE_CONNECTION_LOST_E	//!<The event is triggered when VoLTE registration is lost due to abnormal reasons including Network de-registration.Notes: This should basically cover IMS loss other than IMS procedures e.g. NW initiated IMS loss e.g. Default IMS bearer is lost (re-activation required)
} jdc_client_telephony_notifications_e;

/*! \brief This enum lists down RAT Types*/
typedef enum
{
	JDC_RAT_TYPE_INVALID = -1,
	JDC_RAT_TYPE_GSM = 0,  	//!< 2G Radio Access Technology
	JDC_RAT_TYPE_UMTS,		//!< 3G Radio Access Technology
	JDC_RAT_TYPE_LTE,		//!< 4G Radio Access Technology
	JDC_RAT_TYPE_NR,		//!< 5G/NR Radio Access Technology
	JDC_RAT_TYPE_NBIOT,		//!< NB-IOT Radio Access Technology
	JDC_RAT_TYPE_WBIOT,		//!< WB-IOT Radio Access Technology
	JDC_RAT_TYPE_NR_FR2		//!< 5G NR FR2 (mmWave)
} jdc_client_RAT_type_e;


/* ===============================ENUM END====================================================== */


/* ===============================STRUCTURE START====================================================== */

typedef struct
{
	int16_t sigStrength;					//!< dBm value.
	int16_t sigIndication;					//!< RSSI value if data registered in GSM/LTE/CDMA
	int16_t sigQuality;						//!< RSRQ value if data registered in LTE
	int16_t sigPower;						//!< RSRP value if data registered in LTE
	int16_t sigNoise;						//!< SINR value if data registered in LTE
	uint16_t earfcn;						//!< EARFCN Frequency Used by the device
	int16_t rachMaxPower;					//!< Maximum power used for the latest RACH transmission to set up the RRC connection resulting either in successful or failed instances
	int16_t bler;							//!< Residual BLER at the physical layer
	int16_t cqi;							//!< Channel Quality Indicator
	int16_t timingAdvance;					//!< Current timing advance used by the device to communicate with the eNB.
	int16_t bandIndication;					//!< Current band used by the device. 0 indicates no band information.
	int16_t txPower; 						//!< Transmit power of the device at the time of reading.
} jdc_client_signal_details_t;
#define signalDetails_initializer {-1, -1, INT16_MIN, -1, INT16_MIN, 0, -1, -1, -1, -1, -1, INT16_MIN} /** @hideinitializer */

typedef struct
{
	int16_t nr_sigIndication;				//!< RSSI value if data registered in NR
	int16_t nr_sigQuality;					//!< RSRQ value if data registered in NR
	int16_t nr_sigPower;					//!< RSRP value if data registered in NR
	int16_t nr_sigNoise;					//!< SINR value if data registered in NR
	int16_t nr_rachMaxPower;				//!< Maximum power used for the latest RACH transmission to set up the RRC connection resulting either in successful or failed instances
	int16_t nr_bler;						//!< Residual BLER at the physical layer
	int16_t nr_cqi;							//!< Channel Quality Indicator	
	int16_t nr_timingAdvance;				//!< Current timing advance used by the device to communicate with the eNB.
	int16_t nr_bandIndication;				//!< Current band used by the device. 0 indicates no band information.
	int16_t nr_txPower; 					//!< Transmit power of the device at the time of reading.
	uint32_t nr_arfcn;						//!< Frequency Used by the device
} jdc_client_NR_signal_details_t;
#define nrSignalDetails_initializer {-1, INT16_MIN, -1, INT16_MIN, -1, -1, -1, -1, -1, INT16_MIN, 0} /** @hideinitializer */

typedef struct
{
	jdc_client_radio_notifications_e noti;
	void* info;		/**< Contain additional data based on the type of Notification.\n
						Ex: \n
						JDC_CB_EVENT_ATTACHFAILURE  - info can obtain responses from jdc_client_attach_failure_responses_e
						JDC_CB_EVENT_TAC_UPDATE  - TAC Upate failure reasons as per 3gpp specfs. 0 incase of TAC update success.
						JDC_CB_EVENT_LOW_RADIO_SIGNAL - info can obtain responses from jdc_client_low_radio_signal_type_e
					*/
}jdc_client_radio_noti_details_t;

typedef struct
{
	jdc_client_NR_notifications_e noti;
	void* info;		/**< Contain additional data based on the type of Notification.\n
						JDC_CB_EVENT_NR_FR2_BEAM_FAILURE_DETECTION - info will have reason to failure in string form.
						JDC_CB_EVENT_NR_LOW_RADIO_SIGNAL - info can obtain responses from jdc_client_low_radio_signal_type_e
						JDC_CB_EVENT_NR_REGISTRATION_FAILURE - info can have the REGISTRATION Failure Code(integer).Refer 24.501 version 15.16.0 of 3GPP spec
						JDC_CB_EVENT_NR_PDU_SESSION_ESTABLISHMENT_FAILURE - info can have PDU Session Establishment Failure Code(integer). Refer 24.501 version 15.16.0 of 3GPP spec
						
					*/
}jdc_client_NR_noti_details_t;

typedef struct
{
	char eDRX[JDC_EDRX_LENGTH];				//!< Extended Idle Modem Discontinous Reception
	char ptw[JDC_PTW_LENGTH];				//!< Paging Time Window
	char attachType[JDC_ATTACHTYPE_LENGTH];	//!< ATTACH Type (NIDD /IP)
	char psm[JDC_PSM_LENGTH];				//!< Power Saving Mode
} jdc_client_rrc_connection_details_t;
#define rrcConnectionDetails_initializer {{'\0'}, {'\0'}, {'\0'}, {'\0'}} /** @hideinitializer */

typedef struct
{
	uint64_t numOfTxPackets;                //!< Number of Tx packets
	uint32_t numOfTxPacketsAttemptsFailed; 	//!< Number of Tx packets attempts failed.
	uint64_t numOfRxPackets;				//!< Number of Rx packets
	uint32_t numOfRxPacketsAttemptsFailed; 	//!< Number of Rx packets attempts failed.
	uint64_t bytesTxed;						//!< Number of bytes sent (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB) 
	uint64_t bytesRxed;						//!< Number of bytes received (For NBIoT Devices Data unit will be in KB and for other devices it will be in MB)
	uint32_t dlThroughput;                  //!< Average DL Data Throughput in kbps.
	uint32_t ulThroughput;                  //!< Average UL Data Throughput in kbps.
} jdc_client_data_statistics_t;
#define dataStatistics_initializer {0, 0, 0, 0, 0, 0, 0, 0} /** @hideinitializer */

typedef struct
{
	uint16_t numOfAttachs;				//!< Number of Attachs
	uint16_t numOfAttachAttemptsFailed;	//!< Number of ATTACH attempt failures across all causes
	uint16_t numOfDetachs;				//!< Number of Detachs
} jdc_client_attach_statistics_t;
#define attachStatistics_initializer {0, 0, 0} /** @hideinitializer */

typedef struct
{
	uint16_t numOfRegistrations;				//!< Number of Registrations
	uint16_t numOfRegistrationAttemptsFailed;	//!< Number of Registration attempt failures across all causes
	uint16_t numOfDeregistrations;				//!< Number of DeRegistrations
} jdc_client_NR_registration_statistics_t;
#define registrationStatistics_initializer {0, 0, 0} /** @hideinitializer */

typedef struct
{
	uint32_t ul_datarate;	//!< Uplink Throughput in Kbps
	uint32_t dl_datarate;	//!< Downlink Throughput in Kbps
} jdc_client_throughput_statistics_t;
#define throughputStatistics_initializer {0, 0} /** @hideinitializer */

typedef struct
{
	jdc_client_telephony_notifications_e noti;
	void* info;		/**< Contain additional data based on the type of Notification.\n
						Ex: info contains SIP session End reason/ Code (Specific to Chipset) for events like JDC_CB_EVENT_CALL_ATTEMPTFAILED_E,
							JDC_CB_EVENT_CALL_DISCONNECT_E, JDC_CB_EVENT_CALL_DROP_E
					*/
}jdc_client_telephony_notification_details_t;

typedef struct
{
	uint16_t numOfOutgoingCalls;				//!< Number of Outgoingcalls
	uint16_t numOfIncomingCalls;				//!< Number of Incoming calls
	uint16_t numOfCallAttemptFailures;			//!< Number of Call Attempt Failures
	uint16_t numOfCallDrops;					//!< Number of Call Drops
} jdc_client_telephony_statistics_t;
#define telephonyStatistics_initializer {0, 0, 0, 0} /** @hideinitializer */

typedef struct {
	jdc_client_RAT_type_e rat_type;	//Type of RAT
	bool serving;					//True indicates that the device is currently associated with this cell tower.
	char channel[JDC_MAX_CHANNEL_LENGTH];	//Channel Number (GSM)
	uint16_t mcc; 					//Mobile Country Code (Serving/Neighbouring)
	uint16_t mnc; 					//Mobile Network Code (Serving/Neighbouring)
	uint16_t pci;					//Physical Cell ID
	int16_t rsrp; 					//Received signal strength-power
	int16_t rsrq; 					//Received signal strength-quality
	int16_t rssi; 					//Received signal strength-Indication
	int16_t rscp; 					//Received Signal Code Power (WCDMA)
	int16_t sinr;					//SINR value if data registered in LTE
	uint16_t ta; 					//Timing Advance value for this cell
	uint16_t psc;					//Primary Scrambling Code (GSM)
	uint16_t bsic;					//Base Station Identity Code(GSM) 
	uint16_t frequency;				//Frequency of cell tower(EARFCN)
	uint32_t tac;					//Tracking Area Code
	uint32_t lac;					//Location Area Code
	uint32_t cellId;				//Cellular ID
	uint64_t age;					//Milliseconds since this cell was detected
}jdc_client_cellularInfo_t;

#define cellularInfo_initializer {JDC_RAT_TYPE_INVALID, false, {'\0'}, 0, 0, UINT16_MAX, INT16_MAX, INT16_MAX, INT16_MAX, INT16_MAX, 0, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT64_MAX} /** @hideinitializer */

typedef struct {
	uint8_t beamDetectedCount;		//No of Beam detected
	uint8_t beamReported;			//Beam reported to NW
	int16_t beamTxPower;			//Beam Tx power. Please refer 3GPP spec ts 38.101-2 section 6.2.1 Transmitter characteristics.
	int16_t beamRxPower;			//Note: It is not applicable and ignore it. 
	int16_t beamPower;				//EIRP power of the latched Beam. Please refer 3GPP spec ts 38.101-2 section 6.2.1 Transmitter characteristics.
	uint8_t activeBWPCount;			//No of active BWP assigned to UE.
	uint8_t defaultBWPCount;		//No of default BWP assigned to UE.
	uint16_t beamFailureCount;		//Beam failure count
	uint16_t beamRecoveryCount;		//Beam recovery count
} jdc_client_FR2_beamInfo_t;
#define fr2_beamInfo_initializer {0, 0, 0, 0, 0, 0, 0, 0, 0} /** @hideinitializer */

/* ===============================STRUCTURE END====================================================== */


/* ====================================== FUNCTION START ======================================= */


/**
 * \brief This method is used to get SIM signal details like signal strength, indication etc.
 * \param[out] s_var pointer must be passed. Output data will added to this struct variable.
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getSignalDetails(jdc_client_signal_details_t *s_var);



/**
 * \brief This method is used to get SIM NR signal details like signal strength, indication etc.
 * \param[out] s_var pointer must be passed. Output data will added to this struct variable.
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_NR_getSignalDetails(jdc_client_NR_signal_details_t *s_var);



/**
 * \brief To get the network data service state.
 * \param void
 * \return Jiot_client_DC_data_service_state_e states. \n
 * JDC_DATA_IN_SERVICE (or) \n
 * JDC_DATA_OUT_OF_SERVICE (or) \n
 * JDC_DATA_IN_SERVICE_EMERGENCY (or) \n
 * JDC_DATA_STATE_INVALID (or) \n
 * JDC_DATA_STATE_NOT_SUPPORTED - if API Not supported.
 */
jdc_client_data_service_state_e jdc_client_getDataServiceState(void);



/**
 * \brief This Api used to check if there are issues related to SIM identification/Detection.
 * \param void
 * \return jio_client_DC_sim_identification_status_e. \n
 * JDC_SIM_NOSIM (or) \n
 * JDC_SIM_BLOCKED (or) \n
 * JDC_SIM_UNKNOWN (or) \n
 * JDC_SIM_STATUS_INVALID(if API Not Supported)
 */
jdc_client_sim_identification_status_e jdc_client_getSimIdentificationStatus(void);



/**
 * \brief API to check the Device data connection status.
 * \param NA
 * \return jio_client_DC_data_connection_status_e. \n
 * JDC_DATA_STATUS_CONNECTED (or) \n
 * JDC_DATA_STATUS_DISCONNECTED (or) \n
 * JDC_DATA_STATUS_INVALID (if APi Not supported)
 */
jdc_client_data_connection_status_e jdc_client_getDataConnectionStatus(void);



/**
 * \brief API to get the Network status whether it Attached to network or not.
 * \param NA
 * \return
 * 0 : For DETACH (or) \n
 * 1 : In case of ATTACH (or) \n
 * JDC_API_NOT_SUPPORTED if API is Not Supported.
 */
int16_t jdc_client_getNetworkStatus(void);



/**
 * \brief This is used to get RRC connection details
 * \param[out] rrc_var pointer must be passed. Output data will added to this struct variable.
 * <B>Note</B>: Implementor of this method will allocate and Caller should take care of de-allocating the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getRrcConnectionDetails(
		jdc_client_rrc_connection_details_t *rrc_var);



/**
 * \brief This Api is used to register callback for Network Radio Notifications
 * \param[in] callback function which informs about Radio Notifications mentioned in jdc_client_radio_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForRadioNotifications(void (*fn)(jdc_client_radio_noti_details_t*));



/**
 * \brief This Api is used to register callback for New Radio(5G) Notifications
 * \param[in] callback function which informs about Radio Notifications mentioned in jdc_client_NR_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_NR_registerForRadioNotifications(void (*fn)(jdc_client_NR_noti_details_t*));



/**
 * \brief This Api is used to De-register callback which was registerd for getting Radio Notifications
 * \param[in] callback function which informs about Radio Notifications mentioned in jdc_client_radio_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deRegisterForRadioNotifications(void (*fn)(jdc_client_radio_noti_details_t*));



/**
 * \brief This Api is used to De-register callback for getting Notifications about New Radio(5G)
 * \param[in] callback function which informs about Radio Notifications mentioned in jdc_client_NR_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_NR_deRegisterForRadioNotifications(void (*fn)(jdc_client_NR_noti_details_t*));



/**
 * \brief This API is used to know the cause for RRC Connection initiation.
 * \param NA
 * \return Connection reason (or) NULL if API not supported.\n
 * Possible connection cause reasons: "emergency", "highPriorityAccess", "mt-Access", "mo-Signalling", "mo-Data", "delayTolerantAccess-v1020", "mo-VoiceCall-v1280" \n
 * <B>Note</B>: Caller should take care of deleting the Rrc Connection Cause details memory returned.
 */
char* jdc_client_getRrcConnectionCause(void);



/**
 * \brief This API is used to know the cause for RRC Connection initiation on New Radio.
 * \param NA
 * \return Connection reason (or) NULL if API not supported.\n
 * Possible connection cause reasons: "emergency", "highPriorityAccess", "mt-Access", "mo-Signalling", "mo-Data", "mo-VoiceCall", "mo-VideoCall", "mo-SMS ", " mps-PriorityAccess", "mcs-PriorityAccess" \n
 * <B>Note</B>: Caller should take care of deleting the New Radio Rrc Connection Cause details memory returned.
 */
char* jdc_client_NR_getRrcConnectionCause(void);



/**
 * \brief This API is used to get the EPS bearer details.
 * \param NA
 * \return EPS bearer details in following format [{EPS Bearer-1 struct details}, {EPS Bearer-1 struct details}...].\n
 * 	Structure - {EPS bearer identity, Bearer Type - default/dedicated, EPS QCI, AMBR-DL, AMBR-UL, GBR-DL, GBR-UL}\n
 * Example format:  [ {5, default, 9, 0, 0, 0, 0} {6, default, 5, 0, 0, 0, 0} ] (or)
 *	NULL if API not supported.\n
 *	<B>Note</B>: Caller should take care of deleting the Eps Bearer details memory returned.\n
 */
char* jdc_client_getEpsBearerDetails(void);



/**
 * \brief This API is used to get the Neighbor Cell info stored.
 * \param NA
 * \return Neighboring  cell info details in following format.\n
 *	JSON Array of [Cell ID, RSRP, RSRQ].\n
 *	Example format:  {"lte_cells":[{"cell_id":20,"rsrp":10,"rsrq":20},{"cell_id":30,"rsrp":20,"rsrq":20}],"nr_cells":[{"cell_id":20,"rsrp":10,"rsrq":-20},{"cell_id":20,"rsrp":10,"rsrq":-20}]} \n
 *  For Qualcom chipset PCI will be added in neighboring cell details [Cell ID, PCI, RSRP, RSRQ]. \n
 *  Example for Qualcom {"lte_cells":[{"cell_id":20,"pci":323,"rsrp":10,"rsrq":20},{"cell_id":30,"pci":323,"rsrp":20,"rsrq":20}],"nr_cells":[{"cell_id":20,"pci":323,"rsrp":10,"rsrq":-20},{"cell_id":20,"pci":323,"rsrp":10,"rsrq":-20}]} \n
 *  If there are no neighbouring LTE/NR cells, the respective JSON array will be empty e.g. "lte_cells":[] or "nr_cells":[]  (or) \n
 *	NULL if API not supported.\n
 *	<B>Note</B>: Caller should take care of deleting the Neighbor cell info details memory returned.
 */
char* jdc_client_getNeighborCellInfo(void);



/**
* \brief This API is used to get the PDU session details.
* \param NA
* \return PDU session details in following format \
* [{pdu_session_id , num_recs, {QFI, QOS Flow Type (Non GBR/GBR), GFBR_UL,GFBR_UL unit, GFBR_DL, GFBR_DL unit, MFBR_UL, MFBR_UL Unit, MFBR_DL, MFBR_DL unit}} ] \
* NULL if API not supported or in case of failure scenario.\
* Following are Mandatory params:
	* pdu_session_id
	* num_recs : number of QOS records
	* QFI value range : 1-63  
	* QOS Flow Type either non-GBR or GBR. 0 : non-GBR and 1 : GBR
	* 
* Sample format: 
* [{1, 2, {3,0,6,1,5,1,6,1,10,2}, {9,1,-1,-1,-1,-1,-1,-1,-1,-1}}] \n
* <B>Note</B>: Caller should take care of deleting the Pdu session details memory returned.
*/
char* jdc_client_NR_getPduSessionDetails(void);


/**
 * \brief Function to get Packet Data statistics from last reboot.
 * \param[out] jdc_client_data_statistics_t
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getPacketDataStatistics(jdc_client_data_statistics_t* stats);



/**
 * \brief Function to obtain Attach/Detach  statistics.
 * \param[out] jdc_client_attach_statistics_t
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getAttachStatistics(jdc_client_attach_statistics_t* stats);



/**
 * \brief Function to obtain Registration/Deregistration statistics on New Radio.
 * \param[out] jdc_client_NR_registration_statistics_t
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_NR_getRegistrationStatistics(jdc_client_NR_registration_statistics_t* stats);



/**
 * @deprecated from 4.11
 * \brief This API is used to know whether the device is in Home Network (or) Roaming Network.
 * \param NA
 * \return
 * 0 : Home Network (or) \n
 * 1 : Roaming Network (or) \n
 * JDC_API_NOT_SUPPORTED if API not supported
 */
int16_t jdc_client_getRoamingStatus(void);



/**
 * \brief This API is used to get throughput(Uplink/Downlink) in Kbps provided by platform(vendor/soc/framework).
 * \param[out] jdc_client_throughput_statistics_t pointer - Throughput data will be populated in the tput_stats.
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported
 */
int16_t jdc_client_getThroughputStatsFromPlatform(jdc_client_throughput_statistics_t* tput_stats);



/**
 * \brief This API is used to get throughput(Uplink/Downlink) in Kbps obtained from 3rd party application (Ex: iperf).
 * \param[out] jdc_client_throughput_statistics_t pointer - Throughput data will be populated in the tput_stats.
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported
 */ 
int16_t jdc_client_getThroughputStatsFromApplication(jdc_client_throughput_statistics_t* tput_stats);



/**
 * \brief This API is used to get SIP registration status.
 * \param NA
 * \return jdc_client_sip_registration_status_e - Registraton status (or) JDC_SIP_STATUS_NOT_SUPPORTED_E if API not supported
 */
jdc_client_sip_registration_status_e jdc_client_getSIPRegistrationStatus(void);



/**
 * \brief This Api is used to register callback for getting Telephony Notifications(Callm SMS,Volte etc).
 * \param[in] callback function which informs about Telephony Notifications mentioned in jdc_client_telephony_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_registerForTelephonyNotifications(void (*fn)(jdc_client_telephony_notification_details_t*));



/**
 * \brief This Api is used to De-register callback which was registerd for getting Telephony Notifications
 * \param[in] callback function which informs about Telephony Notifications mentioned in jdc_client_telephony_notifications_e
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or) \n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_deRegisterForTelephonyNotifications(void (*fn)(jdc_client_telephony_notification_details_t*));



/**
 * \brief This Api is used to get Mute Related Parameters. This shall be provided during either 'Call disconnect' and/or 'Call drop' and if not possible then as part of exclusive trigger which is Call Muting event(cumulative stats sent once at call end or call disconnect).
 *
 * Muting Gaps Implementation at modem level:
 * extra small: time duration of audio gap 0.5s – 1 s
 * small: time duration of audio gap >1s – 3 s
 * medium: : time duration of audio gap >3s – 5 s
 * large: time duration of audio gap >5 s
 *
 * Mute Related params(VI3) parameter should be sent only if there are atleast one or more non zero muting gaps in extra small, small, medium, large category. Mute related parameter should not be sent if all the muting gap values are zero { 0,0,0,0 }
 *
 * For RTP Timeout Scenario where No Packet is Received till call end, Large audio gap should accumulate "1" .
 *
 * \param[in] NA
 * \return
 * Example format: { 1,2,3,4}, In success case.
 * NULL - If API is not supported.
 */
char* jdc_client_getMuteRelatedParameters(void);



/**
 * \brief This Api is used to capture RTP Packet Loss occured during Call (Telephony) procedures.
 * \param[in] NA
 * \return
 * Packet loss in range of 0 to 100% in case of success. (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getRTPPacketLoss(void);



/**
 * <B>Note</B>: It is applicable to device having the telephony support.
 * \brief This Api is used to Jitter Loss (Total frame drop count at Jitter Buffer).
 * \param[in] NA
 * \return
 * Number of Packets lost due to Jitter in case of Success.  (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getJitterLoss(void);



/**
 * <B>Note</B>: It is applicable to device having the telephony support.
 * \brief Function to obtain Telephony-Call statistics from last reboot.
 * \param[out] jdc_client_Telephony_statistics_t
 * \return
 * JDC_API_SUCCESS in case of success (or) \n
 * JDC_API_FAILURE if api failed to get the requested data (or)\n
 * JDC_API_NOT_SUPPORTED if API is not supported.
 */
int16_t jdc_client_getTelephonyStatistics(jdc_client_telephony_statistics_t* stats);



/**
 * \brief Function to obtain current Radio Access Type.
 * \param[in] NA
 * \return 
 * Returns current radio access technology (4G or NR),
 * otherwise returns JDC_RAT_TYPE_INVALID if API is not supported or not registered with network.
 */
jdc_client_RAT_type_e jdc_client_getCurrentRAT(void);



/**
* <B>Note</B>: It is applicable to device having the RTLS (Real-Time Location Service) feature support.
* Not required for common DC usecase.
* @brief This API is used to get available cellular info of supported cells listed in jdc_client_RAT_type_e.
* @param[out] cell_info A pointer to array of structures. Each structure refers to individual cell 
* info(listed in jdc_client_RAT_type_e) for serving and neighboring cell information. 
* Access individual cell info like an array, e.g. cell_info[0], cell_info[1] etc.
* <B>Note</B>: Caller should not allocate memory for cell_info (just pass cell_info pointer) argument. 
* The implementor of this API will allocate memory for the jdc_client_cellularInfo_t* array(cell_info) 
* and fill in the details. The caller should take care of deleting the "cell_info" memory.
* @param[out] num_of_cells Total number of cells (serving and neighboring cells) in the 
* jdc_client_cellularInfo_t array 'cell_info'. Cell info will not be available and cell_info should be NULL, 
* if the num_of_cells is 0.
* @return
* JDC_API_SUCCESS in case of success (or) \n
* JDC_API_FAILURE if api failed to get the requested data (or)\n
* JDC_API_NOT_SUPPORTED if API is not supported.
*/
int16_t jdc_client_getAllCellularInfo(jdc_client_cellularInfo_t** cell_info, int16_t* num_of_cells);

/**
 * @brief This API is used to get available FR2 cellular info.
 * @param[out] beam_info - A pointer to structure. 
 * <B>Note</B>: Caller of this api is responsible to allocate and de-allocate the memory.
 * @return
 * JDC_API_SUCCESS in case of success (or)
 * JDC_API_FAILURE if api failed to get the requested data (or)
 * JDC_API_NOT_SUPPORTED if API is not supported.
 * 
 */
int16_t jdc_client_NR_getFR2BeamInfo(jdc_client_FR2_beamInfo_t* beam_info);

/* ====================================== FUNCTION END ======================================= */

#if defined(__cplusplus)
}
#endif

#endif /* JDC_CLIENT_NETWORK_H_ */

/* ===============================END OF THE FILE=============================================== */
