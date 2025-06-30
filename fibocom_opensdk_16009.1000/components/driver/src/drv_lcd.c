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
#include "hal_chip.h"
#include "hwregs.h"
#include "osi_api.h"
#include "osi_profile.h"
#include "osi_byte_buf.h"
#include "drv_lcd.h"
#include "hal_gouda.h"
#include "osi_log.h"
#include "osi_mem.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define LCDD_SPI_FREQ (50000000)
#define LCD_LOCK(lock) osiSemaphoreAcquire(lock)
#define LCD_UNLOCK(lock) osiSemaphoreRelease(lock)
#define BLOCK_SUM 4
static osiSemaphore_t *lcdLock = NULL;
#ifdef CONFIG_FIBOCOM_BASE
static bool g_lcddRotate=true;
typedef enum
{
    LCD_NULL=0,
    LCD_REGISTERED
} lcdReg_t;
#endif

static void _lcdDelayMs(int ms_delay)
{
    osiDelayUS(ms_delay * 1000);
}

typedef struct block_tag
{
    uint16_t is_enable; //1: enable this block;0: disable this block
    uint32_t *mem_ptr;
    uint32_t *alpha_addr;
    uint32_t *uv_addr;
    uint32_t *v_addr;
    lcdBlockCfg_t cfg;
} BLOCK_T;

typedef struct _lcd_rect_t_tag
{
    uint16_t left;   //LCD display window left position
    uint16_t top;    //LCD display window top position
    uint16_t right;  //LCD display window right position
    uint16_t bottom; //LCD display window bottom position
} LCD_RECT_T;

//Brush LCD mode
typedef enum
{
    LCD_BRUSH_MODE_SYNC = 0,
    LCD_BRUSH_MODE_ASYNC
} LCD_BRUSH_MODE_E;

static struct LAYER_SIZE
{
    uint32_t width;
    uint32_t height;
    uint32_t imageBufWidth; //the origin image buffer width
} g_vid_size = {0};

typedef struct
{
    LCD_BRUSH_MODE_E brush_mode; // 1 if brush lcd with synchronistic mean, or else 0
    uint32_t sleep_state;
    lcdSpec_t *lcd_spec;
} LCD_CONFIG_T;

typedef enum _lcd_layer_id_tag
{
    LCD_LAYER_IMAGE,
    LCD_LAYER_OSD1,
    LCD_LAYER_OSD2,
    LCD_LAYER_OSD3,
    LCD_LAYER_ID_MAX
} LCD_LAYER_ID_E;

static uint32_t s_lcd_used_num = 0;
static BLOCK_T g_block_info[BLOCK_SUM];
#ifdef CONFIG_FIBOCOM_BASE
static lcdSpec_t *s_lcd_spec_info_ptr = NULL;
static uint8_t g_ucLcdRegister = LCD_NULL;
static lcdCfg_t  *s_lcd_config_info = NULL;
#else
static lcdSpec_t *s_lcd_spec_info_ptr;
#endif

static LCD_CONFIG_T g_lcd_cfg;

#define LCDC_LAYER_NUM 4

#define IS_VIDEO_LAYER(iBlock) \
    (iBlock == LCD_LAYER_IMAGE && (g_block_info[iBlock].cfg.resolution == LCD_RESOLUTION_YUV420 || g_block_info[iBlock].cfg.resolution == LCD_RESOLUTION_YUV422_YUYV || g_block_info[iBlock].cfg.resolution == LCD_RESOLUTION_YUV420_3PLANE || g_block_info[iBlock].cfg.resolution == LCD_RESOLUTION_YUV422_UYVY))

static HAL_GOUDA_LCD_CONFIG_T gd_lcd_config =
    {

        .b.cs = HAL_GOUDA_LCD_CS_0,
        .b.outputFormat = HAL_GOUDA_LCD_OUTPUT_FORMAT_16_bit_RGB565,
        .b.cs0Polarity = false,
        .b.cs1Polarity = false,
        .b.resetb = true,
        .b.rsPolarity = false,
        .b.wrPolarity = false,
        .b.rdPolarity = false,
        .b.highByte = false,

};

void drvLcdDisableAllBlock(void);
void drvLcdEnableBlock(uint32_t blk_num);
bool drvLcdConfigBlock(uint32_t blk_num, lcdBlockCfg_t config);
void drvLcdSetBlockBuffer(uint32_t blk_num, uint32_t *buf_ptr);
void drvLcdInvalidataRectToRam(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t *buffer);
#ifdef CONFIG_FIBOCOM_BASE
int32_t drvLcdInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
#else
void drvLcdInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
#endif
void drvDisableBlock(uint32_t blk_num);

#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)

const static uint32_t dot_lcd_id_tab[] = {
    DOT_LCD_ID_UNSUPPORT,
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
    AITI_DOT_LCD_ID_ST7571,
    AITI_DOT_LCD_ID_UC1617S,
#endif
};

static bool _is_dotMatLcd(void)
{
    for (uint32_t i = 0; i < sizeof(dot_lcd_id_tab) / sizeof(uint32_t); i++) {
        if (s_lcd_spec_info_ptr->dev_id == dot_lcd_id_tab[i]) {
            return true;
        }
    }
    return false;
}
#endif


extern lcdSpec_t *drvLcdGetSpecInfo(void);

static void _drvGetSemaphore(void)
{
    LCD_LOCK(lcdLock);
}

static void _lcdPutSemaphore(void)
{
    LCD_UNLOCK(lcdLock);
}
#ifndef CONFIG_FIBOCOM_BASE
/*****************************************************************************/
//Description:Set video layer parameter
//
/*****************************************************************************/

bool drvLcdSetImageLayerSize(uint32_t width, uint32_t height, uint32_t imageBufWidth)
{
    g_vid_size.width = width;
    g_vid_size.height = height;
    g_vid_size.imageBufWidth = imageBufWidth;
    return true;
}
#endif
/*****************************************************************************/
//Description: Close the lcd.
//Global resource dependence:
/*****************************************************************************/
void drvLcdClose(void)
{
    uint16_t lcd_id;
    OSI_LOGI(0x10007f3c, "lcd:  :: drvLcdClose [IN]");
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return;
    }
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == false)
#endif
    {
        //close all lcd support
        for (lcd_id = 0; lcd_id < s_lcd_used_num; lcd_id++)
        {
            if (NULL != s_lcd_spec_info_ptr->operation)
            {
            OSI_LOGI(0x10007f3e, "lcd:  :: drvLcdClose [operation close]");

            s_lcd_spec_info_ptr->operation->close();
            }
        }
    halGoudaClose();
    osiThreadSleep(150);
    halGoudaClkDisable();
    halSetGoudaClockMode(false);
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
    }
#ifdef CONFIG_FIBOCOM_BASE
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, "dotMatrixLcdSendData:free lcd buffer.");
        free(s_lcd_spec_info_ptr->lcdbuff);
        s_lcd_spec_info_ptr->lcdbuff = NULL;
    }
#endif
    s_lcd_used_num=0;
    g_lcddRotate=true;
    osiSemaphoreDelete(lcdLock);
    lcdLock=NULL;
#endif
    OSI_LOGI(0x10007f3f, "lcd:    drvLcdClose [OUT]");
}
/*****************************************************************************/
//Description:   get the lcd information.
//Global resource dependence:
/*****************************************************************************/
void drvLcdGetLcdInfo(lcdSpec_t *lcd_info_ptr)
{
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return;
    }
    lcd_info_ptr->dev_id = s_lcd_spec_info_ptr->dev_id;
    lcd_info_ptr->width = s_lcd_spec_info_ptr->width;
    lcd_info_ptr->height = s_lcd_spec_info_ptr->height;
    lcd_info_ptr->bus_mode = s_lcd_spec_info_ptr->bus_mode;
    lcd_info_ptr->controller = s_lcd_spec_info_ptr->controller;
    lcd_info_ptr->operation = s_lcd_spec_info_ptr->operation;
    lcd_info_ptr->is_use_fmark = s_lcd_spec_info_ptr->is_use_fmark;
    lcd_info_ptr->fmark_delay = s_lcd_spec_info_ptr->fmark_delay;
}

static void _drvLcdHandler(void)
{
    int i;

    halGoudaVidLayerClose();

    for (i = 1; i < LCDC_LAYER_NUM; i++)
    {
        if (g_block_info[i].is_enable)
        {
            halGoudaOvlLayerClose(i - 1);
            drvDisableBlock(i);
        }
    }

    drvLcdSetImageLayerSize(0, 0, 0);
    _lcdPutSemaphore();
}

#ifdef CONFIG_FIBOCOM_BASE
/* Timeout time to reset lcd controller while keeping busy, unit: us 

   For those customer whose uses a small lcd clk frequency, such as 10MHz frequency, 240*240 size and by one data lane, 
   it needs at least 100ms for one frame to transfer, so the timeout time should be 100ms plus a little extra time(for 
   command, etc). To include as many customer scenarios as possible, 500ms is recommendable.
*/
#define __WAIT_GOUDA_READY_TIMEOUT  (500 * 1000)

