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

#ifdef CONFIG_FIBOCOM_BASE

typedef struct
{
    lcdSpec_t spec;
    uint32_t pwr_lv; /* POWER_LEVEL_1800MV */
} lcdSpecPriv_t;

static lcdSpecPriv_t *st_spec_ptr = NULL;

#define LCM_WR_REG(Addr, Data)                             \
    {                                                      \
        while (halGoudaWriteReg(Addr, Data) != HAL_ERR_NO) \
            ;                                              \
    }
#define LCD_DataWrite_ST(Data)                        \
    {                                                 \
        while (halGoudaWriteData(Data) != HAL_ERR_NO) \
            ;                                         \
    }
#define LCD_CtrlWrite_ST(Cmd)                       \
    {                                               \
        while (halGoudaWriteCmd(Cmd) != HAL_ERR_NO) \
            ;                                       \
    }

static void _stClose(void);
static uint32_t _stReadIdEx(void *);

static void _stSleepIn(bool is_sleep);
static void _stInit(void);
static void _stInvalidate(void);
static void _stInvalidateRect(
    uint16_t left,  // the left value of the rectangel
    uint16_t top,   // top of the rectangle
    uint16_t right, // right of the rectangle
    uint16_t bottom // bottom of the rectangle
);

static void _stSetDisplayWindow(
    uint16_t left,  // start Horizon address
    uint16_t right, // end Horizon address
    uint16_t top,   // start Vertical address
    uint16_t bottom // end Vertical address
);
static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _stSetDirection(lcdDirect_t direct_type)
{

    OSI_LOGI(0, "lcd[st7789p3]: _stSetDirection = %d", direct_type);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        // Vertical screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x00);
        break;
    case LCD_DIRECT_ROT_90:
        // Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x60);
        break;
    case LCD_DIRECT_ROT_180:
        // Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xC0);
        break;
    case LCD_DIRECT_ROT_270:
        // Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xA0);
        break;
    case LCD_DIRECT_NORMAL2:
        // Vertical screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x80);
        break;
    case LCD_DIRECT_NORMAL2_ROT_90:
        // Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x20);
        break;
    case LCD_DIRECT_NORMAL2_ROT_180:
        // Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x40);
        break;
    case LCD_DIRECT_NORMAL2_ROT_270:
        // Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xE0);
        break;
    default:
        break;
    }

    LCD_CtrlWrite_ST(0x2c);
}

/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
/******************************************************************************/
static void _stSetDisplayWindow(
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
    OSI_LOGI(0, "lcd[st7789p3]: _stSetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

    LCD_CtrlWrite_ST(0x2a);                   // set hori start , end (left, right)
    LCD_DataWrite_ST((newleft >> 8) & 0xff);  // left high 8 b
    LCD_DataWrite_ST(newleft & 0xff);         // left low 8 b
    LCD_DataWrite_ST((newright >> 8) & 0xff); // right high 8 b
    LCD_DataWrite_ST(newright & 0xff);        // right low 8 b

    LCD_CtrlWrite_ST(0x2b);                    // set vert start , end (top, bot)
    LCD_DataWrite_ST((newtop >> 8) & 0xff);    // top high 8 b
    LCD_DataWrite_ST(newtop & 0xff);           // top low 8 b
    LCD_DataWrite_ST((newbottom >> 8) & 0xff); // bot high 8 b
    LCD_DataWrite_ST(newbottom & 0xff);        // bot low 8 b
    LCD_CtrlWrite_ST(0x2c);                    // recover memory write mode
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _stInit(void)
{
    OSI_LOGI(0, "lcd[st7789p3]: _stInit ");

    //************* Start Initial Sequence **********//
    LCD_CtrlWrite_ST(0x11); //Sleep out 
    _lcdDelayMs(120);              //Delay 120ms 
    //************* Start Initial Sequence **********// 
    LCD_CtrlWrite_ST(0x36);
    LCD_DataWrite_ST(0x00);

    LCD_CtrlWrite_ST(0x3A);
    LCD_DataWrite_ST(0x05);

    LCD_CtrlWrite_ST(0xB2);
    LCD_DataWrite_ST(0x0C);
    LCD_DataWrite_ST(0x0C);
    LCD_DataWrite_ST(0x00);
    LCD_DataWrite_ST(0x33);
    LCD_DataWrite_ST(0x33);

    LCD_CtrlWrite_ST(0xB7);
    LCD_DataWrite_ST(0x35);

    LCD_CtrlWrite_ST(0xBB);
    LCD_DataWrite_ST(0x32); //Vcom=1.35V

    LCD_CtrlWrite_ST(0xC2);
    LCD_DataWrite_ST(0x01);

    LCD_CtrlWrite_ST(0xC3);
    LCD_DataWrite_ST(0x15); //GVDD=4.8V  颜色深度

    LCD_CtrlWrite_ST(0xC4);
    LCD_DataWrite_ST(0x20); //VDV, 0x20:0v

    LCD_CtrlWrite_ST(0xC6);
    LCD_DataWrite_ST(0x0F); //0x0F:60Hz

    LCD_CtrlWrite_ST(0xD0);
    LCD_DataWrite_ST(0xA4);
    LCD_DataWrite_ST(0xA1);

    LCD_CtrlWrite_ST(0xE0);
    LCD_DataWrite_ST(0xD0);
    LCD_DataWrite_ST(0x08);
    LCD_DataWrite_ST(0x0E);
    LCD_DataWrite_ST(0x09);
    LCD_DataWrite_ST(0x09);
    LCD_DataWrite_ST(0x05);
    LCD_DataWrite_ST(0x31);
    LCD_DataWrite_ST(0x33);
    LCD_DataWrite_ST(0x48);
    LCD_DataWrite_ST(0x17);
    LCD_DataWrite_ST(0x14);
    LCD_DataWrite_ST(0x15);
    LCD_DataWrite_ST(0x31);
    LCD_DataWrite_ST(0x34);

    LCD_CtrlWrite_ST(0xE1);
    LCD_DataWrite_ST(0xD0);
    LCD_DataWrite_ST(0x08);
    LCD_DataWrite_ST(0x0E);
    LCD_DataWrite_ST(0x09);
    LCD_DataWrite_ST(0x09);
    LCD_DataWrite_ST(0x15);
    LCD_DataWrite_ST(0x31);
    LCD_DataWrite_ST(0x33);
    LCD_DataWrite_ST(0x48);
    LCD_DataWrite_ST(0x17);
    LCD_DataWrite_ST(0x14);
    LCD_DataWrite_ST(0x15);
    LCD_DataWrite_ST(0x31);
    LCD_DataWrite_ST(0x34);
    LCD_CtrlWrite_ST(0x21);

    LCD_CtrlWrite_ST(0x29);
}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[st7789p3]:  _stSleepIn, is_sleep = %d", is_sleep);

    if (is_sleep)
    {
        LCD_CtrlWrite_ST(0x28); // display off
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST(0x10); // enter sleep mode
    }
    else
    {

#if 0
        LCD_CtrlWrite_ST(0x11); // sleep out mode
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST(0x29); // display on
#endif
        _stInit();
    }
}

static void _stClose(void)
{
    OSI_LOGI(0, "lcd[st7789p3]: in _stClose");

    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[st7789p3]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, st_spec_ptr->spec.width - 1, st_spec_ptr->spec.height - 1);
}

