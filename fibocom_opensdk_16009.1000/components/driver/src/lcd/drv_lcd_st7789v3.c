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
#define LCD_DRV_ID_ST7789v3       0X898966
#define LCD_WIDTH  240
#define LCD_HEIGHT 320


#define LCM_WR_REG(Addr, Data)                             \
    {                                                      \
        while (halGoudaWriteReg(Addr, Data) != HAL_ERR_NO) \
            ;                                              \
    }
#define LCD_DataWrite_ST(Data)                    \
    {                                                 \
        while (halGoudaWriteData(Data) != HAL_ERR_NO) \
            ;                                         \
    }
#define LCD_CtrlWrite_ST(Cmd)                   \
    {                                               \
        while (halGoudaWriteCmd(Cmd) != HAL_ERR_NO) \
            ;                                       \
    }

static void _stClose(void);
static uint32_t _stReadIdv3(void);
static void _stSleepIn(bool is_sleep);
static void _stInit(void);
static void _stInvalidate(void);
static void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
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

    OSI_LOGI(0, "lcd[st7789v3]:    _stSetDirection = %d", direct_type);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        //Vertical screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x00);
        break;
    case LCD_DIRECT_ROT_90:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x60);
        break;
    case LCD_DIRECT_ROT_180:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xC0);
        break;
	case LCD_DIRECT_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xA0);
        break;
    case LCD_DIRECT_NORMAL2:
        //Vertical screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x80);
        break;
    case LCD_DIRECT_NORMAL2_ROT_90:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x20);
        break;
    case LCD_DIRECT_NORMAL2_ROT_180:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x40);
        break;
	case LCD_DIRECT_NORMAL2_ROT_270:
        //Horizontal screen  startpoint at topleft
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
    OSI_LOGI(0, "lcd[st7789v3]:    _stSetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

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
    OSI_LOGI(0, "lcd[st7789v3]:   _stInit ");

    LCD_CtrlWrite_ST(0x11);
    /* 
    In st7789v2/v3 datasheet, it is described as follows:
    - It will be necessary to wait 5msec before sending any new commands to a display module 
      following this command to allow time for the supply voltages and clock circuits to stabilize.
    - It will be necessary to wait 120msec after sending sleep out command (when in sleep in mode) 
      before sending an sleep in command.

    But according to IC factory, it is recommended to waiting more than 120ms after command 11h, 
    which is NOT the same as described in the datasheet, to fix little probabilistic issue that 
    blurred screen happens in 1st or 2nd frame.

    2021-06-28: 
    To fix Xindalu ME50C poweron logo blurred screen.

    2021-08-09:
    AITI projects also has the same issue that blurred screen happened but in st7789v3.
    */
    _lcdDelayMs(120);

    LCD_CtrlWrite_ST(0x36);
    LCD_DataWrite_ST(0x00);

    LCD_CtrlWrite_ST(0xb2);
    LCD_DataWrite_ST(0x0c);
    LCD_DataWrite_ST(0x0c);
    LCD_DataWrite_ST(0x00);
    LCD_DataWrite_ST(0x33);
    LCD_DataWrite_ST(0x33);

    LCD_CtrlWrite_ST(0xb7);
    LCD_DataWrite_ST(0x35);

    LCD_CtrlWrite_ST(0xbb);
    LCD_DataWrite_ST(0x37);

    LCD_CtrlWrite_ST(0xc0);
    LCD_DataWrite_ST(0x2c);

    LCD_CtrlWrite_ST(0xc2);
    LCD_DataWrite_ST(0x01);

    LCD_CtrlWrite_ST(0xc3);
    LCD_DataWrite_ST(0x09);   //  07

    LCD_CtrlWrite_ST(0xc4);
    LCD_DataWrite_ST(0x20);

    LCD_CtrlWrite_ST(0xc6);
#ifdef CONFIG_DISPLAY_POWERON_LOGO
    LCD_DataWrite_ST(0x1f);//03
#else
    LCD_DataWrite_ST(0x03);
#endif

    LCD_CtrlWrite_ST(0xd0);
    LCD_DataWrite_ST(0xa4);
    LCD_DataWrite_ST(0xa1);
    
    LCD_CtrlWrite_ST(0xb0);
    LCD_DataWrite_ST(0x00);
    LCD_DataWrite_ST(0xd0);
    
    LCD_CtrlWrite_ST(0xe0);
    LCD_DataWrite_ST(0xd0);
    LCD_DataWrite_ST(0x00);
    LCD_DataWrite_ST(0x05);
    LCD_DataWrite_ST(0x0e);
    LCD_DataWrite_ST(0x15);
    LCD_DataWrite_ST(0x0d);
    LCD_DataWrite_ST(0x37);
    LCD_DataWrite_ST(0x43);
    LCD_DataWrite_ST(0x47);
    LCD_DataWrite_ST(0x09);
    LCD_DataWrite_ST(0x15);
    LCD_DataWrite_ST(0x12);
    LCD_DataWrite_ST(0x16);
    LCD_DataWrite_ST(0x19);

    LCD_CtrlWrite_ST(0xe1);
    LCD_DataWrite_ST(0xd0);
    LCD_DataWrite_ST(0x00);
    LCD_DataWrite_ST(0x05);
    LCD_DataWrite_ST(0x0d);
    LCD_DataWrite_ST(0x0c);
    LCD_DataWrite_ST(0x06);
    LCD_DataWrite_ST(0x2d);
    LCD_DataWrite_ST(0x44);
    LCD_DataWrite_ST(0x40);
    LCD_DataWrite_ST(0x0e);
    LCD_DataWrite_ST(0x1c);
    LCD_DataWrite_ST(0x18);
    LCD_DataWrite_ST(0x16);
    LCD_DataWrite_ST(0x19);

    LCD_CtrlWrite_ST(0x35);
    LCD_DataWrite_ST(0x00);

    LCD_CtrlWrite_ST(0x3A);
    LCD_DataWrite_ST(0x55);


    LCD_CtrlWrite_ST(0x29);

}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[st7789v3]:  _stSleepIn, is_sleep = %d", is_sleep);

    if (is_sleep)
    {
        LCD_CtrlWrite_ST(0x28); //display off
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
    OSI_LOGI(0, "lcd[st7789v3]: in _stClose");

    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[st7789v3]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[st7789v3]: in _stInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[st7789v3]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[st7789v3]: in stRotationInvalidateRect");
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
    OSI_LOGI(0, "lcd[st7789v3]: st7789v3 stEsdCheck");
}