void _reset_gouda(void)
{
    volatile uint32_t *reg_ap_rst0 = (uint32_t *)0x04803018;
    volatile uint32_t *reg_ap_rst1 = (uint32_t *)0x0480301c;
    volatile uint32_t *reg_ap_rst0_clr = (uint32_t *)0x04803818;
    volatile uint32_t *reg_ap_rst1_clr = (uint32_t *)0x0480381c;

    *reg_ap_rst0 |= (0x01 << 4);
    *reg_ap_rst1 |= (0x01 << 6);
    osiDelayUS(100);
    *reg_ap_rst0_clr |= (0x01 << 4);
    *reg_ap_rst1_clr |= (0x01 << 6);

    halGoudaInit(gd_lcd_config, s_lcd_spec_info_ptr->freq);
    OSI_LOGI(0, "Gjh: lcd freq=%d", s_lcd_spec_info_ptr->freq);
}
#endif

#ifdef CONFIG_FIBOCOM_BASE
static int32_t _drvLcdGoudaUpdate(const HAL_GOUDA_WINDOW_T *pWin, bool bSync)
#else
static void _drvLcdGoudaUpdate(const HAL_GOUDA_WINDOW_T *pWin, bool bSync)
#endif
{
    HAL_GOUDA_LCD_CMD_T cmd[13];

#ifdef CONFIG_FIBOCOM_BASE
    if (!((pWin->tlPX <= pWin->brPX) && (pWin->tlPY <= pWin->brPY)))
    {
        _drvLcdHandler();
        return -1;
    }

    if (bSync)
    {
        if (!OSI_LOOP_WAIT_TIMEOUT_US(0 == halGoudaBlitRoi((HAL_GOUDA_WINDOW_T *)pWin, 0, cmd, NULL), __WAIT_GOUDA_READY_TIMEOUT)) {
            OSI_LOGE(0, "Gjh: err: timeout wait halGoudaBlitRoi");
            _reset_gouda();
            _drvLcdHandler();
            return -2;
        }

        if (!OSI_LOOP_WAIT_TIMEOUT_US(false == halGoudaIsActive(), __WAIT_GOUDA_READY_TIMEOUT)) {
            OSI_LOGE(0, "Gjh: err: timeout wait halGoudaIsActive");
            _reset_gouda();
            _drvLcdHandler();
            return -2;
        }
        _drvLcdHandler();
    }
    else
    {
        while (0 != halGoudaBlitRoi((HAL_GOUDA_WINDOW_T *)pWin, 0, cmd, _drvLcdHandler))
            ;
    }
    return 0;
#else
    if (!((pWin->tlPX < pWin->brPX) && (pWin->tlPY < pWin->brPY)))
    {
        _drvLcdHandler();
        return;
    }

    if (bSync)
    {
        while (0 != halGoudaBlitRoi((HAL_GOUDA_WINDOW_T *)pWin, 0, cmd, NULL))
            ;
        while (halGoudaIsActive())
            ;
        
        _drvLcdHandler();
    }
    else
    {
        while (0 != halGoudaBlitRoi((HAL_GOUDA_WINDOW_T *)pWin, 0, cmd, _drvLcdHandler))
            ;
    }
#endif
}

static void _drvLcdGoudaUpdataRam(const HAL_GOUDA_WINDOW_T *pWin, const uint32_t *distBuffer, uint16_t width)
{
    if (!((pWin->tlPX < pWin->brPX) && (pWin->tlPY < pWin->brPY)))
    {
        _drvLcdHandler();
        return;
    }
    while (0 != halGoudaBlitRoi2Ram((uint32_t *)distBuffer, width, pWin, NULL))
        ;

    while (halGoudaIsActive())
        ;
    _drvLcdHandler();
}

#define LCD_RESOLUTION_RGB888 0
#define LCD_RESOLUTION_RGB666 1
#define LCD_RESOLUTION_RGB565 2
#define LCD_RESOLUTION_RGB555 3
#define LCD_RESOLUTION_GREY 4
#define LCD_RESOLUTION_YUV422_UYVY 5
#define LCD_RESOLUTION_YUV420 6
#define LCD_RESOLUTION_YUV400 7
#define LCD_RESOLUTION_YUV420_3PLANE 8
#define LCD_RESOLUTION_YUV422_YUYV 9

static HAL_GOUDA_IMG_FORMAT_T _drvLcdLayerFormatConverter(uint8_t _resolution)
{
    switch (_resolution)
    {
    //case LCD_RESOLUTION_RGB888:
    //return HAL_GOUDA_IMG_FORMAT_RGBA;
    case LCD_RESOLUTION_YUV420:
        return HAL_GOUDA_IMG_FORMAT_IYUV;
    case LCD_RESOLUTION_YUV420_3PLANE:
        return HAL_GOUDA_IMG_FORMAT_IYUV;
    case LCD_RESOLUTION_YUV422_UYVY:
        return HAL_GOUDA_IMG_FORMAT_UYVY;
    case LCD_RESOLUTION_YUV422_YUYV:
        return HAL_GOUDA_IMG_FORMAT_YUYV;
    case LCD_RESOLUTION_RGB565:
    default:
        return HAL_GOUDA_IMG_FORMAT_RGB565;
    }
}

static void drvLcdConfigGoudaLayers()
{
    int32_t i = 0;
    uint8_t bpp = 1;
    HAL_GOUDA_OVL_LAYER_ID_T over_layer_use_id = HAL_GOUDA_OVL_LAYER_ID0;

    for (i = 0; i < LCDC_LAYER_NUM; i++)
    {
        if (g_block_info[i].is_enable)
        {
            //    OSI_LOGI(0xffffffff,"lcd:  :: configure gouda Layer [%d]   = (enable)",  i);

            if (i == 0)
            {
                HAL_GOUDA_VID_LAYER_DEF_T video_layer_cfg;

                video_layer_cfg.fmt = _drvLcdLayerFormatConverter(g_block_info[i].cfg.resolution);
                video_layer_cfg.addrY = (uint32_t *)g_block_info[i].mem_ptr;
                video_layer_cfg.addrU = (uint32_t *)g_block_info[i].uv_addr;

                if (g_block_info[i].cfg.resolution == LCD_RESOLUTION_YUV420_3PLANE)
                    video_layer_cfg.addrV = (uint32_t *)g_block_info[i].v_addr;
                else if ((g_block_info[i].cfg.resolution == LCD_RESOLUTION_YUV422_UYVY) || (g_block_info[i].cfg.resolution == LCD_RESOLUTION_YUV422_YUYV))
                    video_layer_cfg.addrV = video_layer_cfg.addrU + (g_vid_size.width * g_vid_size.height) / 8;
                else
                    video_layer_cfg.addrV = video_layer_cfg.addrU + (g_vid_size.width * g_vid_size.height) / 16;

                video_layer_cfg.alpha = 0xFF;
                video_layer_cfg.cKeyColor = 0;
                video_layer_cfg.cKeyEn = false;
                video_layer_cfg.cKeyMask = HAL_GOUDA_CKEY_MASK_OFF;
                video_layer_cfg.depth = HAL_GOUDA_VID_LAYER_BEHIND_ALL;
                //video_layer_cfg.depth = HAL_GOUDA_VID_LAYER_OVER_ALL;
                //video_layer_cfg.stride = g_block_info[i].cfg.width;

                bpp = (video_layer_cfg.fmt == HAL_GOUDA_IMG_FORMAT_IYUV) ? 1 : 2;
                // Note that stride could be different in case of padding
                video_layer_cfg.stride = g_vid_size.imageBufWidth * bpp;
                video_layer_cfg.pos.tlPX = g_block_info[i].cfg.start_x;
                video_layer_cfg.pos.tlPY = g_block_info[i].cfg.start_y;
                video_layer_cfg.pos.brPX = g_block_info[i].cfg.end_x;
                video_layer_cfg.pos.brPY = g_block_info[i].cfg.end_y;

                video_layer_cfg.width = g_vid_size.width;
                video_layer_cfg.height = g_vid_size.height;
                video_layer_cfg.rotation = g_block_info[i].cfg.rotation; //0: 0; 1: 90; clockwise

                halGoudaVidLayerClose();
                halGoudaVidLayerOpen(&video_layer_cfg);
                //  OSI_LOGI(0xffffffff,"lcd:  :: Open Video Layer [0] addr = (0x%x) -- config layer = %d",  video_layer_cfg.addrY, i);
            }
            else
            {
                HAL_GOUDA_OVL_LAYER_DEF_T over_layer_cfg;

                over_layer_cfg.addr = (uint32_t *)g_block_info[i].mem_ptr;
                over_layer_cfg.fmt = _drvLcdLayerFormatConverter(g_block_info[i].cfg.resolution);
                over_layer_cfg.stride = 0;
                over_layer_cfg.pos.tlPX = g_block_info[i].cfg.start_x;
                over_layer_cfg.pos.tlPY = g_block_info[i].cfg.start_y;
                over_layer_cfg.pos.brPX = g_block_info[i].cfg.end_x;
                over_layer_cfg.pos.brPY = g_block_info[i].cfg.end_y;
                over_layer_cfg.alpha = g_block_info[i].cfg.alpha;
                over_layer_cfg.cKeyColor = g_block_info[i].cfg.colorkey;
                over_layer_cfg.cKeyEn = g_block_info[i].cfg.colorkey_en;

                // open the layer
                halGoudaOvlLayerClose(i - 1);

                halGoudaOvlLayerOpen(i - 1, &over_layer_cfg);
                OSI_LOGI(0x10007f40, "lcd:Over Layer [%d] sx=%d", i - 1, over_layer_cfg.pos.tlPX);
                over_layer_use_id++;
            }
        }
        else
        {
            //   OSI_LOGI(0xffffffff,"lcd:  :: configure gouda Layer [%d]   = (disable)",  i);
        }
    }
}

