/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _FIBO_OPENSDK_COMM_H_
#define _FIBO_OPENSDK_COMM_H_

#include <stdint.h>
#include <stdlib.h>
#include "osi_api.h"
#include "osi_log.h"
#include "hal_config.h"
#include "fibo_opencpu.h"


fibo_flash_partinfo_t *fibo_get_flash_partinfo(void);
fibo_config_t *fibo_get_config(void);
bool fibo_get_usb_custom_enable(void);
bool fibo_get_usb_printer_enable(void);
fibo_usb_buffcfg_t *fibo_get_usb_buffcfg(void);
fibo_usb_buffcfg_t *fibo_get_printer_buffcfg(void);
bool fibo_get_usb_hidacm_enable(void);
fibo_keypad_info_t *fibo_get_keypad_enable(void);
void fibo_iomux_set_init_config(uint32_t **fun_pad);



#endif
