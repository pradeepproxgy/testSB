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

#define LCD_DRV_ID  0x009106
#define LCD_WIDTH   128
#define LCD_HEIGHT  160

static uint16_t g_lcd_ine_mode;
static bool g_lcd_farmk;

#define LCM_WR_REG(Addr, Data)                             \
    {                                                      \
        while (halGoudaWriteReg(Addr, Data) != HAL_ERR_NO) \
            ;                                              \
    }
#define LCD_DataWrite(Data)                    \
    {                                                 \
        while (halGoudaWriteData(Data) != HAL_ERR_NO) \
            ;                                         \
    }
#define LCD_CtrlWrite(Cmd)                   \
    {                                               \
        while (halGoudaWriteCmd(Cmd) != HAL_ERR_NO) \
            ;                                       \
    }

static void _lcdClose(void);
static uint32_t _lcdReadId(void);
static void _lcdSleepIn(bool is_sleep);
static void _lcdInit(void);
static void _lcdInvalidate(void);
static void _lcdInvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
);

static void _lcdSetDisplayWindow(
    uint16_t left,  // start Horizon address
    uint16_t right, // end Horizon address
    uint16_t top,   // start Vertical address
    uint16_t bottom // end Vertical address
);
static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _lcdSetDirection(lcdDirect_t direct_type)
{
    uint8_t mark = 0;

    OSI_LOGI(0, "lcd:    _lcdSetDirection = %d", direct_type);

    halGoudaReadData(0x0b, &mark, 1);
    mark &= (0x01 << 3);

    switch (direct_type)
    {
    case LCD_DIRECT_NORMAL:
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0x00 | mark);
        break;
    case LCD_DIRECT_ROT_90:
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0x60 | mark);
        break;
    case LCD_DIRECT_ROT_180:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0xC0 | mark);
        break;
    case LCD_DIRECT_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0xA0 | mark);
        break;
    case LCD_DIRECT_NORMAL2:
        //Vertical screen  startpoint at bottomleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0x80 | mark);
        break;
    case LCD_DIRECT_NORMAL2_ROT_90:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0x20 | mark);
        break;
    case LCD_DIRECT_NORMAL2_ROT_180:
        //Horizontal screen  startpoint at bottomleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0x40 | mark);
        break;
    case LCD_DIRECT_NORMAL2_ROT_270:
        //Horizontal screen  startpoint at topleft
        LCD_CtrlWrite(0x36);
        LCD_DataWrite(0xE0 | mark);
        break;
    default:
        break;
    }

    LCD_CtrlWrite(0x2c);
}