/*****************************************************************************/
//Description:   entern/exit sleep mode
//Global resource dependence:
/*****************************************************************************/
void drvLcdEnterSleep(bool is_sleep)
{
    if (NULL != s_lcd_spec_info_ptr->operation)
    {
        if (g_lcd_cfg.sleep_state != (uint32_t)is_sleep)
        {

            g_lcd_cfg.sleep_state = (uint32_t)is_sleep;
            if (is_sleep == false)
            {
#ifdef CONFIG_FIBOCOM_BASE
                halPmuSwitchPower(HAL_POWER_LCD, true, true);
                osiThreadSleep(150);
                if (g_ucLcdRegister != LCD_REGISTERED)
                {
                #if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
                    halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
                #endif
                }
                else
                {
                    if (s_lcd_config_info->pwr_domain == LCD_POWER_1V8)
                    {
                        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);
                    }
                    else
                    {
                        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
                    }
                }
#else
                osiThreadSleep(150);
                halPmuSwitchPower(HAL_POWER_LCD, true, true);
#endif
                halGoudaInit(gd_lcd_config, LCDD_SPI_FREQ);

                if (NULL != s_lcd_spec_info_ptr->operation)
                {
                    // s_lcd_spec_info_ptr->operation->init();  // GuoJunhuang: Repreatedly calls function _stInit(), both here and in _stSleepIn().
                    #ifdef CONFIG_FIBOCOM_BASE
                    if (g_ucLcdRegister != LCD_REGISTERED)
                    {
                        g_lcddRotate=true;
                    }
                    else
                    {
                        g_lcddRotate=s_lcd_config_info->is_vertical;
                    }
                    #else
                    s_lcd_spec_info_ptr->operation->init();
                    #endif
                }

                OSI_LOGI(0x10007f41, "lcd gouda init before turn on backlight");
            }
            s_lcd_spec_info_ptr->operation->enterSleep(is_sleep);
            if (is_sleep == true)
            {
                halGoudaClose();
                osiThreadSleep(150);
                halGoudaClkDisable();
                halSetGoudaClockMode(false);
                halPmuSwitchPower(HAL_POWER_LCD, false, false);
                OSI_LOGI(0x10007f42, "lcd gouda init before turn off backlight");
            }
        }
    }
}

bool drvLcdSetDisplayWindow(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    if (NULL != s_lcd_spec_info_ptr->operation)
    {

        s_lcd_spec_info_ptr->operation->setDisplayWindow(left, top, right, bottom);
    }
    else
    {
        return false;
    }
    return true;
}

void drvLcdCheckESD(void)
{
    if (NULL != g_lcd_cfg.lcd_spec->operation->checkEsd)
    {

        g_lcd_cfg.lcd_spec->operation->checkEsd();
    }
}
/*****************************************************************************/
//Description: set display memory
//Global resource dependence:
/*****************************************************************************/
void drvLcdSetBlockBuffer(uint32_t blk_num, uint32_t *buf_ptr)
{
    OSI_ASSERT(blk_num < BLOCK_SUM, "drvLcdSetBlockBuffer number error"); /*assert verified*/
    OSI_ASSERT(buf_ptr != NULL, "drvLcdSetBlockBuffer null");             /*assert verified*/

    g_block_info[blk_num].mem_ptr = buf_ptr;
}

bool drvSetBrushMode(LCD_BRUSH_MODE_E mode)
{

    if (mode == g_lcd_cfg.brush_mode)
    {

        return true;
    }
    g_lcd_cfg.brush_mode = mode;

    return true;
}

bool drvSetUVBufferAddress(
    uint32_t blk_num,
    uint32_t *buf_ptr)
{
    OSI_ASSERT(blk_num == 0, "lcd:  drvSetUVBufferAddress blk");
    g_block_info[blk_num].uv_addr = buf_ptr;
    return true;
}

bool drvSetVBufferAddress(
    uint32_t blk_num,
    uint32_t *buf_ptr)
{
    OSI_ASSERT(blk_num == 0, "lcd:  drvSetVBufferAddress");
    g_block_info[blk_num].v_addr = buf_ptr;
    return true;
}

bool drvLcdSetImageLayerSize(uint32_t width, uint32_t height, uint32_t imageBufWidth)
{
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, " drvLcdSetImageLayerSize is not supported!!!");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
#endif
    g_vid_size.width = width;
    g_vid_size.height = height;
    g_vid_size.imageBufWidth = imageBufWidth;
    return true;
}

/*
src fmt :: 
    #define  LCD_RESOLUTION_RGB888          0
    #define  LCD_RESOLUTION_RGB666          1
    #define  LCD_RESOLUTION_RGB565          2
    #define  LCD_RESOLUTION_RGB555          3
    #define  LCD_RESOLUTION_GREY            4
    #define  LCD_RESOLUTION_YUV422          5
    #define  LCD_RESOLUTION_YUV420          6
    #define  LCD_RESOLUTION_YUV400          7
    #define  LCD_RESOLUTION_YUV420_3PLANE          8
dst format :
    must be LCD_RESOLUTION_RGB565

rotation :
    //0: 0; 1: 90; 2: 180; 3: 270;
note: mocor player_display_v1.c
*/
void drvLcdPictrueConverter(uint8_t src_fmt, uint32_t *src_addr, uint32_t src_w, uint32_t src_h, uint32_t stride,
                            uint32_t *dst_addr, uint32_t dst_w, uint32_t dst_h, uint8_t rotation)
{
    uint32_t *p_y = NULL, *p_uv = NULL;
    lcdBlockCfg_t vid_cfg = {0};

    OSI_LOGI(0x10007f43, "lcd:  david record drvLcdPictrueConverter fmt:%d, srcaddr:0x%x, w/h(%d/%d), dstaddr:0x%x, dstw/h(%d/%d), r=%d",
             src_fmt, src_addr, src_w, src_h, dst_addr, dst_w, dst_h, rotation);

    p_y = (uint32_t *)src_addr;

    if (rotation == 1) // 90
        p_y += (src_h - 1) * (src_w * 2) / 4;
    else if (rotation == 2) // 180
        p_y += (src_w * 2 - 16) / 4;
    else if (rotation == 3) // 270
        p_y += (src_h - 1) * (src_w * 2) / 4;
    else if (rotation != 0)
        return; // error

    if (src_fmt == LCD_RESOLUTION_YUV420)
        p_uv = (uint32_t *)(p_y + (src_w * src_h) / 4);

    halGoudaOvlLayerClose(HAL_GOUDA_OVL_LAYER_ID0);
    halGoudaOvlLayerClose(HAL_GOUDA_OVL_LAYER_ID1);
    halGoudaOvlLayerClose(HAL_GOUDA_OVL_LAYER_ID2);

    drvLcdSetBlockBuffer(0, p_y);
    drvSetUVBufferAddress(0, p_uv);

    vid_cfg.start_x = 0;
    vid_cfg.start_y = 0;
    vid_cfg.end_x = dst_w - 1;
    vid_cfg.end_y = dst_h - 1;
    vid_cfg.width = stride;
    vid_cfg.colorkey_en = 0;
    vid_cfg.alpha = 0xFF;
    vid_cfg.type = 1;
    vid_cfg.resolution = src_fmt;
    vid_cfg.rotation = rotation;

    drvLcdSetImageLayerSize(src_w, src_h, src_w);
    drvLcdConfigBlock(0, vid_cfg);

#if 0 // output to screen
    drvLcdGetLcdInfo(&lcd_info);
    drvLcdInvalidateRect(0, 0, lcd_info.lcd_width - 1, lcd_info.lcd_height - 1);
#else
    drvLcdInvalidataRectToRam(vid_cfg.start_x, vid_cfg.start_y, vid_cfg.end_x, vid_cfg.end_y, dst_addr);

#endif
}

void drvDisableBlock(uint32_t blk_num)
{
    OSI_ASSERT(blk_num < BLOCK_SUM, "drvLcdDisableBlock"); /*assert verified*/
    OSI_LOGI(0x10007f44, "[lcd:   EnableBlock  : %d)", blk_num);

    g_block_info[blk_num].is_enable = false;
}

