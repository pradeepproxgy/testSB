﻿/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
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

//#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_INFO
#include "osi_log.h"
#include "osi_api.h"
#include "hwregs.h"
#include <stdlib.h>
#include "image_sensor.h"
#include "drv_camera.h"
#include "fibocom.h"
#include <fibo_common.h>

#ifdef CONFIG_FIBOCOM_BASE
IMAGE_DEV_T ImageSensor;
#else
static IMAGE_DEV_T ImageSensor;
#endif
static SensorOps_t SensorOpsApi = {
    0,
};

#ifdef CONFIG_FIBOCOM_BASE
extern bool ffw_hal_gpio_pull_up_or_down(uint16_t pin_id,bool is_pull_up);
extern bool ffw_hal_gpioModeSet(uint16_t  pin_id, uint8_t mode);
#endif
#ifdef CONFIG_CAMERA_GC032A_SUPPORT
extern bool camGc032aReg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_GC0310_SUPPORT
extern bool camGc0310Reg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_BF30A2_SUPPORT
extern bool CamBf30a2Reg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_GC2145_SUPPORT
extern bool camGc2145Reg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_GC0406_SUPPORT
extern bool camGc0406Reg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_BF3905_SUPPORT
extern bool camBf3905Reg(SensorOps_t *pSensorOpsCB);
#endif
#ifdef CONFIG_CAMERA_BF20A6_SUPPORT
extern bool CamBf20a6Reg(SensorOps_t *pSensorOpsCB);
#endif
static uint8_t g_cam_index = 0xff;
bool (*pCamRegInit[])(SensorOps_t *) =
    {
#ifdef CONFIG_CAMERA_GC032A_SUPPORT
        camGc032aReg,
#endif
#ifdef CONFIG_CAMERA_GC0310_SUPPORT
        camGc0310Reg,
#endif
#ifdef CONFIG_CAMERA_BF30A2_SUPPORT
        CamBf30a2Reg,
#endif
#ifdef CONFIG_CAMERA_GC2145_SUPPORT
        camGc2145Reg,
#endif
#ifdef CONFIG_CAMERA_GC0406_SUPPORT
        camGc0406Reg,
#endif
#ifdef CONFIG_CAMERA_BF3905_SUPPORT
        camBf3905Reg,
#endif
#ifdef CONFIG_CAMERA_BF20A6_SUPPORT
        CamBf20a6Reg,
#endif
};

static uint32_t prvCamGetCfgCount(void)
{
    uint8_t count = sizeof(pCamRegInit) / sizeof(uint32_t);
    OSI_LOGI(0x10007d6d, "cam:  _drvCamGetCfgCount count=%d \n", count);
    return count;
}

static SensorOps_t *prvCamLoad(void)
{
    uint32_t count = 0;
    uint32_t i = 0;
    OSI_LOGI(0x10007d6e, "cam: _drvCamLoad \n");
    count = prvCamGetCfgCount();
    //temp fix for i2c open fail sensor not work
    if ((count != 0) && (g_cam_index == 0xff))
    {
        for (i = 0; i < count; i++)
        {
            if (pCamRegInit[i](&SensorOpsApi))
            {
                g_cam_index = i;
                return &SensorOpsApi;
            }
        }
    }
    else if (g_cam_index != 0xff)
    {
        return &SensorOpsApi;
    }
    OSI_LOGI(0x10007d6f, "cam: _drvCamLoad null ! \n");
    return NULL;
}

