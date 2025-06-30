/*
 * <audio_nv_arm.h> - <This file defines the basic operation interfaces of manager>
 *
 * Copyright (c) 2019 Unisoc Communications Inc.
 * History
 *      <2021/08/05> <Xiaoyu.He> <create>
 *      This file defines the basic operation interfaces of manager of audio mode controlled and
 * used by arm.
 */

#ifndef _AUDIO_NV_ARM_H
#define _AUDIO_NV_ARM_H

#include "osi_api.h"
#include "osi_log.h"
#include "osi_mem.h"
#include "nvm.h"
//#ifdef CVSRCD_ENABLE
#include "CVS_frame_recd.h"
//#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define PACKED
#else
#define PACKED __packed
#endif
#define AUDIO_NV_ARM_MODE_NAME_MAX_LEN (16)
#define AUDIO_ARM_VOLUME_LEVEL (31)     //15 31
#define AUDIO_NV_ARM_APP_PARA_RESERVE (9)  //10
#define AUDIO_NV_ARM_PARA_RESERVE (64)

#define CONST
#define PUBLIC
#define SRVAPI
#define PROTECTED
#define LOCAL static
#define PRIVATE static

typedef enum _NVITEM_ERROR {
    NVERR_NONE  = 0,   			/* Success */    
    NVERR_SYSTEM,      			/* System error, e.g. hardware failure */
    NVERR_INVALID_PARAM,
    NVERR_NO_ENOUGH_RESOURCE,
    NVERR_NOT_EXIST,
    NVERR_ACCESS_DENY,
    NVERR_INCOMPATIBLE,
    NVERR_NOT_OPENED
}NVITEM_ERROR_E;

#define     NVID_ARM_AUDIO_MODE_NUM   0x1a9   //425
#define    NVID_ARM_AUDIO_MODE_PARA   0x1aa  //426

/**
 * Audio device mode type list.
 */
typedef enum {
	AUDIO_DEVICE_MODE_HANDHOLD,
	AUDIO_DEVICE_MODE_HANDFREE,
	AUDIO_DEVICE_MODE_EARPHONE,
	AUDIO_DEVICE_MODE_EARPHONE_3P,
	AUDIO_DEVICE_MODE_EARFREE,
	AUDIO_DEVICE_MODE_TVOUT,
	AUDIO_DEVICE_MODE_BLUEPHONE,
	AUDIO_DEVICE_MODE_BLUEPHONE_NREC,
	AUDIO_DEVICE_MODE_HANDFREE_EXT,
	AUDIO_DEVICE_MODE_MAX,
	AUDIO_DEVICE_MODE_CUR
}AUDIO_DEVICE_MODE_TYPE_E;

typedef enum {
	AUDIO_DEVICE_MODE_PARAM_PLAY = 0,
	AUDIO_DEVICE_MODE_PARAM_RECORD,
	AUDIO_DEVICE_MODE_PARAM_RECORD_LINEIN,
	AUDIO_DEVICE_MODE_PARA_RESERVE0,
	AUDIO_DEVICE_MODE_PARAM_RESERVE1,
	AUDIO_DEVICE_MODE_PARAM_RESERVE2,
	AUDIO_DEVICE_MODE_PARAM_RESERVE3,
	AUDIO_DEVICE_MODE_PARAM_RESERVE4,
	AUDIO_DEVICE_MODE_PARAM_MAX
}AUDIO_DEVICE_MODE_PARAM_TYPE_E;

/**
 * @AUDIO_AGC_INPUG_GAIN_MP3 : agc type mp3.
 * @AUDIO_AGC_INPUG_GAIN_MIDI : agc type midi.
 * @AUDIO_AGC_INPUG_GAIN_AMR : agc type amr.
 */
typedef enum {
	AUDIO_AGC_INPUG_GAIN_MP3 = 0,
	AUDIO_AGC_INPUG_GAIN_MIDI,
	AUDIO_AGC_INPUG_GAIN_AMR,
	AUDIO_AGC_INPUG_GAIN_RESERVE0,
	AUDIO_AGC_INPUG_GAIN_RESERVE1,
	AUDIO_AGC_INPUG_GAIN_RESERVE2,
	AUDIO_AGC_INPUG_GAIN_RESERVE3,
	AUDIO_AGC_INPUG_GAIN_RESERVE4,
	AUDIO_AGC_INPUG_GAIN_MAX
}AUDIO_AGC_INPUG_GAIN_TYPE_E;

/**
 * @AUDIO_NV_ARM_ALLOC_ERROR : alloc error.
 * @AUDIO_NV_ARM_INPUT_PARA_ERROR : input parameter error.
 * @AUDIO_NV_ARM_MODE_NO_EXIST : the mode with this name is not exist.
 */
 #define BIT_0 0x00000001
