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

#if defined(CONFIG_LCD_UC1617S_SUPPORT)

#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
#define LCD_DRV_ID      AITI_DOT_LCD_ID_UC1617S
#else
#define LCD_DRV_ID      DOT_LCD_ID_UNSUPPORT
#endif

#define LCD_WIDTH       (96) // 96
#define LCD_HEIGHT      (128)
#define CONFIG_LCD_UC1617S_BW_MODE  // 黑白模式

static bool _g_lcd_isActive = false;

static uint8_t _g_inst_regRes = 0x03;     // SET TEMPERATURE COMPENSATION (0~3)
static uint8_t _g_inst_EVReg = 0X3B;      // Set Electronic Volume Register (0~0xFF)
static uint8_t _g_inst_bias = 0x02;       // Set LCD Bias Ratio: 0-1/6 1-1/9 2-1/10 3-1/11 (0~3)
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
static uint8_t _g_inst_linerate = 0x01;   // Set line rate, should refer to datasheet (0~3)
#else
static uint8_t _g_inst_linerate = 0x03;   // Set line rate, should refer to datasheet (0~3)
#endif

#define INST_regRes_MASK            0x24
#define INST_EVReg_MASK             0x00
#define INST_bias_MASK              0xe8
#define INST_RamAddrCtrl_MASK       0x88
#define INST_SetLineRate_MASK       0xA0

#define INST_regRes_VALID_BITS      0x03
#define INST_EVReg_VALID_BITS       0xFF
#define INST_bias_VALID_BITS        0x03
#define INST_SetLineRate_BITS       0x03


//  Select Regulator Resister
#define INST_regRes(x)    (INST_regRes_MASK | ((x) & INST_regRes_VALID_BITS))   

//  Set Electronic Volume Register
#define INST_EVReg(x)     (INST_EVReg_MASK | ((x) & INST_EVReg_VALID_BITS))     

// Set LCD Bias Ratio
#define INST_bias(x)      (INST_bias_MASK | ((x) & INST_bias_VALID_BITS))       
    
// SET RAM ADDRESS CONTROL
#define INST_RamAddrCtrl(ac2,ac1,ac0)  \
                          (INST_RamAddrCtrl_MASK | (((ac2) & 0x01) << 2) | (((ac1) & 0x01) << 1) | ((ac0) & 0x01))

// Set line rate
#define INST_lineRate(x)  (INST_SetLineRate_MASK | ((x) & INST_SetLineRate_BITS))       


#define PAGE_C_COL_NUM              (4)
#define PAGE_C_NUM                  (LCD_WIDTH / PAGE_C_COL_NUM)
#define ROW_NUM                     LCD_HEIGHT
#define LCD_BUFF_SIZE               (PAGE_C_NUM * ROW_NUM)

#define WHICH_PAGE_C(x)             ((x) / PAGE_C_COL_NUM)
#define WHICH_DOT_IN_PAGE_C(x)      ((x) % PAGE_C_COL_NUM)
#define WHICH_ROW(y)                (y)


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

static void _dotMatrixLcdCleanScreen(void);
static int32_t _stDotSetPixel(uint16_t x, uint16_t y, uint8_t val);
static void _stDotSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias);
static void _stDotSetLcmRate(uint8_t rate);
static void _stDotSendBuff(uint8_t column, uint8_t row, void *buff, uint32_t size);
static void _stDotSetAddrOnly(uint8_t column, uint8_t row);
static void _stDotSendBuffOnly(void *buff, uint32_t size);
static void _stDotSetFlushDirection(uint8_t direction);
extern int32_t drv_lcd_read_pin_id(void);


static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

OSI_UNUSED static void _stSetDirection(lcdDirect_t direct_type)
{

    OSI_LOGI(0, "lcd[uc1617s]:    _stSetDirection = %d", direct_type);

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
    uint16_t newleft = left;
    uint16_t newtop = top;
    uint16_t newright = right;
    uint16_t newbottom = bottom;
    // OSI_LOGI(0, "lcd[uc1617s]:    _stSetDisplayWindow page_C start:%d, row start:%d, page_C end:%d, row end:%d", left, top, right, bottom);

    LCD_CtrlWrite_ST(0xf8);         // set window disable

    LCD_CtrlWrite_ST(0xf4);         // set start x
    LCD_CtrlWrite_ST(newleft); 
    LCD_CtrlWrite_ST(0xf5);         // set start y
    LCD_CtrlWrite_ST(newtop); 
    LCD_CtrlWrite_ST(0xf6);         // set end x
    LCD_CtrlWrite_ST(newright);
    LCD_CtrlWrite_ST(0xf7);         // set end y
    LCD_CtrlWrite_ST(newbottom);

    LCD_CtrlWrite_ST(0xf9);         // set window enable
}

