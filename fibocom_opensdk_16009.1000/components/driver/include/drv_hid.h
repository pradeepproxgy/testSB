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

#ifndef _DRV_HID_H_
#define _DRV_HID_H_

#include "osi_compiler.h"

OSI_EXTERN_C_BEGIN

#include <stddef.h>
#include <stdint.h>

#define Hidhand uint32_t
#define HID_KEYBOARD_REPORT_DESC_SIZE  63

typedef enum
{
  HID_REPORT_CALLBACK=0,
  HID_READ_CALLBACK,
}Hidcbtype_t;

typedef enum
{
  HID_REPORT_DESCRIPTOR=0,
  HID_INTERFACE_DESCRIPTOR,
  HID_HID_DESCRIPTOR,
  HID_IN_ENDPOINT_DESCRIPTOR,
  HID_OUT_ENDPOINT_DESCRIPTOR,
}Desctype_t;
/**
 * \brief ccid driver
 */
typedef struct drv_hid drvHID_t;

/**
 * \brief function type callback on ccid device line state change
 */
typedef void (*Hidcallback_t)(uint8_t *buff,uint16_t len, void *cb_ctx);

/**
 * \brief open the ccid device
 *
 * \param name  the CCID device name
 * \return
 *      - NULL  fail
 *      - other point of the instance
 */
int drvHIDOpen(uint32_t name,Hidhand *hid);

/**
 * \brief set CCID line change callback
 * \note the callback will be called in interrupt
 *
 * \param ccid  the CCID device
 * \param cb    callback on line state change
 * \param ctx   caller context
 */
int drvHIDSetCallback(Hidhand hid, Hidcbtype_t type, Hidcallback_t cb, void * ctx);

/**
 * \brief get CCID device online or not (connect to a host and enable the config)
 *
 * \param ccid  the CCID device
 * \return
 *      - true if online else false
 */
//bool drvCCIDOnline(drvCCID_t *ccid);
int drvHIDUpdateDescriptor(Hidhand hid, Desctype_t type, void * Desc, bool usbRst);

/**
 * \brief read CCID slot state, each bit indicated a slot
 *
 * \param ccid      the CCID device
 * \param slotmap   the slotmap must non-null
 * \return
 *      - 0                     succeed
 *      - (negative integer)    negative error number
 */
//int drvCCIDSlotCheck(drvCCID_t *ccid, uint8_t *slotmap);

/**
 * \brief slot state change
 *
 * \param ccid      the ccid device
 * \param index     slot index
 * \param insert    insert or remove
 * \return
 *      - 0                     succeed
 *      - (negative integer)    negative error number
 */
//int drvCCIDSlotChange(drvCCID_t *ccid, uint8_t index, bool insert);

/**
 * \brief read CCID device
 * \note this api is synchronous, if no data coming, the call will block
 *
 * \param ccid  the CCID device
 * \param buf   buf to store read data
 * \param size  buffer size, must greater than struct ccid_bulk_out_header
 * \return
 *      - (negative integer)    negative error number
 *      - other                 actual read size
 */
//int drvHIDKeyWrite(drvHID_t *hid, uint8_t *buf);

/**
 * \brief write CCID device
 *
 * \param ccid  the CCID device
 * \param data  data buffer
 * \param size  data size
 * \return
 *      - (negative integer)    negative error number
 *      - other                 actual write size
 */
int drvHIDWrite(Hidhand hid, uint8_t *buf, uint32_t size);

/**
 * \brief close CCID device
 *
 * \param ccid  the CCID device
 */
int drvHIDClose(Hidhand *hid);

OSI_EXTERN_C_END

#endif