/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
/******************************************************************************/
static void _lcdSetDisplayWindow(
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
    OSI_LOGI(0, "lcd:    _lcdSetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

    LCD_CtrlWrite(0x2a);                   // set hori start , end (left, right)
    LCD_DataWrite((newleft >> 8) & 0xff);  // left high 8 b
    LCD_DataWrite(newleft & 0xff);         // left low 8 b
    LCD_DataWrite((newright >> 8) & 0xff); // right high 8 b
    LCD_DataWrite(newright & 0xff);        // right low 8 b

    LCD_CtrlWrite(0x2b);                    // set vert start , end (top, bot)
    LCD_DataWrite((newtop >> 8) & 0xff);    // top high 8 b
    LCD_DataWrite(newtop & 0xff);           // top low 8 b
    LCD_DataWrite((newbottom >> 8) & 0xff); // bot high 8 b
    LCD_DataWrite(newbottom & 0xff);        // bot low 8 b
    LCD_CtrlWrite(0x2c);                    // recover memory write mode
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _lcdInit(void)
{
    OSI_LOGI(0, "lcd:    gc9106Init ");

    halGoudatResetLcdPin();
    _lcdDelayMs(120);

	LCD_CtrlWrite(0xfe);  //Inter register enable 1  内部寄存器启用1
	LCD_CtrlWrite(0xef);  //Inter register enable 2  内部寄存器启用2

	LCD_CtrlWrite(0xb3);  //要访问"f0h"，需要将"b3h"的位[0]设置为1,要访问"f1h"，需要将"b3h"的位[1]设置为1
	LCD_DataWrite(0x03); //0000 0011    bit[0]=1:可访问"f0h",it[1]=1:可访问"f1h",

	LCD_CtrlWrite(0x36);  //Memory Access Ctrl  存储器接入控制。
	//LCD_DataWrite(0xc8); //0000 1000  bit[7]-bit[5]:这三位控制单片机的内存写/读方向。bit[4]:液晶显示器垂直刷新方向控制。bit[3]: 颜色选择开关控制(0=RGB滤色板，1=BGR滤色板) 。bit[2]:液晶显示器水平刷新方向控制。
							 	//"0x08"---在正常竖放基础上转 180 度竖放;
// #if(ENABLE_XINGTUO_DESIGN)
// 	LCD_DataWrite(0xa8);   //横屏显示 <右下角>
// #else
// 	LCD_DataWrite(0x78);   //横屏显示 <左上角>
// #endif//ENABLE_XINGTUO_DESIGN
    LCD_DataWrite(0x08);

	LCD_CtrlWrite(0x3a);  //COLMOD: Pixel Format Set  像素格式集
	LCD_DataWrite(0x05); //12 bits / pixel,  12 bit/pixel color order (R:5-bit,G:6-bit,B:5-bit),65,536 colors

	LCD_CtrlWrite(0xb6);  //要访问"a3h"，需要将"b6"的位[0]设置为1。若要访问"aah"，需要将"b6"的位[2]设置为1。
	LCD_DataWrite(0x11); //0001 0001  bit[4]=1:可访问"a3h"。
							  	//a3h为 FrameRate 帧速率   帧速率=18 MHz/190/(4*(128*(rtn 1[6：4]+2)*8*rtn1[3：0])， 却省值为0x16 所以 帧速率=18MHz/190/(4*(128*(1+2)+8*6)＝54.3Hz

	LCD_CtrlWrite(0xac);
	LCD_DataWrite(0x0b);

	LCD_CtrlWrite(0xb4);   //Display Inversion Control 显示反转控制
	LCD_DataWrite(0x21);  //固定高值,1点反转
	// LCD_DataWrite(0x00);  //固定高值,1点反转

	LCD_CtrlWrite(0x21);

	//  灰度参考电压
	LCD_CtrlWrite(0xb1);   //若要访问"e6h"，需要将"b1h"的位[6]设置为1。要访问"e7h"，需要将"b1h"的位[7]设置为1。
	LCD_DataWrite(0xc0);  //1100 0000   可访问e6h和e7h

	LCD_CtrlWrite(0xe6);   //VREG1_CTL
	LCD_DataWrite(0x50);	 //LCD_DataWrite(0x50);
	LCD_DataWrite(0x43);	 //LCD_DataWrite(0x43);
	LCD_CtrlWrite(0xe7);   //VREG2_CTL
	LCD_DataWrite(0x56);	 //LCD_DataWrite(0x56);
	LCD_DataWrite(0x43);	 //LCD_DataWrite(0x43);

	LCD_CtrlWrite(0xF0);   //SET_GAMMA1  启用测试命令   ok
	LCD_DataWrite(0x1f);
	LCD_DataWrite(0x41);
	LCD_DataWrite(0x1b);
	LCD_DataWrite(0x55);
	LCD_DataWrite(0x36);
	LCD_DataWrite(0x3d);
	LCD_DataWrite(0x3e);
	LCD_DataWrite(0x0);
	LCD_DataWrite(0x16);
	LCD_DataWrite(0x08);
	LCD_DataWrite(0x09);
	LCD_DataWrite(0x15);
	LCD_DataWrite(0x14);
	LCD_DataWrite(0x0f);

	LCD_CtrlWrite(0xF1);   //SET_GAMMA2
	LCD_DataWrite(0x1f);
	LCD_DataWrite(0x41);
	LCD_DataWrite(0x1b);
	LCD_DataWrite(0x55);
	LCD_DataWrite(0x36);
	LCD_DataWrite(0x3d);
	LCD_DataWrite(0x3e);
	LCD_DataWrite(0x0);
	LCD_DataWrite(0x16);
	LCD_DataWrite(0x08);
	LCD_DataWrite(0x09);
	LCD_DataWrite(0x15);
	LCD_DataWrite(0x14);
	LCD_DataWrite(0x0f);

	LCD_CtrlWrite(0xfe);   //Inter register enable 1 内部寄存器启用1
	LCD_CtrlWrite(0xff);   //WriteComm(0xef);

	LCD_CtrlWrite(0x11);   //Sleep Out
	_lcdDelayMs(120);
	LCD_CtrlWrite(0x29);   //Display ON  开显示
}

static void _lcdSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd:  gc9106  _lcdSleepIn, is_sleep = %d", is_sleep);

    if (is_sleep)
    {
        LCD_CtrlWrite(0x28); //display off
        _lcdDelayMs(120);
        LCD_CtrlWrite(0x10); // enter sleep mode
    }
    else
    {

#if 0
        LCD_CtrlWrite(0x11); // sleep out mode
        _lcdDelayMs(120);
        LCD_CtrlWrite(0x29); // display on
#endif
        _lcdInit();
    }
}

static void _lcdClose(void)
{
    OSI_LOGI(0, "lcd:  gc9106   in _lcdClose");

    _lcdSleepIn(true);
}
static void _lcdInvalidate(void)
{
    OSI_LOGI(0, "lcd:  gc9106   in _lcdInvalidate");

    _lcdSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _lcdInvalidateRect(
    uint16_t left,  //the left value of the rectangel
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd: gc9106 _lcdInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd: gc9106 _lcdInvalidateRect error"); /*assert verified*/
    }

    _lcdSetDisplayWindow(left, top, right, bottom);
}

static void _lcdRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd: gc9106 gc9106RotationInvalidateRect");
    switch (angle)
    {
    case LCD_ANGLE_0:
        _lcdSetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_90:
        _lcdSetDisplayWindow(left, top, bottom, right);
        break;
    case LCD_ANGLE_180:
        _lcdSetDisplayWindow(left, top, right, bottom);
        break;
    case LCD_ANGLE_270:
        _lcdSetDisplayWindow(left, top, bottom, right);
        break;
    default:
        _lcdSetDisplayWindow(left, top, right, bottom);
        break;
    }
}