void drvLcdEnableBlock(uint32_t blk_num)
{
    OSI_ASSERT(blk_num < BLOCK_SUM, "drvLcdEnableBlock"); /*assert verified*/
    OSI_LOGI(0x10007f45, "lcd:  EnableBlock  %d", blk_num);

    g_block_info[blk_num].is_enable = true;
}

uint32_t drvLcdGetFreeBlock(void)
{
    uint8_t i;

    for (i = 1; i < BLOCK_SUM; i++)
    {
        if (g_block_info[i].is_enable == false)
        {

            return i;
        }
    }

    OSI_ASSERT((i < BLOCK_SUM), ("drvLcdGetFreeBlock fail")); /*assert verified*/
    return 0;
}

void drvLcdDisableAllBlock(void)
{
    uint32_t i = 0;
    OSI_LOGI(0x10007f46, "lcd:   DisableAllBlock ");

    for (i = 0; i < LCDC_LAYER_NUM; i++)
    {
        g_block_info[i].is_enable = false;
    }
}

/*****************************************************************************/
//Description: configure block parameters
//Global resource dependence:

/*****************************************************************************/
bool drvLcdConfigBlock(uint32_t blk_num, lcdBlockCfg_t config)
{
    OSI_ASSERT((blk_num < BLOCK_SUM), ("blk_num=%d", blk_num)); /*assert verified*/
    if ((config.start_x >= config.end_x) || (config.start_y >= config.end_y))
    {

        OSI_LOGI(0x10007f47, "lcd: drvLcdConfigBlock error!!!");
        return false;
    }
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return false;
    }

    g_block_info[blk_num].cfg.alpha = config.alpha;
    g_block_info[blk_num].cfg.width = config.width;
    g_block_info[blk_num].cfg.type = config.type;
    g_block_info[blk_num].cfg.start_x = config.start_x;
    g_block_info[blk_num].cfg.start_y = config.start_y;
    g_block_info[blk_num].cfg.end_x = config.end_x;
    g_block_info[blk_num].cfg.end_y = config.end_y;
    g_block_info[blk_num].cfg.colorkey = config.colorkey;
    g_block_info[blk_num].cfg.resolution = config.resolution;
    g_block_info[blk_num].cfg.colorkey_en = config.colorkey_en;
    g_block_info[blk_num].cfg.rotation = config.rotation;

    OSI_LOGI(0x10007f48, "lcd:  :: config block (%d) -- [%d, %d, %d, %d] -%d ", blk_num,
             config.start_x, config.start_y, config.end_x, config.end_y, config.resolution);

    return true;
}

/*****************************************************************************/
//Description:     invalidate a rectang with the internal lcd data buffer
//Global resource dependence:
/*****************************************************************************/
#ifdef CONFIG_FIBOCOM_BASE
int32_t drvLcdInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
#else
void drvLcdInvalidateRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
#endif
{
    HAL_GOUDA_WINDOW_T winRect;
    winRect.tlPX = left;
    winRect.tlPY = top;
    winRect.brPX = right;
    winRect.brPY = bottom;

#ifdef CONFIG_FIBOCOM_BASE
    int32_t ret = -1;
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return -1;
    }
    drvLcdConfigGoudaLayers();
    ret = _drvLcdGoudaUpdate(&winRect, true);
    OSI_LOGI(0x10007f49, "lcd:  :: InvalidateRect [OUT]");
    return ret;
#else
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return;
    }
    drvLcdConfigGoudaLayers();
    _drvLcdGoudaUpdate(&winRect, true);
    OSI_LOGI(0x10007f49, "lcd:  :: InvalidateRect [OUT]");
#endif
}

/*****************************************************************************/
//Description:     converter lcd data to buffer
//Global resource dependence:
/*****************************************************************************/

void drvLcdInvalidataRectToRam(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t *buffer)
{
    HAL_GOUDA_WINDOW_T winRect;

    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return;
    }
    winRect.tlPX = left;
    winRect.tlPY = top;
    winRect.brPX = right;
    winRect.brPY = bottom;
    drvLcdConfigGoudaLayers();
    _drvLcdGoudaUpdataRam(&winRect, buffer, 0);

    OSI_LOGI(0x10007f49, "lcd:  :: InvalidateRect [OUT]");
}

#ifdef CONFIG_FIBOCOM_BASE    
int drvLcdBlockTransfer(const lcdFrameBuffer_t *dataBufferWin, const lcdDisplay_t *lcdRec)
#else
bool drvLcdBlockTransfer(const lcdFrameBuffer_t *dataBufferWin, const lcdDisplay_t *lcdRec)
#endif
{
    lcdBlockCfg_t cfg;
    uint16_t left = 0; //left of the rectangle
    uint16_t top = 0;  //top of the rectangle
    uint16_t right;    //right of the rectangle
    uint16_t bottom;   //bottom of the rectangle.
    uint16_t layer = 0;
    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return false;
    }
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif

#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, " drvLcdBlockTransfer is not supported!!!");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
#endif

    _drvGetSemaphore();
    if (dataBufferWin->colorFormat != LCD_RESOLUTION_RGB565)
    {
#ifdef CONFIG_FIBOCOM_BASE  
        if ((dataBufferWin->colorFormat != LCD_RESOLUTION_YUV422_UYVY) &&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV422_YUYV) &&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV420)&&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV420_3PLANE))
        {
            _lcdPutSemaphore();
            OSI_LOGI(0, "drvLcdBlockTransfer:[ERROR] Frame colorFormat is error.");
            return DRV_LCD_COLOR_FORMAT_ERROR;
        }
#endif
        layer = 0;

        if ((g_vid_size.width == 0) || (g_vid_size.height == 0))
        {
            drvLcdSetImageLayerSize(dataBufferWin->width, dataBufferWin->height, dataBufferWin->widthOriginal);
        }
    }
    else
    {
        layer = 3;
    }

#ifdef CONFIG_FIBOCOM_BASE
    if (NULL == dataBufferWin->buffer)
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvLcdBlockTransfer:[ERROR] Frame buffer is NULL.");
        return DRV_LCD_FRAME_BUFFER_ERROR;
    }
#endif
    cfg.colorkey = 0;
    cfg.resolution = dataBufferWin->colorFormat;
    cfg.type = 1;
    cfg.start_x = dataBufferWin->region_x;
    cfg.start_y = dataBufferWin->region_y;
    cfg.end_x = cfg.start_x + dataBufferWin->width - 1;
    cfg.end_y = cfg.start_y + dataBufferWin->height - 1;
    cfg.colorkey_en = dataBufferWin->keyMaskEnable;
    cfg.colorkey = dataBufferWin->maskColor;
    cfg.width = cfg.end_x - cfg.start_x + 1;
#ifdef CONFIG_FIBOCOM_BASE
    cfg.width = dataBufferWin->widthOriginal;
#else
    //cfg.width = s_lcd_spec_info_ptr->width;
#endif
    cfg.alpha = 0xff;

    cfg.rotation = dataBufferWin->rotation;
    OSI_LOGI(0x10007f4a, "lcd: drvLcdBlockTransfer startx %d", cfg.start_x);

    if (false == drvLcdConfigBlock(layer, cfg))
    {
        _lcdPutSemaphore();
#ifdef CONFIG_FIBOCOM_BASE 
        OSI_LOGI(0, "drvLcdBlockTransfer:[ERROR] drvLcdConfigBlock is failed.");
        return DRV_LCD_CONFIG_BLOCK_FAILED;
#else
        return false;
#endif
    }
    drvLcdSetBlockBuffer(layer, (void *)dataBufferWin->buffer);

    //gouda_event(0x9000c1e5);
    drvLcdEnableBlock(layer);

    left = lcdRec->x;
    top = lcdRec->y;
    right = left + lcdRec->width - 1;
    bottom = top + lcdRec->height - 1;

    if (false == drvLcdSetDisplayWindow(left, top, right, bottom))
    {
        _lcdPutSemaphore();
#ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "drvLcdBlockTransfer:[ERROR] drvLcdSetDisplayWindow is failed.");
        return DRV_LCD_SET_WINDOW_FAILED;
#else
        return false;
#endif
    }

#ifdef CONFIG_FIBOCOM_BASE
    int32_t ret = drvLcdInvalidateRect(left, top, right, bottom);
    // _lcdPutSemaphore();
    if (ret == (-2)) {
        OSI_PRINTFI("Gjh: %s re-do", __func__);
        drvLcdBlockTransfer(dataBufferWin, lcdRec);
    }
    return DRV_LCD_SUCCESS;
#else
    drvLcdInvalidateRect(0, 0, lcdRec->width - 1, lcdRec->height - 1);

    //right = left + dataBufferWin->width - 1;
    //bottom = top + dataBufferWin->height - 1;

    return true;
#endif
}

