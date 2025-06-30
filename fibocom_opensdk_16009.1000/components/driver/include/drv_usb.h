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

#ifndef _DRV_USB_H_
#define _DRV_USB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <osi_api.h>
#include "hal_config.h"
#include "fibocom.h"

#define USB_VID_SPRD (0x1782)
#define USB_VID_RDA_DOWNLOAD (0x0525)

#define USB_PID_RDA_DOWNLOAD (0xa4a7)
#define USB_PID_SERIALS (0x4d10)
#define USB_PID_RNDIS_AND_SERIALS (0x4d11)
#define USB_PID_NPI_SERIAL (0x4d12)
#define USB_PID_ECM_AND_SERIALS (0x4d13)
#define USB_PID_ECM_ACM_SERIAL (0x4d15)
#define USB_PID_MTP_SERIAL (0x4d32)

//8850
#define USB_PID_8850_SERIALS (0x4d17)
#define USB_PID_8850_ACM_SERIALS (0x4d18)
#define USB_PID_8850_RNDIS_AND_SERIALS (0x4d19)
#define USB_PID_8850_ECM_AND_SERIALS (0x4d20)
#define USB_PID_8850_NPI_SERIAL (0x4d30)
#define USB_PID_8850_MTP_SERIAL (0x4d31)


#ifdef CONFIG_FIBOCOM_BASE
//fibocom-MC669
#define FIBO_USB_VID_8850                    (0x2CB7)
#define FIBO_USB_PID_8850_SERIALS            (0x0A0A)
#define FIBO_USB_PID_8850_ECM_AND_SERIALS    (0x0A0B)
#define FIBO_USB_PID_8850_RNDIS_AND_SERIALS  (0x0A0C)

#define ELEC_USB_VID_8850                    (0x3763)
#define ELEC_USB_PID_8850_ECM_AND_SERIALS    (0x3C93)

#endif

enum usb_detect_mode
{
    USB_DETMODE_CHARGER = 0,
    USB_DETMODE_AON = 1,
};

typedef enum
{
    DRV_USB_CHARGER_ONLY = 0,
    DRV_USB_RDA_ROM_SERIAL,                         // [USL0], (0525, a4a7)
    DRV_USB_SERIALS,                                // [USL[0-7]], (1782, 4d10)
    DRV_USB_RNDIS_AND_SERIALS,                      // [RNDIS, USL[0-7]], (1782, 4d11)
    DRV_USB_NPI_SERIAL,                             // [USL1], (1782, 4d12)
    DRV_USB_ECM_AND_SERIALS,                        // [ECM, USL[0-7]], (1782, 4d11)
    DRV_USB_ECM_ACM_SERIALS,                        // [ECM, USL0, USL4, ACM0, ACM1, ACM2, ACM3], (1782, 4d15)
    DRV_USB_SINGLE_INTF_START,                      // single interface work mode start
    DRV_USB_RNDIS_ONLY = DRV_USB_SINGLE_INTF_START, // (1782, 4d11)
    DRV_USB_ECM_ONLY,                               // (1782, 4d11)
    DRV_USB_MTP_ONLY,                               // (1782, 4d11)
    DRV_USB_SINGLE_INTF_END = DRV_USB_MTP_ONLY,     // single interface work mode end
    DRV_USB_HID_ONLY,
    DRV_USB_WORK_MODE_NUM,
} drvUsbWorkMode_t;

typedef enum
{
    DRV_8850USB_CHARGER_ONLY,
    DRV_8850USB_ACM_SERIALS,
    DRV_8850USB_SERIALS,
    DRV_8850USB_RNDIS_AND_SERIALS,
    DRV_8850USB_NPI_SERIAL,
    DRV_8850USB_ECM_AND_SERIALS,
    DRV_8850USB_ECM_ACM_SERIALS,
    DRV_8850USB_RNDIS_ONLY,
    DRV_8850USB_ECM_ONLY,
    DRV_8850USB_MTP_ONLY,
    DRV_8850USB_MSG_ONLY,
    DRV_8850USB_NETCARD_ADAPTION,
#ifdef CONFIG_FIBOCOM_BASE
    DRV_8850USB_ELEC_ECM,
    DRV_8850USB_SERIALS_SPECIAL,
    DRV_USB_PRINTER,
    DRV_USB_PRINTER_AND_SERIALS,
    DRV_USB_HID_ACM_SERIALS,
    DRV_8850USB_HIDKEYBOARD_AND_SERIALS,
    DRV_8850USB_SINGLE_SERIAL,
    DRV_USB_HID_ACM,
#endif
    DRV_8850USB_WORK_MODE_NUM,
} drvUsbWorkMode_8850_t;