static void _lcdInitCmdBeforeDisplayOn(void)
{
    // LCD_CtrlWrite_ST(0xe2);	    // Set reset
    // _lcdDelayMs(10);
    
    LCD_CtrlWrite_ST(INST_regRes(_g_inst_regRes));  // set temperature consenpation
    LCD_CtrlWrite_ST(INST_bias(_g_inst_bias));  // set bias
    LCD_CtrlWrite_ST(0x81); // set vop
    LCD_CtrlWrite_ST(INST_EVReg(_g_inst_EVReg));

    LCD_CtrlWrite_ST(INST_lineRate(_g_inst_linerate));// set line rate
    // LCD_CtrlWrite_ST(0xA3);// set line rate max 0xa0
    LCD_CtrlWrite_ST(0x2B);// set pannel loading	 
    LCD_CtrlWrite_ST(0x40);// set scroll line LSB
    LCD_CtrlWrite_ST(0x50);// set scroll line MSB
    LCD_CtrlWrite_ST(0x2f);// set pump control:internal	
    LCD_CtrlWrite_ST(0x89);// set RAM Address control	
    LCD_CtrlWrite_ST(0xC4);// SET LCD MAPPING CONTROL MY=1
    LCD_CtrlWrite_ST(0xF1);
    LCD_CtrlWrite_ST(0x7F);		   
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _stInit(void)
{
    OSI_LOGI(0, "lcd[uc1617s]:   _stInit ");
    
    _lcdDelayMs(100); // Delay 100ms for stable VDD1/VDD2/VDD3
    
    halGoudatResetLcdPin(); // hardware reset
    _lcdDelayMs(150);   // tR: Reset time > 120ms

    _lcdInitCmdBeforeDisplayOn();

    _stSetDisplayWindow(0, 0, PAGE_C_NUM - 1, ROW_NUM - 1);
    _dotMatrixLcdCleanScreen();
#if defined(CONFIG_LCD_UC1617S_BW_MODE)
    LCD_CtrlWrite_ST(0xAD);//set display enable, B/W Mode
#else
    LCD_CtrlWrite_ST(0xAF);//set display enable, 4 shade mode
#endif
    _lcdDelayMs(20);

    _g_lcd_isActive = true;
    OSI_LOGI(0, "lcd[uc1617s]:   _stInit done. ");
}

static void _stSleepIn(bool is_sleep)
{
    OSI_LOGI(0, "lcd[uc1617s]:  _stSleepIn, is_sleep = %d", is_sleep);
    if (is_sleep) {
        _g_lcd_isActive = false;
#if defined(CONFIG_LCD_UC1617S_BW_MODE)
        LCD_CtrlWrite_ST(0xAC);//set sleep mode, B/W mode
#else
        LCD_CtrlWrite_ST(0xAE);//set sleep mode, 4 shade mode
#endif
    } else {
        _lcdDelayMs(150);
        _g_lcd_isActive = true;
        _lcdInitCmdBeforeDisplayOn();
        _stSetDisplayWindow(0, 0, PAGE_C_NUM - 1, ROW_NUM - 1);
#if defined(CONFIG_LCD_UC1617S_BW_MODE)
        LCD_CtrlWrite_ST(0xAD);//set display enable, B/W Mode
#else
        LCD_CtrlWrite_ST(0xAF);//set display enable, 4 shade mode
#endif
        _lcdDelayMs(20);
    }
}

static void _stClose(void)
{
    _g_lcd_isActive = false;
    OSI_LOGI(0, "lcd[uc1617s]: in _stClose");
    _stSleepIn(true);
}
static void _stInvalidate(void)
{
    OSI_LOGI(0, "lcd[uc1617s]:  in _stInvalidate");

    _stSetDisplayWindow(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _stInvalidateRect(
    uint16_t left,  //the left value of the rectangle
    uint16_t top,   //top of the rectangle
    uint16_t right, //right of the rectangle
    uint16_t bottom //bottom of the rectangle
)
{

    OSI_LOGI(0, "lcd[uc1617s]: in _stInvalidateRect");
    left = (left >= LCD_WIDTH) ? LCD_WIDTH - 1 : left;
    right = (right >= LCD_WIDTH) ? LCD_WIDTH - 1 : right;
    top = (top >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : top;
    bottom = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT - 1 : bottom;

    if ((right < left) || (bottom < top))
    {

        OSI_ASSERT(false, "lcd[uc1617s]: in _stInvalidateRect error"); /*assert verified*/
    }

    _stSetDisplayWindow(left, top, right, bottom);
}

static void _stRotationInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, lcdAngle_t angle)
{

    OSI_LOGI(0, "lcd[uc1617s]: in stRotationInvalidateRect");
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

    OSI_LOGI(0, "lcd[uc1617s]: uc1617s stEsdCheck");
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
        _stDotSetAddrOnly,
        _stDotSendBuffOnly,
        _stDotSetFlushDirection,
        _stDotSetLcmRate,
};

lcdSpec_t g_lcd_uc1617s =
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
        1000000,
        NULL,
        LCD_BUFF_SIZE,
};


static uint32_t _stReadId(void)
{
    OSI_LOGI(0, "lcd[uc1617s]:  uc1617s   _stReadId \n");
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
    int32_t io_id = drv_lcd_read_pin_id();
    if (LCD_DRV_ID == io_id) {
        OSI_LOGI(0, "lcd[uc1617s]:  st is using !");
    }
    return io_id;
    // return LCD_DRV_ID;
#else
    uint32_t ret_id = 0;
    HAL_ERR_T r_err;
    uint8_t id[3] = {0};

    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
#ifdef CONFIG_FIBOCOM_LG610
    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
    halPmuSwitchPower(HAL_POWER_VIBR, true, true);
    halPmuSetPowerLevel(HAL_POWER_VIBR, POWER_LEVEL_2800MV);
#endif
    _lcdDelayMs(10);

    halGoudaReadConfig(g_lcd_uc1617s.bus_mode, g_lcd_uc1617s.is_use_fmark, g_lcd_uc1617s.fmark_delay);
    _lcdDelayMs(140);
    //hwp_analogReg->pad_spi_lcd_cfg2 |= ANALOG_REG_PAD_SPI_LCD_SIO_SPU;
    _lcdDelayMs(10);
    r_err = halGoudaReadData(0xfe, id, 3);

    //hwp_analogReg->pad_spi_lcd_cfg2 =  hwp_analogReg->pad_spi_lcd_cfg2&(~ANALOG_REG_PAD_SPI_LCD_SIO_SPU);
    _lcdDelayMs(10);
    OSI_LOGI(0, "lcd[uc1617s]:  _stReadId get status: 0x%0x, 0x%x, 0x%0x (read return : %d)\n", id[0], id[1], id[2], r_err);
    ret_id = id[2] >> 4;

    OSI_LOGI(0, "lcd[uc1617s]:  _stReadId Production Code: 0x%0x\n", ret_id);

    if (LCD_DRV_ID == ret_id)
    {
        OSI_LOGI(0, "lcd[uc1617s]:  st is using !");
    }
    OSI_LOGI(0, "lcd[uc1617s]: uc1617s ReadId:0x%x \n",ret_id);
    
    ret_id = LCD_DRV_ID;
    OSI_LOGI(0, "lcd[uc1617s]: force to support, id as 0x%x.", LCD_DRV_ID);
    return ret_id;
#endif
}

static uint8_t _dotMatrixLcdReadSegFromBuf(uint8_t page_c, uint8_t row)
{
    return (g_lcd_uc1617s.lcdbuff[row * PAGE_C_NUM + page_c]);
}

static void _dotMatrixLcdSetSegToBuf(uint8_t page_c, uint8_t row, uint8_t segVal)
{
    g_lcd_uc1617s.lcdbuff[row * PAGE_C_NUM + page_c] = segVal;
}

static uint8_t _dotMatrixLcdSetSeg(uint8_t segVal, uint8_t dot, uint8_t val)
{
    uint8_t newSegVal = segVal;
    newSegVal &= (~(0x03 << (dot * 2)));
    newSegVal |= ((val & 0x03) << (dot * 2));
    return newSegVal;
}

static void _dotMatrixLcdSetAddr(uint8_t page_c, uint8_t row)
{
    _stSetDisplayWindow(0, 0, PAGE_C_NUM - 1, ROW_NUM - 1);

    LCD_CtrlWrite_ST(0xf9); // set window enable
    LCD_CtrlWrite_ST(0x00 + (page_c & 0x1F));       // set page start
    LCD_CtrlWrite_ST(0x60 + (row & 0x0F));          // Row start
    LCD_CtrlWrite_ST(0x70 + ((row >> 4 & 0x07)));   // Row start
}


static void _dotMatrixLcdFlushSram(void *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        LCD_DataWrite_ST(*(((uint8_t *)data) + i));
    }
}

static void _dotMatrixLcdCleanScreen(void)
{
    _dotMatrixLcdSetAddr(0, 0);
    _dotMatrixLcdFlushSram(g_lcd_uc1617s.lcdbuff, LCD_BUFF_SIZE);
}

static int32_t _stDotSetPixel(uint16_t x, uint16_t y, uint8_t val)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
        OSI_PRINTFI("%s failed: coordinate out of range!", __func__);
        return -1;
    }

    uint8_t page_c = WHICH_PAGE_C(x);
    uint8_t row = WHICH_ROW(y);
    uint8_t dot = WHICH_DOT_IN_PAGE_C(x);
    uint8_t oldSegVal = _dotMatrixLcdReadSegFromBuf(page_c, row);
    uint8_t newSegVal = _dotMatrixLcdSetSeg(oldSegVal, dot, val);

    // OSI_LOGI(0, "_stDotSetPixel: pixel(%d, %d) val: %d -->  page_c: %d, row: %d, dot: %d [old seg: 0x%02x] [new seg: 0x%02x]", 
                // x, y, val, page_c, row, dot, oldSegVal, newSegVal);
    _dotMatrixLcdSetAddr(page_c, row);
    LCD_DataWrite_ST(newSegVal);

    _dotMatrixLcdSetSegToBuf(page_c, row, newSegVal);
    return 0;
}