#ifdef CONFIG_FIBOCOM_BASE
int drvLcdBlockSetOverLay(const lcdFrameBuffer_t *dataBufferWin)
#else
bool drvLcdBlockSetOverLay(const lcdFrameBuffer_t *dataBufferWin)
#endif
{
    lcdBlockCfg_t cfg;
    uint16_t layer = 0;

    if (s_lcd_spec_info_ptr == NULL)
    {
        OSI_LOGI(0x10007f3d, "lcd: lcd hardware has error!");
        return false;
    }
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif

#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, " drvLcdBlockSetOverLay is not supported!!!");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
#endif

    _drvGetSemaphore();
    if (dataBufferWin->colorFormat != LCD_RESOLUTION_RGB565)
    {
#ifdef CONFIG_FIBOCOM_BASE
        if ((dataBufferWin->colorFormat != LCD_RESOLUTION_YUV422_UYVY) &&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV422_YUYV) &&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV420)&&
            (dataBufferWin->colorFormat != LCD_RESOLUTION_YUV420_3PLANE))
        {
            _lcdPutSemaphore();
            OSI_LOGI(0, "drvLcdBlockSetOverLay:[ERROR] Frame colorFormat is error.");
            return DRV_LCD_COLOR_FORMAT_ERROR;
        }
#endif
        layer = 0;
        if ((g_vid_size.width == 0) || (g_vid_size.height == 0))
        {
            drvLcdSetImageLayerSize(dataBufferWin->width, dataBufferWin->height, dataBufferWin->widthOriginal);
        }
    }
    else
    {
        layer = drvLcdGetFreeBlock();
    }
#ifdef CONFIG_FIBOCOM_BASE
    if (NULL == dataBufferWin->buffer)
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvLcdBlockSetOverLay:[ERROR] Frame buffer is NULL.");
        return DRV_LCD_FRAME_BUFFER_ERROR;
    }
#endif

    cfg.colorkey = 0;
    cfg.resolution = dataBufferWin->colorFormat;
    cfg.type = 1;
    cfg.start_x = dataBufferWin->region_x;
    cfg.start_y = dataBufferWin->region_y;
    cfg.end_x = cfg.start_x + dataBufferWin->width - 1;
    cfg.end_y = cfg.start_y + dataBufferWin->height - 1;
    cfg.colorkey_en = dataBufferWin->keyMaskEnable;
    cfg.colorkey = dataBufferWin->maskColor;

    cfg.width = cfg.end_x - cfg.start_x + 1;
    cfg.alpha = 0xff;

    cfg.rotation = dataBufferWin->rotation;

    if (false == drvLcdConfigBlock(layer, cfg))
    {
        _lcdPutSemaphore();
#ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "drvLcdBlockSetOverLay:[ERROR] drvLcdConfigBlock is failed.");
        return DRV_LCD_CONFIG_BLOCK_FAILED;
#else
        return false;
#endif
    }

    drvLcdSetBlockBuffer(layer, (void *)dataBufferWin->buffer);

    //gouda_event(0x9000c1e5);
    drvLcdEnableBlock(layer);
    _lcdPutSemaphore();
#ifdef CONFIG_FIBOCOM_BASE
    return DRV_LCD_SUCCESS;
#else
    return true;
#endif
}

#ifdef CONFIG_FIBOCOM_BASE
// ============================================================================
// lcddp_FillRect16
// ----------------------------------------------------------------------------
/// This function performs a fill of the active window  with some color.
/// @param bgColor Color with which to fill the rectangle. It is a 16-bit
/// data, as the one of #lcddp_SetPixel16
/// @return #LCDD_ERR_NO, #LCDD_ERR_RESOURCE_BUSY or #LCDD_ERR_NOT_OPENED.
// ============================================================================
int drvlcdFillRect16(const lcdDisplay_t* startPoint, uint16_t bgColor)
{
    // Active window coordinates.
    HAL_GOUDA_WINDOW_T activeWin;
    
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif
     
    _drvGetSemaphore();
    
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, " drvlcdFillRect16 is not supported!!!");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
#endif

    // Set Active window
    activeWin.tlPX = startPoint->x;
    activeWin.brPX = startPoint->x + startPoint->width - 1;
    activeWin.tlPY = startPoint->y;
    activeWin.brPY = startPoint->y + startPoint->height - 1;

    // Check parameters
    // ROI must be within the screen boundary
    bool badParam = false;
    if (g_lcddRotate)
    {   
        //Vertical screen
        if (    (activeWin.tlPX >= s_lcd_spec_info_ptr->width) ||
                (activeWin.brPX >= s_lcd_spec_info_ptr->width) ||
                (activeWin.tlPY >= s_lcd_spec_info_ptr->height) ||
                (activeWin.brPY >= s_lcd_spec_info_ptr->height)
           )
        {
            badParam = true;
        }
    }
    else
    {
        //Horizontal screen
        if (    (activeWin.tlPX >= s_lcd_spec_info_ptr->height) ||
                (activeWin.brPX >= s_lcd_spec_info_ptr->height) ||
                (activeWin.tlPY >= s_lcd_spec_info_ptr->width) ||
                (activeWin.brPY >= s_lcd_spec_info_ptr->width)
           )
        {
            badParam = true;
        }
    }
    if (badParam)
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvlcdFillRect16:[ERROR] Input param is error.");
        return DRV_LCD_INVALID_PARAMETER;
    }
    halGoudaSetBgColor(bgColor);
    if (false == drvLcdSetDisplayWindow(activeWin.tlPX, activeWin.tlPY, activeWin.brPX, activeWin.brPY))
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvlcdFillRect16:[ERROR] drvLcdSetDisplayWindow is failed.");
        return DRV_LCD_SET_WINDOW_FAILED;
    }
    int32_t ret = _drvLcdGoudaUpdate(&activeWin, true);
    if (ret == (-2)) {
        OSI_PRINTFI("Gjh: %s re-do", __func__);
        drvlcdFillRect16(startPoint, bgColor);
    }
    // _lcdPutSemaphore();
    return DRV_LCD_SUCCESS;
}



// ============================================================================
// lcddp_SetPixel16
// ----------------------------------------------------------------------------
/// Draw a 16-bit pixel a the specified position.
/// @param x X coordinate of the point to set.
/// @param y Y coordinate of the point to set.
/// @param pixelData 16-bit pixel data to draw.
/// @return #LCDD_ERR_NO, #LCDD_ERR_RESOURCE_BUSY or #LCDD_ERR_NOT_OPENED.
// ============================================================================
int drvlcdSetPixel16(uint16_t x, uint16_t y, uint16_t pixelData)
{
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        _drvGetSemaphore();
        int32_t ret =  s_lcd_spec_info_ptr->operation->dotSetPixel(x, y, pixelData);
        _lcdPutSemaphore();
        if (ret == -1) {
            return DRV_LCD_INVALID_PARAMETER;
        }
        return DRV_LCD_SUCCESS;
    }
#endif

    HAL_GOUDA_WINDOW_T activeWin;

    _drvGetSemaphore();
    // Set point
    activeWin.tlPX = x;
    activeWin.brPX = x;
    activeWin.tlPY = y;
    activeWin.brPY = y;

    // Check parameters
    // ROI must be within the screen boundary
    bool badParam = false;
    if (g_lcddRotate)
    {   
        //Vertical screen
        if (    (activeWin.tlPX >= s_lcd_spec_info_ptr->width) ||
                (activeWin.brPX >= s_lcd_spec_info_ptr->width) ||
                (activeWin.tlPY >= s_lcd_spec_info_ptr->height) ||
                (activeWin.brPY >= s_lcd_spec_info_ptr->height)
           )
        {
            badParam = true;
        }
    }
    else
    {
        //Horizontal screen
        if (    (activeWin.tlPX >= s_lcd_spec_info_ptr->height) ||
                (activeWin.brPX >= s_lcd_spec_info_ptr->height) ||
                (activeWin.tlPY >= s_lcd_spec_info_ptr->width) ||
                (activeWin.brPY >= s_lcd_spec_info_ptr->width)
           )
        {
            badParam = true;
        }
    }
    if (badParam)
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvlcdSetPixel16:[ERROR] Input param is error.");
        return DRV_LCD_INVALID_PARAMETER;
    }

    halGoudaSetBgColor(pixelData);
    if (false == drvLcdSetDisplayWindow(activeWin.tlPX, activeWin.tlPY, activeWin.brPX, activeWin.brPY))
    {
        _lcdPutSemaphore();
        OSI_LOGI(0, "drvlcdSetPixel16:[ERROR] drvLcdSetDisplayWindow is failed.");
        return DRV_LCD_SET_WINDOW_FAILED;
    }
    int32_t ret = _drvLcdGoudaUpdate(&activeWin, true);
    if (ret == (-2)) {
        OSI_PRINTFI("Gjh: %s re-do", __func__);
        drvlcdSetPixel16(x, y, pixelData);
    }
    // _lcdPutSemaphore();
    return DRV_LCD_SUCCESS;
}