static void _lcdEsdCheck(void)
{

    OSI_LOGI(0, "lcd:  gc9106  _lcdEsdCheck");
}

static const lcdOperations_t gLcdOperations =
    {
        _lcdInit,
        _lcdSleepIn,
        _lcdEsdCheck,
        _lcdSetDisplayWindow,
        _lcdInvalidateRect,
        _lcdInvalidate,
        _lcdClose,
        _lcdRotationInvalidateRect,
        _lcdSetDirection,
        _lcdReadId,
};

const lcdSpec_t g_lcd_gc9106 =
    {
        LCD_DRV_ID,
        LCD_WIDTH,
        LCD_HEIGHT,
        HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&gLcdOperations,
        false,
        0x2a000,
        8000000, // should be less than 10MHz
};

static uint32_t _lcdReadId(void)
{
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[4] = {0};
    OSI_LOGI(0, "lcd: rst and read gc9106 Id \n");
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);
    _lcdDelayMs(10);
    g_lcd_farmk = g_lcd_gc9106.is_use_fmark;
    g_lcd_ine_mode = g_lcd_gc9106.bus_mode;
    halGoudaReadConfig(g_lcd_ine_mode, g_lcd_gc9106.is_use_fmark, g_lcd_gc9106.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    r_err = halGoudaReadData(0x04, id, 4);
    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd:  gc9106  _lcdReadId LCM: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[2], id[1], id[3], r_err);

    ret_id = ((id[3] << 16) | (id[2] << 8) | id[1]);

    if (LCD_DRV_ID == ret_id)
    {
        OSI_LOGI(0, "lcd:  gc9106 is using !");
    }

    halGoudaClose();
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
    return ret_id;
}
