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
#include "drv_config.h"
//#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_INFO
#ifdef CONFIG_CAMERA_BF3905_SUPPORT
#include "osi_log.h"
#include "osi_api.h"
#include "hwregs.h"
#include <stdlib.h>
#include "hal_chip.h"
#include "drv_i2c.h"
#include "image_sensor.h"
#include "drv_camera.h"
#include "drv_names.h"
#include "osi_clock.h"

static osiClockConstrainRegistry_t gcCamCLK = {.tag = HAL_NAME_CAM};

static const cameraReg_t RG_InitVgaMIPI[] =
{
    {0x12, 0x80}, 
    {0xff, 0xff},
    {0xff, 0xff},
    {0xff, 0xff},
    {0xff, 0xff},
    {0xff, 0xff},
    {0xff, 0xff},
    {0x15, 0x12}, 
    {0x3e, 0x83},
    {0x12, 0x00},
    {0x3a, 0x01}, 
    {0x1e, 0x00},
    {0x1b, 0x0e},  
    {0x2a, 0x00},
    {0x2b, 0x20},  
    {0x92, 0x60},
    {0x93, 0x00},
    {0x8a, 0x9f},
    {0x8b, 0x84}, 
    {0x13, 0x00}, 
    {0x01, 0x15},
    {0x02, 0x23},
    {0x9d, 0x20},
    {0x8c, 0x02},
    {0x8d, 0xee},
    {0x13, 0x07},
    {0x5d, 0x03},
    {0xbf, 0x08},
    {0xc1, 0x50},
    {0xc2, 0x00},
    {0xc3, 0x08},
    {0xca, 0x10},
    {0x62, 0x00},
    {0x63, 0x00},
    {0xb9, 0x00},
    {0x64, 0x00},
    {0x0e, 0x10},
    {0x22, 0x10},
    {0xbb, 0x10},
    {0x08, 0x02},
    {0x20, 0x09},
    {0x21, 0x4f},
    {0xd9, 0x25},
    {0xdf, 0x26},
    {0x2f, 0x84},
    {0x16, 0xa1},  
    {0x6c, 0xc2},  
    {0x71, 0x0f},
    {0x7e, 0x84},
    {0x7f, 0x3c},
    {0x60, 0xe5},
    {0x61, 0xf2},
    {0x6d, 0xc0},
    {0x17, 0x00},
    {0x18, 0xa0},
    {0x19, 0x00},
    {0x1a, 0x78},
    {0x03, 0xa0},
    {0x4a, 0x5c},  
    {0xda, 0x00},
    {0xdb, 0xa2},
    {0xdc, 0x00},
    {0xdd, 0x7a},
    {0xde, 0x00},
    {0x33, 0x10},
    {0x34, 0x08},
    {0x36, 0xc5},
    {0x6e, 0x20},
    {0xbc, 0x0d},
    {0x34, 0x1d},
    {0x36, 0x45},
    {0x6e, 0x20},
    {0xbc, 0x0d},
    {0x35, 0x30}, 
    {0x65, 0x2a},
    {0x66, 0x2a}, 
    {0xbd, 0xf4},
    {0xbe, 0x44},
    {0x9b, 0xf4},
    {0x9c, 0x44},
    {0x37, 0xf4},
    {0x38, 0x44},
    {0x70, 0x0b},
    {0x72, 0x4c},
    {0x73, 0x47}, 
    {0x79, 0x24},
    {0x74, 0x6d},
    {0x7a, 0x01}, 
    {0x7b, 0x58},
    {0x7d, 0x00},
    {0x75, 0xaa},
    {0x76, 0x28},
    {0x77, 0x2a},
    {0x39, 0x80},
    {0x3f, 0x80},
    {0x90, 0x20},
    {0x91, 0xd0},
    {0x40, 0x20},
    {0x41, 0x28},
    {0x42, 0x26},
    {0x43, 0x25},
    {0x44, 0x1f},
    {0x45, 0x1a},
    {0x46, 0x16},
    {0x47, 0x12},
    {0x48, 0x0f},
    {0x49, 0x0D},
    {0x4b, 0x0b},
    {0x4c, 0x0a},
    {0x4e, 0x08},
    {0x4f, 0x06},
    {0x50, 0x06},
    {0x5a, 0x56},
    {0x51, 0x13},
    {0x52, 0x05},
    {0x53, 0x91},
    {0x54, 0x72},
    {0x57, 0x96},
    {0x58, 0x35},
    {0x5a, 0xd6},
    {0x51, 0x28},
    {0x52, 0x35},
    {0x53, 0x9e},
    {0x54, 0x7d},
    {0x57, 0x50},
    {0x58, 0x15},
    {0x5c, 0x26},
    {0x56, 0x48},
    {0x55, 0x00},
    {0xb0, 0xe0},
    {0xb3, 0x58},
    {0xb4, 0xa3}, 
    {0xb1, 0xff},
    {0xb2, 0xa0},
    {0xb4, 0x63}, 
    {0xb1, 0xa0},
    {0xb2, 0x90},
    {0x13, 0x07},
    {0x24, 0x40}, 
    {0x25, 0x88}, 
    {0x97, 0x3c},
    {0x98, 0x8a},
    {0x80, 0x92}, 
    {0x81, 0xFF}, 
    {0x82, 0x2a}, 
    {0x83, 0x54},
    {0x84, 0x39},
    {0x85, 0x5d},
    {0x86, 0xc0}, 
    {0x89, 0x1d},
    {0x94, 0xF2}, 
    {0x96, 0xF3},
    {0x9a, 0x50},
    {0x99, 0x10},
    {0x9f, 0xF4},
    {0x6a, 0xc1},
    {0x23, 0x55}, 
    {0xa1, 0x31},
    {0xa2, 0x0d}, 
    {0xa3, 0x27},
    {0xa4, 0x0a}, 
    {0xa5, 0x2c},
    {0xa6, 0x04},
    {0xa7, 0x1a}, 
    {0xa8, 0x18},
    {0xa9, 0x13},
    {0xaa, 0x18},
    {0xab, 0x1c},
    {0xac, 0x3c},
    {0xad, 0xf0},
    {0xae, 0x57},
    {0xd0, 0x92},
    {0xd1, 0x00},
    {0xd3, 0x09},
    {0xd4, 0x24},
    {0xd2, 0x58},
    {0xc5, 0xaa},
    {0xc6, 0x88},
    {0xc7, 0x10},
    {0xc8, 0x0d},
    {0xc9, 0x10},
    {0xd3, 0x09},
    {0xd4, 0x24},
    {0xee, 0x30},
    {0x09, 0x02},
};

