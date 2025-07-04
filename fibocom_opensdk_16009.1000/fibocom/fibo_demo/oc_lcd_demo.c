/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

UINT16 g_testBuff_128_160[10 * 40];
UINT16 g_TestBuffer[320 * 240];

// color code
#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430

void test_printf(void)
{
    for (int n=0; n<10; n++)
    {
        fibo_textTrace("demo.c test %d", n);
        fibo_taskSleep(1000);
    }
}

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i=0; i<count; ++i)
        __init_array_start[i]();
}

void test_colorbar_buffer(UINT16 *p, int w, int h)
{
    int i, j;
    for (i=0; i<h; i++)
    {
        for (j=0; j< w; j++)
        {
            if (j<w/4)
            {
                p[i*w+j] = 0x001f;
            }
            else if (j<w/2)
            {
                p[i*w+j] = 0xf000;
            }
            else if (j< w*3/4)
            {
                p[i*w+j] = 0x07f0;
            }
            else
            {
                p[i*w+j] = 0xf80f;
            }
        }
    }

}
void fill_word_buff(UINT16 *p, int w, int h)
{
    UINT16 i, j;
    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j++)
        {
            p[i*w+j] = 0xf800;
        }
    }

    for (i=10; i<120; i++)
    {
        for (j=80; j<90; j++)
        {
            p[i * w + j] = 0x001f;
        }
    }
    for (i=10; i<120; i++)
    {
        for (j=180; j<190; j++)
        {
            p[i*w+j] = 0x001f;
        }
    }
    for (i=60; i<70; i++)
    {
        for (j=90; j<180; j++)
        {
            p[i*w+j] = 0x001f;
        }
    }
}


static void lcd_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    // srand(100);
    int ret = 0;
    lcdDisplay_t startPoint;
    lcdFrameBuffer_t dataBufferWin;
    lcdFrameBuffer_t data;

    for (int n=0; n<10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(2000);
        fibo_taskSleep(10 * 1000);
        ret=fibo_lcd_init();
        if (ret != 0)
        {
            fibo_textTrace("fibo_lcd_init failed:%d.", ret);
        }
        else
        {
            fibo_textTrace("fibo_lcd_init success.");
        }
        startPoint.x = 0;
        startPoint.y = 0;
        startPoint.width = 240;
        startPoint.height = 0;
        fibo_lcd_FillRect(&startPoint, BLACK);
        fibo_taskSleep(2 * 1000);
        startPoint.x = 50;
        startPoint.y = 100;
        startPoint.width = 100;
        startPoint.height = 50;
        fibo_lcd_FillRect(&startPoint, GRAY);
        fibo_taskSleep(2 * 1000);

        fibo_lcd_SetPixel(10, 10, RED);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_SetPixel(10, 20, GREEN);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_DrawLine(1, 170, 200, 310, WHITE);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_DrawLine(200, 170, 1, 310, WHITE);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_DrawLine(200, 240, 1, 240, WHITE);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_DrawLine(100, 310, 100, 170, WHITE);
        fibo_taskSleep(2 * 1000);
        startPoint.x = 0;
        startPoint.y = 0;
        startPoint.width = 240;
        startPoint.height = 320;
        fibo_lcd_FillRect(&startPoint, BLACK);
        fibo_taskSleep(2 * 1000);

        #if 1
        test_colorbar_buffer(g_TestBuffer, 320, 240);
        dataBufferWin.buffer = (uint16_t *)g_TestBuffer;
        dataBufferWin.colorFormat = LCD_RESOLUTION_RGB565;
        dataBufferWin.region_x = 0;
        dataBufferWin.region_y = 0;
        dataBufferWin.height = 240;
        dataBufferWin.width = 320;
        dataBufferWin.keyMaskEnable = false;
        dataBufferWin.maskColor = 0;
        dataBufferWin.rotation = 0;
        fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL2_ROT_90);
        fibo_lcd_SetOverLay(&dataBufferWin);
        fibo_taskSleep(2000);
        #endif
        #if 1
        fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL2_ROT_90);
        test_colorbar_buffer(g_testBuff_128_160, 40, 10);
        data.buffer = (uint16_t *)g_testBuff_128_160;
        data.colorFormat = LCD_RESOLUTION_RGB565;
        data.region_x = 0;
        data.region_y = 0;
        data.height = 10;
        data.width = 40;
        data.widthOriginal = 40;
        data.keyMaskEnable = false;
        data.maskColor = 0;
        data.rotation = 0;

        startPoint.x = 0;
        startPoint.y = 0;
        startPoint.height = 240;
        startPoint.width = 320;
        fibo_lcd_FrameTransfer(&data, &startPoint);
        fibo_taskSleep(2000);
        data.buffer = (uint16_t *)g_testBuff_128_160;
        data.colorFormat = LCD_RESOLUTION_RGB565;
        data.region_x = 60;
        data.region_y = 60;
        data.height = 10;
        data.width = 40;
        data.widthOriginal = 40;
        data.keyMaskEnable = false;
        data.maskColor = 0;
        data.rotation = 0;

        startPoint.x = 60;
        startPoint.y = 60;
        startPoint.height = 10;
        startPoint.width = 40;
        fibo_lcd_FrameTransfer(&data, &startPoint);
        fibo_taskSleep(2000);
        fibo_lcd_Sleep(true);
        fibo_taskSleep(2000);
        fibo_taskSleep(2000);
        fibo_taskSleep(2000);
        fibo_lcd_Sleep(false);
        fibo_taskSleep(2000);
        startPoint.x = 0;
        startPoint.y = 0;
        startPoint.width = 320;
        startPoint.height = 240;
        fibo_lcd_FillRect(&startPoint, BLACK);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_SetPixel(10, 10, RED);
        fibo_taskSleep(2 * 1000);
        fibo_lcd_SetPixel(10, 20, GREEN);
        fibo_taskSleep(2 * 1000);
        #endif
#if 0
        dataBufferWin.buffer = (uint16_t *)gImage_Imag;
        dataBufferWin.colorFormat = LCD_RESOLUTION_RGB565;
        dataBufferWin.region_x = 0;
        dataBufferWin.region_y = 0;
        dataBufferWin.height = 240;
        dataBufferWin.width = 320;
        dataBufferWin.keyMaskEnable = false;
        dataBufferWin.maskColor = 0;
        dataBufferWin.rotation = 0;
        fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL2_ROT_90);
        fibo_lcd_SetOverLay(&dataBufferWin);

        fill_word_buff(g_TestBuffer, 320, 240);
        data.buffer = (uint16_t *)g_TestBuffer;
        data.colorFormat = LCD_RESOLUTION_RGB565;
        data.region_x = 0;
        data.region_y = 0;
        data.height = 240;
        data.width = 320;
        data.keyMaskEnable = true;
        data.maskColor = 0xf800;
        data.rotation = 0;

        startPoint.x = 0;
        startPoint.y = 0;
        startPoint.height = 240;
        startPoint.width = 320;
        fibo_lcd_FrameTransfer(&data, &startPoint);
        fibo_taskSleep(5000);
#endif
        fibo_lcd_deinit();
    }
    test_printf();
    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(lcd_thread_entry, "lcd_custhread", 1024 * 8, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
