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

static INT32 result = -1;

void oc_gpioisrcallback(void *param)
{
    hal_gpioisr_t *isr_cfg = (hal_gpioisr_t*)param;
    fibo_gpio_get(9, &(isr_cfg->level));
    fibo_textTrace("test 9 pin level is %d", isr_cfg->level);
    fibo_gpio_mode_set(1, 0);
    fibo_gpio_cfg(1, 1);
    result = fibo_gpio_set(1, isr_cfg->level);
    fibo_textTrace("test pin 1 level set ret %d", result);
}

static void gpio_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    INT32 ret = -1;
    INT32 i = 0;
    uint16_t pin = 39;
    UINT8 level= -1;

/*-----------test gpio output-------------*/

    ret = fibo_gpio_pull_disable(pin);  // set pin to default mode
    if(ret ==-1)
    {
        fibo_textTrace("fibo_gpio_pull_disable failed");
        return;
    }

    ret = fibo_gpio_mode_set(pin, 0);  // set pin to gpio mode
    if(ret ==-1)
    {
        fibo_textTrace("fibo_gpio_mode_set failed");
        return;
    }

    fibo_textTrace("set pin output");
    ret = fibo_gpio_cfg(pin, 1);  // set pin to output
    if(ret ==-1)
    {
        fibo_textTrace("fibo_gpio_cfg failed");
        return;
    }

    fibo_textTrace("set pin high");
    ret = fibo_gpio_set(pin, 1);  // set pin output high
    if(ret ==-1)
    {
        fibo_textTrace("fibo_gpio_set high failed");
        return;
    }

    result = fibo_gpio_get(pin, &level);
    fibo_textTrace("fibo_gpio_get result = %d, level = %d", result, level);
    fibo_taskSleep(3000);  //wait 3s

    fibo_textTrace("set pin low");
    ret = fibo_gpio_set(pin, 0);  // set pin output low
    if(ret ==-1)
    {
        fibo_textTrace("fibo_gpio_set low failed");
        return;
    }

    result = fibo_gpio_get(pin, &level);
    fibo_textTrace("fibo_gpio_get result = %d, level = %d", result, level);
    fibo_taskSleep(3000);  //wait 3s


/*------------test gpio input--------------*/

    result = fibo_gpio_pull_disable(pin);
    fibo_textTrace("test fibo_gpio_pull_disable result = %d", result);

    result = fibo_gpio_mode_set(pin, 0);
    fibo_textTrace("test fibo_gpio_mode_set result = %d", result);

    result = fibo_gpio_cfg(pin, 0);
    fibo_textTrace("test fibo_gpio_cfg result = %d", result);

    result = fibo_gpio_get(pin, &level);
    fibo_textTrace("test fibo_gpio_get result = %d, level = %d", result, level);

    for(i = 0; i < 3; i++)
    {
        result = fibo_gpio_pull_up_or_down(pin, false);
        fibo_textTrace("test fibo_gpio_pull down result = %d", result);

        result = fibo_gpio_get(pin, &level);
        fibo_textTrace("test fibo_gpio_get result = %d, level = %d", result, level);
        fibo_taskSleep(2000);

        result = fibo_gpio_pull_up_or_down(pin, true);
        fibo_textTrace("test fibo_gpio_pull up result = %d", result);

        result = fibo_gpio_get(pin, &level);
        fibo_textTrace("test fibo_gpio_get result = %d, level = %d", result, level);
        fibo_taskSleep(2000);
    }


/*------------test gpio interrupt--------------*/

    fibo_gpio_pull_disable(pin);  // set pin to default mode

    oc_isr_t g_oc_ist_prt = {
        .is_debounce = true,
        .intr_enable = true,
        .intr_level = false,
        .intr_falling = true,
        .inte_rising = true,
        .callback = oc_gpioisrcallback,
    };

    result = fibo_gpio_isr_init(9, &g_oc_ist_prt);
    fibo_textTrace("test fibo_gpio_isr_init result = %d", result);
    fibo_taskSleep(3000);

    fibo_thread_delete();
}

void* appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(gpio_thread_entry, "gpio_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