sensorInfo_t bf3905Info =
    {
        "bf3905",         //		const char *name; // name of sensor
        DRV_I2C_BPS_100K, //		drvI2cBps_t baud;
        0xdc >> 1,        //		uint8_t salve_i2c_addr_w;	 // salve i2c write address
        0xdd >> 1,        //		uint8_t salve_i2c_addr_r;	 // salve i2c read address
        0,                //		uint8_t reg_addr_value_bits; // bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
        {0x39, 0x05},     //		uint8_t sensorid[2];
        640,              //		uint16_t spi_pixels_per_line;	// max width of source image
        480,              //		uint16_t spi_pixels_per_column; // max height of source image
        1,                //		uint16_t rstActiveH;	// 1: high level valid; 0: low level valid
        100,              //		uint16_t rstPulseWidth; // Unit: ms. Less than 200ms
        1,                //		uint16_t pdnActiveH;	// 1: high level valid; 0: low level valid
        0,                //		uint16_t dstWinColStart;
        640,              //		uint16_t dstWinColEnd;
        0,                //		uint16_t dstWinRowStart;
        480,              //		uint16_t dstWinRowEnd;
        1,                //		uint16_t spi_ctrl_clk_div;
#ifdef CONFIG_8850_FPGA_BOARD
        DRV_NAME_I2C2, //		uint32_t i2c_name;
#else
        DRV_NAME_I2C1, //		uint32_t i2c_name;
#endif
        0, //		uint32_t nPixcels;
        2, //		uint8_t captureDrops;
        0,
        0,
        NULL, //		uint8_t *buffer;
        {NULL, NULL},
        false,
        true,
        true,
        false, //		bool isCamIfcStart;
        false, //		bool scaleEnable;
        false, //		bool cropEnable;
        false, //		bool dropFrame;
        false, //		bool spi_href_inv;
        false, //		bool spi_little_endian_en;
        false,
        false,
        true,
        SENSOR_VDD_2800MV,     //		cameraAVDD_t avdd_val; // voltage of avdd
        SENSOR_VDD_1800MV,     //		cameraAVDD_t iovdd_val;
        CAMA_CLK_OUT_FREQ_24M, //		cameraClk_t sensorClk;
        ROW_RATIO_1_1,         //		camRowRatio_t rowRatio;
        COL_RATIO_1_1,         //		camColRatio_t colRatio;
        CAM_FORMAT_YUV,        //		cameraImageFormat_t image_format; // define in SENSOR_IMAGE_FORMAT_E enum,
        SPI_MODE_NO,           //		camSpiMode_t camSpiMode;
        SPI_OUT_Y1_U0_Y0_V0,   //		camSpiYuv_t camYuvMode;
        camCaptureIdle,        //		camCapture_t camCapStatus;
        camCsi_In,
        NULL,                   //		drvIfcChannel_t *camp_ipc;
        NULL,                   //		drvI2cMaster_t *i2c_p;
        NULL,                   //		CampCamptureCB captureCB;
        NULL,
};

