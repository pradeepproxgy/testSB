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

typedef struct
{
    CAM_DEV_T camdev;
    lcdSpec_t lcddev;
} camprevExample_t;

static camprevExample_t camPrev;

extern void test_printf(void);
static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void prvBlackPrint(void)
{
    fibo_textTrace("cam LCD display black");

    lcdDisplay_t lcdRec;

    lcdRec.x = 0;
    lcdRec.y = 0;
    lcdRec.width = camPrev.lcddev.width;
    lcdRec.height = camPrev.lcddev.height;
    fibo_lcd_FillRect(&lcdRec, BLACK);
}

static void prvCameraPrint(void *buff)
{
    lcdFrameBuffer_t dataBufferWin;
    lcdDisplay_t lcdRec;
    uint32_t offset;
    // if the image is yuv422 , offset = yaddr+(height-1)*width*2
    offset = camPrev.camdev.img_width * (camPrev.camdev.img_height - 1) * 2;
    dataBufferWin.buffer = (uint16_t *)((uint32_t)buff + offset);
    dataBufferWin.colorFormat = LCD_RESOLUTION_YUV422_YUYV;
    dataBufferWin.keyMaskEnable = false;

    dataBufferWin.region_x = 0;
    dataBufferWin.region_y = 0;
    dataBufferWin.height = camPrev.lcddev.height;
    dataBufferWin.width = camPrev.lcddev.width;
    dataBufferWin.widthOriginal = camPrev.camdev.img_width;
    dataBufferWin.rotation = 1;
    lcdRec.x = 0;
    lcdRec.y = 0;
    lcdRec.width = camPrev.lcddev.width;
    lcdRec.height = camPrev.lcddev.height;  // because the image is 248*320, if rotation 90 degree, just can show the 240*240,

    fibo_lcd_Setvideosize(camPrev.camdev.img_height, camPrev.camdev.img_width, camPrev.camdev.img_width);
    fibo_lcd_FrameTransfer(&dataBufferWin, &lcdRec);
}

static void prvLcdInit(void)
{
    uint32_t uldevid;
    fibo_textTrace("cam LCD init and start test");
    fibo_lcd_init();
    fibo_lcd_Getinfo(&uldevid, &camPrev.lcddev.width, &camPrev.lcddev.height);
    prvBlackPrint();
    fibo_textTrace("cam lcd read width %d, height %d", camPrev.lcddev.width, camPrev.lcddev.height);
    fibo_taskSleep(1000);
    // halPmuSwitchPower(HAL_POWER_BACK_LIGHT, true, true);
}

static void camera_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    uint16_t *pCamPreviewDataBuffer = NULL;
    UINT32 ulTimer = 0;

    fibo_camera_StartPreview();
    while (1)
    {
        if (fibo_camera_GetPreviewBuf(&pCamPreviewDataBuffer) == 0)
        {
            ulTimer++;
            // deal data
            fibo_textTrace("cam deal data");
            prvCameraPrint(pCamPreviewDataBuffer);
            memset(pCamPreviewDataBuffer, 0, camPrev.camdev.nPixcels);
            fibo_camera_PrevNotify((uint16_t *)pCamPreviewDataBuffer);
        }

        if (ulTimer >= 40)
        {
            fibo_camera_StopPreview();
            pCamPreviewDataBuffer = NULL;
            if (fibo_camera_CaptureImage(&pCamPreviewDataBuffer) == 0)
            {
                fibo_textTrace("cam capture start");
                prvBlackPrint();
                fibo_taskSleep(1000);
                prvCameraPrint(pCamPreviewDataBuffer);
                fibo_taskSleep(3000);
                fibo_textTrace("cam capture end");
                prvBlackPrint();
                fibo_taskSleep(1000);
            }
            fibo_camera_StartPreview();
            ulTimer = 0;
        }
        fibo_taskSleep(1);
    }
    fibo_textTrace("camPrevTask osiThreadExit");
    test_printf();
    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();
    fibo_textTrace("Camera prev example entry");
    if (fibo_camera_init() != 0)
    {
        fibo_textTrace("Camera init fail,maybe no memory");
        return 0;
    }

    fibo_camera_GetSensorInfo(&camPrev.camdev);
    fibo_textTrace("cam read img_width %d, img_height %d", camPrev.camdev.img_width, camPrev.camdev.img_height);
    prvLcdInit();

    fibo_thread_create(camera_thread_entry, "camera_custhread", 1024 * 8, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
