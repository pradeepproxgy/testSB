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

#include "osi_api.h"
#include "osi_log.h"
#include "hwregs.h"
#include "drv_config.h"
#include "hal_gouda.h"
#include "hal_chip.h"
#include "drv_lcd.h"

#define LCD_DRV_ID_ST7789H2 0x858552
#define LCD_WIDTH 240
#define LCD_HEIGHT 320

static uint16_t st7789h2_line_mode;
static bool st7789h2_farmk;

#define LCM_WR_REG(Addr, Data)                             \
    {                                                      \
        while (halGoudaWriteReg(Addr, Data) != HAL_ERR_NO) \
            ;                                              \
    }
#define LCD_DataWrite_ST7789H2(Data)                  \
    {                                                 \
        while (halGoudaWriteData(Data) != HAL_ERR_NO) \
            ;                                         \
    }
#define LCD_CtrlWrite_ST7789H2(Cmd)                 \
    {                                               \
        while (halGoudaWriteCmd(Cmd) != HAL_ERR_NO) \
            ;                                       \
    }

static void _st7789h2Close(void);
static uint32_t _st7789h2ReadId(void);
static void _st7789h2SleepIn(bool is_sleep);
static void _st7789h2Init(void);
static void _st7789h2Invalidate(void);
static void _st7789h2InvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
);

static void _st7789h2SetDisplayWindow(
    uint16_t left,  // start Horizon address
    uint16_t right, // end Horizon address
    uint16_t top,   // start Vertical address
    uint16_t bottom // end Vertical address
);
static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _st7789h2SetDirection(lcdDirect_t direct_type)
{

    OSI_LOGI(0x1000910e, "lcd:    _st7789h2SetDirection = %d", direct_type);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        LCD_CtrlWrite_ST7789H2(0x36);
        LCD_DataWrite_ST7789H2(0x00);
        break;
    case LCD_DIRECT_ROT_90:
        LCD_CtrlWrite_ST7789H2(0x36);
        LCD_DataWrite_ST7789H2(0x60);
        break;
    }

    LCD_CtrlWrite_ST7789H2(0x2c);
}

