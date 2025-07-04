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

#ifndef _FS_MOUNT_SDCARD_H_
#define _FS_MOUNT_SDCARD_H_

#include "osi_compiler.h"
#include "didi_app.h"

OSI_EXTERN_C_BEGIN

#include <stdbool.h>

/**
 * \brief mount sdcard
 *
 * \return
 *      - true on succeed else fail
 */
bool fsMountSdcard();

/**
 * \brief umount sdcard
 */
void fsUmountSdcard();

#ifdef CONFIG_FIBOCOM_DIDI
bool fsSdcardExist();
#endif
OSI_EXTERN_C_END

#endif