static bool prvCamSetFrameBuffer(uint16_t *FrameBuf[], uint8_t nBufcounts)
{
    ImageSensor.pSensorInfo->previewdata[0] = NULL;
    ImageSensor.pSensorInfo->previewdata[1] = NULL;
    ImageSensor.pSensorInfo->capturedata = NULL;

    if (nBufcounts > 2)
        return false;
    if (FrameBuf[0] == NULL)
        return false;

    ImageSensor.framebuffer_count = nBufcounts;

    for (uint8_t i = 0; i < nBufcounts; i++)
    {
        ImageSensor.pSensorInfo->previewdata[i] = (uint8_t *)(OSI_ALIGN_UP(FrameBuf[i], CONFIG_CACHE_LINE_SIZE));
    }
    //OSI_ALIGN_UP for ptr addr dma cache line
    ImageSensor.pSensorInfo->capturedata = (uint8_t *)(OSI_ALIGN_UP(FrameBuf[0], CONFIG_CACHE_LINE_SIZE));

    //create semphore for task sync
    ImageSensor.pSensorInfo->cam_sem_capture = osiSemaphoreCreate(1, 0);
    ImageSensor.pSensorInfo->cam_sem_preview = osiSemaphoreCreate(2, 0);

    return true;
}

void drvCamTestMode(bool on)
{
    if (ImageSensor.poweron_flag)
    {
        ImageSensor.pSensorOpsApi->cameraTestMode(on);
    }
}

void drvCamClose(void)
{
    if (ImageSensor.poweron_flag)
    {
        ImageSensor.poweron_flag = false;
        ImageSensor.pSensorOpsApi->cameraClose();
        if (ImageSensor.poweron_flag == false)
        {
            for (uint8_t i = 0; i < ImageSensor.framebuffer_count; i++)
            {
                if (ImageSensor.pFramebuffer[i] != NULL)
                {
                    free(ImageSensor.pFramebuffer[i]);
                }
            }
            ImageSensor.framebuffer_count = 0;

            OSI_LOGI(0x10007d70, "cam:drvCamClose,osiSemaphoreDelete");
            osiSemaphoreDelete(ImageSensor.pSensorInfo->cam_sem_capture);
            osiSemaphoreDelete(ImageSensor.pSensorInfo->cam_sem_preview);
        }
    }
}

bool drvCamCaptureImage(uint16_t **pFrameBuf)
{
    if (ImageSensor.poweron_flag)
    {
        if (ImageSensor.pSensorInfo->image_format == CAM_FORMAT_RAW8)
            ImageSensor.pSensorOpsApi->cameraCaptureImage(ImageSensor.nPixcels * 2);
        else
            ImageSensor.pSensorOpsApi->cameraCaptureImage(ImageSensor.nPixcels);
        if (osiSemaphoreTryAcquire(ImageSensor.pSensorInfo->cam_sem_capture, 5000))
        {
            *pFrameBuf = (uint16_t *)ImageSensor.pSensorInfo->capturedata;
            return true;
        }
        else
        {
            OSI_LOGI(0x10007d71, "cam:drvCamCaptureImage timeout");
            *pFrameBuf = NULL;
            return false;
        }
    }
    else
    {
        *pFrameBuf = NULL;
        return false;
    }
}

bool drvCamStopPreview(void)
{
    OSI_LOGI(0x10007d72, "cam: drvCamStopPreview");
    if (ImageSensor.poweron_flag)
    {
        ImageSensor.pSensorOpsApi->cameraStopPrev();
        return true;
    }
    else
        return true;
}

void drvCamPreviewQBUF(uint16_t *pFrameBuf)
{
    OSI_LOGD(0x10007d73, "cam: drvCamPreviewQBUF");
    uint32_t critical = osiEnterCritical();
    ImageSensor.pSensorOpsApi->cameraPrevNotify();
    osiExitCritical(critical);
}

uint16_t *drvCamPreviewDQBUF(void)
{
    uint8_t turn = 0;
    OSI_LOGD(0x10007d74, "cam: drvCamPreviewDQBUF");
    if (!osiSemaphoreTryAcquire(ImageSensor.pSensorInfo->cam_sem_preview, 1000))
        return NULL;
        //get valid bufnumber
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
    turn = 1 - ImageSensor.pSensorInfo->page_turn;
#endif
    OSI_LOGI(0x10007d75, "cam: drvCamPreviewDataAcquire turn =%d buf=%08x\n", turn, ImageSensor.pSensorInfo->previewdata[turn]);
    return (uint16_t *)ImageSensor.pSensorInfo->previewdata[turn];
}