static void prvCamBf3905PowerOn(void)
{
    sensorInfo_t *p = &bf3905Info;
    halPmuSetPowerLevel(HAL_POWER_CAMD, p->dvdd_val);
    halPmuSwitchPower(HAL_POWER_CAMD, true, false);
    osiDelayUS(1000);
    halPmuSetPowerLevel(HAL_POWER_CAMA, p->avdd_val);
    halPmuSwitchPower(HAL_POWER_CAMA, true, false);
    osiDelayUS(1000);
}

static void prvCamBf3905PowerOff(void)
{
    halPmuSwitchPower(HAL_POWER_CAMA, false, false);
    osiDelayUS(1000);
    halPmuSwitchPower(HAL_POWER_CAMD, false, false);
    osiDelayUS(1000);
}

static bool prvCamBf3905I2cOpen(uint32_t name, drvI2cBps_t bps)
{
    sensorInfo_t *p = &bf3905Info;
#ifdef CONFIG_FIBOCOM_BASE
    if (name == 0 || p->i2c_p != NULL)
#else
    if (name == 0 || bps != DRV_I2C_BPS_100K || p->i2c_p != NULL)
#endif
    {
        return false;
    }
    p->i2c_p = drvI2cMasterAcquire(name, bps);
    if (p->i2c_p == NULL)
    {
        OSI_LOGE(0, "cam i2c open fail");
        return false;
    }
    return true;
}

static void prvCamBf3905I2cClose()
{
    sensorInfo_t *p = &bf3905Info;
    if (p->i2c_p != NULL)
        drvI2cMasterRelease(p->i2c_p);
    p->i2c_p = NULL;
}

static void prvCamReadReg(uint8_t addr, uint8_t *data, uint32_t len)
{
    sensorInfo_t *p = &bf3905Info;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false, true};
#else
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
#endif
    if (p->i2c_p != NULL)
    {
        drvI2cRead(p->i2c_p, &idAddress, data, len);
    }
    else
    {
        OSI_LOGE(0, "i2c is not open");
    }
}

static bool prvCamWriteRegList(const cameraReg_t *regList, uint16_t len)
{
    sensorInfo_t *p = &bf3905Info;
    uint16_t regCount;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false, true};
#else
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false};
#endif
    for (regCount = 0; regCount < len; regCount++)
    {
        OSI_LOGI(0, "cam write reg %x,%x", regList[regCount].addr, regList[regCount].data);
        wirte_data.addr_data = regList[regCount].addr;
        if (drvI2cWrite(p->i2c_p, &wirte_data, &regList[regCount].data, 1))
            osiDelayUS(5);
        else
            return false;
    }
    return true;
}

static bool prvCamBf3905Rginit(sensorInfo_t *info)
{
    OSI_LOGI(0, "cam prvCamBf3905Rginit %d", info->sensorType);
    switch (info->sensorType)
    {
    case camCsi_In:
        if (!prvCamWriteRegList(RG_InitVgaMIPI, sizeof(RG_InitVgaMIPI) / sizeof(cameraReg_t)))
            return false;
        break;
    default:
        return false;
    }
    return true;
}