/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
/******************************************************************************/
static void _st7789h2SetDisplayWindow(
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
    OSI_LOGI(0x1000910f, "lcd:st7789h2SetDisplayWindow L = %d, top = %d, R = %d, bot = %d", left, top, right, bottom);

    LCD_CtrlWrite_ST7789H2(0x2a);                   // set hori start , end (left, right)
    LCD_DataWrite_ST7789H2((newleft >> 8) & 0xff);  // left high 8 b
    LCD_DataWrite_ST7789H2(newleft & 0xff);         // left low 8 b
    LCD_DataWrite_ST7789H2((newright >> 8) & 0xff); // right high 8 b
    LCD_DataWrite_ST7789H2(newright & 0xff);        // right low 8 b

    LCD_CtrlWrite_ST7789H2(0x2b);                    // set vert start , end (top, bot)
    LCD_DataWrite_ST7789H2((newtop >> 8) & 0xff);    // top high 8 b
    LCD_DataWrite_ST7789H2(newtop & 0xff);           // top low 8 b
    LCD_DataWrite_ST7789H2((newbottom >> 8) & 0xff); // bot high 8 b
    LCD_DataWrite_ST7789H2(newbottom & 0xff);        // bot low 8 b
    LCD_CtrlWrite_ST7789H2(0x2c);                    // recover memory write mode
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _st7789h2Init(void)
{
    OSI_LOGI(0x100089cb, "lcd:    _st7789h2Init ");
    LCD_CtrlWrite_ST7789H2(0x11); //Sleep out
    _lcdDelayMs(100);             //Delay 120ms

    LCD_CtrlWrite_ST7789H2(0x36); // Memory data access control
    LCD_DataWrite_ST7789H2(0x00);

    LCD_CtrlWrite_ST7789H2(0x3a); // interface pixedl format
    LCD_DataWrite_ST7789H2(0x55); //RGB565, 16 bits, 65k rgb?

    LCD_CtrlWrite_ST7789H2(0xe7);
    LCD_DataWrite_ST7789H2(0x10); //enable two dataline

    LCD_CtrlWrite_ST7789H2(0xb2); // porch setting
    LCD_DataWrite_ST7789H2(0x0c); //default:0x0c	0x7f// back porch
    LCD_DataWrite_ST7789H2(0x0c); //default:0x0c	0x7f//front porch
    LCD_DataWrite_ST7789H2(0x00); // Disable seperate porch control
    LCD_DataWrite_ST7789H2(0x33); // B/F porch in idle mode
    LCD_DataWrite_ST7789H2(0x33); // B/F porch in partial mode?

    LCD_CtrlWrite_ST7789H2(0xb7); // gate control
    LCD_DataWrite_ST7789H2(0x35); // vgh = 13.26v, vgl = -10.43v

    LCD_CtrlWrite_ST7789H2(0xbb); // VCOM setting
    LCD_DataWrite_ST7789H2(0x35); // VCOM=1.425v

    LCD_CtrlWrite_ST7789H2(0xc0); // LCM setting?
    LCD_DataWrite_ST7789H2(0x2c);

    LCD_CtrlWrite_ST7789H2(0xc2); // VDV VRH command enable??
    LCD_DataWrite_ST7789H2(0x01);

    LCD_CtrlWrite_ST7789H2(0xc3); // VRH set
    LCD_DataWrite_ST7789H2(0x0b); // VAP(GVDD) = 4.1+

    LCD_CtrlWrite_ST7789H2(0xc4); // VDV set
    LCD_DataWrite_ST7789H2(0x20);

    LCD_CtrlWrite_ST7789H2(0xc6); // frame rate control
    LCD_DataWrite_ST7789H2(0x0b); // 0x1f, 39hz; 0x0f, 60hz; 0x0B,69hz

    LCD_CtrlWrite_ST7789H2(0xd0); // power control 1
    LCD_DataWrite_ST7789H2(0xa4); // fixed
    LCD_DataWrite_ST7789H2(0xa1); // AVDD=6.8, AVCL=-4.8, VDS=2.3

    LCD_CtrlWrite_ST7789H2(0xe0); // positive voltage gamma control
    LCD_DataWrite_ST7789H2(0xd0);
    LCD_DataWrite_ST7789H2(0x00);
    LCD_DataWrite_ST7789H2(0x02);
    LCD_DataWrite_ST7789H2(0x07);
    LCD_DataWrite_ST7789H2(0x0b);
    LCD_DataWrite_ST7789H2(0x1a);
    LCD_DataWrite_ST7789H2(0x31);
    LCD_DataWrite_ST7789H2(0x54);
    LCD_DataWrite_ST7789H2(0x40);
    LCD_DataWrite_ST7789H2(0x29);
    LCD_DataWrite_ST7789H2(0x12);
    LCD_DataWrite_ST7789H2(0x12);
    LCD_DataWrite_ST7789H2(0x12);
    LCD_DataWrite_ST7789H2(0x17);

    LCD_CtrlWrite_ST7789H2(0xe1); // negative voltage gamma control
    LCD_DataWrite_ST7789H2(0xd0);
    LCD_DataWrite_ST7789H2(0x00);
    LCD_DataWrite_ST7789H2(0x02);
    LCD_DataWrite_ST7789H2(0x07);
    LCD_DataWrite_ST7789H2(0x05);
    LCD_DataWrite_ST7789H2(0x25);
    LCD_DataWrite_ST7789H2(0x2d);
    LCD_DataWrite_ST7789H2(0x44);
    LCD_DataWrite_ST7789H2(0x45);
    LCD_DataWrite_ST7789H2(0x1c);
    LCD_DataWrite_ST7789H2(0x18);
    LCD_DataWrite_ST7789H2(0x16);
    LCD_DataWrite_ST7789H2(0x1c);
    LCD_DataWrite_ST7789H2(0x1d);

    //Set_Tear_Scanline
    LCD_CtrlWrite_ST7789H2(0x44);
    LCD_DataWrite_ST7789H2(0x00);
    LCD_DataWrite_ST7789H2(0x08);

    //enable fmark ST
    LCD_CtrlWrite_ST7789H2(0x35);
    LCD_DataWrite_ST7789H2(0x01);

    LCD_CtrlWrite_ST7789H2(0x29);

    _lcdDelayMs(20);
    LCD_CtrlWrite_ST7789H2(0x2c);
}

static void _st7789h2SleepIn(bool is_sleep)
{
    OSI_LOGI(0x10009110, "lcd:   _st7789h2SleepIn, is_sleep = %d", is_sleep);

    if (is_sleep)
    {
        LCD_CtrlWrite_ST7789H2(0x28); //display off
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST7789H2(0x10); // enter sleep mode
    }
    else
    {

#if 0
        LCD_CtrlWrite_ST7789H2(0x11); // sleep out mode
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST7789H2(0x29); // display on
#endif
        _st7789h2Init();
    }
}

static void _st7789h2Close(void)
{
    OSI_LOGI(0x10007dd8, "lcd:     in GC9304_Close");

    _st7789h2SleepIn(true);
}
static void _st7789h2Invalidate(void)
{
    OSI_LOGI(0x10009111, "lcd:     in _st7789h2Invalidate");

    _st7789h2SetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _st7789h2InvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0x10009112, "lcd:  _st7789h2InvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd:  _st7789h2InvalidateRect error"); /*assert verified*/
    }

    _st7789h2SetDisplayWindow(left, top, right, bottom);
}