bool drvCamStartPreview(void)
{
    OSI_LOGI(0x10007d76, "cam: drvCamStartPreview");
    if (ImageSensor.poweron_flag)
    {
        if (ImageSensor.pSensorInfo->image_format == CAM_FORMAT_RAW8)
            ImageSensor.pSensorOpsApi->cameraStartPrev(ImageSensor.nPixcels * 2);
        else
            ImageSensor.pSensorOpsApi->cameraStartPrev(ImageSensor.nPixcels);
        return true;
    }
    else
        return false;
}

#ifdef CONFIG_FIBOCOM_BASE
void drvCamCleanBuf(void)
{
    if(ImageSensor.nPixcels == 0)
    {
        return;
    }

    if(ImageSensor.pFramebuffer[0] != NULL)
    {
        memset((uint8_t *)ImageSensor.pFramebuffer[0],0x00,(ImageSensor.nPixcels + CONFIG_CACHE_LINE_SIZE));
    }

#ifndef CONFIG_CAMERA_SINGLE_BUFFER
    if(ImageSensor.pFramebuffer[1] != NULL)
    {
        memset((uint8_t *)ImageSensor.pFramebuffer[1],0x00,(ImageSensor.nPixcels + CONFIG_CACHE_LINE_SIZE));
    } 
#endif
}
#endif

bool drvCamPowerOn(void)
{
    OSI_LOGI(0x10007d77, "cam: drvCamPowerOn");
    if (ImageSensor.poweron_flag)
        return true;
    else
        ImageSensor.poweron_flag = ImageSensor.pSensorOpsApi->cameraOpen();

    if (ImageSensor.poweron_flag == false)
    {
        for (uint8_t i = 0; i < ImageSensor.framebuffer_count; i++)
        {
            if (ImageSensor.pFramebuffer[i] != NULL)
            {
                free(ImageSensor.pFramebuffer[i]);
            }
        }
        osiSemaphoreDelete(ImageSensor.pSensorInfo->cam_sem_capture);
        osiSemaphoreDelete(ImageSensor.pSensorInfo->cam_sem_preview);
    }
    return ImageSensor.poweron_flag;
}

bool drvCamReleaseSem(void)
{
    OSI_LOGI(0x1000aa34, "cam: drvCamReleaseSem");
    if (!ImageSensor.poweron_flag)
        return true;

    osiSemaphoreRelease(ImageSensor.pSensorInfo->cam_sem_capture);
    osiSemaphoreRelease(ImageSensor.pSensorInfo->cam_sem_preview);
    return true;
}

#ifdef CONFIG_FIBOCOM_BASE
bool drvCamGetSensorInfo(CAM_DEV_T *pCamDevice)
#else
void drvCamGetSensorInfo(CAM_DEV_T *pCamDevice)
#endif
{
    OSI_LOGI(0x10007d78, "cam: drvCamGetSensorInfo");
    if (ImageSensor.pSensorOpsApi != NULL)
    {
        pCamDevice->pNamestr = (char *)ImageSensor.pSensorInfo->name;
        pCamDevice->img_width = ImageSensor.pSensorInfo->spi_pixels_per_line;
        pCamDevice->img_height = ImageSensor.pSensorInfo->spi_pixels_per_column;
        pCamDevice->nPixcels = ImageSensor.nPixcels;
    }
    else
    {
        pCamDevice->pNamestr = " ";
        pCamDevice->img_width = 0;
        pCamDevice->img_height = 0;
        pCamDevice->nPixcels = 0;
        ImageSensor.nPixcels = 0;
    #ifdef CONFIG_FIBOCOM_BASE
            return false;
    #endif
        }
#ifdef CONFIG_FIBOCOM_BASE
        return true;
#endif
}

