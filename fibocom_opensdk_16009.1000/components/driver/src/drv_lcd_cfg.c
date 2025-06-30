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

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "fibocom.h"
#include "hal_chip.h"
#include "hwregs.h"
#include "osi_api.h"
#include "osi_log.h"
#include "osi_profile.h"
#include "osi_byte_buf.h"
#include "drv_lcd.h"

extern const lcdSpec_t g_lcd_gc9305;
extern const lcdSpec_t g_lcd_st7735s;
extern const lcdSpec_t g_lcd_st7789h2;
extern const lcdSpec_t g_lcd_gc9307;
#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_SUPPORT_LCD_GC9106
extern const lcdSpec_t g_lcd_gc9106;
#endif
#ifdef CONFIG_FIBOCOM_SUPPORT_LCD_ST7789p3
extern const lcdSpec_t g_lcd_st7789_p3;
#endif
#ifdef CONFIG_FIBOCOM_LCD_ST7789_4LINE
#ifdef CONFIG_FIBOCOM_LCD_ST7789_240x240
extern const lcdSpec_t g_lcd_st7789v2_G4;
#else
extern const lcdSpec_t g_lcd_st7789v1;
#endif
#else
extern const lcdSpec_t g_lcd_st7789v2;
#endif

// dot matrix lcd
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
#if defined(CONFIG_LCD_ST7567A_SUPPORT)
extern const lcdSpec_t g_lcd_st7567a;
#endif
#endif

#endif

static uint8_t g_lcd_index = 0xff;

static lcdSpec_t *lcd_cfg_tab[] =
    {
        //(lcdSpec_t *)&g_lcd_gc9305, //modify by fibocom 20220311
        //(lcdSpec_t *)&g_lcd_st7735s,
        //(lcdSpec_t *)&g_lcd_st7789h2,
        (lcdSpec_t *)&g_lcd_gc9307,
#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_SUPPORT_LCD_GC9106
        (lcdSpec_t *)&g_lcd_gc9106,
#endif
#ifdef CONFIG_FIBOCOM_SUPPORT_LCD_ST7789p3
        (lcdSpec_t *)&g_lcd_st7789_p3,
#endif
#ifdef CONFIG_FIBOCOM_LCD_ST7789_4LINE
#ifdef CONFIG_FIBOCOM_LCD_ST7789_240x240
        (lcdSpec_t *)&g_lcd_st7789v2_G4,
#else
        (lcdSpec_t *)&g_lcd_st7789v1,
#endif
#else
        (lcdSpec_t *)&g_lcd_st7789v2,
#endif

// dot matrix lcd
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
#if defined(CONFIG_LCD_ST7567A_SUPPORT)
        (lcdSpec_t *)&g_lcd_st7567a,
#endif
#endif

#endif
};
static uint32_t _drvLcdGetCfgCount(void)
{
    uint32_t count;
    count = sizeof(lcd_cfg_tab) / sizeof(lcd_cfg_tab[0]);
    OSI_LOGI(0x10007cc6, "lcd:  _drvLcdGetCfgCount count=%d \n", count);

    return count;
}

/*****************************************************************************/
//  Description:    This function is used to get LCD spec information
/*****************************************************************************/
lcdSpec_t *drvLcdGetSpecInfo(void)
{
    uint32_t count = 0;
    uint32_t i = 0;
    uint32_t lcm_dev_id = 0;
    OSI_LOGI(0x10007cc7, "lcd:  drvLcdGetSpecInfo \n");
    count = _drvLcdGetCfgCount();
    if ((count != 0) && (g_lcd_index == 0xff))
    {
        for (i = 0; i < count; i++)
        {
#ifdef CONFIG_FIBOCOM_BASE
            if (lcd_cfg_tab[i]->operation->readIdEx)
            {
                lcm_dev_id = lcd_cfg_tab[i]->operation->readIdEx((void*)lcd_cfg_tab[i]);
                OSI_LOGI(0x10007cc8, "lcd:  drvLcdGetSpecInfo id=0x%x \n", lcm_dev_id);
                if (lcm_dev_id == lcd_cfg_tab[i]->dev_id)
                {
                    g_lcd_index = i;
                    OSI_LOGI(0x10007cc9, "lcd:  drvLcdGetSpecInfo g_lcd_index =0x%x \n", g_lcd_index);
                    return lcd_cfg_tab[g_lcd_index];
                }
            }
#endif
            if (lcd_cfg_tab[i]->operation->readId)
            {
                lcm_dev_id = lcd_cfg_tab[i]->operation->readId();
                OSI_LOGI(0x10007cc8, "lcd:  drvLcdGetSpecInfo id=0x%x \n", lcm_dev_id);
            }
            if (lcm_dev_id == lcd_cfg_tab[i]->dev_id)
            {
                g_lcd_index = i;
                OSI_LOGI(0x10007cc9, "lcd:  drvLcdGetSpecInfo g_lcd_index =0x%x \n", g_lcd_index);
                return lcd_cfg_tab[g_lcd_index];
            }
        }
    }
    else if (g_lcd_index != 0xff)
    {
        return lcd_cfg_tab[g_lcd_index];
    }
    OSI_LOGI(0x10007cca, "lcd:  drvLcdGetSpecInfo null ! \n");
    return NULL;
}

#ifdef __cplusplus
}

#endif // End of lcm_cfg_info.c