bool drvLcdSetBrushDirection(lcdDirect_t direct_type)
{
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {
        OSI_LOGI(0, " drvLcdSetBrushDirection is not supported!!!");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
#endif
#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif

    if (direct_type > LCD_DIRECT_NORMAL2_ROT_270)
    {
        OSI_LOGI(0, "drvLcdSetBrushDirection:[ERROR] direct_type is error.");
        return false;
    }
    if (NULL != s_lcd_spec_info_ptr->operation)
    {
        _drvGetSemaphore();
        s_lcd_spec_info_ptr->operation->setBrushDirection(direct_type);
        if ((direct_type==LCD_DIRECT_NORMAL)||(direct_type==LCD_DIRECT_NORMAL2)||
            (direct_type==LCD_DIRECT_ROT_180)||(direct_type==LCD_DIRECT_NORMAL2_ROT_180))
        {
            g_lcddRotate=true;
        }
        else
        {
            g_lcddRotate=false;
        }
        _lcdPutSemaphore();
    }
    else
    {
        OSI_LOGI(0, "drvLcdSetBrushDirection:[ERROR] s_lcd_spec_info_ptr is NULL.");
        return false;
    }

    return true;
}

//ç»çº¿
//x1,y1:èµ·ç¹åæ 
//x2,y2:ç»ç¹åæ 
int TestlcdDrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{  
    int lret;
    uint16_t t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;

#ifdef CONFIG_FIBOCOM_BASE
    if (g_lcd_cfg.sleep_state == true)
    {
        OSI_LOGI(0, "lcd: lcd has sleep!");
        return DRV_LCD_NOT_INIT;
    }
#endif
    delta_x=x2-x1; //è®¡ç®åæ å¢é
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)
        incx=1; //è®¾ç½®åæ­¥æ¹å
    else if(delta_x==0)
        incx=0;//åç´ï¿?
    else
    {
        incx=-1;
        delta_x=-delta_x;
    }
    
    if(delta_y>0)
        incy=1;
    else if(delta_y==0)
        incy=0;//æ°´å¹³ï¿?
    else
    {
        incy=-1;
        delta_y=-delta_y;
    }
    
    if( delta_x>delta_y)
        distance=delta_x; //éååºæ¬å¢éåæ ï¿?
    else 
        distance=delta_y;

    for(t=0; t<=distance+1; t++ ) //ç»çº¿è¾åº
    {
        if ((lret= drvlcdSetPixel16(uRow,uCol,color)) != DRV_LCD_SUCCESS)//ç»ç¹
        {
            return lret;
        }
        xerr+=delta_x ;
        yerr+=delta_y ;

        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }

        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
    return DRV_LCD_SUCCESS;
}
#endif

// dot matrix LCD API
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)

void dotMatrixLcdSetDisplayParam(uint8_t contrastRatio_coarse, uint8_t contrastRatio_fine, uint8_t bias)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSetDisplayParam:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation) {
        s_lcd_spec_info_ptr->operation->dotSetDisplayParam(contrastRatio_coarse, contrastRatio_fine, bias);
    } else {
        OSI_LOGI(0, "dotMatrixLcdSetDisplayParam:[ERROR] func NULL.");
    }
}

void dotMatrixLcdSetLcmRate(uint8_t rate)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSetLcmRate:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation->dotSetLcmRate) {
        s_lcd_spec_info_ptr->operation->dotSetLcmRate(rate);
    } else {
        OSI_LOGI(0, "dotMatrixLcdSetLcmRate:[ERROR] func NULL.");
    }
}


void dotMatrixLcdSendData(uint8_t column, uint8_t row, void *buff, uint32_t size)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSendData:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation) {
        _drvGetSemaphore();
        s_lcd_spec_info_ptr->operation->dotSendBuff(column, row, buff, size);
        _lcdPutSemaphore();
    } else {
        OSI_LOGI(0, "dotMatrixLcdSendData:[ERROR] func NULL.");
    }
}

void dotMatrixLcdSetAddrOnly(uint8_t column, uint8_t row)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSetAddrOnly:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation) {
        if (NULL != s_lcd_spec_info_ptr->operation->dotSetAddrOnly) {
            _drvGetSemaphore();
            s_lcd_spec_info_ptr->operation->dotSetAddrOnly(column, row);
            _lcdPutSemaphore();
        }
    } else {
        OSI_LOGI(0, "dotMatrixLcdSetAddrOnly:[ERROR] func NULL.");
    }
}

void dotMatrixLcdSendBuffOnly(void *buff, uint32_t size)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSendBuffOnly:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation) {
        if (NULL != s_lcd_spec_info_ptr->operation->dotSendBuffOnly) {
            _drvGetSemaphore();
            s_lcd_spec_info_ptr->operation->dotSendBuffOnly(buff, size);
            _lcdPutSemaphore();
        }
    } else {
        OSI_LOGI(0, "dotMatrixLcdSendBuffOnly:[ERROR] func NULL.");
    }
}

void dotMatrixLcdSetFlushDirection(uint8_t direction)
{
    if (NULL == s_lcd_spec_info_ptr) {
        OSI_LOGI(0, "dotMatrixLcdSetFlushDirection:[ERROR] LCD has not initialized yet.");
        return;
    }
    if (NULL != s_lcd_spec_info_ptr->operation) {
        if (NULL != s_lcd_spec_info_ptr->operation->dotSetFlushDirection) {
            _drvGetSemaphore();
            s_lcd_spec_info_ptr->operation->dotSetFlushDirection(direction);
            _lcdPutSemaphore();
        }
    } else {
        OSI_LOGI(0, "dotMatrixLcdSetFlushDirection:[ERROR] func NULL.");
    }
}

#endif

#ifdef CONFIG_FIBOCOM_BASE
int drvLcdInit(void)
#else
bool drvLcdInit(void)
#endif
{
    OSI_LOGI(0x10007f4b, "lcd:  drvLcdInit");
    uint32_t clk;
    if (lcdLock == NULL)
    {
        lcdLock = osiSemaphoreCreate(1, 1);
    }
    else
    {
        OSI_LOGI(0x10007f4c, "lcd:  drvLcd already Init");
        return true;
    }
#ifdef CONFIG_FIBOCOM_BASE
    if (lcdLock == NULL)
    {
        OSI_LOGI(0, "drvLcdInit:[ERROR] osiSemaphoreCreate failed.");
        return DRV_LCD_SEM_CREATE_FAILED;
    }
#endif

#ifdef CONFIG_FIBOCOM_BASE
    if (g_ucLcdRegister != LCD_REGISTERED)
    {
#if defined(CONFIG_FIBOCOM_OPENCPU) && defined(CONFIG_FIBOCOM_LIANDI)
        if (LOGO_FILE_EXIST != glogo_stat)
        {
            //add lcd power on reset for liandi
            hwp_iomux->pad_keyout_5_cfg_reg = 0x11100000;
            osiDelayUS(10000);
            hwp_iomux->pad_keyout_5_cfg_reg = 0x11000000;
            osiDelayUS(150000);
        }
#endif
        s_lcd_spec_info_ptr = drvLcdGetSpecInfo();
    }
    else
    {
        if (NULL != s_lcd_config_info)
        {
            OSI_LOGI(0, "drvLcdInit:[DEBUG] get lcd power domain:%d vertical:%d",
                s_lcd_config_info->pwr_domain, s_lcd_config_info->is_vertical);
        }
        else
        {
            osiSemaphoreDelete(lcdLock);
            lcdLock=NULL;
            OSI_LOGI(0, "drvLcdInit:[ERROR] get s_lcd_config_info failed.");
            return DRV_LCD_GET_SPEC_FAILED;
        }
    }
#else
    s_lcd_spec_info_ptr = drvLcdGetSpecInfo();
#endif
    if (NULL != s_lcd_spec_info_ptr)
    {
        OSI_ASSERT(NULL != s_lcd_spec_info_ptr->operation, "lcd init null"); /*assert verified*/
        #ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "drvLcdInit:[DEBUG] get lcd width:%d height:%d", s_lcd_spec_info_ptr->width, s_lcd_spec_info_ptr->height);
        OSI_LOGI(0, "drvLcdInit:[DEBUG] get lcd freq:%d", s_lcd_spec_info_ptr->freq);
        #endif
    }
    else
    {
#ifdef CONFIG_FIBOCOM_BASE
        osiSemaphoreDelete(lcdLock);
        lcdLock=NULL;
        OSI_LOGI(0, "drvLcdInit:[ERROR] drvLcdGetSpecInfo failed.");
        return DRV_LCD_GET_SPEC_FAILED;
#else
        return false;
#endif
    }
    OSI_LOGI(0x10007f4d, "lcd:  get  w %d  h %d", s_lcd_spec_info_ptr->width, s_lcd_spec_info_ptr->height);
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
    if (_is_dotMatLcd()) {

        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);

        if (s_lcd_spec_info_ptr->lcdbuffsize != 0) {
            OSI_LOGI(0, " drvLcdInit: malloc buff for dot matrix lcd");
            if (s_lcd_spec_info_ptr->lcdbuff != NULL) {
                free(s_lcd_spec_info_ptr->lcdbuff);
                s_lcd_spec_info_ptr->lcdbuff = NULL;
            }
            OSI_LOGI(0, "dotMatrixLcdSendData:free lcd buffer.");
            s_lcd_spec_info_ptr->lcdbuff = malloc(s_lcd_spec_info_ptr->lcdbuffsize);
            if (s_lcd_spec_info_ptr->lcdbuff == NULL) {
                osiSemaphoreDelete(lcdLock);
                lcdLock=NULL;
                OSI_LOGE(0, "drvLcdInit:[ERROR] failed to malloc buff for dot matrix lcd");
                return DRV_LCD_GET_SPEC_FAILED;
            }
            memset(s_lcd_spec_info_ptr->lcdbuff, 0, s_lcd_spec_info_ptr->lcdbuffsize);
        }
    }
