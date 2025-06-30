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

#include "fibocom.h"
#include "osi_api.h"
#include "osi_log.h"
#include "hwregs.h"
#include "drv_config.h"
#include "hal_gouda.h"
#include "hal_chip.h"
#include "drv_lcd.h"

#define LCD_DRV_ID_GC9307 0x009307
#define LCD_WIDTH 240
#define LCD_HEIGHT 240

static uint16_t gc9307_line_mode;
static bool gc9307_farmk;

#define LCM_WR_REG(Addr, Data)                             \
    {                                                      \
        while (halGoudaWriteReg(Addr, Data) != HAL_ERR_NO) \
            ;                                              \
    }
#define LCD_DataWrite_GC9307(Data)                    \
    {                                                 \
        while (halGoudaWriteData(Data) != HAL_ERR_NO) \
            ;                                         \
    }
#define LCD_CtrlWrite_GC9307(Cmd)                   \
    {                                               \
        while (halGoudaWriteCmd(Cmd) != HAL_ERR_NO) \
            ;                                       \
    }

static void _gc9307Close(void);
static uint32_t _gc9307ReadId(void);
static void _gc9307SleepIn(bool is_sleep);
static void _gc9307Init(void);
static void _gc9307Invalidate(void);
static void _gc9307InvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
);

static void _gc9307SetDisplayWindow(
    uint16_t left,  // start Horizon address
    uint16_t right, // end Horizon address
    uint16_t top,   // start Vertical address
    uint16_t bottom // end Vertical address
);
static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _gc9307SetDirection(lcdDirect_t direct_type)
{

    OSI_LOGI(0, "lcd:    _gc9307SetDirection = %d", direct_type);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0x00);
        break;
    case LCD_DIRECT_ROT_90:
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0x60);
        break;
    case LCD_DIRECT_ROT_180:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0xC0);
        break;
    case LCD_DIRECT_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0xA0);
        break;
    case LCD_DIRECT_NORMAL2:
        //Vertical screen  startpoint at bottomleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0x80);
        break;
    case LCD_DIRECT_NORMAL2_ROT_90:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0x20);
        break;
    case LCD_DIRECT_NORMAL2_ROT_180:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0x40);
        break;
    case LCD_DIRECT_NORMAL2_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_GC9307(0x36);
        LCD_DataWrite_GC9307(0xE0);
        break;
    default:
        break;
    }

    LCD_CtrlWrite_GC9307(0x2c);
}