#define BIT_1 0x00000002
#define BIT_2 0x00000004
#define BIT_3 0x00000008

typedef enum {
	AUDIO_NV_ARM_NO_ERROR = 0x0,
	AUDIO_NV_ARM_ALLOC_ERROR = BIT_0,
	AUDIO_NV_ARM_INPUT_PARA_ERROR = BIT_1,
	AUDIO_NV_ARM_MODE_NO_EXIST = BIT_3,

	AUDIO_NV_ARM_ERROR
}AUDIO_NV_ARM_RESULT_E;

/**
 * type of app, like:ring tone;on off tone;key tone;multimedia play(mp3/aac/....)
 * according to application
 */
typedef enum {
	AUDIO_ARM_APP_TYPE_0,
	AUDIO_ARM_APP_TYPE_1,
#ifdef ARM_NV_VOL_TYPE_8
	AUDIO_ARM_APP_TYPE_2,
	AUDIO_ARM_APP_TYPE_3,
	AUDIO_ARM_APP_TYPE_4,
	AUDIO_ARM_APP_TYPE_5,
	AUDIO_ARM_APP_TYPE_6,
	AUDIO_ARM_APP_TYPE_7,
#endif
	AUDIO_ARM_APP_TYPE_MAX
}AUDIO_ARM_APP_TYPE_E;

typedef enum {
	NV_AUDIO_ARM_HANDSET_E,
	NV_AUDIO_ARM_HANDSFREE_E,
	NV_AUDIO_ARM_HEADSET_E,
	NV_AUDIO_ARM_EARFREE_E,
	NV_AUDIO_ARM_BTHS_E,
	NV_AUDIO_ARM_MAX
	}NV_AUDIO_ARM_E_;
typedef uint32 NV_AUDIO_ARM_E;

typedef struct Audio_Nv_Arm_Device_Path_Struct {
	uint16 valid_dev_set_count;
	//AUDMOD_DEVICECTL_T bit0-bit7:
	//ucADChannel&0x1,
	//ucDAChannel&0x1,
	//ucDA0Reverse&0x1,
	//ucDA1Reverse&0x1,
	//ucStereoEn&0x1,
	//ucSpkAmpEn&0x1,
	//ucSpkAmpEn&0x1,
	//ucMixerInput&0x1
	uint16 reserve;//shujing add for align
	uint16 dev_set[AUDIO_DEVICE_MODE_PARAM_MAX];
}AUDIO_NV_ARM_DEVICE_PATH_T;

/**
 * include eq_switch/agc_input_gain/arm_volume_table.
 */
typedef struct Audio_Nv_Arm_App_Config_Info_Struct {
	uint16 eq_switch;
	uint16 agc_input_gain[AUDIO_AGC_INPUG_GAIN_MAX];
	uint16 valid_volume_level_count;
	uint32 arm_volume[AUDIO_ARM_VOLUME_LEVEL];
	uint16 reserve[AUDIO_NV_ARM_APP_PARA_RESERVE];
}AUDIO_NV_ARM_APP_CONFIG_INFO_T;

/**
 * @aud_proc_exp_control :switch of agc/lcf/eq_select--original dsp audio nv:extend2[110].
 */
typedef struct Audio_Nv_Arm_App_Set_Struct {
	uint16 valid_app_set_count;
	uint16 valid_agc_input_gain_count;
	uint16 aud_proc_exp_control[2];
	AUDIO_NV_ARM_APP_CONFIG_INFO_T app_config_info[2];
}AUDIO_NV_ARM_APP_SET_T;

typedef struct Audio_Nv_Arm_Mode_Struct {
	AUDIO_NV_ARM_DEVICE_PATH_T dev_path_set;
	AUDIO_NV_ARM_APP_SET_T app_config_info_set;
	uint16 midi_opt;
	uint16 aud_dev;
	uint16 reserve[AUDIO_NV_ARM_PARA_RESERVE];
//#ifdef CVSRCD_ENABLE
	CVSI_CTRL_PARAM_recd cvs_ctrl_param_rcd;
//#endif
}AUDIO_NV_ARM_MODE_STRUCT_T;

/**
 * @ucModeName :node name.
 * @tAudioNvArmModeStruct :Audio structure.
 */
typedef struct Audio_Nv_Arm_Mode_Info_struct {
	uint8 ucModeName[AUDIO_NV_ARM_MODE_NAME_MAX_LEN];
	AUDIO_NV_ARM_MODE_STRUCT_T tAudioNvArmModeStruct;
}AUDIO_NV_ARM_MODE_INFO_T;

