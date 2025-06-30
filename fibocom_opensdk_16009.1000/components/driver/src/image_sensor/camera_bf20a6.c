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
#ifdef CONFIG_CAMERA_BF20A6_SUPPORT
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

#ifdef CONFIG_FIBOCOM_BASE
static const cameraReg_t RG_InitVgaOnlyY[] =
{
    {0xf2,0x01},
    {0xe0,0x00},
    {0x12,0xC3},//ZX:10

    {0x3a,0x02}, //2bit spi
    {0xe1,0x51},
    {0xe3,0x12},

    {0x2a,0x98},
    {0xcd,0x17},
    {0xe2,0x09},
    {0xe4,0x92},
    {0xe5,0x22},
    {0xe6,0x24},
    {0xe7,0x54},
    {0xe8,0xf2},
    {0x4a,0x00},
    {0x00,0x03},
    {0x1f,0x02},
    {0x22,0x02},

    {0xa0,0x08},
    {0x01,0x20},
    {0x02,0x10},
    {0x13,0x08},
    {0x8a,0x96},
    {0x8b,0x0a},
    {0x87,0x18},


    {0x34,0x48},//lens
    {0x35,0x40},
    {0x36,0x40},

    {0x72,0x47},
    {0x74,0xa2},
    {0x75,0xa9},
    {0x78,0x12},
    {0x79,0xa0},
    {0x7a,0x94},
    {0x7c,0x97},
    {0x40,0x30},
    {0x41,0x30},
    {0x42,0x28},
    {0x43,0x1f},
    {0x44,0x1c},
    {0x45,0x16},
    {0x46,0x13},
    {0x47,0x10},
    {0x48,0x0D},
    {0x49,0x0C},
    {0x4B,0x0A},
    {0x4C,0x0B},
    {0x4E,0x09},
    {0x4F,0x08},
    {0x50,0x08},


    {0x5f,0x29},
    {0x23,0x33},
    {0xa1,0x10},//AWB
    {0xa2,0x0d},
    {0xa3,0x30},
    {0xa4,0x06},
    {0xa5,0x22},
    {0xa6,0x56},
    {0xa7,0x18},
    {0xa8,0x1a},
    {0xa9,0x12},
    {0xaa,0x12},
    {0xab,0x16},
    {0xac,0xb3},
    {0xba,0x12},
    {0xbb,0x12},
    {0xad,0x12},
    {0xae,0x56},//56
    {0xaf,0x0a},
    {0x3b,0x30},
    {0x3c,0x12},
    {0x3d,0x22},
    {0x3e,0x3f},
    {0x3f,0x28},
    {0xb8,0xc3},
    {0xb9,0xA3},
    {0x39,0x47},//pure color threshold
    {0x26,0x13},
    {0x27,0x16},
    {0x28,0x14},
    {0x29,0x18},
    {0xee,0x0d},

    {0x13,0x05},
    {0x24,0x3c},
    {0x81,0x20},
    {0x82,0x40},
    {0x83,0x30},
    {0x84,0x58},
    {0x85,0x30},
    {0x92,0x08},
    {0x86,0xc0},
    {0x8a,0x96},
    {0x91,0xff},
    {0x94,0x62},
    {0x9a,0x18},//outdoor threshold
    {0xf0,0x43},
    {0x51,0x07},//color normal
    {0x52,0x1B},
    {0x53,0x7A},
    {0x54,0x61},
    {0x55,0x56},
    {0x56,0x03},
    {0x7e,0x15},
    {0x57,0x37},//A光color
    {0x58,0x1E},
    {0x59,0xAF},
    {0x5a,0xC5},
    {0x5b,0x55},
    {0x5c,0x18},
    {0x5d,0x00},
    {0x7d,0x36},
    {0x5e,0x10},

    {0xd6,0x88},//contrast
    {0xd5,0x25},//低光加亮度
    {0xb0,0xa4},//灰色区域降饱和度
    {0xb5,0x08},//低光降饱和度阈值
    {0xb1,0xc8},//saturation
    {0xb2,0xc8},
    {0xb3,0xd0},
    {0xb4,0xc0},

    {0x32,0x10},
    {0xa0,0x89},

    {0x0b,0x00},

    {0x17,0x00},
    {0x18,0xA0},
    {0x19,0x00},
    {0x1a,0x78},

    {0x15,0x10},
};
#endif