static void prvCamIsrCB(void *ctx)
{
    static uint8_t pictureDrop = 0;
    static uint8_t prev_ovf = 0;
    static uint8_t cap_ovf = 0;
    sensorInfo_t *p = &bf3905Info;
    REG_CAMERA_IRQ_CAUSE_T cause;
    cameraIrqCause_t mask = {0, 0, 0, 0};
    cause.v = hwp_camera->irq_cause;
    hwp_camera->irq_clear = cause.v;
    OSI_LOGI(0, "cam bf3905 prvCsiCamIsrCB %d,%d,%d,%d", cause.b.vsync_f, cause.b.ovfl, cause.b.dma_done, cause.b.vsync_r);
    OSI_LOGI(0, "p->camCapStatus %d", p->camCapStatus);
    switch (p->camCapStatus)
    {
    case camCaptureState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            drvCamCmdSetFifoRst();
            drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
            p->isCamIfcStart = true;
            mask.overflow = 1;
            mask.dma = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = camCaptureState2;
        }
        break;
    case camCaptureState2:
        if (cause.b.ovfl)
        {
            OSI_LOGI(0, "cam ovfl ");
            drvCameraControllerEnable(false);
            drvCamCmdSetFifoRst();
            drvCameraControllerEnable(true);
            cap_ovf = 1;
        }
        if (cause.b.dma_done)
        {
            drvCamClrIrqMask();
            if (cap_ovf == 1)
            {
                cap_ovf = 0;
                if (p->isCamIfcStart)
                    drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->camCapStatus = camCaptureState1;
                mask.fend = 1;
                drvCamSetIrqMask(mask);
                return;
            }
            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->capturedata))
                {
                    OSI_LOGI(0, "cam p->captureDrops %d ", p->captureDrops);
                    OSI_LOGI(0, "cam pictureDrop %d ", pictureDrop);
                    if (pictureDrop < p->captureDrops)
                    {
                        mask.fend = 1;
                        drvCamSetIrqMask(mask);
                        p->camCapStatus = camCaptureState1;
                        pictureDrop++;
                    }
                    else
                    {
                        p->camCapStatus = camCaptureIdle;
                        osiSemaphoreRelease(p->cam_sem_capture);
                        if (pictureDrop >= p->captureDrops)
                            pictureDrop = 0;
                        p->isFirst = false;
                    }
                }
            }
        }
        break;
    case campPreviewState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            OSI_LOGI(0, "cam mask ovf %d, firstframe %d", prev_ovf, p->isFirstFrame);
            if (prev_ovf || p->isFirstFrame)
            {
                drvCamCmdSetFifoRst();
                drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->isCamIfcStart = true;
                p->isFirstFrame = false;
                prev_ovf = 0;
                hwp_camera->irq_clear |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
            }
            //hwp_camera->irq_clear |= 1 << 0;
            mask.overflow = 1;
            mask.dma = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = campPreviewState2;
        }
        break;
    case campPreviewState2:
        if (cause.b.ovfl)
        {
            OSI_LOGI(0, "cam ovfl ");
            prev_ovf = 1;

            drvCamClrIrqMask();
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            //stop
            p->camCapStatus = campPreviewState1;
            return;
        }
        if (cause.b.dma_done)
        {
            drvCamClrIrqMask();
            if (prev_ovf == 1)
            {
                prev_ovf = 0;
                if (p->isCamIfcStart)
                    drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->camCapStatus = campPreviewState1;
                mask.fend = 1;
                drvCamSetIrqMask(mask);
                return;
            }
            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]))
                {
                    OSI_LOGI(0, "bf3905 preview release data %x", p->previewdata[p->page_turn]);
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
                    p->page_turn = 1 - p->page_turn;
#endif
                    osiSemaphoreRelease(p->cam_sem_preview);
                    p->isFirst = false;
                    OSI_LOGI(0, "bf3905 preview_page data %d", p->preview_page);
                    if (--p->preview_page)
                    {
                        p->camCapStatus = campPreviewState1;
                        mask.fend = 1;
                        drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                        p->isCamIfcStart = true;
                        drvCamSetIrqMask(mask);
                    }
                    else
                    {
                        OSI_LOGI(0, "cam go to idle");
                        p->camCapStatus = camCaptureIdle;
                        drvCameraControllerEnable(false);
                    }
                }
                else
                {
                    drvCamClrIrqMask();
                    p->camCapStatus = campPreviewState1;
                    mask.fend = 1;
                    drvCamSetIrqMask(mask);
                    OSI_LOGI(0, "cam dma stop error");
                }
            }
        }
        break;
    default:
        break;
    }
}