void _stInvalidateRect(
    uint16_t left,  // the left value of the rectangel
    uint16_t top,   // top of the rectangle
    uint16_t right, // right of the rectangle
    uint16_t bottom // bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[st7789p3]: in _stInvalidateRect");
    left = (left >= st_spec_ptr->spec.width) ? st_spec_ptr->spec.width - 1 : left;
    right = (right >= st_spec_ptr->spec.width) ? st_spec_ptr->spec.width - 1 : right;
    top = (top >= st_spec_ptr->spec.height) ? st_spec_ptr->spec.height - 1 : top;
    bottom = (bottom >= st_spec_ptr->spec.height) ? st_spec_ptr->spec.height - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[st7789p3]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[st7789p3]: in stRotationInvalidateRect");
    switch (angle)
    {
    case LCD_ANGLE_0:
        _stSetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_90:
        _stSetDisplayWindow(left, top, bottom, right);
        break;
    case LCD_ANGLE_180:
        _stSetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_270:
        _stSetDisplayWindow(left, top, bottom, right);
        break;
    default:
        _stSetDisplayWindow(left, top, right, bottom);
        break;
    }
}

static void _stEsdCheck(void)
{

    OSI_LOGI(0, "lcd[st7789p3]: stEsdCheck");
}

static uint32_t _stReadIdEx(void *spec)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0, "lcd[st7789p3]: _stReadId \n");

    if (NULL == spec)
        return 0;

    st_spec_ptr = (lcdSpecPriv_t *)spec;
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    halPmuSetPowerLevel(HAL_POWER_LCD, st_spec_ptr->pwr_lv);

    _lcdDelayMs(10);

    halGoudaReadConfig(st_spec_ptr->spec.bus_mode, st_spec_ptr->spec.is_use_fmark, st_spec_ptr->spec.fmark_delay);
    _lcdDelayMs(140);
    // hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    _lcdDelayMs(10);
    r_err = halGoudaReadData(0x04, id, 4);
    // hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd[st7789p3]:  _stReadId LCM: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[3], id[2], id[1], r_err);

    ret_id = ((id[3] << 16) | (id[2] << 8) | id[1]);

    if (st_spec_ptr->spec.dev_id == ret_id)
    {
        OSI_LOGI(0, "lcd[st7789p3]:  st is using !");
    }
    OSI_LOGI(0, "lcd[st7789p3]: ReadId:0x%x \n", ret_id);
    // halGoudaClose();
    // halPmuSwitchPower(HAL_POWER_LCD, false, false);
    return ret_id;
}

static const lcdOperations_t st7789_Operations = {
    .init = _stInit,
    .enterSleep = _stSleepIn,
    .checkEsd = _stEsdCheck,
    .setDisplayWindow = _stSetDisplayWindow,
    .invlaidateRect = _stInvalidateRect,
    .invalidate = _stInvalidate,
    .close = _stClose,
    .rotationInvalidateRect = _stRotationInvalidateRect,
    .setBrushDirection = _stSetDirection,
    .readIdEx = _stReadIdEx,
};

const lcdSpecPriv_t g_lcd_st7789_p3 = {
    {
        0X8181b3,
        240,
        240,
        HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&st7789_Operations,
        false,
        0x2a000,
        50000000,
    },
    POWER_LEVEL_3200MV,
};

#endif