static const cameraReg_t RG_InitVgaSPI[] =
    {
    {0xf2,0x01},
    {0xe0,0x00},
    {0x12,0xC0},//ZX:10
    {0x3a,0x02},
    {0xe1,0x92},
    {0xe3,0x12},//12
    {0x2a,0x98},

    {0x0e,0x47},
    {0x0f,0x60},
    {0x10,0x57},
    {0x11,0x60},
    {0x30,0x61},
    {0x62,0xcd},
    {0x63,0x1a},
    {0x64,0x38},
    {0x65,0x52},
    {0x66,0x68},
    {0x67,0xc2},
    {0x68,0xa7},
    {0x69,0xab},
    {0x6a,0xad},
    {0x6b,0xa9},
    {0x6c,0xc4},
    {0x6d,0xc5},
    {0x6e,0x18},
    {0xc0,0x20},
    {0xc1,0x24},
    {0xc2,0x29},
    {0xc3,0x25},
    {0xc4,0x28},
    {0xc5,0x2a},
    {0xc6,0x41},
    {0xca,0x23},
    {0xcd,0x34},
    {0xce,0x32},
    {0xcf,0x35},
    {0xd0,0x6c},
    {0xd1,0x6e},
    {0xd2,0xcb},
    {0xe2,0x09},
    {0xe4,0x73},//92
    {0xe5,0x22},
    {0xe6,0x24},
    {0xe7,0x64},
    {0xe8,0xf2},
    {0x4a,0x00},
    {0x00,0x03},
    {0x1f,0x02},
    {0x22,0x02},

    {0xa0,0x08},
    {0x01,0x20},
    {0x02,0x10},
    {0x13,0x08},
    {0x8a,0x96},
    {0x8b,0x0a},
    {0x87,0x18},


    {0x34,0x48},//lens
    {0x35,0x40},
    {0x36,0x40},
    {0x71,0x44},
    {0x72,0x48},
    {0x74,0xa2},
    {0x75,0xa9},
    {0x78,0x01},
    {0x79,0xa0},
    {0x7a,0x94},
    {0x7c,0x97},
    {0x40,0x30},
    {0x41,0x30},
    {0x42,0x28},
    {0x43,0x1f},
    {0x44,0x1c},
    {0x45,0x16},
    {0x46,0x13},
    {0x47,0x10},
    {0x48,0x0D},
    {0x49,0x0C},
    {0x4B,0x0A},
    {0x4C,0x0B},
    {0x4E,0x09},
    {0x4F,0x08},
    {0x50,0x08},


    {0x5f,0x29},
    {0x23,0x33},
    {0xa1,0x10},//AWB
    {0xa2,0x0d},
    {0xa3,0x30},
    {0xa4,0x06},
    {0xa5,0x22},
    {0xa6,0x56},
    {0xa7,0x18},
    {0xa8,0x1a},
    {0xa9,0x12},
    {0xaa,0x12},
    {0xab,0x16},
    {0xac,0xb3},
    {0xba,0x12},
    {0xbb,0x12},
    {0xad,0x12},
    {0xae,0x56},//56
    {0xaf,0x0a},
    {0x3b,0x30},
    {0x3c,0x12},
    {0x3d,0x22},
    {0x3e,0x3f},
    {0x3f,0x28},
    {0xb8,0xc3},
    {0xb9,0xA3},
    {0x39,0x47},//pure color threshold
    {0x26,0x13},
    {0x27,0x16},
    {0x28,0x14},
    {0x29,0x18},
    {0xee,0x0d},

    {0x13,0x05},
    {0x24,0x3c},
    {0x81,0x20},
    {0x82,0x40},
    {0x83,0x30},
    {0x84,0x58},
    {0x85,0x30},
    {0x92,0x08},
    {0x86,0xc0},
    {0x8a,0x96},
    {0x91,0xff},
    {0x94,0x62},
    {0x9a,0x18},//outdoor threshold
    {0xf0,0x43},
    {0x51,0x07},//color normal
    {0x52,0x1B},
    {0x53,0x7A},
    {0x54,0x61},
    {0x55,0x56},
    {0x56,0x03},
    {0x7e,0x15},
    {0x57,0x37},
    {0x58,0x1E},
    {0x59,0xAF},
    {0x5a,0xC5},
    {0x5b,0x55},
    {0x5c,0x18},
    {0x5d,0x00},
    {0x7d,0x36},
    {0x5e,0x10},

    {0xd6,0xa8},//contrast
    {0xd5,0x00},
    {0xb0,0xa4},
    {0xb5,0x08},
    {0xb1,0xc8},//saturation
    {0xb2,0xc8},
    {0xb3,0xd0},
    {0xb4,0xc0},

    {0x32,0x00},
    {0xa0,0x09},

    {0x0b,0x00},

    {0x17,0x00},
    {0x18,0xA0},
    {0x19,0x00},
    {0x1a,0x78},
    {0xf1,0x02},
    {0x15,0x10},
};