static void _stDotSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias)
{
    _g_inst_regRes = contrastRatio_coarse;
    _g_inst_EVReg = contrastRatio_fine;
    _g_inst_bias = bias;
    OSI_LOGI(0, "lcd[uc1617s]:   set lcd dispaly param. contrastRatio_coarse=%d, contrastRatio_fine=%d, bias=%d", contrastRatio_coarse, contrastRatio_fine, bias);

    if (_g_lcd_isActive) {
        LCD_CtrlWrite_ST(INST_regRes(_g_inst_regRes));  // set temperature consenpation
        LCD_CtrlWrite_ST(INST_bias(_g_inst_bias));  // set bias
        LCD_CtrlWrite_ST(0x81); // set vop
        LCD_CtrlWrite_ST(INST_EVReg(_g_inst_EVReg));
    }

}

static void _stDotSetLcmRate(uint8_t rate)
{
    _g_inst_linerate = rate;
    OSI_LOGI(0, "lcd[uc1617s]: set LCM line rate: %d", rate);

    if (_g_lcd_isActive) {
        LCD_CtrlWrite_ST(INST_lineRate(_g_inst_linerate));// set line rate
    }
}

static void _stDotSendBuff(uint8_t column, uint8_t row, void *buff, uint32_t size)
{
    OSI_LOGI(0, "lcd[uc1617s]: Original send data: column=%d, row=%d, data size=%d", column, row, size);
    _dotMatrixLcdSetAddr(column, row);
    _dotMatrixLcdFlushSram(buff, size);
}

