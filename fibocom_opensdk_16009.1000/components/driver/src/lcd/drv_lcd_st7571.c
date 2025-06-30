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

#if defined(CONFIG_LCD_ST7571_SUPPORT)

#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
#define LCD_DRV_ID      AITI_DOT_LCD_ID_ST7571
#else
#define LCD_DRV_ID      DOT_LCD_ID_UNSUPPORT
#endif

#define LCD_WIDTH       128 // 96
#define LCD_HEIGHT      128
#define LCD_BUFF_SIZE   (LCD_WIDTH) * ((LCD_HEIGHT) / 8)

#define INST_regRes_MASK            0x20
#define INST_EVReg_MASK             0x00
#define INST_bias_MASK              0x50

#define INST_regRes_VALID_BITS      0x07
#define INST_EVReg_VALID_BITS       0x3F
#define INST_bias_VALID_BITS        0x07

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

static uint8_t _g_inst_regRes = 0x06;     //  (0~7): Select Regulator Resister
static uint8_t _g_inst_EVReg = 0x18;      //  (0~63): Set Electronic Volume Register
static uint8_t _g_inst_bias = 0x07;       // (0~7): 0-1/5 1-1/6 2-1/7 3-1/8 4-1/9 5-1/10 6-1/11 7-1/12
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
extern int32_t drv_lcd_read_pin_id(void);


static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _stSetDirection(lcdDirect_t direct_type)
{

    OSI_LOGI(0, "lcd[st7571]:    _stSetDirection = %d", direct_type);

    // switch (direct_type)
    // {
    // case LCD_DIRECT_NORMAL:
    //     //Vertical screen  startpoint at topleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0x48);
    //     break;
    // case LCD_DIRECT_ROT_90:
    //     //Horizontal screen  startpoint at topleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0xe8);
    //     break;
    // case LCD_DIRECT_ROT_180:
    //     //Horizontal screen  startpoint at topleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0x88);
    //     break;
	// case LCD_DIRECT_ROT_270:
    //     //Horizontal screen  startpoint at topleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0x28);
    //     break;
    // case LCD_DIRECT_NORMAL2:
    //     //Vertical screen  startpoint at bottomleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0xc8);
    //     break;
    // case LCD_DIRECT_NORMAL2_ROT_90:
    //     //Horizontal screen  startpoint at bottomleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0xa8);
    //     break;
    // case LCD_DIRECT_NORMAL2_ROT_180:
    //     //Horizontal screen  startpoint at bottomleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0x08);
    //     break;
	// case LCD_DIRECT_NORMAL2_ROT_270:
    //     //Horizontal screen  startpoint at topleft
    //     LCD_CtrlWrite_ST(0x36);
    //     LCD_DataWrite_ST(0x68);
    //     break;
    // }

    // LCD_CtrlWrite_ST(0x2c);
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
    // uint16_t newleft = left;
    // uint16_t newtop = top;
    // uint16_t newright = right;
    // uint16_t newbottom = bottom;
    // OSI_LOGI(0, "lcd[st7571]:    _stSetDisplayWindow L = %d, top = %d, R = %d, bottom = %d", left, top, right, bottom);

    // LCD_CtrlWrite_ST(0x2a);                   // set hori start , end (left, right)
    // LCD_DataWrite_ST((newleft >> 8) & 0xff);  // left high 8 b
    // LCD_DataWrite_ST(newleft & 0xff);         // left low 8 b
    // LCD_DataWrite_ST((newright >> 8) & 0xff); // right high 8 b
    // LCD_DataWrite_ST(newright & 0xff);        // right low 8 b

    // LCD_CtrlWrite_ST(0x2b);                    // set vert start , end (top, bot)
    // LCD_DataWrite_ST((newtop >> 8) & 0xff);    // top high 8 b
    // LCD_DataWrite_ST(newtop & 0xff);           // top low 8 b
    // LCD_DataWrite_ST((newbottom >> 8) & 0xff); // bot high 8 b
    // LCD_DataWrite_ST(newbottom & 0xff);        // bot low 8 b
    // LCD_CtrlWrite_ST(0x2c);                    // recover memory write mode
}