sensorInfo_t bf20a6Info =
    {
        "bf20a6",         //		const char *name; // name of sensor
#ifdef CONFIG_FIBOCUS_1081_FEATURE
        DRV_I2C_BPS_400K, //		drvI2cBps_t baud;
#else
        DRV_I2C_BPS_100K, //		drvI2cBps_t baud;
#endif
        0xdc >> 1,        //		uint8_t salve_i2c_addr_w;	 // salve i2c write address
        0xdd >> 1,        //		uint8_t salve_i2c_addr_r;	 // salve i2c read address
        0,                //		uint8_t reg_addr_value_bits; // bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
        {0x20, 0xa6},     //		uint8_t sensorid[2];
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
        DRV_NAME_I2C1,    //		uint32_t i2c_name;
        0,                //		uint32_t nPixcels;
        1,                //		uint8_t captureDrops;
        0,
        0,
        NULL, //		uint8_t *buffer;
        {NULL, NULL},
        false,
        true,
        true,
        false, //		bool isCamIfcStart;
        false, //		bool scaleEnable;
        true,  //		bool cropEnable;
        false, //		bool dropFrame;
        false, //		bool spi_href_inv;
        false, //		bool spi_little_endian_en;
        false,
        false,
        false,                 //       ddr_en
        SENSOR_VDD_2800MV,     //		cameraAVDD_t avdd_val; // voltage of avdd
        SENSOR_VDD_1800MV,     //		cameraAVDD_t iovdd_val;
        CAMA_CLK_OUT_FREQ_24M, //		cameraClk_t sensorClk;
        ROW_RATIO_1_1,         //		camRowRatio_t rowRatio;
        COL_RATIO_1_1,         //		camColRatio_t colRatio;
#ifdef CONFIG_FIBOCUS_1081_FEATURE
        CAM_FORMAT_ONLY_Y,        //		cameraImageFormat_t image_format; // define in SENSOR_IMAGE_FORMAT_E enum,
#else
        CAM_FORMAT_YUV,        //		cameraImageFormat_t image_format; // define in SENSOR_IMAGE_FORMAT_E enum,
#endif
        SPI_MODE_MASTER2_2,    //		camSpiMode_t camSpiMode;
        SPI_OUT_Y1_V0_Y0_U0,   //		camSpiYuv_t camYuvMode;
        camCaptureIdle,        //		camCapture_t camCapStatus;
        camSpi_In,
        NULL, //		drvIfcChannel_t *camp_ipc;
        NULL, //		drvI2cMaster_t *i2c_p;
        NULL, //		CampCamptureCB captureCB;
        NULL,
};

static void prvCamBf20a6PowerOn(void)
{
    sensorInfo_t *p = &bf20a6Info;
    halPmuSetPowerLevel(HAL_POWER_CAMD, p->dvdd_val);
    halPmuSwitchPower(HAL_POWER_CAMD, true, false);
    osiDelayUS(1000);
    halPmuSetPowerLevel(HAL_POWER_CAMA, p->avdd_val);
    halPmuSwitchPower(HAL_POWER_CAMA, true, false);
    osiDelayUS(1000);
}

static void prvCamBf20a6PowerOff(void)
{
    halPmuSwitchPower(HAL_POWER_CAMA, false, false);
    osiDelayUS(1000);
    halPmuSwitchPower(HAL_POWER_CAMD, false, false);
    osiDelayUS(1000);
}

static bool prvCamBf20a6I2cOpen(uint32_t name, drvI2cBps_t bps)
{
    sensorInfo_t *p = &bf20a6Info;
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
        OSI_LOGE(0x10007d55, "cam i2c open fail");
        return false;
    }
    return true;
}

