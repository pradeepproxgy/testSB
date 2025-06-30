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

#ifndef _NVM_8811_H_
#define _NVM_8811_H_

#include "hal_config.h"
#include "osi_compiler.h"
#include "nvm_item_id_8811.h"

OSI_EXTERN_C_BEGIN

#define NV_FULL_NAME_MAX (40)

#define FACTORYNV_DIR CONFIG_FS_FACTORY_MOUNT_POINT
#define RUNNINGNV_DIR CONFIG_FS_MODEM_NVM_DIR

#define PHASECHECK_FNAME FACTORYNV_DIR "/phasecheck.bin"
#define NVDIRECT_FNAME FACTORYNV_DIR "/imei.bin"

// The order is important. In NV .prj file, the order of modules should
// match the order in the array.
//
// It is possible that ResearchDownload will perform "CHECK_NV". The
// comparison base is the generated nvbin, and the compared data is the
// result of nvmReadFixedBin. And memory comparison (except the leading
// 4 bytes) is used.

static const nvDescription_t gNvDesc[] = {
    {NVID_IMEI1, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI2, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI3, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_IMEI4, 0, NULL, FACTORYNV_DIR, NULL},
    {NVID_MODEM_RF_NV, FIXNV_COMPRESS_ENABLE | RUNNINGNV_COMPRESS_ENABLE, "rf_nv.bin", FACTORYNV_DIR, RUNNINGNV_DIR},
    {NVID_CALIB_PARAM, FIXNV_COMPRESS_ENABLE, "calibparam.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_CALIB_GSM, FIXNV_COMPRESS_ENABLE, "gsm_rf_calib.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_CALIB_LTE, FIXNV_COMPRESS_ENABLE, "lte_rf_calib.bin", FACTORYNV_DIR, NULL},
    {NVID_RF_GOLDEN_BOARD, FIXNV_COMPRESS_ENABLE, "golden_board.bin", FACTORYNV_DIR, NULL},
    {NVID_NB_NV, FIXNV_COMPRESS_ENABLE | RUNNINGNV_COMPRESS_ENABLE, "nb_static_nv.bin", FACTORYNV_DIR, RUNNINGNV_DIR},
};

OSI_EXTERN_C_END
#endif
