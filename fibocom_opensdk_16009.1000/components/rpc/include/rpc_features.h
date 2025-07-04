/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _RPC_FEATURES_H_
#define _RPC_FEATURES_H_

#include "rpc_config.h"
#include "hal_config.h"
#include "audio_config.h"

#if !defined(CONFIG_RPC_CFW_API)
#define DISABLE_RPC_aud_SetCloseDelayFlag
#define DISABLE_RPC_CFW_GetNetinfo
#define DISABLE_RPC_CFW_SimManageChannel
#define DISABLE_RPC_CFW_SetPdnActTimerAndMaxCount
#define DISABLE_RPC_CFW_SimlockImeiVerify
#define DISABLE_RPC_CFW_EmodSetLteLockBand
#define DISABLE_RPC_CSW_SetAudioZspVqeCalibParam
#define DISABLE_RPC_sim_VsimProcessApduCB
#define DISABLE_RPC_CFW_getDnsServerbyPdp
#define DISABLE_RPC_CFW_SimAddPbkEntry
#define DISABLE_RPC_CFW_CbSetType
#define DISABLE_RPC_CFW_SatGetSubMenuCurrentItem
#define DISABLE_RPC_hal_zspVoicePlayStop
#define DISABLE_RPC_CFW_SimGetFileUpdateCount
#define DISABLE_RPC_SimPollReq
#define DISABLE_RPC_CFW_CfgRestoreFactoryProfile
#define DISABLE_RPC_CSW_WriteCalibParamToFlash
#define DISABLE_RPC_CFW_nvSetPsmT3412
#define DISABLE_RPC_sim_GetSimCardModeCB
#define DISABLE_RPC_CFW_CcGetCsfbmtFlag
#define DISABLE_RPC_CFW_SimGetICCID
#define DISABLE_RPC_CFW_SimUpdateRecord
#define DISABLE_RPC_CFW_SwitchUSBShareNetwork
#define DISABLE_RPC_CFW_CcPlayTone
#define DISABLE_RPC_CFW_CcEmcDial
#define DISABLE_RPC_CFW_nvSetPsmEnable
#define DISABLE_RPC_CFW_EmodGetNvVersion
#define DISABLE_RPC_CFW_SimInit
#define DISABLE_RPC_CFW_GetFreeCID
#define DISABLE_RPC_CFW_nvmWriteStatic
#define DISABLE_RPC_CFW_GetGprsActState
#define DISABLE_RPC_CFW_GetNWTimerOutFlag
#define DISABLE_RPC_CFW_GetRFTemperature
#define DISABLE_RPC_CFW_ErrcSetMeasThreshold
#define DISABLE_RPC_CFW_CellReselctOffset
#define DISABLE_RPC_CFW_SmsInitComplete
#define DISABLE_RPC_CFW_ImsSetSrvccMode
#define DISABLE_RPC_CFW_EmodEngineerCmd
#define DISABLE_RPC_CFW_StackInit
#define DISABLE_RPC_CFW_SimGetFileStatus
#define DISABLE_RPC_CFW_SSCheckTestSim
#define DISABLE_RPC_CFW_SetStaticScene
#define DISABLE_RPC_CFW_SatGetSendSmsSCInfo
#define DISABLE_RPC_CFW_CfgNwGetOperatorInfo
#define DISABLE_RPC_CFW_ImsSetCevdp
#define DISABLE_RPC_CFW_GnssStartHandshakeStack
#define DISABLE_RPC_CFW_SatGetSubMenuTitle
#define DISABLE_RPC_CFW_SmsMtSmsPPAckReq
#define DISABLE_RPC_CFW_SetGsmGprsOnly
#define DISABLE_RPC_CFW_SetSimFileUpdateCountMode
#define DISABLE_RPC_CFW_EmodGetLteLockCell
#define DISABLE_RPC_CFW_GetJammingDetect
#define DISABLE_RPC_CFW_CfgSetErrorReport
#define DISABLE_RPC_CFW_WifiSentestOff
#define DISABLE_RPC_CFW_CfgGetIndicatorEvent
#define DISABLE_RPC_CFW_GetGprsSum
#define DISABLE_RPC_CFW_CcExtConfCallAdd
#define DISABLE_RPC_CFW_CcInitiateSpeechCall_V2
#define DISABLE_RPC_AUD_ToneStopEnd
#define DISABLE_RPC_CFW_nvSetCiotApnRateCtrl
#define DISABLE_RPC_CSW_SetVqeCalibParamSwitch
#define DISABLE_RPC_CFW_GprsSetInternalPdpCtx
#define DISABLE_RPC_CFW_CfgSetSimSwitch
#define DISABLE_RPC_CFW_CfgSetSmsShowTextModeParam
#define DISABLE_RPC_CFW_GprsManualAcc
#define DISABLE_RPC_CFW_EmodSaveNvData
#define DISABLE_RPC_TM_KillAlarm
#define DISABLE_RPC_CFW_SimReadElementary
#define DISABLE_RPC_CFW_SimReadRecordAllParam
#define DISABLE_RPC_CFW_GprsGetDynamicIP
#define DISABLE_RPC_rpcPeerTagSupported
#define DISABLE_RPC_CFW_GetSMSConcat
#define DISABLE_RPC_CFW_GprsSendDataV2
#define DISABLE_RPC_CFW_ImsEnVolte
#define DISABLE_RPC_CFW_SetSmsSeviceMode
#define DISABLE_RPC_CFW_SatGetSendDTMFText
#define DISABLE_RPC_CFW_SetPlmn
#define DISABLE_RPC_CFW_CfgGetClip
#define DISABLE_RPC_CFW_SatGetSetupCallAddr
#define DISABLE_RPC_CFW_GetPdnDeactTimerAndMaxCount
#define DISABLE_RPC_CFW_EmodGetBatteryInfo
#define DISABLE_RPC_CFW_SimUpdateVoiceMailInfo
#define DISABLE_RPC_CFW_SetEleFence
#define DISABLE_RPC_aud_SetAudDeviceCFG
#define DISABLE_RPC_CFW_SimSetACMMax
#define DISABLE_RPC_CFW_GetSimAdnId
#define DISABLE_RPC_CFW_EmodUpdateNwBind
#define DISABLE_RPC_aud_StreamStop
#define DISABLE_RPC_CFW_GetEleFence
#define DISABLE_RPC_CFW_GetSSimFileInfoFromAP
#define DISABLE_RPC_CFW_SatGetGetInputDefaultText
#define DISABLE_RPC_CFW_nvGetPsmT3412
#define DISABLE_RPC_CFW_StartNstMode
#define DISABLE_RPC_CFW_CfgSetAudioAudioMode
#define DISABLE_RPC_CFW_CfgGetNewSmsOption
#define DISABLE_RPC_CFW_NwGetLockBCCHCtoA
#define DISABLE_RPC_hal_zspVoiceRecordStart
#define DISABLE_RPC_CSW_DumpPcmDataToTflash
#define DISABLE_RPC_DM_VoisTestModeSetup
#define DISABLE_RPC_CFW_SimSetPrefOperatorList
#define DISABLE_RPC_DM_ZspMusicVibrateStop
#define DISABLE_RPC_CFW_CfgGetSmsFormat
#define DISABLE_RPC_CFW_SendMtSmsAckPPError
#define DISABLE_RPC_CFW_MeWriteMessageV2
#define DISABLE_RPC_hal_zspVoicePlayStart
#define DISABLE_RPC_aud_StreamStart
#define DISABLE_RPC_CFW_EmodSetLteLockTddOrFdd
#define DISABLE_RPC_CFW_EmodSpCleanInfo
#define DISABLE_RPC_CFW_CcCallHoldMultiparty_V2
#define DISABLE_RPC_CFW_RpmWriteDynamic
#define DISABLE_RPC_CFW_GprsSetCtxEQos
#define DISABLE_RPC_CFW_SetEleFenceTimer
#define DISABLE_RPC_CFW_nvGetCopsTimer
#define DISABLE_RPC_CFW_NwGetCurrentOperator
#define DISABLE_RPC_CFW_RpmResetRpmFunction
#define DISABLE_RPC_CFW_SetSoftSimDefaultInfo
#define DISABLE_RPC_CFW_CfgNwGetNetWorkMode
#define DISABLE_RPC_CFW_nvGetNasTimer
#define DISABLE_RPC_CFW_CfgGetToneDuration
#define DISABLE_RPC_CFW_nvSetEdrxValue
#define DISABLE_RPC_CFW_CfgSetCallWaiting
#define DISABLE_RPC_CFW_RpmAllowPdpActivating
#define DISABLE_RPC_CFW_SimReadMessage
#define DISABLE_RPC_CFW_LocGetWifiscanRes
#define DISABLE_RPC_CFW_GprsSetEdrx
#define DISABLE_RPC_CFW_ClearGprsSum
#define DISABLE_RPC_CFW_EmodSpLteDummyPara
#define DISABLE_RPC_CFW_SsSetFacilityLock
#define DISABLE_RPC_hal_zspMusicPlayStop
#define DISABLE_RPC_CFW_SetDnsServerbyPdp
#define DISABLE_RPC_CFW_GetCcCount
#define DISABLE_RPC_CFW_CfgSetSSN
#define DISABLE_RPC_CFW_CfgGetAudioAudioMode
#define DISABLE_RPC_CFW_GprsSetReqQosUmts
#define DISABLE_RPC_CFW_SatGetSetupEventList
#define DISABLE_RPC_CFW_GprsGetPdpCxtV2
#define DISABLE_RPC_CFW_SatGetSetupIdleModeText
#define DISABLE_RPC_CFW_nvSetLocPriority_V2
#define DISABLE_RPC_CFW_EmodSpecialNvProcess
#define DISABLE_RPC_CFW_SimGetSaveFileFlag
#define DISABLE_RPC_CFW_CfgSetPbkStorage
#define DISABLE_RPC_CFW_CfgSetPeriodTauAftLocalRel
#define DISABLE_RPC_CFW_SimReadFiles
#define DISABLE_RPC_CFW_CcAcceptSpeechCall
#define DISABLE_RPC_CFW_GnssStopHandshakeStack
#define DISABLE_RPC_CFW_NwSetBlackListTimer
#define DISABLE_RPC_CFW_ImsCheckStatusAfterCc
#define DISABLE_RPC_CFW_GetRRCRelEx
#define DISABLE_RPC_CFW_EmodSaveIMEI
#define DISABLE_RPC_CFW_NwSetNetMode
#define DISABLE_RPC_CFW_SmsWriteMessage_V2
#define DISABLE_RPC_CFW_CfgGetIMSI
#define DISABLE_RPC_CFW_nvGetPsmT3324
#define DISABLE_RPC_CFW_SwitchPort
#define DISABLE_RPC_CFW_SatActivation
#define DISABLE_RPC_CFW_SsQueryCallWaiting
#define DISABLE_RPC_CFW_CfgGetSmsStorageInfo
#define DISABLE_RPC_CFW_CfgSetCdrxNoSleepEnable
#define DISABLE_RPC_CSW_SetAudioZspPreProcessCalibParam
#define DISABLE_RPC_CFW_nvSetEdrxEnable
#define DISABLE_RPC_CFW_EmodImsNvParamSet
#define DISABLE_RPC_CFW_ImsGetSrvccCap
#define DISABLE_RPC_CFW_SimGetPrefOperatorList
#define DISABLE_RPC_CFW_SimClose
#define DISABLE_RPC_CFW_nvSetCiotErwopdn
#define DISABLE_RPC_CFW_CfgSetColp
#define DISABLE_RPC_CFW_SetUsrHisFreqInfo
#define DISABLE_RPC_CFW_EmodRrdmParam
#define DISABLE_RPC_CFW_nvSetCiotCpciot
#define DISABLE_RPC_CFW_SimRefreshFiles
#define DISABLE_RPC_CFW_nvGetEdrxValue
#define DISABLE_RPC_CFW_GetApnInfo
#define DISABLE_RPC_CFW_SatGetChannelID
#define DISABLE_RPC_CFW_WifiSentestSetParam
#define DISABLE_RPC_CFW_SetRatePriority
#define DISABLE_RPC_CFW_RpmNvGetDefaultPdnSettingApn
#define DISABLE_RPC_CFW_NwSetBlackList_V2
#define DISABLE_RPC_CFW_SatGetSendSmsAlpha
#define DISABLE_RPC_CFW_SetCDACValue
#define DISABLE_RPC_CFW_CfgGetAudioOutputParam
#define DISABLE_RPC_CFW_SatGetSendSmsData
#define DISABLE_RPC_CFW_SimlockDataCheckUnlock
#define DISABLE_RPC_rpcGetPeerTags
#define DISABLE_RPC_CFW_CfgSimGetMeProfile
#define DISABLE_RPC_CFW_SetPdnDeactTimerAndMaxCount
#define DISABLE_RPC_DM_StartAudioEx
#define DISABLE_RPC_CFW_SatGetOpenChannelAPN
#define DISABLE_RPC_CFW_CfgGetLoudspeakerVolumeLevel
#define DISABLE_RPC_CFW_SendMtSmsAckPPErrorWithData
#define DISABLE_RPC_CFW_GetSATIndFormate
#define DISABLE_RPC_rpcHelloOnCP
#define DISABLE_RPC_CFW_GetStaticScene
#define DISABLE_RPC_CFW_CfgQuerySmsService
#define DISABLE_RPC_CFW_CfgSimSetMeProfile
#define DISABLE_RPC_CFW_SatGetGetInputLengthRange
#define DISABLE_RPC_CFW_GetStackSimFileIDByPath_V2
#define DISABLE_RPC_CFW_GetCSPagingFlag
#define DISABLE_RPC_CFW_GetAoHandle
#define DISABLE_RPC_SimSendStatusReq
#define DISABLE_RPC_CFW_CfgGetCdrxNoSleepEnable
#define DISABLE_RPC_CFW_SimGetProviderId
#define DISABLE_RPC_CFW_CcSetCsfbmtFlag
#define DISABLE_RPC_CFW_NWSetStackRat
#define DISABLE_RPC_CFW_CfgSetStoredPlmnList
#define DISABLE_RPC_CFW_CfgSetClir
#define DISABLE_RPC_CFW_SmsAbortSendMessage
#define DISABLE_RPC_CFW_SetLCTimerEx
#define DISABLE_RPC_CFW_SimUpdateSaveFile
#define DISABLE_RPC_CFW_CfgGetColp
#define DISABLE_RPC_CFW_SatGetMenuTitle
#define DISABLE_RPC_aud_SetBtClk
#define DISABLE_RPC_CFW_SatSetFacility
#define DISABLE_RPC_CFW_NwSysConfig
#define DISABLE_RPC_CFW_SetRFFreqPwrRange
#define DISABLE_RPC_CFW_SetRFFreqPwrRange_V2
#define DISABLE_RPC_CFW_CfgSetSmsStorage
#define DISABLE_RPC_CFW_SimGetAuthenticationStatus
#define DISABLE_RPC_CFW_NwGetAllDetach
#define DISABLE_RPC_DM_ZspMusicPlayStart
#define DISABLE_RPC_CFW_SatGetSendUSSDDisplayInfo
#define DISABLE_RPC_CFW_SatGetSendSSDisplayInfo
#define DISABLE_RPC_CFW_NwSetSimSwich
#define DISABLE_RPC_CFW_SsQueryFacilityLock
#define DISABLE_RPC_CFW_SetSMSConcat
#define DISABLE_RPC_sim_VsimResetCB
#define DISABLE_RPC_CFW_CfgSetFirstPowerOnFlag
#define DISABLE_RPC_CFW_SetPollingTimer
#define DISABLE_RPC_CFW_SimPollReq
#define DISABLE_RPC_CFW_SatGetOpenChannelUserInfo
#define DISABLE_RPC_CFW_MeDeleteMessageV2
#define DISABLE_RPC_aud_HeadsetConfig
#define DISABLE_RPC_aud_StreamRecordStop
#define DISABLE_RPC_CFW_SetDnsServerbyPdpV2
#define DISABLE_RPC_CFW_SimSelectApplication
#define DISABLE_RPC_CFW_GetRFFreqPwrRange
#define DISABLE_RPC_CFW_GetRFFreqPwrRange_V2
#define DISABLE_RPC_CFW_EmodSetBindInfo
#define DISABLE_RPC_CFW_ImsGetCevdp
#define DISABLE_RPC_CFW_SatTerminalEnvelope
#define DISABLE_RPC_CFW_SatGetOpenChannelBearerDes
#define DISABLE_RPC_CFW_CcGetCallStatus
#define DISABLE_RPC_CFW_GetSmsSeviceMode
#define DISABLE_RPC_CFW_EmodSyncInfoTest
#define DISABLE_RPC_CFW_nvGetCiotNonip
#define DISABLE_RPC_CFW_NwDeRegister
#define DISABLE_RPC_CFW_GprsGetDefaultPdnInfo
#define DISABLE_RPC_DM_ZspMusicPlayStop
#define DISABLE_RPC_CFW_CcEmcDialEx
#define DISABLE_RPC_aud_bbatPcmBufferPlayStop
#define DISABLE_RPC_CFW_SatGetPlayTone
#define DISABLE_RPC_CFW_nvSetBlackListEnable
#define DISABLE_RPC_CFW_GprsGetPdpIpv6Addr
#define DISABLE_RPC_hal_btVoiceStart
#define DISABLE_RPC_CFW_StopNstMode
#define DISABLE_RPC_CFW_SimGetPbkStorage
#define DISABLE_RPC_CFW_GetRRCRel
#define DISABLE_RPC_aud_bbatPcmBufferPlayStart
#define DISABLE_RPC_CFW_NwSetMaxAttFailForIratReq
#define DISABLE_RPC_CFW_SatGetMenuCurrentItem
#define DISABLE_RPC_DM_AudSetSharkingMode
#define DISABLE_RPC_DM_ZspVoiceVibrateStart
#define DISABLE_RPC_CFW_SaveSsimFileInfo
#define DISABLE_RPC_CFW_SatResponse
#define DISABLE_RPC_CFW_SmsDeleteMessage
#define DISABLE_RPC_CFW_nvGetPsmEnable
#define DISABLE_RPC_CFW_GprsGetDynamicAPN
#define DISABLE_RPC_CFW_rpmNvGetLastRat
#define DISABLE_RPC_CFW_GprsCtxEpsModify
#define DISABLE_RPC_CFW_SimReadPreferPLMN
#define DISABLE_RPC_CFW_SatGetPlayToneDuration
#define DISABLE_RPC_CFW_GprsGetMInQos
#define DISABLE_RPC_CFW_SmsCbGetNode
#define DISABLE_RPC_CFW_SimGetACMMax
#define DISABLE_RPC_CFW_ImsSetDapsCallback
#define DISABLE_RPC_CFW_ImsDoSavedEmcDail
#define DISABLE_RPC_CFW_CcReleaseCallX
#define DISABLE_RPC_CFW_EmodSetLteLockBandV2
#define DISABLE_RPC_CFW_SimGetMeProfile
#define DISABLE_RPC_CFW_RpmWriteStatic
#define DISABLE_RPC_CFW_NwGetCsgMode
#define DISABLE_RPC_CFW_CfgNwSetFM
#define DISABLE_RPC_CFW_CfgSetNewSmsOption
#define DISABLE_RPC_CFW_GetDrxCyc
#define DISABLE_RPC_CFW_CcCallMultipartyFunction
#define DISABLE_RPC_CFW_GetLmtMaxLen
#define DISABLE_RPC_CFW_SatGetCurCMD
#define DISABLE_RPC_CFW_CfgGetNwOperDispFormat
#define DISABLE_RPC_CFW_CcGetMptyNum
#define DISABLE_RPC_CFW_nvGetCiotCpciot
#define DISABLE_RPC_CFW_AttachCSAndPS
#define DISABLE_RPC_CFW_UpdateNV
#define DISABLE_RPC_CFW_EmodGetLteCalibInfo
#define DISABLE_RPC_CFW_ShellControl_V2
#define DISABLE_RPC_CFW_WifiScanStop
#define DISABLE_RPC_CFW_SetDefaultPcoControl
#define DISABLE_RPC_TM_ListAlarm
#define DISABLE_RPC_DM_AudStreamStop
#define DISABLE_RPC_CFW_SimListCountPbkEntries
#define DISABLE_RPC_CFW_EmodGetBindInfo
#define DISABLE_RPC_CFW_SatGetFacility
#define DISABLE_RPC_CFW_SimlockSetVersion
#define DISABLE_RPC_CFW_SimWriteMessage
#define DISABLE_RPC_CFW_NwSetFrequencyBand
#define DISABLE_RPC_CFW_SetSimStatus
#define DISABLE_RPC_CFW_EmodGetPocPara
#define DISABLE_RPC_CFW_CcStopTone
#define DISABLE_RPC_CFW_SsQueryClip
#define DISABLE_RPC_CFW_GprsGetNsapi
#define DISABLE_RPC_CFW_EmodGetLteBand
#define DISABLE_RPC_CFW_SatSetEventList
#define DISABLE_RPC_CFW_DetachCSAndPS
#define DISABLE_RPC_CFW_GetIMEIFromAP
#define DISABLE_RPC_CFW_SimlockToolGetUid
#define DISABLE_RPC_CFW_SimListPbkEntries
#define DISABLE_RPC_CFW_GprsSetDefaultPdnMode
#define DISABLE_RPC_rpcHelloOnAP
#define DISABLE_RPC_CFW_GprsAct
#define DISABLE_RPC_CFW_EmodClearUserInfo
#define DISABLE_RPC_CFW_RpmGetRpmCR1
#define DISABLE_RPC_CFW_isPdpActiveAvailable
#define DISABLE_RPC_CFW_SimReadBinary
#define DISABLE_RPC_CFW_SimReadBinaryV2
#define DISABLE_RPC_CFW_CfgNwGetLockedFrequencyBand
#define DISABLE_RPC_CFW_EmodGetLteLockTddOrFdd
#define DISABLE_RPC_CFW_MeGetStorageInfoV2
#define DISABLE_RPC_CFW_GetLETRealTimeInfo
#define DISABLE_RPC_SimSwitch
#define DISABLE_RPC_CFW_EmodSetLteLockCell
#define DISABLE_RPC_CFW_SimGetServiceProviderName
#define DISABLE_RPC_CFW_SmsListFree
#define DISABLE_RPC_CFW_DecodeCDMAImsi
#define DISABLE_RPC_CFW_SatGetGetInputText
#define DISABLE_RPC_CFW_CfgGetCharacterSet
#define DISABLE_RPC_CFW_SetJammingDetectFlag
#define DISABLE_RPC_CFW_GprsHostAddress
#define DISABLE_RPC_CFW_GetCellInfoV2
#define DISABLE_RPC_CFW_nvGetBlackListEnable
#define DISABLE_RPC_CFW_EmodGetProductInfo
#define DISABLE_RPC_CFW_ImsSetPdpStatus
#define DISABLE_RPC_CFW_GprsSendCtxCfg_V2
#define DISABLE_RPC_DM_VoisSetCfg
#define DISABLE_RPC_CFW_GetPollingTimer
#define DISABLE_RPC_CFW_EmodGetCEMode
#define DISABLE_RPC_CFW_GprsSetstatus
#define DISABLE_RPC_CFW_GprsGetstatus
#define DISABLE_RPC_CFW_SmsSendMessage_V3
#define DISABLE_RPC_CFW_GprsSetPdpCxtV2
#define DISABLE_RPC_CFW_CbReadContext
#define DISABLE_RPC_getDNSServer
#define DISABLE_RPC_CFW_CfgNwSetCgclassType
#define DISABLE_RPC_CFW_EmodSetCEMode
#define DISABLE_RPC_CFW_CfgNwGetCgclassType
#define DISABLE_RPC_VTSP_startVolteCodec
#define DISABLE_RPC_CFW_SatGetReceiveData
#define DISABLE_RPC_CFW_CfgGetResultCodePresentMode
#define DISABLE_RPC_CFW_NwAbortListOperators
#define DISABLE_RPC_CFW_SetRRCRel
#define DISABLE_RPC_CFW_SetCurrPreferPLMNList
#define DISABLE_RPC_CSW_VersionControl
#define DISABLE_RPC_CFW_CfgDeviceSwithOff
#define DISABLE_RPC_CFW_CfgGetPocEnable
#define DISABLE_RPC_CSW_ExportCalibOrImportCalibFlashParam
#define DISABLE_RPC_CFW_SatGetSetupCallCapCfg
#define DISABLE_RPC_hal_zspVoiceDtmfStop
#define DISABLE_RPC_CFW_EmodSetNvData
#define DISABLE_RPC_CFW_CfgGetExtendedErrorV1
#define DISABLE_RPC_CFW_CbGetContext
#define DISABLE_RPC_CFW_GetDefaultLteCidCtoA
#define DISABLE_RPC_CFW_GetPdnActTimerAndMaxCount
#define DISABLE_RPC_CFW_CfgSetPbkStrorageInfo
#define DISABLE_RPC_DM_PlayToneEx
#define DISABLE_RPC_CFW_CfgNwGetOperatorName
#define DISABLE_RPC_CFW_CfgSetToneDuration
#define DISABLE_RPC_CFW_RpmGetRpmFx
#define DISABLE_RPC_CFW_SatGetSetupCallText
#define DISABLE_RPC_CFW_SatGetSendSSInfo
#define DISABLE_RPC_CFW_EmodGetIMEI
#define DISABLE_RPC_CFW_GprsCtxModifyRej
#define DISABLE_RPC_CFW_GprsSecPdpAct
#define DISABLE_RPC_CFW_EmodGetTraceFlag
#define DISABLE_RPC_CFW_GprsActEx
#define DISABLE_RPC_CFW_EmodGetBaseBandVersionV1
#define DISABLE_RPC_CFW_NwGetAvailableOperators
#define DISABLE_RPC_CFW_SatResponseV2
#define DISABLE_RPC_CFW_CusPrivatePsm
#define DISABLE_RPC_CFW_SetPsmT3412Len
#define DISABLE_RPC_CFW_SimReadRecordWithLen
#define DISABLE_RPC_CFW_nvSetCiotSmsWoCombAtt
#define DISABLE_RPC_DM_RestartAudioEx
#define DISABLE_RPC_CFW_CfgGetLoudspeakerVolumeLevelRange
#define DISABLE_RPC_CFW_GprsSetCtxTFT
#define DISABLE_RPC_CFW_SatGetEventList
#define DISABLE_RPC_CFW_NwGetAutoAttachFlag
#define DISABLE_RPC_CFW_CfgGetEchoMode
#define DISABLE_RPC_CFW_EmodOutfieldTestStartV2
#define DISABLE_RPC_CFW_GetLETRealTimeInfoV2
#define DISABLE_RPC_CFW_GprsCtxModifyAcc
#define DISABLE_RPC_CFW_GetICCID_V2
#define DISABLE_RPC_CFW_CfgNwSetLockedFrequencyBand
#define DISABLE_RPC_CFW_UsimDecodeEFFcpV2
#define DISABLE_RPC_DM_ZspMusicPlayStopEx
#define DISABLE_RPC_CFW_CbDeAct
#define DISABLE_RPC_CFW_SsSetCallForwarding
#define DISABLE_RPC_CFW_CfgSetSmsStorageInfo
#define DISABLE_RPC_CFW_SwitchDualSimSlot
#define DISABLE_RPC_CFW_SimReset
#define DISABLE_RPC_CFW_CfgGetSmsOverflowInd
#define DISABLE_RPC_CFW_SimlockDataCheckLock
#define DISABLE_RPC_CFW_CcEmcDialEx_V2
#define DISABLE_RPC_CFW_CfgSetEnhancePwrSweepNonCE
#define DISABLE_RPC_CFW_SatGetSubMenuItemByID
#define DISABLE_RPC_CFW_GetLimiteTimer
#define DISABLE_RPC_CFW_SatGetOpenChannelNetInfo
#define DISABLE_RPC_CFW_MeListMessageV2
#define DISABLE_RPC_hal_WriteGsmCalib
#define DISABLE_RPC_hal_zspVoiceDtmfStart
#define DISABLE_RPC_CFW_NwGetLteSignalQuality
#define DISABLE_RPC_CFW_NwSetRegistration
#define DISABLE_RPC_CFW_GprsCtxModify
#define DISABLE_RPC_CFW_NwGetQualReport
#define DISABLE_RPC_CFW_CfgSetNewSmsOptionMT
#define DISABLE_RPC_CFW_EmodReadBindSIM
#define DISABLE_RPC_CFW_CfgSetLoudspeakerVolumeLevel
#define DISABLE_RPC_CFW_NwSetCSMode
#define DISABLE_RPC_CFW_SetPsDataAttribute
#define DISABLE_RPC_CFW_NwSetCsgMode
#define DISABLE_RPC_CFW_NwSetEpsMode
#define DISABLE_RPC_CFW_NWGetRat
#define DISABLE_RPC_CFW_GetRatePriority
#define DISABLE_RPC_CFW_nvGetCiotUpciot
#define DISABLE_RPC_CFW_SimWakeup
#define DISABLE_RPC_CFW_SatGetTimerManagement
#define DISABLE_RPC_CFW_GprsGetPdpAddr
#define DISABLE_RPC_CFW_RpmGetRpmLRX
#define DISABLE_RPC_CFW_SetNvUpdateFlag
#define DISABLE_RPC_CFW_SmsGetSmsType
#define DISABLE_RPC_CFW_SimGetPbkStrorageInfo
#define DISABLE_RPC_CFW_nvSetCopsTimer
#define DISABLE_RPC_CFW_SatGetCurrentCmd
#define DISABLE_RPC_CSW_SetAudioCodecCalibParam
#define DISABLE_RPC_CFW_GprsIpDnsInfo
#define DISABLE_RPC_CFW_CfgSetIMSI
#define DISABLE_RPC_CFW_SimlockDataProcess
#define DISABLE_RPC_hal_btVoiceStop
#define DISABLE_RPC_CFW_SsQueryColp
#define DISABLE_RPC_CFW_GprsRemovePdpCxt
#define DISABLE_RPC_CFW_CcReleaseCall_V2
#define DISABLE_RPC_CFW_SsSendUSSD
#define DISABLE_RPC_CFW_GetFreeUTI
#define DISABLE_RPC_SimSelectApplicationReq
#define DISABLE_RPC_CFW_RpmGetRpmCpdpx
#define DISABLE_RPC_CFW_SetLTEFreqPwrRange
#define DISABLE_RPC_CFW_SatGetSendData
#define DISABLE_RPC_CFW_SatGetMenuNextItem
#define DISABLE_RPC_CFW_nvGetCiotErwopdn
#define DISABLE_RPC_CFW_SatGetSetupIdleMode
#define DISABLE_RPC_CFW_nvGetCiotApnRateCtrl
#define DISABLE_RPC_CFW_NwSetAutoAttachFlag
#define DISABLE_RPC_CFW_CcReleaseCall
#define DISABLE_RPC_CFW_SimGetSmsTotalNum
#define DISABLE_RPC_CFW_GetStatusInfo
#define DISABLE_RPC_CFW_NwCSGSetRegistration
#define DISABLE_RPC_CFW_ImsSsUtAddrSet
#define DISABLE_RPC_CFW_CcGetCurrentCall
#define DISABLE_RPC_CFW_CfgSetAntenna
#define DISABLE_RPC_CFW_CfgSetClip
#define DISABLE_RPC_CFW_NwGetHNBName
#define DISABLE_RPC_CFW_nvGetEdrxPtw
#define DISABLE_RPC_CFW_nvSetPsmT3324
#define DISABLE_RPC_audevPlayVoiceTone
#define DISABLE_RPC_CFW_SetRRCRelEx
#define DISABLE_RPC_CFW_nvGetEdrxEnable
#define DISABLE_RPC_CFW_CfgGetCdmaImsi
#define DISABLE_RPC_CFW_GetEleFenceTimer
#define DISABLE_RPC_VTSP_stopVolteCodec
#define DISABLE_RPC_CFW_NwSetLockBCCH
#define DISABLE_RPC_CFW_ImsSrvccCapSet
#define DISABLE_RPC_CFW_NwGetLockBCCH
#define DISABLE_RPC_CFW_EmodGetLteLockBandV2
#define DISABLE_RPC_CFW_GprsGetDefaultPdnMode
#define DISABLE_RPC_CFW_nvSetCiotUpciot
#define DISABLE_RPC_CFW_SimDeleteMessage_V2
#define DISABLE_RPC_DM_AudSetToneMode
#define DISABLE_RPC_CFW_ImsInit
#define DISABLE_RPC_CFW_CfgGetConnectResultFormatCode
#define DISABLE_RPC_CFW_EmodResetImsDelta
#define DISABLE_RPC_CFW_SimSetMeProfile
#define DISABLE_RPC_CFW_RpmGetRpmN1
#define DISABLE_RPC_CFW_WifiSentestOn
#define DISABLE_RPC_aud_Setup
#define DISABLE_RPC_CFW_GetFreqScanAllInfo
#define DISABLE_RPC_CFW_SatGetOpenChannelDestAddress
#define DISABLE_RPC_CFW_GprsSetReqQos
#define DISABLE_RPC_CFW_SetSignalQuatityFirst
#define DISABLE_RPC_CFW_SimCloseEx
#define DISABLE_RPC_CFW_nvGetLocPriority
#define DISABLE_RPC_CFW_SatGetGetInKeyText
#define DISABLE_RPC_CFW_SatGetSendDTMFInfo
#define DISABLE_RPC_CFW_GetGsmGprsOnly
#define DISABLE_RPC_CFW_GetMNCLen
#define DISABLE_RPC_CFW_EmodGetLteLockBand
#define DISABLE_RPC_CFW_NwGetCsgOperators
#define DISABLE_RPC_CFW_GetLCTimerEx
#define DISABLE_RPC_CFW_SimSetACM
#define DISABLE_RPC_CFW_SatGetSendDTMF
#define DISABLE_RPC_CFW_NwChangeRat
#define DISABLE_RPC_CFW_GetComm
#define DISABLE_RPC_vois_TestModeSetup
#define DISABLE_RPC_CFW_NwGetSignalQuality
#define DISABLE_RPC_CFW_EmodOutfieldTestStart
#define DISABLE_RPC_CFW_GetUsimAID
#define DISABLE_RPC_CFW_CfgNwGetOperatorIdWithIndex
#define DISABLE_RPC_CFW_EmodGetNvData
#define DISABLE_RPC_CFW_CfgSetCharacterSet
#define DISABLE_RPC_CFW_SetGcfTest
#define DISABLE_RPC_CFW_EmodAudioTestStart
#define DISABLE_RPC_CFW_CbWriteContext
#define DISABLE_RPC_CFW_SimTPDUCommand
#define DISABLE_RPC_CFW_CcAcceptSpeechCallEx
#define DISABLE_RPC_SMS_DM_Init_V2
#define DISABLE_RPC_CFW_SetInitWithSim
#define DISABLE_RPC_CFW_EmodSetPSMThd
#define DISABLE_RPC_CFW_SmsMMAReq
#define DISABLE_RPC_CFW_CheckSupportLteBand
#define DISABLE_RPC_CFW_NwGetStatus
#define DISABLE_RPC_CFW_CfgSetResultCodePresentMode
#define DISABLE_RPC_CFW_GetLctMaxLen
#define DISABLE_RPC_CFW_SimlockSetHuk
#define DISABLE_RPC_CFW_ImsDisable
#define DISABLE_RPC_CFW_SimUpdateRecordAllParam
#define DISABLE_RPC_DM_StopAudioEx
#define DISABLE_RPC_CFW_SsSetClip
#define DISABLE_RPC_CFW_CfgSetNwStatus
#define DISABLE_RPC_CFW_GenerateSoftSimImsi
#define DISABLE_RPC_CFW_GetBootCausesFromAP
#define DISABLE_RPC_CFW_SmsReadMessage
#define DISABLE_RPC_CFW_ImsIsSet
#define DISABLE_RPC_CFW_SimGetDFStatus
#define DISABLE_RPC_hal_zspVoiceRecordStop
#define DISABLE_RPC_CFW_CfgGetPhoneFunctionality
#define DISABLE_RPC_CFW_GprsGetInternalPdpCtx
#define DISABLE_RPC_CFW_EmodImsNvParamGet
#define DISABLE_RPC_CFW_CfgGetSmsStorage
#define DISABLE_RPC_CFW_ImsGetSrvccMode
#define DISABLE_RPC_CFW_LcsMtlrRsp
#define DISABLE_RPC_CFW_GetCDACValue
#define DISABLE_RPC_CFW_SetRauAfterCallFlag
#define DISABLE_RPC_CFW_SimGetOperatorName
#define DISABLE_RPC_CFW_SimGetOperatorNameV2
#define DISABLE_RPC_CFW_GetLETRealTimeInfoV3
#define DISABLE_RPC_CFW_NWGetStackRat
#define DISABLE_RPC_CFW_EmodSetPowerMode
#define DISABLE_RPC_vois_GetSampleRate
#define DISABLE_RPC_CFW_CfgNwGetFrequencyBand
#define DISABLE_RPC_CFW_CfgStoreUserProfile
#define DISABLE_RPC_CFW_NwJammingRssiEnquiry
#define DISABLE_RPC_CFW_ImsSuspend
#define DISABLE_RPC_vois_SetBtNRECFlag
#define DISABLE_RPC_CFW_CfgGetUserProfile
#define DISABLE_RPC_CFW_SatGetSendSmsServiceCenterAddr
#define DISABLE_RPC_CFW_RpmSetRpmLRX
#define DISABLE_RPC_CFW_SimDeletePbkEntry
#define DISABLE_RPC_CFW_GprsSendData
#define DISABLE_RPC_CFW_SsQueryClir
#define DISABLE_RPC_aud_SetLoopbackGain
#define DISABLE_RPC_CFW_SimGetSmsParameters
#define DISABLE_RPC_CFW_SimSetSuspend
#define DISABLE_RPC_CFW_EmodReadBindNW
#define DISABLE_RPC_CFW_SatGetPlayToneText
#define DISABLE_RPC_CFW_SimGetPrefOperatorListMaxNum
#define DISABLE_RPC_CFW_SimSetProiorityResetFlag
#define DISABLE_RPC_CFW_CfgSetResultCodeFormatMode
#define DISABLE_RPC_AUD_VoiceCoderChangeInd
#define DISABLE_RPC_CFW_SmsSetUnSent2Sent
#define DISABLE_RPC_CFW_UsimDecodeEFFcp
#define DISABLE_RPC_CFW_RpmSetRpmN1
#define DISABLE_RPC_CFW_CfgGetSSN
#define DISABLE_RPC_CFW_CfgSetSmsFormat
#define DISABLE_RPC_CFW_SimSetResume
#define DISABLE_RPC_CFW_CfgSetConnectResultFormatCode
#define DISABLE_RPC_CFW_EmodSetPocPara
#define DISABLE_RPC_CFW_SimGetPbkEntry
#define DISABLE_RPC_CFW_ImsGetTerminalMode
#define DISABLE_RPC_CFW_nvSetNwSearchPrior
#define DISABLE_RPC_CFW_RpmGetRpmCBR1
#define DISABLE_RPC_CFW_CfgGetIncomingCallResultMode
#define DISABLE_RPC_CFW_EmodGetLteLockFreq
#define DISABLE_RPC_CFW_SmsSendMessage_V2
#define DISABLE_RPC_CFW_CfgGetNwStatus
#define DISABLE_RPC_CFW_GetNetDLPsmParam
#define DISABLE_RPC_CFW_IsFreeUTI
#define DISABLE_RPC_CFW_SsSendUSSD_V2
#define DISABLE_RPC_CFW_NwGetAvailableOperators_V2
#define DISABLE_RPC_CFW_CcRejectCall
#define DISABLE_RPC_CFW_SsSetClir
#define DISABLE_RPC_CFW_GetSimType
#define DISABLE_RPC_CFW_EmodAudioTestEnd
#define DISABLE_RPC_CFW_RpmSetRpmT1
#define DISABLE_RPC_CFW_SimSetSaveFileFlag
#define DISABLE_RPC_CFW_CfgGetNewSmsOptionMT
#define DISABLE_RPC_CFW_NwSetBlackList
#define DISABLE_RPC_aud_StreamRecord
#define DISABLE_RPC_CFW_CfgSetSoftReset
#define DISABLE_RPC_CFW_GetLteBlkList
#define DISABLE_RPC_CFW_GetStackSimFileID
#define DISABLE_RPC_CFW_EmodGetPSMThd
#define DISABLE_RPC_CFW_EmodSetTracePort
#define DISABLE_RPC_CFW_GprsGetReqQosUmts
#define DISABLE_RPC_CFW_SatGetSubMenuNextItem
#define DISABLE_RPC_SimSelectApplicationReqV2
#define DISABLE_RPC_CFW_InvalideTmsiPTmis
#define DISABLE_RPC_CFW_WifiSentestGetSingleRes
#define DISABLE_RPC_CFW_SimChangePassword
#define DISABLE_RPC_CFW_CfgGetCallWaiting
#define DISABLE_RPC_CFW_SetLctLoopCout
#define DISABLE_RPC_CFW_EmodGrrLteFreq
#define DISABLE_RPC_CFW_CfgSetPhoneFunctionality
#define DISABLE_RPC_CFW_CfgSetSmsOverflowInd
#define DISABLE_RPC_CFW_SimAddPbkEntry_V2
#define DISABLE_RPC_CFW_GetDetectMBS
#define DISABLE_RPC_CFW_GprsSetPsm
#define DISABLE_RPC_CFW_SsChangePassword
#define DISABLE_RPC_CFW_GetSignalQuatityFirst
#define DISABLE_RPC_CFW_ImsResume
#define DISABLE_RPC_CFW_GetLTEFreqPwrValue
#define DISABLE_RPC_CFW_NWSetRat
#define DISABLE_RPC_CFW_SimGetCdmaImsi
#define DISABLE_RPC_CFW_ErrcGetMeasThreshold
#define DISABLE_RPC_CSW_SetAndGetMicGain
#define DISABLE_RPC_CFW_CfgGetSimSwitch
#define DISABLE_RPC_CFW_EmodSetVoicePrefer
#define DISABLE_RPC_CFW_SimGetACM
#define DISABLE_RPC_CFW_RpmGetRpmFlag
#define DISABLE_RPC_CFW_SimGetLanguage
#define DISABLE_RPC_CFW_CheckSupportLteFreq
#define DISABLE_RPC_DM_StopToneEx
#define DISABLE_RPC_CFW_SmsMtSmsPPAckReqWithData
#define DISABLE_RPC_CFW_MeReadMessageV2
#define DISABLE_RPC_CFW_SatGetMenuFirstItem
#define DISABLE_RPC_CSW_SetAudioZspPostProcessCalibParam
#define DISABLE_RPC_CFW_GetNMR
#define DISABLE_RPC_CFW_GprsGetPdpIpv4Addr
#define DISABLE_RPC_CFW_RpmGetRpmT1
#define DISABLE_RPC_CFW_SimGetStatus
#define DISABLE_RPC_CFW_GetSimSPN
#define DISABLE_RPC_DM_AudSetAudDeviceCFG
#define DISABLE_RPC_CFW_CfgGetDefaultSmsParam
#define DISABLE_RPC_CFW_NwSetNetTAU
#define DISABLE_RPC_CFW_NwSetLocalRelease
#define DISABLE_RPC_DM_ZspVoiceVibrateStop
#define DISABLE_RPC_CFW_RpmCheckAllowRebootingPdpActiving
#define DISABLE_RPC_CFW_SmsGetMessageNodeV2
#define DISABLE_RPC_CFW_SmsSendPduMessage
#define DISABLE_RPC_CFW_SetLmtLoopCout
#define DISABLE_RPC_AUD_SetCodecOpStatus
#define DISABLE_RPC_CFW_CfgGetResultCodeFormatMode
#define DISABLE_RPC_CFW_SatGetOpenChannelNetInfoV2
#define DISABLE_RPC_CFW_EmodSetLteBand
#define DISABLE_RPC_CFW_SimlockAuthStart
#define DISABLE_RPC_CFW_CfgGetSoftReset
#define DISABLE_RPC_CFW_AttDetach
#define DISABLE_RPC_CFW_SatGetSubMenuFirstItem
#define DISABLE_RPC_rpcPeerTagCount
#define DISABLE_RPC_DSPExt_GetVoiceCoderType
#define DISABLE_RPC_SimPollOffReq
#define DISABLE_RPC_CFW_RpmSetRpmFx
#define DISABLE_RPC_CFW_GetCellInfo
#define DISABLE_RPC_CFW_NwGetFTA
#define DISABLE_RPC_CFW_CfgSetSmsCB
#define DISABLE_RPC_CFW_CfgNwSetNetWorkMode
#define DISABLE_RPC_CFW_CfgSetIncomingCallResultMode
#define DISABLE_RPC_CFW_EmodGetLpsFlag
#define DISABLE_RPC_CFW_SsSetColp
#define DISABLE_RPC_CFW_EmodL1Param
#define DISABLE_RPC_CFW_SimlockAuthEnd
#define DISABLE_RPC_CFW_SmsGetMessageNodeEx
#define DISABLE_RPC_CFW_CfgGetSmsCB
#define DISABLE_RPC_CFW_ImsSetTerminalMode
#define DISABLE_RPC_CFW_SimGetEID
#define DISABLE_RPC_DM_AudStreamStart
#define DISABLE_RPC_CFW_SimInitStage1
#define DISABLE_RPC_CFW_SetJammingDetectParamEx
#define DISABLE_RPC_CFW_SimWritePreferPLMN
#define DISABLE_RPC_CFW_WifiScanStart
#define DISABLE_RPC_CFW_nvGetNwSearchPrior
#define DISABLE_RPC_CFW_SetCID
#define DISABLE_RPC_CFW_CfgGetSmsShowTextModeParam
#define DISABLE_RPC_CFW_SimlockImeiProcess
#define DISABLE_RPC_CFW_GetDefaultPcoControl
#define DISABLE_RPC_CFW_SimReadVoiceMailInfo
#define DISABLE_RPC_CFW_rpmNvSetLastRat
#define DISABLE_RPC_CFW_EmodGetRFCalibrationInfo
#define DISABLE_RPC_CFW_CfgGetErrorReport
#define DISABLE_RPC_CFW_CfgSetIndicatorEvent
#define DISABLE_RPC_CFW_GprsManualRej
#define DISABLE_RPC_CFW_EmodGetVoicePrefer
#define DISABLE_RPC_CFW_CfgGetPbkStorage
#define DISABLE_RPC_hal_zspMusicPlayStart
#define DISABLE_RPC_CFW_NwGetSysConfig
#define DISABLE_RPC_CFW_CbAct
#define DISABLE_RPC_CFW_SimSetSmsParameters
#define DISABLE_RPC_CFW_CcGetCurrentCallV2
#define DISABLE_RPC_CFW_SetVocLoopType
#define DISABLE_RPC_CFW_RpmWriteDynamicEx
#define DISABLE_RPC_CFW_NwSetBlackWhiteList
#define DISABLE_RPC_CFW_ImsSetUaVersionInfo
#define DISABLE_RPC_CFW_SimGetPUCT
#define DISABLE_RPC_CFW_SatGetRawData
#define DISABLE_RPC_CFW_SimReadRecord
#define DISABLE_RPC_CFW_GprsSetSmsSeviceMode
#define DISABLE_RPC_CFW_GetFreqScanInfo
#define DISABLE_RPC_CFW_SimGetFacilityLock
#define DISABLE_RPC_CFW_CcCallHoldMultiparty
#define DISABLE_RPC_CFW_CfgNwSetFrequencyBand
#define DISABLE_RPC_CFW_WriteLongSmsInfo
#define DISABLE_RPC_CFW_CfgSetSmsParam
#define DISABLE_RPC_CFW_nvSetCiotNonip
#define DISABLE_RPC_aud_SetSharkingMode
#define DISABLE_RPC_CFW_CfgSetEchoMode
#define DISABLE_RPC_CFW_SetPowerOffFlag
#define DISABLE_RPC_DM_AudStreamRecord
#define DISABLE_RPC_CFW_SetLctMaxLen
#define DISABLE_RPC_CFW_CfgGetClir
#define DISABLE_RPC_CFW_SsQueryColr
#define DISABLE_RPC_CFW_SimSetFacilityLock
#define DISABLE_RPC_CFW_GetRauAfterCallFlag
#define DISABLE_RPC_CFW_SmsMemoryFullReq
#define DISABLE_RPC_CFW_CcReleaseCallX_V2
#define DISABLE_RPC_CFW_SetQualReport
#define DISABLE_RPC_CFW_NstGetrssi
#define DISABLE_RPC_aud_CodecNvInit
#define DISABLE_RPC_CFW_GetScanRssiInfo
#define DISABLE_RPC_CFW_CfgGetEnhancePwrSweepNonCE
#define DISABLE_RPC_CFW_CfgSetPocEnable
#define DISABLE_RPC_CFW_EmodGetTracePort
#define DISABLE_RPC_CFW_GetBandFromFreq
#define DISABLE_RPC_CFW_NwGetSimSwitch
#define DISABLE_RPC_CFW_GetDefaultApn
#define DISABLE_RPC_CFW_SimEnterAuthentication
#define DISABLE_RPC_CFW_SimSetSaveFileList
#define DISABLE_RPC_CFW_GetLmtLoopCout
#define DISABLE_RPC_CFW_SmsListMessages
#define DISABLE_RPC_CFW_ImsIsRegistered
#define DISABLE_RPC_CFW_SatGetPollIntervalDuration
#define DISABLE_RPC_CFW_nvSetNasTimer
#define DISABLE_RPC_CFW_NwSetFTA
#define DISABLE_RPC_CFW_RpmSetRpmFlag
#define DISABLE_RPC_CFW_GprsSetMinQos
#define DISABLE_RPC_CFW_GetSimPHYType
#define DISABLE_RPC_CFW_SatTerminalResponse
#define DISABLE_RPC_CFW_GetSimLockStatusFromAP
#define DISABLE_RPC_CFW_SmsSetUnRead2Read
#define DISABLE_RPC_CFW_GetLctLoopCout
#define DISABLE_RPC_CFW_GetNetinfoStatistic
#define DISABLE_RPC_CFW_SetMoSmsRetryCount
#define DISABLE_RPC_CFW_CfgCheckFirstPowerOnFlag
#define DISABLE_RPC_CFW_SatGetSendSmsTPDU
#define DISABLE_RPC_CFW_SatGetDisplayText
#define DISABLE_RPC_CFW_GetGprsAttState
#define DISABLE_RPC_CFW_SatRefreshImsiChange
#define DISABLE_RPC_CFW_NwIsUseISIM
#define DISABLE_RPC_CFW_CfgSelectSmsService
#define DISABLE_RPC_DM_AudStreamRecordStop
#define DISABLE_RPC_CFW_LcsMolrReq
#define DISABLE_RPC_CFW_nvGetCiotSmsWoCombAtt
#define DISABLE_RPC_CFW_ImsGetPdpStatus
#define DISABLE_RPC_CFW_SetJammingDetectParam
#define DISABLE_RPC_CFW_ShellControl
#define DISABLE_RPC_CFW_GprsAtt
#define DISABLE_RPC_CFW_ImsEnable
#define DISABLE_RPC_CFW_CfgSetNwOperDispFormat
#define DISABLE_RPC_CFW_EnableLpsLock
#define DISABLE_RPC_CFW_SatGetSendUSSDInfo
#define DISABLE_RPC_CFW_SmsMoInit
#define DISABLE_RPC_CFW_SimInitStage3
#define DISABLE_RPC_CFW_ImsActImsPdp
#define DISABLE_RPC_CFW_SimlockEncryptKeys
#define DISABLE_RPC_CFW_SimGetOperationTimes
#define DISABLE_RPC_CFW_CfgSetDefaultSmsParam
#define DISABLE_RPC_CFW_SetT3302
#define DISABLE_RPC_CFW_GetPlmn
#define DISABLE_RPC_CFW_EmodGetGsmCalibInfo
#define DISABLE_RPC_CFW_SsSetCallWaiting
#define DISABLE_RPC_CFW_CcExtConfCallDial
#define DISABLE_RPC_CFW_CfgGetStoredPlmnList
#define DISABLE_RPC_CFW_nvSetLocPriority
#define DISABLE_RPC_vois_SetCfg
#define DISABLE_RPC_CFW_CheckTestSim
#define DISABLE_RPC_CFW_GetT3302
#define DISABLE_RPC_CFW_GetT3245
#define DISABLE_RPC_CFW_GprsGetSmsSeviceMode
#define DISABLE_RPC_CFW_GprsGetCtxEQos
#define DISABLE_RPC_CFW_GprsClrIpDns
#define DISABLE_RPC_CFW_SimUpdateBinary
#define DISABLE_RPC_CFW_SimUpdateBinaryV2
#define DISABLE_RPC_CFW_RpmSetRpmCR1
#define DISABLE_RPC_CSW_SetAudioZspNxpCalibParam
#define DISABLE_RPC_CFW_CheckIfAllowedRecvSms
#define DISABLE_RPC_CFW_EmodSetLteLockFreq
#define DISABLE_RPC_CFW_SetInitStack
#define DISABLE_RPC_CFW_CfgGetPbkStrorageInfo
#define DISABLE_RPC_CFW_GetSimStatus
#define DISABLE_RPC_CFW_CbSetContext
#define DISABLE_RPC_DM_ZspMusicVibrateStart
#define DISABLE_RPC_CFW_EmodRrtmParam
#define DISABLE_RPC_CFW_NwErrcStatisCfg
#define DISABLE_RPC_CFW_GprsGetReqQos
#define DISABLE_RPC_cfw_SimReadElementaryFile
#define DISABLE_RPC_CFW_ReleaseCID
#define DISABLE_RPC_CFW_EmodGetAllMemoryInfo
#define DISABLE_RPC_CFW_SetLmtMaxLen
#define DISABLE_RPC_CFW_GetSimFileUpdateCount
#define DISABLE_RPC_Cfw_SetTimezone
#define DISABLE_RPC_CFW_CfgGetSmsParam
#define DISABLE_RPC_CFW_CfgSetEventReport
#define DISABLE_RPC_CFW_CcInitiateSpeechCallEx_V2
#define DISABLE_RPC_DM_ZspMusicPlayStartEx
#define DISABLE_RPC_CFW_GetGsmBlkList
#define DISABLE_RPC_CFW_SmsSetRead2UnRead
#define DISABLE_RPC_CFW_SimDeleteMessage
#define DISABLE_RPC_aud_ClrBtClk
#define DISABLE_RPC_CFW_ImsSetVolte
#define DISABLE_RPC_CFW_SatGetRefreshFilesList
#define DISABLE_RPC_CFW_SsQueryCallForwarding
#define DISABLE_RPC_CFW_SimlockFacauthPubkey
#define DISABLE_RPC_CFW_GprsSendDataAvaliable
#define DISABLE_RPC_CFW_SetNWTimerOutFlag
#define DISABLE_RPC_hal_WriteLteCalib
#define DISABLE_RPC_CFW_EmodUpdateSimBind
#define DISABLE_RPC_CFW_WifiSentestGetResult
#define DISABLE_RPC_CFW_CfgNwGetOperatorId
#define DISABLE_RPC_CFW_GprsGetNetEQos
#define DISABLE_RPC_CFW_SetCSPagingFlag
#define DISABLE_RPC_CFW_CfgSetAudioOutputParam
#define DISABLE_RPC_CFW_SetLimiteTimer
#define DISABLE_RPC_CFW_SimSetPUCT
#define DISABLE_RPC_CFW_EmodSetTraceFlag
#define DISABLE_RPC_aud_SetLdoVB
#define DISABLE_RPC_CFW_SatRefreshFileChange
#define DISABLE_RPC_CFW_nvSetEdrxPtw
#define DISABLE_RPC_DM_AudSetup
#define DISABLE_RPC_CFW_NwGetImei
#define DISABLE_RPC_CFW_SetComm
#define DISABLE_RPC_DM_VoisSetBtNRECFlag
#define DISABLE_RPC_CFW_NwSetBSF
#define DISABLE_RPC_CFW_NstConfig
#define DISABLE_RPC_CFW_NwSetnSecurityUsedFlag
#define DISABLE_RPC_CFW_GetUsrHisFreqInfo
#define DISABLE_RPC_CFW_GetDftPdnInfoFromAP
#define DISABLE_RPC_CFW_SatGetMenuItemByID
#define DISABLE_RPC_CFW_CfgGetEventReport
#define DISABLE_RPC_CFW_NwSetPocSimIndex
#define DISABLE_RPC_CFW_NwSetXCAP
#define DISABLE_RPC_CFW_ImsFreeUti
#define DISABLE_RPC_CFW_CfgGetPeriodTauAftLocalRel
#define DISABLE_RPC_CFW_SatGetLanguageNotification
#define DISABLE_RPC_CFW_EmodOutfieldTestEnd
#define DISABLE_RPC_CFW_GetFreqScanEnd
#define DISABLE_RPC_CFW_SmsSetClass0StorageStatus
#define DISABLE_RPC_CFW_SmsGetClass0StorageStatus
#define DISABLE_RPC_CFW_SetMeaReportFlag
#define DISABLE_RPC_CFW_GetMeaReportFlag
#define DISABLE_RPC_CFW_NwSetWeakSignalEnable
#define DISABLE_RPC_CFW_NwGetWeakSignalEnable
#define DISABLE_RPC_CFW_NwSetBandPowerBoost
#define DISABLE_RPC_CFW_NwSetPrePlmnList
#define DISABLE_RPC_CFW_NwSetDrxCycleLen
#define DISABLE_RPC_CFW_NwGetDrxCycleLen
#define DISABLE_RPC_CFW_NwSetRachFailBar
#define DISABLE_RPC_CFW_NwGetRachFailBar
#define DISABLE_RPC_CFW_NwGetSupportRat
#define DISABLE_RPC_CFW_SmsSetSmFullForwardFlashFlag
#define DISABLE_RPC_CFW_SmsGetSmFullForwardFlashFlag
#define DISABLE_RPC_CFW_CfgSetCombineDetachFlag
#define DISABLE_RPC_CFW_CfgGetCombineDetachFlag
#define DISABLE_RPC_CFW_CfgSetUseRplmnRatFlag
#define DISABLE_RPC_CFW_CfgGetUseRplmnRatFlag
#define DISABLE_RPC_CFW_EmodGetLteBandCalibInfo
#define DISABLE_RPC_CFW_TestCardAutoPdnControl
#define DISABLE_RPC_CFW_SmsSetSpecialDcsFlag
#define DISABLE_RPC_CFW_SmsGetSpecialDcsFlag
#define DISABLE_RPC_CFW_NwSetCdrxRptCfg //merge patch SPCSS01326898 by fibocom
#else /*!defined(CONFIG_RPC_CFW_API)*/
#define DISABLE_RPC_sys_nst_config
#define DISABLE_RPC_sys_nst_getrssi
#define DISABLE_RPC_sys_start_nst_mode
#define DISABLE_RPC_sys_stop_nst_mode
#define DISABLE_RPC_osi_get_allMemory_Info
#define DISABLE_RPC_rfd_get_rf_powerrange
#define DISABLE_RPC_rfd_set_rf_powerrange
#define DISABLE_RPC_rf_getlte_calibbandinfo
#ifdef CONFIG_SOC_8850
#ifndef CONFIG_AUDIO_VOICE_ENABLE
#define DISABLE_RPC_AUD_VoiceCoderChangeInd
#endif
#define DISABLE_RPC_AUD_SetCodecOpStatus
#define DISABLE_RPC_AUD_ToneStopEnd
#define DISABLE_RPC_CFW_GetSimLockStatusFromAP
#endif

#endif /*!defined(CONFIG_RPC_CFW_API)*/

#endif
