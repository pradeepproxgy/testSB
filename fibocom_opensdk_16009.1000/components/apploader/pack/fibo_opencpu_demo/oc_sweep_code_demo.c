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

static camastae_t camctx = {0};
uint8_t *g_Databuf = NULL;

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

#ifdef SAVE_IMAGE_TO_SDCARD
static void savetoSD(char *filename, uint8_t *in_buff, uint32_t len)
{
    int fd = 0;

    fd = vfs_open(filename, O_RDWR | O_CREAT | O_TRUNC);
    if (fd < 0)
    {
        fibo_textTrace("cam open %s error numer %d", filename, errno);
        return;
    }


    fibo_textTrace("cam write image in sdcard");
    ssize_t size = vfs_write(fd, in_buff, len);
    if (size != len)
    {
        fibo_textTrace("cam write size error %d", size);
    }
    vfs_close(fd);
    fibo_textTrace("cam write data size %d", size);
}
#endif

static void sweep_code_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    uint16_t *pCamPreviewDataBuffer = NULL;
    uint8_t aucResult[4096];
    uint32_t ulReslen;
    int type = 0;
    bool ret = false;

#ifdef SAVE_IMAGE_TO_SDCARD
    static int count = 0;
    char achFileName[20];
    uint32_t ulSweepSize = 0;
#endif
    fibo_camera_StartPreview();
    while (1)
    {
        memset(aucResult, 0, sizeof aucResult);
        ulReslen = 0;
        if (fibo_camera_GetPreviewBuf(&pCamPreviewDataBuffer) == 0)
        {

#ifdef SAVE_IMAGE_TO_SDCARD
            count++;
            memset(achFileName, 0, 20);
            sprintf(achFileName, "/sdcard0/%d.bin", count);
            ulSweepSize = camctx.sweepsize;
            savetoSD(achFileName, pCamPreviewDataBuffer, ulSweepSize);
            if (count > 20)
                break;
#endif
            ret = fibo_sweep_code(&camctx, pCamPreviewDataBuffer, g_Databuf, aucResult, &ulReslen, &type);
            if (ret)
            {
                if (ulReslen > 0)
                {
                    fibo_textTrace("Decode content %d, %s", ulReslen, aucResult);
                    break;
                }
            }
            else
            {
                memset(pCamPreviewDataBuffer, 0, camctx.CamDev.nPixcels);
                fibo_camera_PrevNotify((uint16_t *)pCamPreviewDataBuffer);
                continue;
            }
        }
    }

    memset(g_Databuf, 0, camctx.sweepsize+32);
    free(g_Databuf);
    g_Databuf = NULL;

    fibo_textTrace("sweep code fibo_thread_delete");
    fibo_thread_delete();
}

static void prvcamctx_init(camastae_t *st)
{
    memset(&st->CamDev, 0, sizeof(CAM_DEV_T));
    memset(&st->lcddev, 0, sizeof(lcdSpec_t));
    st->camTask = NULL;
    st->Decodestat = 0;
    st->Openstat = false;
    st->MemoryState = true;
    st->gCamPowerOnFlag = false;
    st->issnapshot = false;
    st->height = 0;
    st->width = 0;
    st->sweepsize = 0;
    st->times = 0;
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    uint32_t height = 480;
    uint32_t width = 640;

    prvInvokeGlobalCtors();
    fibo_textTrace("sweep code example entry");

    if (((height == 60) && (width == 80)) ||
            ((height == 120) && (width == 160)) ||
            ((height == 240) && (width == 320)) ||
            ((height == 480) && (width == 640)))
    {
        if (!camctx.Openstat)
        {
            prvcamctx_init(&camctx);

            camctx.sweepsize = height * width;
            if ((g_Databuf=(uint8_t *)malloc(camctx.sweepsize + 32)) == NULL)
            {
                fibo_textTrace("malloc for sweep code buf failed");
                return 0;
            }

            if (0 != fibo_camera_init())
            {
                fibo_textTrace("Camera init fail");
                return 0;
            }

            fibo_camera_GetSensorInfo(&camctx.CamDev);
            fibo_textTrace("cam read img_width %d, img_height %d", camctx.CamDev.img_width, camctx.CamDev.img_height);

            if (!camctx.gCamPowerOnFlag)
            {
                camctx.height = height;
                camctx.width = width;
                camctx.times = camctx.CamDev.img_width/width;
                camctx.gCamPowerOnFlag = true;
                camctx.Openstat = true;
                fibo_textTrace("sweepCode:open height=%d,width=%d,times=%d,ulSweepSize=%d.", height, width, camctx.times, camctx.sweepsize);

                fibo_thread_create(sweep_code_thread_entry, "sweepcode_custhread", 1024*80, &camctx, OSI_PRIORITY_NORMAL);
            }
        }
    }
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