#ifdef CONFIG_FIBOCOM_BASE
typedef void (*usbStateChangeCallback_t)(uint8_t state, void *cb_ctx);
struct usb_state_change
{
    uint8_t new_sta;
    usbStateChangeCallback_t callback;
    void *cb_ctx;
};
typedef struct usb_state_change usb_state_change_t;
#endif

/**
 * @brief Init usb module, assert if fail
 */
void drvUsbInit();

/**
 * @brief Get usb work queue
 *
 * This work queue should only be used by USB class drivers.
 *
 * @return usb work queue
 */
osiWorkQueue_t *drvUsbWorkQueue();

/**
 * @brief Set USB work mode
 *
 * If it is the same as current mode, do nothing.
 * Else if USB is working, stop it thereafter reset mode.
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false not support or fail
 */
bool drvUsbSetWorkMode(drvUsbWorkMode_t mode);

/**
 * @brief Mtp notify vfs to change file
 */
void MtpNotify();

/**
 * @brief Set USB work mode
 *
 * 8850 FPGA USB NPI SERIASL DEBUG
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false  fail
 */
bool setNPISerial();

/**
 * @brief Set USB work mode
 *
 * 8850 FPGA USB SERIASLS DEBUG
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false  fail
 */
bool setSerials();

/**
 * @brief Set USB work mode
 *
 * 8850 FPGA USB ACM SERIASLS DEBUG
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false  fail
 */
bool setACMSerials();

/**
 * @brief Set USB work mode
 *
 * 8850 FPGA USB ECM DEBUG
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false  fail
 */
bool setEcm();

/**
 * @brief Set USB work mode
 *
 * 8850 FPGA USB RNDIS DEBUG
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false  fail
 */
bool setRndis();

/**
 * @brief Get current USB work mode
 *
 * @return  current work mode
 */
drvUsbWorkMode_t drvUsbCurrentMode();

/**
 * @brief Set 8850 USB work mode
 *
 * If it is the same as current mode, do nothing.
 * Else if USB is working, stop it thereafter reset mode.
 *
 * @param mode  usb work mode
 * @return
 *      - true  success
 *      - false not support or fail
 */
bool drv8850UsbSetWorkMode(drvUsbWorkMode_8850_t mode);

/**
 * @brief Get current 8850 USB work mode
 *
 * @return  current work mode
 */
drvUsbWorkMode_8850_t drv8850UsbCurrentMode();

/**
 * @brief the Usb is connected to host or not
 * @return
 *      - true  connected
 *      - flase not connect
 */
bool drvUsbIsConnected();

/**
 * @brief Enable usb feature
 *
 * @param debounce  debounce time in milliseconds
 */
void drvUsbEnable(uint32_t debounce);

/**
 * @brief Disable usb feature
 */
void drvUsbDisable();

/**
 * @brief Enable 8850 usb feature
 *
 * @param debounce  debounce time in milliseconds
 */
void drvmUsbEnable();

/**
 * @brief Disable 8850 usb feature
 */
void drvmUsbDisable();

/**
 * @brief reset usb feature
 */
void drvUsbReset();

/**
 * @brief Check usb is enabled or not
 *
 * @return  enabled if true else false
 */
bool drvUsbIsEnabled();

/**
 * @brief wakeup the host
 *
 * @return false if not support else true
 */
bool drvUsbRemoteWakeup();

/**
 * @brief set usb cable attached
 *
 * @param attach    usb cable attached
 */
void drvUsbSetAttach(bool attach);

#ifdef CONFIG_FIBOCOM_BASE
void drv8850UsbSetId(uint16_t vid, uint16_t pid);
void drv8850UsbSetBcd(uint16_t bcd);
#ifdef CONFIG_FIBOCOM_HID_KEYBOARD_SUPPORT
void drv8850UsbHidSetATEnable(bool atEn);
#endif
#ifdef CONFIG_FIBOCOM_ENABLE_USB_SUSPEND_URC
/**
 * @brief usb suspend or other status
 *
 * @return false if not support else true
 */
bool drvUsbIsSuspend();
#endif

void drvUsbRestart();
void drvUsbStop();
#endif
#ifdef CONFIG_FIBOCOM_BASE
void drvUsbStateNotice(void);
uint8_t drvGetUsbState(void);
int32_t drvSetUsbStateCallback(usbStateChangeCallback_t cb, void* cb_ctx);
#endif
/**
 * @brief usb netcard if active
 *
 * @return false if not support else true
 */
bool drvUsbNetcardIsActive();

#ifdef __cplusplus
}
#endif

#endif // _DRV_USB_H_
