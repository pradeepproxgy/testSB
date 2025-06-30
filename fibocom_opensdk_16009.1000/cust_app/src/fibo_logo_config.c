/**
 * @file usr_config.c
 * @author Ethan Guo (Guo Junhuang) (ethan.guo@fibocom.com)
 * @brief User configuration of FIBO LOGO function.
 * @version 0.1
 * @date 2024-06-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "fibo_logo.h"
#include "fibocom.h"

#if 1 // default value
fibo_logo_cfg_t g_fibo_logo_cfg = {
    /**
     * @note  
     * lcd_power:
     *       major_dev |      minor_dev      |   value
     *       ----------|---------------------|--------------------------------
     *          0-pmu  | platform PMU id     | voltage value, 0 as turning off
     *          1-pin  | fibocom pin number  | 0 / 1
     */
    .lcd_power_major_dev        = (uint32_t)(-1),
    .lcd_power_minor_dev        = (uint32_t)(-1),
    .lcd_power_on_val           = (uint32_t)(-1),
    .lcd_power_off_val          = (uint32_t)(-1),
    .lcd_power_delay_ms         = (uint32_t)(-1),

    /**
     * @note 
     * lcd_bl:
     *       major_dev |      minor_dev      |   value
     *       ----------|---------------------|------------------------
     *          0-pin  | fibocom pin number  | 0 / 1
     *          1-sink | fibocom sink number | current value or level
     *          2-pwm  | fibocom pwm number  | valid duty cycle of PWM
     */
    .lcd_bl_major_dev           = (uint32_t)(-1),
    .lcd_bl_minor_dev           = (uint32_t)(-1),
    .lcd_bl_on_val              = (uint32_t)(-1),
    .lcd_bl_off_val             = (uint32_t)(-1),

    .lcd_direction              = (uint32_t)(-1),
    
    .lcd_iovcc_mV               = 0, // 0 as default and do noting

    .display_bgcolor_if_failed  = false,
};
#else // example
#include "hal_chip.h"

fibo_logo_cfg_t g_fibo_logo_cfg = {
    .lcd_power_major_dev        = 0, // no ext power supply
    .lcd_power_minor_dev        = HAL_POWER_KEYLED,
    .lcd_power_on_val           = 2800,
    .lcd_power_off_val          = 0,
    .lcd_power_delay_ms         = (uint32_t)(-1),

    .lcd_bl_major_dev           = 1,    // use sink as backlight control
    .lcd_bl_minor_dev           = 1,
    .lcd_bl_on_val              = 200,
    .lcd_bl_off_val             = 0,

    .lcd_direction              = 0,

    .lcd_iovcc_mV               = 2800, // 0 as default and do noting
    
    .display_bgcolor_if_failed  = true,
    
    .poweron_logo_path = "/ext/logo.bmp",
    .poweron_logo_bgcolor = BLACK,
    
    .update_logo_path = {"/ext/update_logo_0.bmp", "/ext/update_logo_1.bmp"},
    .update_logo_bgcolor = WHITE,
};
#endif

/**
 * @brief Display power-on LOGO during power-on.
 *        This function will be called before appimg_enter(), which is located in kernel.
 * 
 */
void fibo_display_poweron_logo(void)
{
#ifdef CONFIG_FIBOCOM_POWERON_LOGO_SUPPORT
    fibo_logo_display(g_fibo_logo_cfg.poweron_logo_path, g_fibo_logo_cfg.poweron_logo_bgcolor);
#endif
}