static void prvCamBf20a6I2cClose()
{
    sensorInfo_t *p = &bf20a6Info;
    if (p->i2c_p != NULL)
        drvI2cMasterRelease(p->i2c_p);
    p->i2c_p = NULL;
}

static void prvCamWriteOneReg(uint8_t addr, uint8_t data)
{
    sensorInfo_t *p = &bf20a6Info;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false, true};
#else
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
#endif
    if (p->i2c_p != NULL)
    {
        drvI2cWrite(p->i2c_p, &idAddress, &data, 1);
    }
    else
    {
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static void prvCamReadReg(uint8_t addr, uint8_t *data, uint32_t len)
{
    sensorInfo_t *p = &bf20a6Info;
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
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static bool prvCamWriteRegList(const cameraReg_t *regList, uint16_t len)
{
    sensorInfo_t *p = &bf20a6Info;
    uint16_t regCount;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false, true};
#else
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false};
#endif
    prvCamWriteOneReg(0xf4, 0x1c);
    osiDelayUS(1000);
    for (regCount = 0; regCount < len; regCount++)
    {
        OSI_LOGI(0x10007d57, "cam write reg %x,%x", regList[regCount].addr, regList[regCount].data);
        wirte_data.addr_data = regList[regCount].addr;
        if (drvI2cWrite(p->i2c_p, &wirte_data, &regList[regCount].data, 1))
            osiDelayUS(5);
        else
            return false;
    }
    return true;
}

static bool prvCamBf20a6Rginit(sensorInfo_t *info)
{
    switch (info->sensorType)
    {
    case camSpi_In:
#ifdef CONFIG_FIBOCOM_BASE
        if (info->image_format == CAM_FORMAT_ONLY_Y) {
            if (!prvCamWriteRegList(RG_InitVgaOnlyY, sizeof(RG_InitVgaOnlyY) / sizeof(cameraReg_t)))
                return false;
        } else {
            if (!prvCamWriteRegList(RG_InitVgaSPI, sizeof(RG_InitVgaSPI) / sizeof(cameraReg_t)))
                return false;
        }
#else
        if (!prvCamWriteRegList(RG_InitVgaSPI, sizeof(RG_InitVgaSPI) / sizeof(cameraReg_t)))
            return false;
#endif
        break;
    default:
        return false;
    }
    return true;
}

static void prvCamIsrCB(void *ctx)
{
    sensorInfo_t *p = &bf20a6Info;
    static uint8_t pictureDrop = 0;
    REG_CAMERA_IRQ_CAUSE_T cause;
    cameraIrqCause_t mask = {0, 0, 0, 0};
    cause.v = hwp_camera->irq_cause;
    hwp_camera->irq_clear = cause.v;
    OSI_LOGI(0x10009fd8, "cam bf20a6 prvCsiCamIsrCB %d,%d,%d,%d", cause.b.vsync_f, cause.b.ovfl, cause.b.dma_done, cause.b.vsync_r);
    switch (p->camCapStatus)
    {
    case camCaptureState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            drvCamCmdSetFifoRst();
            if (p->isCamIfcStart == true)
            {
                drvCampStopTransfer(p->nPixcels, p->capturedata);
                p->isCamIfcStart = false;
            }
            drvCampStartTransfer(p->nPixcels, p->capturedata);
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = camCaptureState2;
            p->isCamIfcStart = true;
        }
        break;
    case camCaptureState2:
        if (cause.b.vsync_f)
        {
            if (p->isCamIfcStart)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->capturedata))
                {
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
                        OSI_LOGD(0x10009fd9, "cam bf20a6 campture release %x", p->capturedata);
                        osiSemaphoreRelease(p->cam_sem_capture);
                        if (pictureDrop >= p->captureDrops)
                            pictureDrop = 0;
                        p->isFirst = false;
                    }
                }
                else
                {
                    drvCampStartTransfer(p->nPixcels, p->capturedata);
                    mask.fend = 1;
                    p->isCamIfcStart = true;
                    drvCamSetIrqMask(mask);
                }
            }
        }
        break;
    case campPreviewState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            drvCamCmdSetFifoRst();
            if (p->isCamIfcStart)
            {
                drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->isCamIfcStart = false;
            }
            drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = campPreviewState2;
            p->isCamIfcStart = true;
        }
        break;
    case campPreviewState2:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            drvCamCmdSetFifoRst();
            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]))
                {
                    OSI_LOGD(0x10009fda, "cam bf20a6 preview release %x", p->previewdata[p->page_turn]);
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
                    p->page_turn = 1 - p->page_turn;
#endif
                    osiSemaphoreRelease(p->cam_sem_preview);
                    p->isFirst = false;
                    if (--p->preview_page)
                    {
                        drvCamClrIrqMask();
                        drvCamCmdSetFifoRst();
                        drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                        p->isCamIfcStart = true;
                        mask.fend = 1;
                        drvCamSetIrqMask(mask);
                    }
                    else
                    {
                        drvCamClrIrqMask();
                        p->camCapStatus = camCaptureIdle;
                    }
                }
                else
                {
                    p->camCapStatus = campPreviewState1;
                    mask.fend = 1;
                    drvCamSetIrqMask(mask);
                }
            }
        }
        break;
    default:
        break;
    }
}

