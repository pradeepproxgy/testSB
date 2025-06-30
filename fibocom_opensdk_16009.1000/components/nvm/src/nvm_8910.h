/* Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _NVM_8910_H_
#define _NVM_8910_H_

#include "hal_config.h"
#include "osi_compiler.h"
#include "nvm_config.h"

OSI_EXTERN_C_BEGIN

#define NV_FULL_NAME_MAX (40)

#define FACTORYNV_DIR CONFIG_FS_FACTORY_MOUNT_POINT
#define MODEMNV_DIR CONFIG_FS_MODEM_MOUNT_POINT "/nvm"
#define RUNNINGNV_DIR CONFIG_FS_MODEM_NVM_DIR

#define PHASECHECK_FNAME FACTORYNV_DIR "/phasecheck.bin"
#define NVDIRECT_FNAME FACTORYNV_DIR "/imei.bin"

enum
{
    NVID_MODEM_STATIC_NV = 0x259,
    NVID_MODEM_PHY_NV = 0x25a,
    NVID_MODEM_DYNAMIC_NV = 0x25b,
    NVID_MODEM_STATIC_CARD2_NV = 0x25c,
    NVID_MODEM_PHY_CARD2_NV = 0x25d,
    NVID_MODEM_DYNAMIC_CARD2_NV = 0x25e,

    NVID_MODEM_RF_NV = 0x26c,
    NVID_RF_CALIB_GSM = 0x26d,
    NVID_RF_CALIB_LTE = 0x26e,
    NVID_RF_GOLDEN_BOARD = 0x26f,

    NVID_CFW_NV = 0x277,
    NVID_AUD_CALIB = 0x278,
    NVID_PHY_NV_CATM = 0x279,
    NVID_AUD_CODEC_NV = 0x27a,
    NVID_AUD_POC_NV = 0x27b,

    NVID_BT_CONFIG = 0x191,
    NVID_BT_ADDRESS = 0x192,
    NVID_BT_SPRD = 0x1ba,
};

static const nvDescription_t gNvDesc[] = {
    {NVID_IMEI1, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI2, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI3, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI4, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_CALIB_PARAM, NV_COMPRESS_ENABLE, "calibparam.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_CALIB_GSM, NV_COMPRESS_ENABLE, "gsm_rf_calib.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_CALIB_LTE, NV_COMPRESS_ENABLE, "lte_rf_calib.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_GOLDEN_BOARD, NV_COMPRESS_ENABLE, "golden_board.bin", FACTORYNV_DIR, NULL},
    {NVID_BT_CONFIG, NV_COMPRESS_ENABLE, "bt_config.bin", FACTORYNV_DIR, NULL},

    {NVID_MODEM_STATIC_NV, NV_COMPRESS_ENABLE, "static_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_PHY_NV, NV_COMPRESS_ENABLE, "phy_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_DYNAMIC_NV, NV_COMPRESS_ENABLE, "dynamic_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_STATIC_CARD2_NV, NV_COMPRESS_ENABLE, "static_nv_2.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_PHY_CARD2_NV, NV_COMPRESS_ENABLE, "phy_nv_2.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_DYNAMIC_CARD2_NV, NV_COMPRESS_ENABLE, "dynamic_nv_2.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_MODEM_RF_NV, NV_COMPRESS_ENABLE, "rf_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_CFW_NV, NV_COMPRESS_ENABLE, "cfw_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_AUD_CALIB, NV_COMPRESS_ENABLE, "audio_calib.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_BT_ADDRESS, NV_COMPRESS_ENABLE, "db_record.bin", RUNNINGNV_DIR, RUNNINGNV_DIR}, //no init data
    {NVID_BT_SPRD, NV_COMPRESS_ENABLE, "bt_sprd.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_AUD_CODEC_NV, NV_COMPRESS_ENABLE, "audio_codec_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_AUD_POC_NV, NV_COMPRESS_ENABLE, "audio_poc_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_PHY_NV_CATM, NV_COMPRESS_ENABLE, "phy_nv_catm.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_SIM_LOCK_CUSTOMIZE_DATA, NV_COMPRESS_ENABLE, "sim_lock_customize_data.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_SIM_LOCK_USER_DATA, NV_COMPRESS_ENABLE, "sim_lock_user_data.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_SIM_LOCK_CONTROL_KEY_ID, NV_COMPRESS_ENABLE, "sim_lock_control_key.bin", MODEMNV_DIR, RUNNINGNV_DIR},

    {NVID_IMS_CSM_NV, NV_COMPRESS_ENABLE, "ims_csm_setting.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_IMS_SAPP_NV, NV_COMPRESS_ENABLE, "ims_sapp_setting.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_IMS_ISIM_NV, NV_COMPRESS_ENABLE, "ims_isim_setting.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_SIM_DELTA_NV, NV_COMPRESS_ENABLE, "sim_delta_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_SIM_DELTA_CARD2_NV, NV_COMPRESS_ENABLE, "sim_delta_nv_card2.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_PLMN_DELTA_NV, NV_COMPRESS_ENABLE, "plmn_delta_nv.bin", MODEMNV_DIR, RUNNINGNV_DIR},
    {NVID_PLMN_DELTA_CARD2_NV, NV_COMPRESS_ENABLE, "plmn_delta_nv_card2.bin", MODEMNV_DIR, RUNNINGNV_DIR},
};

OSI_EXTERN_C_END
#endif