void camBf3905TestMode(bool on)
{
}

bool camBf3905Open(void)
{
    OSI_LOGI(0, "camBf3905Open");
    sensorInfo_t *p = &bf3905Info;
    osiRequestSysClkActive(&gcCamCLK);
    drvCamSetPdn(false);
    osiDelayUS(1000);
    prvCamBf3905PowerOn();
    drvCamSetMCLK(p->sensorClk);
    osiDelayUS(1000);
    drvCamSetPdn(true);
    osiDelayUS(1000);
    drvCamSetPdn(false);
    if (!prvCamBf3905I2cOpen(p->i2c_name, p->baud))
    {
        OSI_LOGE(0, "cam prvCamBf3905I2cOpen fail");
        prvCamBf3905I2cClose();
        prvCamBf3905PowerOff();
        return false;
    }
    if (!prvCamBf3905Rginit(p))
    {
        OSI_LOGE(0, "cam prvCamBf3905Rginit fail");
        prvCamBf3905I2cClose();
        prvCamBf3905PowerOff();
        return false;
    }
    drvCampRegInit(p);
    drvCamSetIrqHandler(prvCamIsrCB, NULL);

    p->isCamOpen = true;
    drvCameraControllerEnable(true);
    return true;
}

void camBf3905Close(void)
{
    sensorInfo_t *p = &bf3905Info;
    if (p->isCamOpen)
    {
        prvCamBf3905I2cClose();
        osiReleaseClk(&gcCamCLK);
        drvCamSetPdn(true);
        osiDelayUS(1000);
        drvCamDisableMCLK();
        osiDelayUS(1000);
        prvCamBf3905PowerOff();
        drvCamSetPdn(false);
        drvCampRegDeInit();
        drvCamDisableIrq();
        p->isFirst = true;
        p->isCamOpen = false;
    }
    else
    {
        p->isCamOpen = false;
    }
}

void camBf3905GetId(uint8_t *data, uint8_t len)
{
    sensorInfo_t *p = &bf3905Info;
    if (p->isCamOpen)
    {
        if (data == NULL || len < 1)
            return;
        prvCamReadReg(0xfc, &data[0], 1);
        osiDelayUS(5);
        prvCamReadReg(0xfc, &data[0], 1);
        osiDelayUS(5);
        prvCamReadReg(0xfd, &data[1], 1);
    }
}

bool camBf3905CheckId(void)
{
    sensorInfo_t *p = &bf3905Info;
    uint8_t sensorID[2] = {0, 0};
    if (!p->isCamOpen)
    {
        prvCamBf3905PowerOn();
        drvCamSetMCLK(p->sensorClk);
        osiDelayUS(1000);
        drvCamSetPdn(false);
        osiDelayUS(1000);
        drvCamSetPdn(true);
        osiDelayUS(1000);
        drvCamSetPdn(false);
        if (!prvCamBf3905I2cOpen(p->i2c_name, p->baud))
        {
            OSI_LOGE(0, "cam prvCamBf3905I2cOpen failed");
            return false;
        }
        if (!p->isCamOpen)
        {
            p->isCamOpen = true;
        }
        camBf3905GetId(sensorID, 2);
        OSI_LOGI(0, "cam get id 0x%x,0x%x", sensorID[0], sensorID[1]);
        if ((p->sensorid[0] == sensorID[0]) && (p->sensorid[1] == sensorID[1]))
        {
            OSI_LOGI(0, "check id successful");
            camBf3905Close();
            return true;
        }
        else
        {
            camBf3905Close();
            OSI_LOGE(0, "check id error");
            return false;
        }
    }
    else
    {
        OSI_LOGE(0, "camera already opened !");
        return false;
    }
}

