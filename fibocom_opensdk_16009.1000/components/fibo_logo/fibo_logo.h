/**
 * @file fibo_logo.h
 * @author Ethan Guo (Guo Junhuang) (ethan.guo@fibocom.com)
 * @brief FIBO LOGO function. It can be used as displaying power-on LOGO and updating prompt LOGO.
 * @version 0.1
 * @date 2024-06-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __FIBO_LOGO_CONFIG_H_
#define __FIBO_LOGO_CONFIG_H_

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

// RGB565 color code
#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430


/**
 * @brief FIBO LOGO screen direction.
 * 
 * 
 */

typedef enum
{
    FIBO_LOGO_DIR_NORMAL = 0x00,
    FIBO_LOGO_DIR_ROT_90, // Rotation 90
    FIBO_LOGO_DIR_ROT_180, // Rotation 180
    FIBO_LOGO_DIR_NORMAL2,
    FIBO_LOGO_DIR_NORMAL2_ROT_90,
    FIBO_LOGO_DIR_NORMAL2_ROT_180,
    FIBO_LOGO_DIR_ROT_270,  // Rotation 270
    FIBO_LOGO_DIR_NORMAL2_ROT_270,
} fibo_logo_dir_t;

/**
 * @brief FIBO LOGO function configuration.
 * 
 * 
 */
typedef struct {
    /**
     * @note  
     * lcd_power:
     *       major_dev |      minor_dev      |   value
     *       ----------|---------------------|--------------------------------
     *          0-pmu  | platform PMU id     | voltage value, 0 as turning off
     *          1-pin  | fibocom pin number  | 0 / 1
     */
    uint32_t lcd_power_major_dev;       // Major device id of extra lcd power supply.
    uint32_t lcd_power_minor_dev;       // Minor device id of extra lcd power supply.
    uint32_t lcd_power_on_val;          // Value of extra lcd power supply power-on.
    uint32_t lcd_power_off_val;         // Value of extra lcd power supply power-off.
    uint32_t lcd_power_delay_ms;        // Delay time for extra lcd power voltage stabilization (in milliseconds).

    /**
     * @note 
     * lcd_bl:
     *       major_dev |      minor_dev      |   value
     *       ----------|---------------------|------------------------
     *          0-pin  | fibocom pin number  | 0 / 1
     *          1-sink | fibocom sink number | current value or level
     *          2-pwm  | fibocom pwm number  | valid duty cycle of PWM
     */
    uint32_t lcd_bl_major_dev;          // Major device id of lcd backlight contorled.
    uint32_t lcd_bl_minor_dev;          // Minor device id of lcd backlight contorled.
    uint32_t lcd_bl_on_val;             // Value of lcd backlight contorled power-on.
    uint32_t lcd_bl_off_val;            // Value of lcd backlight contorled power-off.

    uint32_t lcd_iovcc_mV;              // IO operation voltage of LCD interface driver power supply (in mV), 
                                        // 0 as default voltage and do nothing (e.g. MC669's default voltage is 1.8V ).

    uint32_t lcd_direction;             // Direction of LCD screen transmission.

    bool     display_bgcolor_if_failed; // Whether display background color when image file prasing fails.

    char    *poweron_logo_path;         // Full path of power-on LOGO image file.
    uint16_t poweron_logo_bgcolor;      // Background color while displaying power-on LOGO.

    char    *update_logo_path[2];       // Full path of updating prompt LOGO image file (two image files to display in turns).
    uint16_t update_logo_bgcolor;       // Background color while displaying updating prompt LOGO.
} fibo_logo_cfg_t;

/**
 * @brief Display LOGO by specifying a BMP image file, and fill background color.
 * 
 * @param img_path Image file full path.
 * @param bg_color background color.
 * @return int32_t 0: successful.
 *                 -1: failed.
 * @note Noted that it can be called more that one time, and it won't initialize LCD repeatedly.
 */
int32_t fibo_logo_display(char *img_path, uint16_t bg_color);

/**
 * @brief Release LOGO function resource, including releasing LCD, turning off power supply and backlight, etc.
 * 
 * @return int32_t 0: successful.
 *                 -1: failed.
 */
int32_t fibo_logo_release(void);

#endif
