/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void pwm_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    int32_t index = 1;
    uint16_t duty = 10;
    uint16_t arr = 19;
    uint16_t psc = 0;

    uint16_t pin = 39;
    UINT8 mode = 1;

    fibo_gpio_mode_set(pin, mode);

    //Before configuring PWM output, ensure that the corresponding PIN uses the PWM function
    fibo_pwm_open(index);
    fibo_pwm_config(index, duty, arr, psc);

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(pwm_thread_entry, "pwm_custhread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

