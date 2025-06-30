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

#define LCD_DRV_ID_ST7796u2       0X7796
#define LCD_WIDTH  320
#define LCD_HEIGHT 480


static uint16_t st_line_mode;
static bool st_farmk;


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
static uint32_t _stReadId(void);
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

    OSI_LOGI(0, "lcd[st7796u2]:    _stSetDirection = %d", direct_type);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        //Vertical screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x48);
        break;
    case LCD_DIRECT_ROT_90:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xe8);
        break;
    case LCD_DIRECT_ROT_180:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x88);
        break;
	case LCD_DIRECT_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x28);
        break;
    case LCD_DIRECT_NORMAL2:
        //Vertical screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xc8);
        break;
    case LCD_DIRECT_NORMAL2_ROT_90:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0xa8);
        break;
    case LCD_DIRECT_NORMAL2_ROT_180:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x08);
        break;
	case LCD_DIRECT_NORMAL2_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite_ST(0x36);
        LCD_DataWrite_ST(0x68);
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
    OSI_LOGI(0, "lcd[st7796u2]:    _stSetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

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
    OSI_LOGI(0, "lcd[st7796u2]:   _stInit ");

    _lcdDelayMs(10);   // wait enough before starting sending initialized command

    // Sleep Out
    LCD_CtrlWrite_ST(0x11);
    _lcdDelayMs(10);

	//************* Start Initial Sequence **********// 
    // Command 2 control
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0xc3);  // enable command 2 part I
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0x96);  // enable command 2 part II

	// Memory Access Control 
    LCD_CtrlWrite_ST(0x36);
    // LCD_DataWrite_ST(0x08); // RGB --> BGR
    LCD_DataWrite_ST(0x48); // set (0, 0) piont at left&top, RGB --> BGR

    // Display Inversion On
	LCD_CtrlWrite_ST(0X21); 

    // Blanking Porch Control
	LCD_CtrlWrite_ST(0Xb5);
	LCD_DataWrite_ST(0xc3);
	LCD_DataWrite_ST(0xc3);
	LCD_DataWrite_ST(0xc3);
	LCD_DataWrite_ST(0xc3);

    // VCOM Control
	LCD_CtrlWrite_ST(0Xc5);
	LCD_DataWrite_ST(0x2a);
	// LCD_DataWrite_ST(0x2f);

    // Power Control 1
	LCD_CtrlWrite_ST(0Xc0);
	LCD_DataWrite_ST(0xe0);
	LCD_DataWrite_ST(0x25);

    // Power Control 2
	LCD_CtrlWrite_ST(0Xc1);
	LCD_DataWrite_ST(0x09);

    // Power Control 3
	LCD_CtrlWrite_ST(0Xc2);
	LCD_DataWrite_ST(0x0a); // Source & Gamma driving current level: Medium
	
    // VCOM Control
	LCD_CtrlWrite_ST(0Xc1);
	LCD_DataWrite_ST(0x2f);

    // Positive Gamma Control
	LCD_CtrlWrite_ST(0Xe0);
	LCD_DataWrite_ST(0xf0);
	LCD_DataWrite_ST(0x03);
	LCD_DataWrite_ST(0x0a);
	LCD_DataWrite_ST(0x11);
	LCD_DataWrite_ST(0x14);
	LCD_DataWrite_ST(0x1c);
	LCD_DataWrite_ST(0x3b);
	LCD_DataWrite_ST(0x55);
	LCD_DataWrite_ST(0x4a);
	LCD_DataWrite_ST(0x0a);
	LCD_DataWrite_ST(0x13);
	LCD_DataWrite_ST(0x14);
	LCD_DataWrite_ST(0x1c);
	LCD_DataWrite_ST(0x1f);

    // Negative Gamma Control
	LCD_CtrlWrite_ST(0Xe1);
	LCD_DataWrite_ST(0xf0);
	LCD_DataWrite_ST(0x03);
	LCD_DataWrite_ST(0x0a);
	LCD_DataWrite_ST(0x0c);
	LCD_DataWrite_ST(0x0c);
	LCD_DataWrite_ST(0x09);
	LCD_DataWrite_ST(0x36);
	LCD_DataWrite_ST(0x54);
	LCD_DataWrite_ST(0x49);
	LCD_DataWrite_ST(0x0f);
	LCD_DataWrite_ST(0x1b);
	LCD_DataWrite_ST(0x18);
	LCD_DataWrite_ST(0x1b);
	LCD_DataWrite_ST(0x1f);

    // Tearing Effect Line On
    LCD_CtrlWrite_ST(0x35);
    LCD_DataWrite_ST(0x00);

    // RGB interface color format: 16bit/pixel,
    // Control interface color format: 16bit/pixel
	LCD_CtrlWrite_ST(0X3a);
	LCD_DataWrite_ST(0x55); 

    // Command 2 control
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0x3c); // disable command 2 part I
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0x69); // disable command 2 part II

    // Display On
	LCD_CtrlWrite_ST(0X29);
}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[st7796u2]:  _stSleepIn, is_sleep = %d", is_sleep);

    if (is_sleep) {
        LCD_CtrlWrite_ST(0x28); //display off
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST(0x10); // enter sleep mode
    } else {
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
    OSI_LOGI(0, "lcd[st7796u2]: in _stClose");

    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[st7796u2]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangle
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[st7796u2]: in _stInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[st7796u2]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[st7796u2]: in stRotationInvalidateRect");
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

    OSI_LOGI(0, "lcd[st7796u2]: st7796u2 stEsdCheck");
}