static void _st7789h2RotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0x10007dcf, "lcd: gc9305 gc9305RotationInvalidateRect");
    switch (angle)
    {
    case LCD_ANGLE_0:
        _st7789h2SetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_90:
        _st7789h2SetDisplayWindow(left, top, bottom, right);
        break;
    case LCD_ANGLE_180:
        _st7789h2SetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_270:
        _st7789h2SetDisplayWindow(left, top, bottom, right);
        break;
    default:
        _st7789h2SetDisplayWindow(left, top, right, bottom);
        break;
    }
}

static void _st7789h2EsdCheck(void)
{

    OSI_LOGI(0x10009113, "lcd:  st7789h2sEsdCheck");
}

static const lcdOperations_t st7789h2sOperations =
    {
        _st7789h2Init,
        _st7789h2SleepIn,
        _st7789h2EsdCheck,
        _st7789h2SetDisplayWindow,
        _st7789h2InvalidateRect,
        _st7789h2Invalidate,
        _st7789h2Close,
        _st7789h2RotationInvalidateRect,
        NULL,
        _st7789h2ReadId,
};

const lcdSpec_t g_lcd_st7789h2 =
    {
        LCD_DRV_ID_ST7789H2,
        LCD_WIDTH,
        LCD_HEIGHT,
        HAL_GOUDA_SPI_LINE_3_2_LANE,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&st7789h2sOperations,
        false,
        0x2a000,
        10000000, // 500000, //10000000, 10M
};

static uint32_t _st7789h2ReadId(void)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0x10009114, "lcd:st7789h2ReadId \n");
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    _lcdDelayMs(10);
    st7789h2_farmk = g_lcd_st7789h2.is_use_fmark;
    st7789h2_line_mode = g_lcd_st7789h2.bus_mode;

    halGoudaReadConfig(st7789h2_line_mode, g_lcd_st7789h2.is_use_fmark, g_lcd_st7789h2.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    _lcdDelayMs(10);
    r_err = halGoudaReadData(0x04, id, 4);
    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    OSI_LOGI(0x10009115, "lcd:st7789h2ReadId LCM: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[1], id[2], id[3], r_err);
    ret_id = ((id[3] << 16) | (id[2] << 8) | id[1]);

    if (LCD_DRV_ID_ST7789H2 == ret_id)
    {
        OSI_LOGI(0x10007dde, "lcd:   is using !");
    }
    halGoudaClose();
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
    osiDebugEvent(0x666661d);
    osiDebugEvent(ret_id);
    return ret_id;
}