/**
 * This Function is used to init Mode ARM Manager.
 * return Operation results. \sa AUDIO_NV_ARM_RESULT_E
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_InitModeManager(void);

/**
 * This Function is used to get the number of mode in the current mode manager.
 * return The total number of registered modes.
 */
PUBLIC uint32 AUDIONVARM_GetModeNumber(void);

/**
 * This Function is used to add one mode into mode manager.
 * @parameter ptMode : Added mode.
 * return Operation results. \sa AUDIO_NV_ARM_RESULT_E.
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_AddMode(
	AUDIO_NV_ARM_MODE_INFO_T* ptMode);

/**
 * This Function is used to remove one mode from mode manager.
 * @parameter pucModeName : mode name which is needed to remove from this mode manager.
 * return Operation results. \sa AUDIO_NV_ARM_RESULT_E.
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_RemoveMode(
	const char* pucModeName);

/**
 * This Function is used to get the named mode information from linked list managed by Device Mode Manager.
 * @parameter pucModeName : mode name.
 * @parameter ptMode : [OUT] Address which the mode parameter output.
 * return Operation results. \sa AUDIO_NV_ARM_RESULT_E.
 */
PUBLIC  AUDIO_NV_ARM_RESULT_E AUDIONVARM_GetModeParam(
	const char* pucModeName,
	AUDIO_NV_ARM_MODE_INFO_T* ptMode);

/**
 * This Function is used to set mode parameters to the appointed device mode managed by Mode Manager.
 * @parameter pucModeName : mode name.
 * @parameter ptMode : [IN] Address which the mode parameter saved.
 * return Operation results. \sa AUDIO_NV_ARM_RESULT_E.
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_SetModeParam(
	const char* pucModeName,
	AUDIO_NV_ARM_MODE_INFO_T* ptMode);

/**
 * This Function is used to get the mode name by index of arm nv list in ram.
 * @parameter index : arm nv index.
 * return mode name. \sa char*.
 */
PUBLIC char *AUDIONVARM_GetAudioModeName (
	uint32 index);

/**
 * This Function is used to get the mode name by arm nv id.
 * @parameter arm_nv_id : arm nv id.
 * return mode name. \sa char*.
 */
PUBLIC char *AUDIONVARM_GetAudioModeNameById (
	NV_AUDIO_ARM_E arm_nv_id);

/**
 * This Function is used to get the mode type by mode name.
 * @parameter mode_name_ptr : nv name ptr.
 * return mode name. \sa char*.
 */
PUBLIC AUDIO_DEVICE_MODE_TYPE_E AUDIONVARM_GetAudioModeType(
	char *mode_name_ptr);

/**
 * This Function is used to get the device control info from the named mode information.
 * @parameter pucModeName : mode name, its max length is not more than 16 bytes..
 * @parameter eModeParaType : Mode para type.
 * @parameter ptDevCtrInfo : [OUT] Address to save the device basic control info in the named mode.
 * return Operation resules. \sa AUDIO_NV_ARM_RESULT_E
 */
 /*
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_GetAudioModeDevCtrInfo (
	const char* pucModeName,
	AUDIO_DEVICE_MODE_PARAM_TYPE_E eModeParaType,
	AUDMOD_DEVICECTL_T *ptDevCtrInfo);
*/
/**
 * This Function is used to get the named mode information from flash.
 * @parameter pucModeName : mode name, its max length is not more than 16 bytes..
 * @parameter ptMode :[OUT] Address which the mode parameter output.
 * return Operation resules. \sa AUDIO_NV_ARM_RESULT_E
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_ReadModeParamFromFlash(
	const char* pucModeName,
	AUDIO_NV_ARM_MODE_INFO_T* ptMode);

/**
 * This Function is used to set the named mode information from flash.
 * @parameter pucModeName : mode name, its max length is not more than 16 bytes.
 * @parameter ptMode :[IN] Address which the new mode parameter saved.
 * return Operation resules. \sa AUDIO_NV_ARM_RESULT_E
 */
PUBLIC AUDIO_NV_ARM_RESULT_E AUDIONVARM_WriteModeParamToFlash(
	const char* pucModeName,
	AUDIO_NV_ARM_MODE_INFO_T* ptMode);

NVITEM_ERROR_E Audio_NvitemRead(
    uint16 identifier,
    uint16 count,
    uint8 *buf_ptr);
NVITEM_ERROR_E Audio_NvitemWrite(
    uint16 identifier,
    uint16 count,
    const uint8 *buf_ptr,
    uint8 basync);

#ifdef __cplusplus
}
#endif

#endif 
