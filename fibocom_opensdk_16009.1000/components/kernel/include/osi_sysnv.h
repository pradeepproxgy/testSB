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

#ifndef _OSI_SYSNV_H_
#define _OSI_SYSNV_H_

#include "kernel_config.h"
#include "osi_compiler.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fibocom.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_KERNEL_SYSNV_ENABLED
#ifdef __SYSNV_DEFINE_VAR
#define OSI_SYSNV_VAR(decl, ...) OSI_DEF_GLOBAL_VAR(decl, ##__VA_ARGS__)
#else
#define OSI_SYSNV_VAR(decl, ...) OSI_DECL_GLOBAL_VAR(decl, ##__VA_ARGS__)
#endif
#else
#define OSI_SYSNV_VAR(decl, ...) OSI_DEF_CONST_VAR(decl, ##__VA_ARGS__)
#endif

#define OSI_SYSNV_OPT(decl, ...) OSI_DEF_CONST_VAR(decl, ##__VA_ARGS__)

#ifdef CONFIG_MC661_CN_19_70_YINGSHI
OSI_SYSNV_VAR(bool gSysnvTraceEnabled, false);
#else
OSI_SYSNV_VAR(bool gSysnvTraceEnabled, true);
#endif
OSI_SYSNV_VAR(bool gSysnvCalibTraceEnabled, false);
OSI_SYSNV_VAR(bool gSysnvDeepSleepEnabled, true);
#ifdef CONFIG_KERNEL_PSM_DEFAULT_ENABLE
OSI_SYSNV_VAR(bool gSysnvPsmSleepEnabled, true);
#else
OSI_SYSNV_VAR(bool gSysnvPsmSleepEnabled, false);
#endif
OSI_SYSNV_VAR(bool gSysnvUsbMode, 0);
#ifdef CONFIG_FIBOCOM_ECM_ENABLE
OSI_SYSNV_VAR(int gSysnvUsbWorkMode, 5); //if is in linux test auto ecm, at+gtusbmode? value should is 74
#elif defined(CONFIG_FIBOCOM_RNDIS_ENABLE)
OSI_SYSNV_VAR(int gSysnvUsbWorkMode, 3); //if is in windows test auto gtrndis, at+gtusbmode? value should is 75, we can use gtusbmode set value switch in win or linux.
#else
OSI_SYSNV_VAR(int gSysnvUsbWorkMode, 2);
#endif
OSI_SYSNV_VAR(int gSysnvTraceDevice, 1); // osi_trace.h
#ifdef CONFIG_FIBOCOM_BASE
OSI_SYSNV_VAR(int gSysnvDiagDevice, 2); // diag.h
#else
OSI_SYSNV_VAR(int gSysnvDiagDevice, 1);  // diag.h
#endif

#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_FIBOCOM_SIMHOTPLUG_ENABLE
OSI_SYSNV_VAR(bool gSysnvSimHotPlug, true);
#else
OSI_SYSNV_VAR(bool gSysnvSimHotPlug, false);
#endif
OSI_SYSNV_VAR(bool gSysnvSimVoltTrigMode, false);
#else
#if defined(CONFIG_SOC_8850)
OSI_SYSNV_VAR(bool gSysnvSimVoltTrigMode, false);
#elif defined(CONFIG_SOC_8910)
OSI_SYSNV_VAR(bool gSysnvSimVoltTrigMode, true);
#else
OSI_SYSNV_VAR(bool gSysnvSimVoltTrigMode, true);
#endif
#endif
OSI_SYSNV_VAR(int gSysnvUsbDetMode, 0); // drv_usb.h
#if defined(CONFIG_FIBOCOM_INTERNAL_EXTERNAL_SHARE_IP)//8850 neiwaizhi use same netif need AT+SYSNV=1,"nat_cfg",255
OSI_SYSNV_VAR(int gSysnvNATCfg, 255);
#else
OSI_SYSNV_VAR(int gSysnvNATCfg, 0);
#endif
OSI_SYSNV_VAR(unsigned gSysnvFstraceMask, 0);
OSI_SYSNV_VAR(bool gSysnvPoffChargeEnable, false);
OSI_SYSNV_VAR(bool gSysnvPm2GpioInd, false);
OSI_SYSNV_VAR(bool gSysnvPm3GpioInd, false);
OSI_SYSNV_VAR(bool gSysnvUart1RxPsmPullUp, false);
OSI_SYSNV_VAR(bool gSysnvPm2BootEnabled, false);
OSI_SYSNV_VAR(int gSysnvSelfRegisterEnabled, 0);
#ifndef CONFIG_SOC_8811
OSI_SYSNV_VAR(int gSysnvLogOutput, 0);
#endif
OSI_SYSNV_VAR(int gSysnvSysWdtFeedPeriod, 5000);
OSI_SYSNV_VAR(int gSysnvPmicWdtFeedPeriod, 4000);
#ifdef CONFIG_SOC_8850
OSI_SYSNV_VAR(bool gSysnvImsLogOutput, true);
#endif
#if defined(CONFIG_WDT_ENABLE)
OSI_SYSNV_VAR(bool gBsWatchdogReset, true);
#endif
OSI_SYSNV_VAR(int gSysnvNwRatMode, 4);

/**
 * load system NV from persitent storage
 *
 * After load, the system NV in memory will be changed.
 *
 * \return
 *      - true on success
 *      - false on error
 */
bool osiSysnvLoad(void);

/**
 * save system NV to persitent storage
 *
 * \return
 *      - true on success
 *      - false on error
 */
bool osiSysnvSave(void);

/**
 * clear system NV from persitent storage
 */
void osiSysnvClear(void);

/**
 * get system NV encoded buffer size
 *
 * System NV encoded size may change when value changed.
 *
 * \return  encoded system NV buffer size
 */
int osiSysnvSize(void);

/**
 * load encoded system NV to buffer
 *
 * \p size must be larger than or equal to encoded system NV size, which
 * can be get by \p osiSysnvSize.
 *
 * \param buf   buffer for encoded system NV
 * \param size  buffer size
 * \return
 *      - encoded system NV size
 *      - -1 on error
 */
int osiSysnvLoadToMem(void *buf, size_t size);

/**
 * save buffer to system NV persistent storage
 *
 * After save, the system NV value in effect keep unchanged. It is
 * recommended to reboot system to make new value take effect.
 *
 * It is only used for PC tools. The content in persistent storage is encoded,
 * and the provided buffer must match the codec in platform.
 *
 * \param buf       buffer to be written
 * \param size      buffer size
 * \return
 *      - byte count written, should be the same as \p size
 *      - -1 if write failed
 */
int osiSysnvSaveFromMem(const void *buf, size_t size);
#ifdef CONFIG_FIBOCOM_BASE
int Getusbmode();
#endif

#ifdef __cplusplus
}
#endif
#endif