static const lcdOperations_t st7796u2Operations =
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
        _stReadId,
};

lcdSpec_t g_lcd_st7796u2 =
    {
        LCD_DRV_ID_ST7796u2,
        LCD_WIDTH,
        LCD_HEIGHT,
        // HAL_GOUDA_SPI_LINE_3_2_LANE,
		HAL_GOUDA_SPI_LINE_3,
		// HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&st7796u2Operations,
        false,
        0x2a000,
        50000000,
};


static uint32_t _stReadId(void)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0, "lcd[st7796u2]:  st7796u2   _stReadId \n");
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
#ifdef CONFIG_FIBOCOM_LG610
	halPmuSwitchPower(HAL_POWER_VIBR, true, true);	
    halPmuSetPowerLevel(HAL_POWER_VIBR, POWER_LEVEL_2800MV);
#endif
    _lcdDelayMs(10);
    
    st_farmk = g_lcd_st7796u2.is_use_fmark;
    st_line_mode = g_lcd_st7796u2.bus_mode;

    halGoudaReadConfig(st_line_mode, g_lcd_st7796u2.is_use_fmark, g_lcd_st7796u2.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;

    // Command 2 control
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0xc3);  // enable command 2 part I
	LCD_CtrlWrite_ST(0Xf0);
	LCD_DataWrite_ST(0x96);  // enable command 2 part II

    // interface mode control
    LCD_CtrlWrite_ST(0xb0); 
    LCD_DataWrite_ST(0x80); // DIN/SDA pin is used for 3/4 wire serial interface and DOUT pin is not used

    _lcdDelayMs(10);
    // Read Display ID
    r_err = halGoudaReadData(0xd3, id, 4);
    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd[st7796u2]:  Read Display ID: 0x%0x 0x%0x (read return : %d)\n", id[3], id[2], r_err);

    ret_id = ((id[2] << 8) | id[1]);

    if (LCD_DRV_ID_ST7796u2 == ret_id)
    {
        OSI_LOGI(0, "lcd[st7796u2]:  st is using !");
    }
    OSI_LOGI(0, "lcd[st7796u2]: st7796u2 ReadId:0x%x \n",ret_id);
    halGoudaClose();
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
#ifdef CONFIG_FIBOCOM_LG610
	halPmuSwitchPower(HAL_POWER_VIBR, false, false);
#endif
    return ret_id;
}