#endif

    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    //halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
#if !(defined(CONFIG_FIBOCOM_BASE) && defined(CONFIG_SOC_8850))
    // Un-supported PMU in 8850
    halPmuSwitchPower(HAL_POWER_VIBR, true, true);
    //halPmuSetPowerLevel(HAL_POWER_VIBR, POWER_LEVEL_2800MV);
#endif
#ifndef CONFIG_FIBOCOM_BASE
    // Backlight should not be turn on in lcd_init, it is always turned on after one frame of picture has transfered.
    halPmuSwitchPower(HAL_POWER_BACK_LIGHT, true, true);
#endif

#ifdef CONFIG_FIBOCOM_BASE
    if (g_ucLcdRegister != LCD_REGISTERED)
    {
#if defined(CONFIG_AITI_CUS_LCD_SUPPORT)
        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
    #endif
    #ifdef CONFIG_FIBOCOM_OPENCPU
    //#ifdef CONFIG_FIBOCOM_LG610
    #ifdef CONFIG_FIBOCOM_LIANDI
        extern int32_t gbrightness_lev;
        extern int32_t fibo_sink_OnOff(uint32_t sinknum, bool onOff);
        extern int32_t fibo_sinkLevel_Set(uint32_t sinknum, uint32_t light_level);

        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
        halPmuSwitchPower(HAL_POWER_VIBR, true, true);
        halPmuSetPowerLevel(HAL_POWER_VIBR, POWER_LEVEL_2800MV);
        halPmuSwitchPower(HAL_POWER_BACK_LIGHT, true, true);

        if (0 != gbrightness_lev)
        {
            //OSI_PRINTFI("[%s %d] set brightness_lev %d", __FUNCTION__, __LINE__, gbrightness_lev);
            fibo_sink_OnOff(0, true);
            fibo_sinkLevel_Set(0, gbrightness_lev);
        }
    #endif
    //#endif
    #endif
    }
#endif
    _lcdDelayMs(10);

    g_lcd_cfg.brush_mode = LCD_BRUSH_MODE_SYNC;

    g_lcd_cfg.lcd_spec = s_lcd_spec_info_ptr;

    g_lcd_cfg.sleep_state = false;

    if (s_lcd_spec_info_ptr->freq == 0)
    {
        clk = 20000000;
    }
    else
    {
        clk = s_lcd_spec_info_ptr->freq;
    } 
    halGoudaInit(gd_lcd_config, clk);

    drvLcdDisableAllBlock();

    if (NULL != s_lcd_spec_info_ptr->operation)
    {

        s_lcd_spec_info_ptr->operation->init();
        #ifdef CONFIG_FIBOCOM_BASE
        if (g_ucLcdRegister != LCD_REGISTERED)
        {
            g_lcddRotate=true;
        }
        else
        {
            g_lcddRotate=s_lcd_config_info->is_vertical;
        }
        s_lcd_used_num=1;
        OSI_LOGI(0, "drvLcdInit:[DEBUG] g_lcddRotate:%d", g_lcddRotate);
        #endif
    }
#ifdef CONFIG_FIBOCOM_BASE
    else
    {
#if defined(CONFIG_DOT_MATRIX_LCD_SUPPORT)
        if (_is_dotMatLcd()) {

            if (s_lcd_spec_info_ptr->lcdbuffsize != 0) {
                free(s_lcd_spec_info_ptr->lcdbuff);
                s_lcd_spec_info_ptr->lcdbuff = NULL;
            }
        }
#endif
        osiSemaphoreDelete(lcdLock);
        lcdLock=NULL;
        OSI_LOGI(0, "drvLcdInit:[ERROR] lcd spec operation is NULL.");
        return DRV_LCD_GET_OPERATION_FAILED;
    }
    OSI_LOGI(0x10007f4e, "lcd:  drvLcdInit end");
    return DRV_LCD_SUCCESS;
#else
    OSI_LOGI(0x10007f4e, "lcd:  drvLcdInit end");
    return true;
#endif
}

int convert_yuyv_to_rgb(unsigned char *inBuf, unsigned char *outBuf, int imgWidth, int imgHeight, int inBufFormat)
{
    int rows, cols;
    int y, u, v, r, g, b;
    unsigned char *YUVdata, *RGBdata;
    int Ypos = 0, Upos = 0, Vpos = 0; //in the inBuf's postion
    unsigned int i = 0;

    YUVdata = inBuf;
    RGBdata = outBuf;

    switch (inBufFormat)
    {
    case YUYV:
        /*  YUYV */
        Ypos = 0;
        Upos = Ypos + 1;
        Vpos = Upos + 2;
        break;
    case YVYU:
        /*  YVYU */
        Ypos = 0;
        Vpos = Ypos + 1;
        Upos = Vpos + 2;
        break;
    case UYVY:
        /*  UYVY */
        Ypos = 1;
        Upos = Ypos - 1;
        Vpos = Upos + 1;
        break;
    default:
        break;
    }
    for (rows = 0; rows < imgHeight; rows++)
    {
        for (cols = 0; cols < imgWidth; cols++)
        {
            y = YUVdata[Ypos];
            u = YUVdata[Upos] - 128;
            v = YUVdata[Vpos] - 128;

            r = y + v + ((v * 103) >> 8);
            g = y - ((u * 88) >> 8) - ((v * 183) >> 8);
            b = y + u + ((u * 198) >> 8);

            r = r > 255 ? 255 : (r < 0 ? 0 : r);
            g = g > 255 ? 255 : (g < 0 ? 0 : g);
            b = b > 255 ? 255 : (b < 0 ? 0 : b);

            *(RGBdata++) = (((g & 0x1c) << 3) | (b >> 3));
            *(RGBdata++) = ((r & 0xf8) | (g >> 5));

            Ypos += 2;
            i++;
            if (!(i & 0x01))
            {
                Upos = Ypos - 1;
                Vpos = Ypos + 1;
            }
        }
    }
    return 0;
}



#ifdef CONFIG_FIBOCOM_BASE 
void drvLcdGoudaInit(HAL_GOUDA_SPI_LINE_T spiLineType, uint32_t spiclk)
{
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
    halGoudaLineTypeConfig(spiLineType);
    halGoudaInit(gd_lcd_config, spiclk);
}
int32_t drvLcdGoudaWriteCmd(uint16_t cmd)
{    
    int32_t lRet=0;
    lRet=halGoudaWriteCmd(cmd);  
    return lRet;
}
int32_t drvLcdGoudaWriteData(uint16_t data)
{    
   int32_t lRet=0;
    lRet=halGoudaWriteData(data);  
    return lRet;
}
int32_t drvLcdGoudaReadData(uint16_t addr, uint8_t *pData, uint32_t len)
{    
    int32_t lRet=0;
    lRet=halGoudaReadData(addr, pData, len);  
    return lRet;
}
void drvLcdGoudaSetSpiFreq(uint32_t spiclk)
{
    halGoudaSetSpiFreq(spiclk);
}
void drvLcdGoudaSerialMode(HAL_GOUDA_SPI_LINE_T spiLineType)
{
    halGoudaSerialMode(spiLineType);
}
int drvLcdRegister(lcdCfg_t *lcd, void *data)
{
    uint32_t lcm_dev_id = 0;
    
    if ((NULL == lcd)||(NULL == lcd->lcd_spec)||(NULL == lcd->lcd_spec->operation))
    {
        return DRV_LCD_INVALID_PARAMETER;
    }
    
    if (lcd->lcd_spec->operation->readId)
    {
        halPmuSwitchPower(HAL_POWER_LCD, true, true);
        if (lcd->pwr_domain == LCD_POWER_1V8)
        {
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);
        }
        else
        {
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
        }
#if defined(CONFIG_FIBOCOM_OPENCPU) && defined(CONFIG_FIBOCOM_LIANDI)
        if (LOGO_FILE_EXIST != glogo_stat)
        {
            //add lcd power on reset for liandi
            hwp_iomux->pad_keyout_5_cfg_reg = 0x11100000;
            osiDelayUS(10000);
            hwp_iomux->pad_keyout_5_cfg_reg = 0x11000000;
            osiDelayUS(150000);
        }
#endif
        _lcdDelayMs(10);
        halGoudaReadConfig(lcd->lcd_spec->bus_mode, lcd->lcd_spec->is_use_fmark, lcd->lcd_spec->fmark_delay);
        _lcdDelayMs(150);
        lcm_dev_id = lcd->lcd_spec->operation->readId();
        OSI_LOGI(0, "lcd:  drvLcdRegister id=0x%x \n", lcm_dev_id);
  
        if (lcm_dev_id == lcd->lcd_spec->dev_id)
        {
             s_lcd_config_info = lcd;
             s_lcd_spec_info_ptr = lcd->lcd_spec;
             g_ucLcdRegister = LCD_REGISTERED;
             OSI_LOGI(0, "lcd:  drvLcdRegister success\n");
             return DRV_LCD_SUCCESS;
        }
    }

    g_ucLcdRegister = LCD_NULL;
    OSI_LOGI(0, "lcd:  drvLcdRegister failed\n");    
    return DRV_LCD_INVALID_PARAMETER;
}
#endif