static void _lcdInitCmdBeforeDisplayOn(void)
{
    LCD_CtrlWrite_ST(0xE1); // Release Power Save Mode
    
    // LCD_CtrlWrite_ST(0xe2); // software reset
    // _lcdDelayMs(150);

    LCD_CtrlWrite_ST(0x2C); // Power Control, VC: ON VR: OFF VF: OFF
    _lcdDelayMs(200); // Delay 200ms
    LCD_CtrlWrite_ST(0x2E); // Power Control, VC: ON VR: ON VF: OFF
    _lcdDelayMs(200); // Delay 200ms
    LCD_CtrlWrite_ST(0x2F); // Power Control, VC: ON VR: ON VF: ON
    _lcdDelayMs(10); // Delay 10ms

    LCD_CtrlWrite_ST(0xAE); // Display OFF

    LCD_CtrlWrite_ST(0x38); // MODE SET
    LCD_CtrlWrite_ST(0xB8); // FR=1011 => 85Hz
                            // BE[1:0]=1,0 => booster efficiency Level-3

    LCD_CtrlWrite_ST(0xA0); // ADC select, ADC=1 =>reverse direction
    LCD_CtrlWrite_ST(0xC8); // SHL select, SHL=1 => reverse direction
    LCD_CtrlWrite_ST(0x44); // Set initial COM0 register
    LCD_CtrlWrite_ST(0x00); //
    LCD_CtrlWrite_ST(0x40); // Set initial display line register
    LCD_CtrlWrite_ST(0x00); //

    LCD_CtrlWrite_ST(0xF3); // Release Bias Power Save Mode

    LCD_CtrlWrite_ST(0x04); //
    LCD_CtrlWrite_ST(0x93); // Set FRC and PWM mode (4FRC & 15PWM)

    LCD_CtrlWrite_ST(0xAB); // OSC. ON

    LCD_CtrlWrite_ST(0x67); // DC-DC step up, 8 times boosting circuit

    LCD_CtrlWrite_ST(0x7B); // Extension Command Set 3
    // LCD_CtrlWrite_ST(0x11); // Black/White mode
    LCD_CtrlWrite_ST(0x10); // Gray-Scale mode
    LCD_CtrlWrite_ST(0x00); // Return to Normal Mode

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
    OSI_LOGI(0, "lcd[st7571]:   _stInit ");

    _lcdDelayMs(100); // Delay 100ms for stable VDD1/VDD2/VDD3
    
    halGoudatResetLcdPin(); // hardware reset
    _lcdDelayMs(150);   // tR: Reset time > 120ms

    _lcdInitCmdBeforeDisplayOn();
    _dotMatrixLcdCleanScreen();
    LCD_CtrlWrite_ST(0xAF); // Display ON

    OSI_LOGI(0, "lcd[st7571]:   _stInit done. ");
    _g_lcd_isActive = true;
}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[st7571]:  _stSleepIn, is_sleep = %d", is_sleep);
    if (is_sleep) {
        _g_lcd_isActive = false;
        LCD_CtrlWrite_ST(0xAE); //display off
        _lcdDelayMs(120);
        LCD_CtrlWrite_ST(0xa9); // power-save mode (sleep mode)
        LCD_CtrlWrite_ST(0xa8); // normal mode
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
    OSI_LOGI(0, "lcd[st7571]: in _stClose");
    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[st7571]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangle
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[st7571]: in _stInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[st7571]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[st7571]: in stRotationInvalidateRect");
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

    OSI_LOGI(0, "lcd[st7571]: st7571 stEsdCheck");
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
};