static const lcdOperations_t st7789v3Operations =
{
        _stInit,
        _stSleepIn,
        _stEsdCheck,
        _stSetDisplayWindow,
        _stInvalidateRect,
        _stInvalidate,
        _stClose,
        _stRotationInvalidateRect,
        _stSetDirection,
        _stReadIdv3,
};

const lcdSpec_t g_lcd_st7789v3 =
{
        LCD_DRV_ID_ST7789v3,
        LCD_WIDTH,
        LCD_HEIGHT,
        HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&st7789v3Operations,
        false,
        0x2a000,
        50000000,
};

static uint32_t _stReadIdv3(void)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0, "lcd[st7789v3]:  st7789v3   _stReadId \n");
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_2800MV);
#endif
#ifdef CONFIG_FIBOCOM_LG610
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
	halPmuSwitchPower(HAL_POWER_VIBR, true, true);	
    halPmuSetPowerLevel(HAL_POWER_VIBR, POWER_LEVEL_2800MV);
#endif
    _lcdDelayMs(10);

    halGoudaReadConfig(g_lcd_st7789v3.bus_mode, g_lcd_st7789v3.is_use_fmark, g_lcd_st7789v3.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    _lcdDelayMs(10);
    r_err = halGoudaReadData(0x04, id, 4);
    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd[st7789v3]:  _stReadId LCM: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[3], id[2], id[1], r_err);

    ret_id = ((id[3] << 16) | (id[2] << 8) | id[1]);

    if (LCD_DRV_ID_ST7789v3 == ret_id)
    {
        OSI_LOGI(0, "lcd[st7789v3]:  st is using !");
    }
    OSI_LOGI(0, "lcd[st7789v3]: st7789v3 ReadId:0x%x \n",ret_id);
#ifndef CONFIG_FIBOCOM_BOOT_LCD_SUPPORT
    halGoudaClose();
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
#endif
#ifdef CONFIG_FIBOCOM_LG610
	halPmuSwitchPower(HAL_POWER_VIBR, false, false);
#endif
    return ret_id;
}
#endif