bool drvCamGePrevStatus(void)
{
    ImageSensor.pSensorOpsApi->cameraGetSensorInfo(&(ImageSensor.pSensorInfo));
    return !ImageSensor.pSensorInfo->isStopPrev;
}

bool drvCamInit(void)
{
    OSI_LOGI(0x10007d79, "cam: drvCamInit");
    SensorOps_t *campoint = NULL;
// pin select by user
// #ifdef CONFIG_FIBOCOM_MC661
//     ffw_hal_gpioModeSet(57,1);
//     ffw_hal_gpioModeSet(58,1);
//     ffw_hal_gpio_pull_up_or_down(57,true);
//     ffw_hal_gpio_pull_up_or_down(58,true);
// #endif    
    //find sensor
    if (ImageSensor.poweron_flag)
        return true;
    campoint = prvCamLoad();
    if (!campoint)
        return false;
    ImageSensor.pSensorOpsApi = campoint;
    if (ImageSensor.pSensorOpsApi != NULL)
    {
        //get sensor info
        ImageSensor.pSensorOpsApi->cameraGetSensorInfo(&(ImageSensor.pSensorInfo));
        //calc sensor output data size in bytes
#ifdef CONFIG_FIBOCOM_BASE
        if ((ImageSensor.pSensorInfo->image_format == CAM_FORMAT_RAW8) || (ImageSensor.pSensorInfo->image_format == CAM_FORMAT_ONLY_Y)) {
            ImageSensor.nPixcels = ImageSensor.pSensorInfo->spi_pixels_per_line * ImageSensor.pSensorInfo->spi_pixels_per_column;
        } else {
            ImageSensor.nPixcels = ImageSensor.pSensorInfo->spi_pixels_per_line * ImageSensor.pSensorInfo->spi_pixels_per_column * 2;
        }
#else
        if (ImageSensor.pSensorInfo->image_format == CAM_FORMAT_RAW8)
            ImageSensor.nPixcels = ImageSensor.pSensorInfo->spi_pixels_per_line * ImageSensor.pSensorInfo->spi_pixels_per_column;
        else
            ImageSensor.nPixcels = ImageSensor.pSensorInfo->spi_pixels_per_line * ImageSensor.pSensorInfo->spi_pixels_per_column * 2;
#endif

        //malloc sensor framedata buffer
#ifdef CONFIG_CAMERA_SINGLE_BUFFER
        ImageSensor.pFramebuffer[0] = NULL;
#else
        ImageSensor.pFramebuffer[0] = NULL;
        ImageSensor.pFramebuffer[1] = NULL;
#endif
        //malloc more data size for dma cache line
        uint32_t alloc_size;
        if (ImageSensor.pSensorInfo->image_format == CAM_FORMAT_RAW8)
            alloc_size = (ImageSensor.nPixcels * 2) + CONFIG_CACHE_LINE_SIZE;
        else
            alloc_size = ImageSensor.nPixcels + CONFIG_CACHE_LINE_SIZE;
        ImageSensor.pFramebuffer[0] = (uint16_t *)calloc(alloc_size, sizeof(uint8_t));
        if (ImageSensor.pFramebuffer[0] == NULL)
        {
            return false;
        }
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
        ImageSensor.pFramebuffer[1] = (uint16_t *)calloc(alloc_size, sizeof(uint8_t));
        if (ImageSensor.pFramebuffer[1] == NULL)
        {
            free(ImageSensor.pFramebuffer[0]);
            ImageSensor.pFramebuffer[0] = NULL;
            return false;
        }
#endif
        //set sensor framebuffer
        prvCamSetFrameBuffer(&(ImageSensor.pFramebuffer[0]), SENSOR_FRAMEBUFFER_NUM);

        return true;
    }
    else
        return false;
}