lcdSpec_t g_lcd_st7571 =
    {
        LCD_DRV_ID,
        LCD_WIDTH,
        LCD_HEIGHT,
        // HAL_GOUDA_SPI_LINE_3_2_LANE,
		// HAL_GOUDA_SPI_LINE_3,
		HAL_GOUDA_SPI_LINE_4,
        LCD_CTRL_SPI,
        (lcdOperations_t *)&lcdOperations,
        false,
        0x2a000,
        5000000,    // st7571 Serial Clock Period Min.: 200ns
        NULL,
        LCD_BUFF_SIZE,
};


static uint32_t _stReadId(void)
{
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
    int32_t io_id = drv_lcd_read_pin_id();
    if (LCD_DRV_ID == io_id) {
        OSI_LOGI(0, "lcd[st7571]:  st is using !");
    }
    return io_id;
#else
    uint32_t ret_id = 0;

    OSI_LOGI(0, "lcd[st7571]:  st7571 don't support to read IC ID. \n");
    ret_id = LCD_DRV_ID;

    OSI_LOGI(0, "lcd[uc1617s]: force to support, id as 0x%x.", LCD_DRV_ID);
    return ret_id;
#endif
}



static uint8_t _dotMatrixLcdReadSegFromBuf(uint16_t column, uint16_t page)
{
    return (g_lcd_st7571.lcdbuff[page * (LCD_WIDTH) + column]);
}

static void _dotMatrixLcdSetSegToBuf(uint16_t column, uint16_t page, uint8_t segVal)
{
    g_lcd_st7571.lcdbuff[page * (LCD_WIDTH) + column] = segVal;
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
    LCD_CtrlWrite_ST(((column >> 4) & 0x07) + 0x10);
    LCD_CtrlWrite_ST(column & 0x0f);
}

static void _dotMatrixLcdCleanScreen()
{
    for (uint8_t j = 0; j < 16; j++) {
        _dotMatrixLcdSetAddr(0, j);
        for (uint8_t i = 0; i < 128; i++) {
            LCD_DataWrite_ST(0x00);
            LCD_DataWrite_ST(0x00);
        }
    }
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
    uint8_t oldSegVal = _dotMatrixLcdReadSegFromBuf(col, page);
    uint8_t newSegVal = _dotMatrixLcdSetSeg(oldSegVal, bit, val);

    // OSI_PRINTFI("dotMatrixLcdSetPixel: pixel(%d, %d) val: %d -->  col: %d, page: %d, bit: %d", x, y, val, col, page, bit);
    _dotMatrixLcdSetAddr(col, page);
    LCD_DataWrite_ST(newSegVal);
    LCD_DataWrite_ST(newSegVal);
    _dotMatrixLcdSetSegToBuf(col, page, newSegVal);
    return 0;
}

static void _stDotSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias)
{
    _g_inst_regRes = contrastRatio_coarse;
    _g_inst_EVReg = contrastRatio_fine;
    _g_inst_bias = bias;
    OSI_LOGI(0, "lcd[st7571]:   set lcd dispaly param. ");

    if (_g_lcd_isActive) {
        LCD_CtrlWrite_ST(INST_regRes(_g_inst_regRes)); // Select regulator register(1+(Ra+Rb))

        LCD_CtrlWrite_ST(0x81); // Set Reference Voltage 
        LCD_CtrlWrite_ST(INST_EVReg(_g_inst_EVReg)); // EV=35 => Vop =10.556V
        
        LCD_CtrlWrite_ST(INST_bias(_g_inst_bias)); // Set LCD Bias=1/9 V0
    }
}


static void _stDotSendBuff(uint8_t column, uint8_t row, void *buff, uint32_t size)
{
    OSI_LOGI(0, "lcd[st7571]: Original send data: column=%d, row=%d, data size=%d", column, row, size);
    _dotMatrixLcdSetAddr(column, row);
    for (uint32_t i = 0; i < size; i++) {
        LCD_DataWrite_ST(((uint8_t *)buff)[i]);
    }
}


#endif