/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
/******************************************************************************/
static void _gc9307SetDisplayWindow(
    uint16_t left,  // start Horizon address
    uint16_t top,   // start Vertical address
    uint16_t right, // end Horizon address
    uint16_t bottom // end Vertical address
)
{
    uint16_t newleft = left;
    uint16_t newtop = top;
    uint16_t newright = right;
    uint16_t newbottom = bottom;
    OSI_LOGI(0, "lcd:    _gc9307SetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

    LCD_CtrlWrite_GC9307(0x2a);                   // set hori start , end (left, right)
    LCD_DataWrite_GC9307((newleft >> 8) & 0xff);  // left high 8 b
    LCD_DataWrite_GC9307(newleft & 0xff);         // left low 8 b
    LCD_DataWrite_GC9307((newright >> 8) & 0xff); // right high 8 b
    LCD_DataWrite_GC9307(newright & 0xff);        // right low 8 b

    LCD_CtrlWrite_GC9307(0x2b);                    // set vert start , end (top, bot)
    LCD_DataWrite_GC9307((newtop >> 8) & 0xff);    // top high 8 b
    LCD_DataWrite_GC9307(newtop & 0xff);           // top low 8 b
    LCD_DataWrite_GC9307((newbottom >> 8) & 0xff); // bot high 8 b
    LCD_DataWrite_GC9307(newbottom & 0xff);        // bot low 8 b
    LCD_CtrlWrite_GC9307(0x2c);                    // recover memory write mode
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _gc9307Init(void)
{
    OSI_LOGI(0, "lcd:    gc9307Init ");
    LCD_CtrlWrite_GC9307(0x11);
    _lcdDelayMs(120);

    LCD_CtrlWrite_GC9307(0xfe);
    LCD_CtrlWrite_GC9307(0xef);

    LCD_CtrlWrite_GC9307(0x36);
    LCD_DataWrite_GC9307(0x48);
    LCD_CtrlWrite_GC9307(0x3a);
    LCD_DataWrite_GC9307(0x05);

    LCD_CtrlWrite_GC9307(0x86);
    LCD_DataWrite_GC9307(0x98);

    LCD_CtrlWrite_GC9307(0x89);
    LCD_DataWrite_GC9307(0x03);

    LCD_CtrlWrite_GC9307(0x8b);
    LCD_DataWrite_GC9307(0x80);//ff en

    LCD_CtrlWrite_GC9307(0x8d);
    LCD_DataWrite_GC9307(0x22);
    LCD_CtrlWrite_GC9307(0x8e);
    LCD_DataWrite_GC9307(0x0f);
    LCD_CtrlWrite_GC9307(0xe8);
    LCD_DataWrite_GC9307(0x12);
    LCD_DataWrite_GC9307(0x00);

    LCD_CtrlWrite_GC9307(0xff);
    LCD_DataWrite_GC9307(0x62);

    LCD_CtrlWrite_GC9307(0x99);
    LCD_DataWrite_GC9307(0x3e);
    LCD_CtrlWrite_GC9307(0x9d);
    LCD_DataWrite_GC9307(0x4b);
    LCD_CtrlWrite_GC9307(0xc3);
    LCD_DataWrite_GC9307(0x27);
    LCD_CtrlWrite_GC9307(0xc4);
    LCD_DataWrite_GC9307(0x18);
    LCD_CtrlWrite_GC9307(0xc9);
    LCD_DataWrite_GC9307(0x14);

    LCD_CtrlWrite_GC9307(0xf0);
    LCD_DataWrite_GC9307(0x91);
    LCD_DataWrite_GC9307(0x1b);
    LCD_DataWrite_GC9307(0xa);
    LCD_DataWrite_GC9307(0x9);
    LCD_DataWrite_GC9307(0x5);
    LCD_DataWrite_GC9307(0x3f);
    LCD_CtrlWrite_GC9307(0xf1);
    LCD_DataWrite_GC9307(0x57);
    LCD_DataWrite_GC9307(0x8f);
    LCD_DataWrite_GC9307(0x8f);
    LCD_DataWrite_GC9307(0x36);
    LCD_DataWrite_GC9307(0x36);
    LCD_DataWrite_GC9307(0x7f);
    LCD_CtrlWrite_GC9307(0xf2);
    LCD_DataWrite_GC9307(0x40);
    LCD_DataWrite_GC9307(0x0);
    LCD_DataWrite_GC9307(0x9);
    LCD_DataWrite_GC9307(0x9);
    LCD_DataWrite_GC9307(0x5);
    LCD_DataWrite_GC9307(0x2b);
    LCD_CtrlWrite_GC9307(0xf3);
    LCD_DataWrite_GC9307(0x41);
    LCD_DataWrite_GC9307(0x4f);
    LCD_DataWrite_GC9307(0x6f);
    LCD_DataWrite_GC9307(0x23);
    LCD_DataWrite_GC9307(0x2f);
    LCD_DataWrite_GC9307(0x9f);

    LCD_CtrlWrite_GC9307(0x35);
    LCD_DataWrite_GC9307(0x00);

    LCD_CtrlWrite_GC9307(0x44);
    LCD_DataWrite_GC9307(0x00);
    LCD_DataWrite_GC9307(0x0a);
    LCD_CtrlWrite_GC9307(0x11);
    _lcdDelayMs(120);

    LCD_CtrlWrite_GC9307(0x29);
    LCD_CtrlWrite_GC9307(0x2c);
}

static void _gc9307SleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd:  gc9307  _gc9307SleepIn, is_sleep = %d", is_sleep);

    if (is_sleep)
    {
        LCD_CtrlWrite_GC9307(0x28); //display off
        _lcdDelayMs(120);
        LCD_CtrlWrite_GC9307(0x10); // enter sleep mode
    }
    else
    {
        halGoudatResetLcdPin();
        _lcdDelayMs(100);
        _gc9307Init();
    }
}

static void _gc9307Close(void)
{
    OSI_LOGI(0, "lcd:  gc9307   in GC9304_Close");

    _gc9307SleepIn(true);
}
static void _gc9307Invalidate(void)
{
    OSI_LOGI(0, "lcd:  gc9307   in _gc9307Invalidate");

    _gc9307SetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _gc9307InvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{
    OSI_LOGI(0, "lcd: gc9307 _gc9307InvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {
        OSI_ASSERT(false, "lcd: gc9307 _gc9307InvalidateRect error"); /*assert verified*/
    }
    _gc9307SetDisplayWindow(left, top, right, bottom);
}

static void _gc9307RotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{
    OSI_LOGI(0, "lcd: gc9307 gc9307RotationInvalidateRect");
    switch (angle)
    {
    case LCD_ANGLE_0:
        _gc9307SetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_90:
        _gc9307SetDisplayWindow(left, top, bottom, right);
        break;
    case LCD_ANGLE_180:
        _gc9307SetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_270:
        _gc9307SetDisplayWindow(left, top, bottom, right);
        break;
    default:
        _gc9307SetDisplayWindow(left, top, right, bottom);
        break;
    }
}

static void _gc9307EsdCheck(void)
{
    OSI_LOGI(0, "lcd:  gc9307  gc9307EsdCheck");
}

static const lcdOperations_t gc9307Operations =
    {
        _gc9307Init,
        _gc9307SleepIn,
        _gc9307EsdCheck,
        _gc9307SetDisplayWindow,
        _gc9307InvalidateRect,
        _gc9307Invalidate,
        _gc9307Close,
        _gc9307RotationInvalidateRect,
        _gc9307SetDirection,
        _gc9307ReadId,
};

const lcdSpec_t g_lcd_gc9307 =
    {
        LCD_DRV_ID_GC9307,
        LCD_WIDTH,
        LCD_HEIGHT,
        HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&gc9307Operations,
        false,
        0x2a000,
        50000000,
};

static uint32_t _gc9307ReadId(void)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0, "lcd: rst and read gc9307 Id \n");
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    #if defined(CONFIG_MC665_CN_36_90_QDTIANYAN)
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3000MV);
    #endif
    _lcdDelayMs(10);
    gc9307_farmk = g_lcd_gc9307.is_use_fmark;
    gc9307_line_mode = g_lcd_gc9307.bus_mode;
    //halGoudatResetLcdPin();
    halGoudaReadConfig(gc9307_line_mode, g_lcd_gc9307.is_use_fmark, g_lcd_gc9307.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    r_err = halGoudaReadData(0x04, id, 4);
    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd:  gc9307  _gc9307ReadId LCM: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[3], id[2], id[1], r_err);

    ret_id = ((id[3] << 16) | (id[2] << 8) | id[1]);

    if (LCD_DRV_ID_GC9307 == ret_id)
    {
        OSI_LOGI(0, "lcd:  gc9307 is using !");
    }

    halGoudaClose();
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
    return ret_id;
}