static void _stDotSetAddrOnly(uint8_t column, uint8_t row)
{
    OSI_LOGI(0, "lcd[uc1617s]: _stDotSetBuffStartAddr: column=%d, row=%d", column, row);
    _dotMatrixLcdSetAddr(column, row);
}

static void _stDotSendBuffOnly(void *buff, uint32_t size)
{
    _dotMatrixLcdFlushSram(buff, size);
}

typedef enum {
    DOTLCD_DIRECTION_COL_ROW_P = 0,  // column increases first, row +1
    DOTLCD_DIRECTION_ROW_ROW_P,      // Row increases first,    row +1
    DOTLCD_DIRECTION_COL_ROW_M,      // column increases first, row -1
    DOTLCD_DIRECTION_ROW_ROW_M,      // Row increases first,    row -1
} dotlcd_direct_t;

static void _stDotSetFlushDirection(uint8_t direction)
{
    // LCD_CtrlWrite_ST(0xf8);         // set window disable
    _lcdDelayMs(100);

    switch (direction) {
        case DOTLCD_DIRECTION_COL_ROW_P: 
        {
            LCD_CtrlWrite_ST(INST_RamAddrCtrl(0, 0, 1));
        }
        break;

        case DOTLCD_DIRECTION_ROW_ROW_P: 
        {
            LCD_CtrlWrite_ST(INST_RamAddrCtrl(0, 1, 1));
        }
        break;

        case DOTLCD_DIRECTION_COL_ROW_M: 
        {
            LCD_CtrlWrite_ST(INST_RamAddrCtrl(1, 0, 1));
        }
        break;

        case DOTLCD_DIRECTION_ROW_ROW_M: 
        {
            LCD_CtrlWrite_ST(INST_RamAddrCtrl(1, 1, 1));
        }
        break;

        default:
        {
            OSI_PRINTFI("%s failed: param error!", __func__);
        }
        break;
    }
    // _stSetDisplayWindow(0, 0, PAGE_C_NUM - 1, ROW_NUM - 1);
    // _dotMatrixLcdCleanScreen();
    // LCD_CtrlWrite_ST(0xf9); // set window enable
}


#endif