void CamBf20a6TestMode(bool on)
{
}

bool CamBf20a6Open(void)
{
    OSI_LOGI(0x10009fdb, "CamBf20a6Open");
    sensorInfo_t *p = &bf20a6Info;
    osiRequestSysClkActive(&gcCamCLK);
    drvCamSetPdn(false);
    osiDelayUS(100);
    prvCamBf20a6PowerOn();
    osiDelayUS(100);
    drvCamSetMCLK(p->sensorClk);
    osiDelayUS(100);
    drvCamSetPdn(true);
    osiDelayUS(100);
    drvCamSetPdn(false);
    osiDelayUS(200);

    if (!prvCamBf20a6I2cOpen(p->i2c_name, p->baud))
    {
        OSI_LOGE(0x10009fdc, "cam prvCamBf20a6I2cOpen fail");
        prvCamBf20a6I2cClose();
        prvCamBf20a6PowerOff();
        return false;
    }
    if (!prvCamBf20a6Rginit(&bf20a6Info))
    {
        OSI_LOGE(0x10009fdd, "cam prvCamBf20a6Rginit fail");
        prvCamBf20a6I2cClose();
        prvCamBf20a6PowerOff();
        return false;
    }
    osiDelayUS(200);

    drvCampRegInit(&bf20a6Info);
    drvCamSetIrqHandler(prvCamIsrCB, NULL);
    p->isCamOpen = true;
    drvCameraControllerEnable(true);
    return true;
}

