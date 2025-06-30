/* Copyright (C) 2017 RDA Technologies Limited and/or its affiliates("RDA").
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
#include "boot_platform.h"
#include "hal_adi_bus.h"
#include "hwregs.h"
#include "osi_compiler.h"
#include "osi_api.h"
#include "osi_tick_unit.h"
#include "hal_config.h"
#include "hal_chip.h"
#include "hal_blue_screen.h"
#include "cmsis_core.h"
#include "fibocom.h"
#include "fibo_boot_cfg.h"

int fibo_get_pwkey_over7s_reset()
{
    return PWKEY_OVER7S_RESET; 
}