void camBf3905CaptureImage(uint32_t size)
{
    sensorInfo_t *p = &bf3905Info;
    if (size != 0)
    {
        cameraIrqCause_t mask = {0, 0, 0, 0};
        if (p->isCamIfcStart == true)
        {
            drvCampStopTransfer(p->nPixcels, p->capturedata);
            p->isCamIfcStart = false;
        }
        p->nPixcels = size;
        p->camCapStatus = camCaptureState1;
        mask.fend = 1;
        drvCamSetIrqMask(mask);
    }
}

void camBf3905PrevStart(uint32_t size)
{
    sensorInfo_t *p = &bf3905Info;
    OSI_LOGI(0, "camBf3905PrevStart p->preview_page=%d ", p->preview_page);
    if (p->preview_page == 0)
    {
        cameraIrqCause_t mask = {0, 0, 0, 0};
        drvCamSetIrqMask(mask);
        if (p->isCamIfcStart == true)
        {
            drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
            p->isCamIfcStart = false;
        }
        p->nPixcels = size;
        p->camCapStatus = campPreviewState1;
        p->isStopPrev = false;
#ifdef CONFIG_CAMERA_SINGLE_BUFFER
        p->preview_page = 1;
#else
        p->preview_page = 2;
#endif
        p->isFirstFrame = true;
        mask.fend = 1;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void camBf3905PrevNotify(void)
{
    sensorInfo_t *p = &bf3905Info;
    uint32_t critical = osiEnterCritical();
    if (p->isCamOpen && !p->isStopPrev)
    {
        if (p->preview_page == 0)
        {
            cameraIrqCause_t mask = {0, 0, 0, 0};
            p->isFirstFrame = true;
            p->camCapStatus = campPreviewState1;
            p->preview_page++;
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            drvCameraControllerEnable(true);
        }
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
        else
        {
            if (p->preview_page < 2)
                p->preview_page++;
        }
#endif
    }
    osiExitCritical(critical);
}

void camBf3905StopPrev(void)
{
    sensorInfo_t *p = &bf3905Info;
    drvCamClrIrqMask();
    if (p->isCamIfcStart == true)
    {
        drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
        p->isCamIfcStart = false;
    }
    drvCameraControllerEnable(false);
    p->camCapStatus = camCaptureIdle;
    p->isStopPrev = true;
    p->preview_page = 0;
}

void camBf3905SetFlash(uint8_t level)
{
}

void camBf3905Brightness(uint8_t level)
{
}

void camBf3905Contrast(uint8_t level)
{
}

void camBf3905ImageEffect(uint8_t effect_type)
{
}
void camBf3905Ev(uint8_t level)
{
}

void camBf3905AWB(uint8_t mode)
{
}

void camBf3905GetSensorInfo(sensorInfo_t **pSensorInfo)
{
    OSI_LOGI(0, "CamBf3905GetSensorInfo %08x", &bf3905Info);
    *pSensorInfo = &bf3905Info;
}

bool camBf3905Reg(SensorOps_t *pSensorOpsCB)
{
    if (camBf3905CheckId())
    {
        pSensorOpsCB->cameraOpen = camBf3905Open;
        pSensorOpsCB->cameraClose = camBf3905Close;
        pSensorOpsCB->cameraGetID = camBf3905GetId;
        pSensorOpsCB->cameraCaptureImage = camBf3905CaptureImage;
        pSensorOpsCB->cameraStartPrev = camBf3905PrevStart;
        pSensorOpsCB->cameraPrevNotify = camBf3905PrevNotify;
        pSensorOpsCB->cameraStopPrev = camBf3905StopPrev;
        pSensorOpsCB->cameraSetAWB = camBf3905AWB;
        pSensorOpsCB->cameraSetBrightness = camBf3905Brightness;
        pSensorOpsCB->cameraSetContrast = camBf3905Contrast;
        pSensorOpsCB->cameraSetEv = camBf3905Ev;
        pSensorOpsCB->cameraSetImageEffect = camBf3905ImageEffect;
        pSensorOpsCB->cameraGetSensorInfo = camBf3905GetSensorInfo;
        pSensorOpsCB->cameraFlashSet = camBf3905SetFlash;
        pSensorOpsCB->cameraTestMode = camBf3905TestMode;
        return true;
    }
    return false;
}
#endif
