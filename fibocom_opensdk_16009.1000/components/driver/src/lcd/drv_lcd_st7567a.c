/**
 * @file drv_lcd_st7567a.c
 * @author Guo Junhuang (ethan.guo@fibocom.com)
 * @brief device driver layer of dot Matrix lcd st7567a.
 * @version 0.1
 * @date 2021-09-02
 * @note Notice that it should be enabled by CONFIG_DOT_MATRIX_LCD_SUPPORT and CONFIG_LCD_ST7567A_SUPPORT.
 *       
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "fibocom.h"
#include "osi_api.h"
#include "osi_log.h"
#include "hwregs.h"
#include "drv_config.h"
#include "hal_gouda.h"
#include "hal_chip.h"
#include "drv_lcd.h"
#include "string.h"
#include "stdlib.h"

#if defined(CONFIG_LCD_ST7567A_SUPPORT)

#define LCD_DRV_ID      DOT_LCD_ID_UNSUPPORT

#define LCD_WIDTH       132
#define LCD_HEIGHT      64
#define LCD_BUFF_SIZE   (LCD_WIDTH) * ((LCD_HEIGHT) / 8)

#define LCD_PAGE        8
#define LCD_LINE        64

#define INST_regRes_MASK            0x20
#define INST_EVReg_MASK             0x00
#define INST_bias_MASK              0xa2
#define INST_STARTLINE_MASK         0x40

#define INST_regRes_VALID_BITS      0x07
#define INST_EVReg_VALID_BITS       0x3F
#define INST_bias_VALID_BITS        0x01//0x07
#define INST_STARTLINE_VALID_BITS   0x3F

//  (0~7): Select Regulator Resister
#define INST_regRes(x)    (INST_regRes_MASK | ((x) & INST_regRes_VALID_BITS))   

//  (0~63): Set Electronic Volume Register
#define INST_EVReg(x)     (INST_EVReg_MASK | ((x) & INST_EVReg_VALID_BITS))     

// (0~7): 0-1/5 1-1/6 2-1/7 3-1/8 4-1/9 5-1/10 6-1/11 7-1/12
#define INST_bias(x)      (INST_bias_MASK | ((x) & INST_bias_VALID_BITS))       

#define PAGE_LINE_NUM   8

#define WHICH_PAGE(y)               ((y) / PAGE_LINE_NUM)
#define WHICH_BIT_IN_PAGE(y)        ((y) % PAGE_LINE_NUM)
#define WHICH_COLUMN(x)             (x)

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

static uint8_t _g_inst_regRes = 0x04;     //  (0~7): Select Regulator Resister
static uint8_t _g_inst_EVReg = 0x20;      //  (0~63): Set Electronic Volume Register
static uint8_t _g_inst_bias = 0x00;
static bool _g_lcd_isActive = false;

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
static void _dotMatrixLcdCleanScreen(void);
static int32_t _stDotSetPixel(uint16_t x, uint16_t y, uint8_t val);
static void _stDotSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias);
static void _stDotSendBuff(uint8_t column, uint8_t row, void *buff, uint32_t size);
static void _stDotSetFlushDirection(uint8_t direction);


static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _stSetDirection(lcdDirect_t direct_type)
{
    OSI_LOGI(0, "lcd[st7567a]:    _stSetDirection = %d", direct_type);

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

}


static void _lcdInitCmdBeforeDisplayOn(void)
{
    LCD_CtrlWrite_ST(0xA4); //Cancel All Pixel ON
    LCD_CtrlWrite_ST(0xAE); //display off
    LCD_CtrlWrite_ST(0xE2); //SoftwareReset
    LCD_CtrlWrite_ST(0xA0); //Select SEG Normal Direction
    LCD_CtrlWrite_ST(0xC0); //Select COM Normal Direction
    LCD_CtrlWrite_ST(0x24); //Select Regulation Ratio=5.0
    LCD_CtrlWrite_ST(0x2C); //Booster ON
    LCD_CtrlWrite_ST(0x2E); //Regulator ON
    LCD_CtrlWrite_ST(0x2F); //Follower ON

    // LCD_CtrlWrite_ST(0x24); //Select Regulation Ratio=5.0
    // LCD_CtrlWrite_ST(0x81); //Set EV Command
    // LCD_CtrlWrite_ST(0x20); //Set EV=32
    // LCD_CtrlWrite_ST(0xA2); //Select 1/9Bias

    LCD_CtrlWrite_ST(INST_regRes(_g_inst_regRes)); // Select regulator register(1+(Ra+Rb))
    LCD_CtrlWrite_ST(0x81); // Set Reference Voltage 
    LCD_CtrlWrite_ST(INST_EVReg(_g_inst_EVReg)); // EV=35 => Vop =10.556V
    LCD_CtrlWrite_ST(INST_bias(_g_inst_bias)); // Set LCD Bias=1/9 V0
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _stInit(void)
{
    OSI_LOGI(0, "lcd[st7567a]:   _stInit ");
    _lcdDelayMs(100); // Delay 100ms for stable VDD1/VDD2/VDD3
    
    halGoudatResetLcdPin(); // hardware reset
    _lcdDelayMs(10);

    _lcdInitCmdBeforeDisplayOn();
    _dotMatrixLcdCleanScreen();
    LCD_CtrlWrite_ST(0xAF); // Display ON

    OSI_LOGI(0, "lcd[st7567a]:   _stInit done. ");
    _g_lcd_isActive = true;
}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[st7567a]:  _stSleepIn, is_sleep = %d", is_sleep);
    if (is_sleep) {
        _g_lcd_isActive = false;
        LCD_CtrlWrite_ST(0xAE); //display off
        LCD_CtrlWrite_ST(0xA5); //All Pixel ON
    } else {
#if 1
        _g_lcd_isActive = true;
        _lcdInitCmdBeforeDisplayOn();
        LCD_CtrlWrite_ST(0xAF); // Display ON
#else
        LCD_CtrlWrite_ST(0xE1); // Release Power Save Mode
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST(0xAF); // Display ON
#endif
    }
}

static void _stClose(void)
{
    _g_lcd_isActive = false;
    OSI_LOGI(0, "lcd[st7567a]: in _stClose");
    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[st7567a]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangle
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[st7567a]: in _stInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[st7567a]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[st7567a]: in stRotationInvalidateRect");
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

    OSI_LOGI(0, "lcd[st7567a]: st7567a stEsdCheck");
}

static const lcdOperations_t lcdOperations =
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

        // only for dot matrix LCD
        _stDotSetPixel,
        _stDotSendBuff,
        _stDotSetDisplayParam,
        NULL,
        NULL,
        _stDotSetFlushDirection,
};

lcdSpec_t g_lcd_st7567a =
    {
        LCD_DRV_ID,
        LCD_WIDTH,
        LCD_HEIGHT,
		HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&lcdOperations,
        false,
        0x2a000,
        5000000,    // st7567a Serial Clock Period Min.: 200ns
        NULL,
        LCD_BUFF_SIZE,  // no need for inner lcd buffer
};

static uint16_t st_line_mode;
static bool st_farmk;

static uint32_t _stReadId(void)
{
    uint32_t ret_id = 0;
    uint8_t id[2] = {0};
    HAL_ERR_T r_err;

    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);

    _lcdDelayMs(10);
    st_farmk = g_lcd_st7567a.is_use_fmark;
    st_line_mode = g_lcd_st7567a.bus_mode;

    halGoudaReadConfig(st_line_mode, g_lcd_st7567a.is_use_fmark, g_lcd_st7567a.fmark_delay);
    _lcdDelayMs(10);

    r_err = halGoudaReadData(0xFC, id, 2);
    OSI_LOGI(0, "lcd[st7567a]:  Status=0x%x 0x%x (read return : %d)", id[0], id[1], r_err);
    
    OSI_LOGI(0, "lcd[st7567a]:  st7567a don't support to read IC ID. \n");
    ret_id = LCD_DRV_ID;

    OSI_LOGI(0, "lcd[uc1617s]: force to support, id as 0x%x.", LCD_DRV_ID);
    return ret_id;
}


static uint8_t _dotMatrixLcdSetSeg(uint8_t segVal, uint8_t bit, uint8_t val)
{
    if (val == 0) {
        return segVal & (~(0x01 << bit));
    } else {
        return segVal | (0x01 << bit);
    }
}


static void _dotMatrixLcdSetAddr(uint8_t column, uint8_t page)
{
    column = column;
    LCD_CtrlWrite_ST(0xb0 + page);
    LCD_CtrlWrite_ST(((column >> 4) & 0x0f) + 0x10);
    LCD_CtrlWrite_ST(column & 0x0f);
}

static void _dotMatrixLcdCleanScreen()
{
    for (uint8_t j = 0; j < LCD_PAGE; j++) {
        _dotMatrixLcdSetAddr(0, j);
        for (uint8_t i = 0; i < LCD_WIDTH; i++) {
            LCD_DataWrite_ST(0x00);
        }
    }
}


OSI_UNUSED static uint8_t _dotMatrixLcdReadModifyWrite(uint16_t column, uint16_t page, uint8_t bit, uint8_t val)
{
    HAL_ERR_T r_err;
    uint8_t rData[2] = {0};

    LCD_CtrlWrite_ST(0xE0);
    _dotMatrixLcdSetAddr(column, page);

    r_err = halGoudaReadData(0x04, rData, 2);
    OSI_LOGI(0, "lcd[st7567a]:  _dotMatrixLcdReadModifyWrite: 0x%0x, 0x%x (read return : %d)\n", rData[0], rData[1], r_err);

    uint8_t oldSegVal = rData[1];
    uint8_t newSegVal = _dotMatrixLcdSetSeg(oldSegVal, bit, val);
    LCD_DataWrite_ST(newSegVal);
    LCD_CtrlWrite_ST(0xEE);
    return 0;
}

static uint8_t _dotMatrixLcdReadSegFromBuf(uint16_t column, uint16_t page)
{
    return (g_lcd_st7567a.lcdbuff[page * (LCD_WIDTH) + column]);
}

static void _dotMatrixLcdSetSegToBuf(uint16_t column, uint16_t page, uint8_t segVal)
{
    g_lcd_st7567a.lcdbuff[page * (LCD_WIDTH) + column] = segVal;
}

static int32_t _stDotSetPixel(uint16_t x, uint16_t y, uint8_t val)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
        OSI_PRINTFI("%s failed: coordinate out of range!", __func__);
        return -1;
    }
    uint8_t col = WHICH_COLUMN(x);
    uint8_t page = WHICH_PAGE(y);
    uint8_t bit = WHICH_BIT_IN_PAGE(y);

    #if 0
    _dotMatrixLcdReadModifyWrite(col, page, bit, val);
    #else
    uint8_t oldSegVal = _dotMatrixLcdReadSegFromBuf(col, page);
    uint8_t newSegVal = _dotMatrixLcdSetSeg(oldSegVal, bit, val);

    // OSI_PRINTFI("dotMatrixLcdSetPixel: pixel(%d, %d) val: %d -->  col: %d, page: %d, bit: %d", x, y, val, col, page, bit);
    _dotMatrixLcdSetAddr(col, page);
    LCD_DataWrite_ST(newSegVal);
    _dotMatrixLcdSetSegToBuf(col, page, newSegVal);
    #endif
    return 0;
}

static void _stDotSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias)
{
    _g_inst_regRes = contrastRatio_coarse;
    _g_inst_EVReg = contrastRatio_fine;
    _g_inst_bias = bias;
    OSI_LOGI(0, "lcd[st7567a]:   set lcd dispaly param. ");

    if (_g_lcd_isActive) {
        LCD_CtrlWrite_ST(INST_regRes(_g_inst_regRes)); // Select regulator register(1+(Ra+Rb))

        LCD_CtrlWrite_ST(0x81); // Set Reference Voltage 
        LCD_CtrlWrite_ST(INST_EVReg(_g_inst_EVReg)); // EV=35 => Vop =10.556V
        
        LCD_CtrlWrite_ST(INST_bias(_g_inst_bias)); // Set LCD Bias=1/9 V0
    }
}


static void _stDotSendBuff(uint8_t column, uint8_t row, void *buff, uint32_t size)
{
    OSI_LOGI(0, "lcd[st7567a]: Original send data: column=%d, row=%d, data size=%d", column, row, size);

    if ((column + size) > LCD_WIDTH) {
        OSI_LOGE(0, "lcd[st7567a]: out of range: column(%d) + size(%d) > LCD width(%d)", column, size, LCD_WIDTH);
        return;
    }
    _dotMatrixLcdSetAddr(column, row);
    for (uint32_t i = 0; i < size; i++) {
        LCD_DataWrite_ST(((uint8_t *)buff)[i]);
    }

    memcpy(&g_lcd_st7567a.lcdbuff[row * (LCD_WIDTH) + column], buff, size);
}

typedef enum {
    DOTLCD_DIRECT_NORMAL = 0,// SEG0 -> SEG131  COM0 -> COM63
    DOTLCD_DIRECT_MY,   // COM63 -> COM0
    DOTLCD_DIRECT_MX,   // SEG131 -> SEG0
    DOTLCD_DIRECT_MY_MX,// SEG131 -> SEG0 COM63 -> COM0
} dotlcd_direct_t;

static void _stDotSetFlushDirection(uint8_t direction)
{
    // LCD_CtrlWrite_ST(0xf8);         // set window disable
    _lcdDelayMs(100);

    switch (direction) {
        case DOTLCD_DIRECT_NORMAL: 
        {
            LCD_CtrlWrite_ST(0xa0);
            LCD_CtrlWrite_ST(0xC0);
        }
        break;

        case DOTLCD_DIRECT_MY: 
        {
            LCD_CtrlWrite_ST(0xa0);
            LCD_CtrlWrite_ST(0xC8);
        }
        break;

        case DOTLCD_DIRECT_MX: 
        {
            LCD_CtrlWrite_ST(0xa1);
            LCD_CtrlWrite_ST(0xC0);
        }
        break;

        case DOTLCD_DIRECT_MY_MX: 
        {
            LCD_CtrlWrite_ST(0xa1);
            LCD_CtrlWrite_ST(0xC8);
        }
        break;

        default:
        {
            LCD_CtrlWrite_ST(0xa0);
            LCD_CtrlWrite_ST(0xC0);
        }
        break;
    }
}

#endif