void CamBf20a6Close(void)
{
    sensorInfo_t *p = &bf20a6Info;
    if (p->isCamOpen)
    {
        prvCamBf20a6I2cClose();
        osiDelayUS(200);
        osiReleaseClk(&gcCamCLK);
        drvCamDisableMCLK();
        osiDelayUS(20);
        prvCamBf20a6PowerOff();
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

void CamBf20a6GetId(uint8_t *data, uint8_t len)
{
    sensorInfo_t *p = &bf20a6Info;
    if (p->isCamOpen)
    {
        if (data == NULL || len < 1)
            return;
        prvCamReadReg(0xfc, data, len);
    }
}

bool CamBf20a6CheckId(void)
{
    sensorInfo_t *p = &bf20a6Info;
    uint8_t sensorID[2] = {0, 0};
    if (!p->isCamOpen)
    {
        drvCamSetPdn(false);
        osiDelayUS(5);
        prvCamBf20a6PowerOn();
        drvCamSetMCLK(p->sensorClk);
        drvCamSetPdn(true);
        osiDelayUS(5);
        drvCamSetPdn(false);
        osiDelayUS(200);
        if (!prvCamBf20a6I2cOpen(p->i2c_name, p->baud))
        {
            OSI_LOGE(0x10009fdc, "cam prvCamBf20a6I2cOpen fail");
            return false;
        }
        if (!p->isCamOpen)
        {
            p->isCamOpen = true;
        }
        CamBf20a6GetId(sensorID, 2);
        OSI_LOGI(0x10007d67, "cam get id 0x%x,0x%x", sensorID[0], sensorID[1]);
        if ((p->sensorid[0] == sensorID[0]) && (p->sensorid[1] == sensorID[1]))
        {
            OSI_LOGI(0x10007d68, "check id successful");
            CamBf20a6Close();
            return true;
        }
        else
        {
            CamBf20a6Close();
            OSI_LOGE(0x10007d69, "check id error");
            return false;
        }
    }
    else
    {
        OSI_LOGE(0x10007d6a, "camera already opened !");
        return false;
    }
}

void CamBf20a6CaptureImage(uint32_t size)
{
    sensorInfo_t *p = &bf20a6Info;
    if (size != 0)
    {
        cameraIrqCause_t mask = {0, 0, 0, 0};
        drvCamSetIrqMask(mask);
        if (p->isCamIfcStart == true)
        {
            drvCampStopTransfer(p->nPixcels, p->capturedata);
            p->isCamIfcStart = false;
        }
        p->nPixcels = size;
        p->camCapStatus = camCaptureState1;

        mask.fend = 1;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void CamBf20a6PrevStart(uint32_t size)
{
    sensorInfo_t *p = &bf20a6Info;
    OSI_LOGI(0x10009fde, "CamBf20a6PrevStart p->preview_page=%d preview size %d", p->preview_page, size);
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
        mask.fend = 1;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void CamBf20a6PrevNotify(void)
{
    sensorInfo_t *p = &bf20a6Info;
    if (p->isCamOpen && !p->isStopPrev)
    {
        if (p->preview_page == 0)
        {
            cameraIrqCause_t mask = {0, 0, 0, 0};

            p->camCapStatus = campPreviewState1;
            p->preview_page++;
            mask.fend = 1;
            drvCamSetIrqMask(mask);
        }
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
        else
        {
            if (p->preview_page < 2)
                p->preview_page++;
        }
#endif
    }
}

void CamBf20a6StopPrev(void)
{
    sensorInfo_t *p = &bf20a6Info;
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

void CamBf20a6SetFalsh(uint8_t level)
{
    if (level >= 0 && level < 16)
    {
        if (level == 0)
        {
            halPmuSwitchPower(HAL_POWER_CAMFLASH, false, false);
        }
        else
        {
            halPmuSetCamFlashLevel(level);
            halPmuSwitchPower(HAL_POWER_CAMFLASH, true, false);
        }
    }
}

void CamBf20a6Brightness(uint8_t level)
{
}

void CamBf20a6Contrast(uint8_t level)
{
}

void CamBf20a6ImageEffect(uint8_t effect_type)
{
}
void CamBf20a6Ev(uint8_t level)
{
}

void CamBf20a6AWB(uint8_t mode)
{
}

void CamBf20a6GetSensorInfo(sensorInfo_t **pSensorInfo)
{
    OSI_LOGI(0x10009fdf, "CamBf20a6GetSensorInfo %08x", &bf20a6Info);
    *pSensorInfo = &bf20a6Info;
}

bool CamBf20a6Reg(SensorOps_t *pSensorOpsCB)
{
    if (CamBf20a6CheckId())
    {
        pSensorOpsCB->cameraOpen = CamBf20a6Open;
        pSensorOpsCB->cameraClose = CamBf20a6Close;
        pSensorOpsCB->cameraGetID = CamBf20a6GetId;
        pSensorOpsCB->cameraCaptureImage = CamBf20a6CaptureImage;
        pSensorOpsCB->cameraStartPrev = CamBf20a6PrevStart;
        pSensorOpsCB->cameraPrevNotify = CamBf20a6PrevNotify;
        pSensorOpsCB->cameraStopPrev = CamBf20a6StopPrev;
        pSensorOpsCB->cameraSetAWB = CamBf20a6AWB;
        pSensorOpsCB->cameraSetBrightness = CamBf20a6Brightness;
        pSensorOpsCB->cameraSetContrast = CamBf20a6Contrast;
        pSensorOpsCB->cameraSetEv = CamBf20a6Ev;
        pSensorOpsCB->cameraSetImageEffect = CamBf20a6ImageEffect;
        pSensorOpsCB->cameraGetSensorInfo = CamBf20a6GetSensorInfo;
        pSensorOpsCB->cameraFlashSet = CamBf20a6SetFalsh;
        pSensorOpsCB->cameraTestMode = CamBf20a6TestMode;
        return true;
    }
    return false;
}
#endif
