/* Copyright (C) 2020 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _USB__COPS_FUNC_H_
#define _USB__COPS_FUNC_H_

#include "osi_compiler.h"

OSI_EXTERN_C_BEGIN

#include "usb/usb_composite_device.h"
#include "fibocom.h"

typedef enum
{
    USB_FUNC_GENERIC_SERIAL = 0,
    USB_FUNC_DEBUG_SERIAL,
    USB_FUNC_CDC_ACM,
    USB_FUNC_CCID,
    USB_FUNC_CDC_ECM,
    USB_FUNC_RNDIS,
    USB_FUNC_COUNT,
} usbCopsFuncID_t;

/**
 * @brief Create a generic serial function instance
 *
 * @param name  serial name
 * @return
 *      - NULL  fail
 *      - other the serial function
 */
copsFunc_t *createSerialFunc(uint32_t name);
#ifdef CONFIG_FIBOCOM_BASE
copsFunc_t *createPrinterFunc(uint32_t name);
#endif
#ifdef CONFIG_FIBOCOM_CUSTOM_BULK_DEVICE_SUPPORT
copsFunc_t *createVendorFunc(uint32_t name);
#endif
#ifdef CONFIG_FIBOCOM_HID_KEYBOARD_SUPPORT
copsFunc_t *createHidKeyboardFunc(uint32_t name);
#endif
/**
 * @brief Create a debugging serial function instance
 *
 * @param name  serial name
 * @return
 *      - NULL  fail
 *      - other the serial function
 */
copsFunc_t *createDebugSerialFunc(uint32_t name);

/**
 * @brief Create a rndis function instance
 *
 * @return
 *      - NULL  fail
 *      - other the rndis function
 */
copsFunc_t *createRndisFunc(void);

/**
 * @brief Create an ECM function instance
 *
 * @return
 *      - NULL  fail
 *      - other the rndis function
 */
copsFunc_t *createEcmFunc(void);

/**
 * @brief Create a CCID function instance
 *
 * @param name  device name
 * @return
 *      - NULL  fail
 *      - other the CCID function
 */
copsFunc_t *createCCIDFunc(uint32_t name);

/**
 * @brief Create a CDC-ACM function
 *
 * @param name  function name
 * @return
 *      - NULL  fail
 *      - other the function
 */
copsFunc_t *createCdcAcmFunc(uint32_t name);

/**
 * @brief Create a DEBUG-CDC-ACM function
 *
 * @param name  function name
 * @return
 *      - NULL  fail
 *      - other the function
 */
copsFunc_t *createDebugCdcAcmFunc(uint32_t name);

/**
 * @brief Create a MTP function
 *
 * @param name  function name
 * @return
 *      - NULL  fail
 *      - other the function
 */
copsFunc_t *createMTPFunc(uint32_t name);

/**
 * @brief Create a MassStorage function
 *
 * @param name  function name
 * @return
 *      - NULL  fail
 *      - other the function
 */
copsFunc_t *createMassStorageFunc(uint32_t name);

/**
 * @brief Create a HID function
 *
 * @param name  function name
 * @return
 *      - NULL  fail
 *      - other the function
 */
copsFunc_t *createHidFunc(uint32_t name);

OSI_EXTERN_C_END

#endif