void test_colorbar_buffer(uint16_t *p, int w, int h)
{
    int i,j;
    for(i=0; i<h; i++)
    {
        for(j=0; j<w; j++)
        {
            if(j<w/4)
            {
                p[i*w+j]=0x001f;
            }
            else if (j<w/2)
            {
                p[i*w+j]=0xf000;
            }
            else if (j<w*3/4)
            {
                p[i*w+j]=0x07f0;
            }
            else
            {
                p[i*w+j]=0xf80f;
            }
        }
    }

}


void fill_word_buff(uint16_t *p, int w, int h)
{
    uint16_t i,j;
    for(i=0;i<h;i++)
    {
        for(j=0;j<w;j++)
        {
            p[i*w+j]=0xf800;
        }
    }

    for(i=10;i<120;i++)
    {
        for(j=80;j<90;j++)
        {
            p[i*w+j]=0x001f;
        }
    }
    for(i=10;i<120;i++)
    {
        for(j=180;j<190;j++)
        {
            p[i*w+j]=0x001f;
        }
    }
    for(i=60;i<70;i++)
    {
        for(j=90;j<180;j++)
        {
            p[i*w+j]=0x001f;
        }
    }
}


void lcd_test(void)
{
    lcdDisplay_t startPoint;
    lcdFrameBuffer_t dataBufferWin;
    lcdFrameBuffer_t dataWin;

    if(drvLcdInit() != DRV_LCD_SUCCESS)
    {
        return;
    }

    startPoint.x=0;
    startPoint.y=0;
    startPoint.width=240;
    startPoint.height=320;
    drvlcdFillRect16(&startPoint, BLACK);
    osiThreadSleep(2000);
    
    startPoint.x=50;
    startPoint.y=100;
    startPoint.width=100;
    startPoint.height=50;
    drvlcdFillRect16(&startPoint, GRAY);
    osiThreadSleep(2000);
    
    drvlcdSetPixel16(10, 10, RED);
    drvlcdSetPixel16(10, 20, GREEN);
    TestlcdDrawLine(1, 170, 200, 310,WHITE);
    osiThreadSleep(1000);
    TestlcdDrawLine(200, 170, 1, 310,WHITE);
    osiThreadSleep(1000);
    TestlcdDrawLine(200, 240, 1, 240,WHITE);
    osiThreadSleep(1000);
    TestlcdDrawLine(100, 310, 100, 170,WHITE);
    
    osiThreadSleep(2000);
    uint16_t *g_TestBuffer = (uint16_t *)malloc(240 * 320 * 2);
    if (g_TestBuffer == NULL)
    return;

    uint16_t *g_testBuff_128_160 = (uint16_t *)malloc(128 * 160 * 2);
    if (g_testBuff_128_160 == NULL)
    {
        free(g_TestBuffer);
        return;
    }


    fill_word_buff(g_TestBuffer,320,240);
    dataBufferWin.buffer=(uint16_t *)g_TestBuffer;
    dataBufferWin.colorFormat=LCD_RESOLUTION_RGB565;
    dataBufferWin.region_x=0;
    dataBufferWin.region_y=0;
    dataBufferWin.height=240;
    dataBufferWin.width=320;
    dataBufferWin.keyMaskEnable=false;
    dataBufferWin.maskColor=0;
    dataBufferWin.rotation=0;
    drvLcdSetBrushDirection(LCD_DIRECT_ROT_90);
    OSI_LOGI(0, "lcd:  drvLcdSetBrushDirection");
    drvLcdBlockSetOverLay(&dataBufferWin);

    test_colorbar_buffer(g_testBuff_128_160,160,128);
    dataWin.buffer=(uint16_t *)g_testBuff_128_160;
    dataWin.colorFormat=LCD_RESOLUTION_RGB565;
    dataWin.region_x=159;
    dataWin.region_y=110;
    dataWin.height=128;
    dataWin.width=160;
    dataWin.keyMaskEnable=false;
    dataWin.maskColor=0;
    dataWin.rotation=0;
    
    startPoint.x=0;
    startPoint.y=0;
    startPoint.height=240;
    startPoint.width=320;
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(2000);

    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL);
    startPoint.x=0;
    startPoint.y=0;
    startPoint.width=240;
    startPoint.height=320;
    drvlcdFillRect16(&startPoint, WHITE);

    drvLcdSetBrushDirection(LCD_DIRECT_ROT_90);
    
    dataWin.buffer=(uint16_t *)g_testBuff_128_160;
    dataWin.colorFormat=LCD_RESOLUTION_RGB565;
    dataWin.region_x=30;
    dataWin.region_y=30;
    dataWin.height=128;
    dataWin.width=160;
    dataWin.widthOriginal=160;
    dataWin.keyMaskEnable=false;
    dataWin.maskColor=0;
    dataWin.rotation=0;
    
    startPoint.x=30;
    startPoint.y=30;
    startPoint.height=128;
    startPoint.width=160;
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(3000);

    //fill_word_buff(g_TestBuffer,320,240);
    dataBufferWin.buffer=(uint16_t *)g_TestBuffer;
    dataBufferWin.colorFormat=LCD_RESOLUTION_RGB565;
    dataBufferWin.region_x=0;
    dataBufferWin.region_y=0;
    dataBufferWin.height=240;
    dataBufferWin.width=320;
    dataBufferWin.keyMaskEnable=true;
    dataBufferWin.maskColor=0xf800;
    dataBufferWin.rotation=0;
    OSI_LOGI(0, "lcd:  drvLcdSetBrushDirection");
    //drvLcdBlockSetOverLay(&dataBufferWin);

    //fill_word_buff(g_TestBuffer,320,240);
    //dataWin.buffer=(uint16_t *)g_TestBuffer;
    //dataWin.colorFormat=LCD_RESOLUTION_RGB565;
    //dataWin.region_x=0;
    //dataWin.region_y=0;
    //dataWin.height=240;
    //dataWin.width=320;
    //dataWin.keyMaskEnable=true;
    //dataWin.maskColor=0xf800;
    //dataWin.rotation=0;
    
    startPoint.x=0;
    startPoint.y=0;
    startPoint.height=240;
    startPoint.width=320;
    drvLcdBlockTransfer(&dataBufferWin, &startPoint);
    osiThreadSleep(2000);
    drvLcdEnterSleep(true);
    osiThreadSleep(4000);
    drvLcdEnterSleep(false);
    startPoint.x=0;
    startPoint.y=0;
    startPoint.width=240;
    startPoint.height=320;
    drvlcdFillRect16(&startPoint, BLACK);
    osiThreadSleep(2000);
    drvlcdSetPixel16(10, 10, RED);
    drvlcdSetPixel16(10, 20, GREEN);
    osiThreadSleep(4000);

    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL);
    
    dataWin.buffer=(uint16_t *)g_testBuff_128_160;
    dataWin.colorFormat=LCD_RESOLUTION_RGB565;
    dataWin.region_x=0;
    dataWin.region_y=0;
    dataWin.height=128;
    dataWin.width=160;
    dataWin.widthOriginal=160;
    dataWin.keyMaskEnable=false;
    dataWin.maskColor=0;
    dataWin.rotation=0;
    
    startPoint.x=0;
    startPoint.y=0;
    startPoint.height=128;
    startPoint.width=160;

    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);
    
    drvLcdSetBrushDirection(LCD_DIRECT_ROT_90);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);
    drvLcdSetBrushDirection(LCD_DIRECT_ROT_180);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);
    drvLcdSetBrushDirection(LCD_DIRECT_ROT_270);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);
    

    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL2);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);

    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL2_ROT_90);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);

    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL2_ROT_180);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);
    drvLcdSetBrushDirection(LCD_DIRECT_NORMAL2_ROT_270);
    drvLcdBlockTransfer(&dataWin, &startPoint);
    osiThreadSleep(4000);
    drvlcdFillRect16(&startPoint, BLACK);

    
    free(g_TestBuffer);
    free(g_testBuff_128_160);
    drvLcdClose();
}


/* end of lcd.c*/
