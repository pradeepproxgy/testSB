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
#ifdef CONFIG_CAMERA_GC0406_SUPPORT
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

//static int64_t time =0;
//static int32_t vsync_times = 0;
static const cameraReg_t RG_InitVgaMIPI2[] =
    {
        /*system*/
        {0xfe, 0x80},
        {0xfe, 0x80},
        {0xfe, 0x80},
        {0xf7, 0x01},
        {0xf8, 0x05},
        {0xf9, 0x0f}, //[0] not_use_pll
        {0xfa, 0x00},
        {0xfc, 0x0f}, //[0] apwd
        {0xfe, 0x00},
        /*analog & CISCTL*/
        {0xfe, 0x00},
        {0x03, 0x01},
        {0x04, 0xda},
        {0x05, 0x02},
        {0x06, 0x97}, //HB
        {0x07, 0x01},
        {0x08, 0x30},
        {0x0a, 0x00}, //row_start
        {0x0c, 0x04},
        {0x0d, 0x01},
        {0x0e, 0xe8},
        {0x0f, 0x03},
        {0x10, 0x30}, //win_width
        {0x17, 0x42}, //Don't Change Here!!!
        {0x18, 0x12},
        {0x19, 0x0b},
        {0x1a, 0x1a},
        {0x1e, 0x50},
        {0x1f, 0x80},
        {0x24, 0xc8}, //c8
        {0x25, 0xe2},
        {0x27, 0xaf},
        {0x28, 0x24},
        {0x29, 0x14},
        {0x2f, 0x14},
        {0x3f, 0x18},
        {0x40, 0x26},
        {0x72, 0xab}, //20151012 0x9b
        {0x73, 0x4a}, //20151012 0xea
        {0x74, 0x40},
        {0x75, 0x10},
        {0x76, 0xa2}, //20151012 0x92
        {0x7a, 0x4e},
        {0xc1, 0x12},
        {0xc2, 0x0c},
        {0xcf, 0x48},
        {0xdc, 0x75},
        {0xeb, 0x78},
        /*ISP*/
        {0xfe, 0x00},
        {0x8d, 0x03},
        {0x90, 0x01},
        {0x92, 0x00}, //Don't Change Here!!!
        {0x94, 0x00}, //Don't Change Here!!!
        {0x95, 0x01},
        {0x96, 0xe0},
        {0x97, 0x03},
        {0x98, 0x20},
        /*gain*/
        {0xb0, 0x50}, //global_gain
        {0xb1, 0x01},
        {0xb2, 0x00},
        {0xb3, 0x40},
        {0xb4, 0x40},
        {0xb5, 0x40},
        {0xb6, 0x00},
        {0x30, 0x00},
        {0x31, 0x01},
        {0x32, 0x02},
        {0x33, 0x03},
        {0x34, 0x07},
        {0x35, 0x0b},
        {0x36, 0x0f},
        /*BLK*/
        {0xfe, 0x00},
        {0x40, 0x26},
        {0x4f, 0xc3},
        {0x5e, 0x00},
        {0x5f, 0x80},
        /*dark sun*/
        {0xfe, 0x00},
        {0xe0, 0x9e},
        {0xe1, 0x80},
        {0xe4, 0x0f},
        {0xe5, 0xff},
        /*mipi*/
        {0xfe, 0x03},
        {0x10, 0x00},
        {0x01, 0x03},
        {0x02, 0x22},
        {0x03, 0x96},
        {0x04, 0x01},
        {0x05, 0x00},
        {0x06, 0x80},
        {0x10, 0x94},
        {0x11, 0x2a},
        {0x12, 0x20}, //MIPI BUFFER
        {0x13, 0x03}, //MIPI BUFFER
        {0x15, 0x00}, //FRAME INTO LP
        {0x21, 0x10}, //T_LPX
        {0x22, 0x00},
        {0x23, 0x30},
        {0x24, 0x02},
        {0x25, 0x12},
        {0x26, 0x03}, // T_Clk_TRAIL
        {0x29, 0x01}, //T_HS_PREPARE
        {0x2a, 0x0a},
        {0x2b, 0x06}, //T_HS_TRAIL modify by yuquan.zeng 0x03 ---> 0x06
        {0xfe, 0x00},
        {0xf9, 0x0e},  //[0] not_use_pll
        {0xfc, 0x0e},  //[0] apwd
        {0xfe, 0x00}}; /*    sensor_init  */

sensorInfo_t gc0406Info =
    {
        "gc0406",         //		const char *name; // name of sensor
        DRV_I2C_BPS_100K, //		drvI2cBps_t baud;
        0x42 >> 1,        //		uint8_t salve_i2c_addr_w;	 // salve i2c write address
        0x43 >> 1,        //		uint8_t salve_i2c_addr_r;	 // salve i2c read address
        0,                //		uint8_t reg_addr_value_bits; // bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
        {0x04, 0x69},     //		uint8_t sensorid[2];
        800,              //		uint16_t spi_pixels_per_line;	// max width of source image
        480,              //		uint16_t spi_pixels_per_column; // max height of source image
        1,                //		uint16_t rstActiveH;	// 1: high level valid; 0: low level valid
        100,              //		uint16_t rstPulseWidth; // Unit: ms. Less than 200ms
        1,                //		uint16_t pdnActiveH;	// 1: high level valid; 0: low level valid
        0,                //		uint16_t dstWinColStart;
        800,              //		uint16_t dstWinColEnd;
        0,                //		uint16_t dstWinRowStart;
        480,              //		uint16_t dstWinRowEnd;
        1,                //		uint16_t spi_ctrl_clk_div;
        DRV_NAME_I2C1,    //		uint32_t i2c_name;
        0,                //		uint32_t nPixcels;
        2,                //		uint8_t captureDrops;
        0,
        0,
        NULL, //		uint8_t *buffer;
        {NULL, NULL},
        false,                 //isFirstFrame;
        true,                  //		 bool isStopPrev;
        true,                  //		bool isFirst;
        false,                 //		bool isCamIfcStart;
        false,                 //		bool scaleEnable;
        false,                 //		bool cropEnable;
        false,                 //		bool dropFrame;
        false,                 //		bool spi_href_inv;
        false,                 //		bool spi_little_endian_en;
        false,                 //		iscapture
        false,                 //		isopen
        true,                  //		ddr_enable
        SENSOR_VDD_2800MV,     //		cameraAVDD_t avdd_val; // voltage of avdd
        SENSOR_VDD_1800MV,     //		cameraAVDD_t iovdd_val;
        CAMA_CLK_OUT_FREQ_24M, //		cameraClk_t sensorClk;
        ROW_RATIO_1_1,         //		camRowRatio_t rowRatio;
        COL_RATIO_1_1,         //		camColRatio_t colRatio;
        CAM_FORMAT_RAW8,       //		cameraImageFormat_t image_format; // define in SENSOR_IMAGE_FORMAT_E enum,
        SPI_MODE_NO,           //		camSpiMode_t camSpiMode;
        SPI_OUT_Y1_U0_Y0_V0,   //		camSpiYuv_t camYuvMode;
        camCaptureIdle,        //		camCapture_t camCapStatus;
        camCsi_In,             //
        NULL,                  //		drvIfcChannel_t *camp_ipc;
        NULL,                  //		drvI2cMaster_t *i2c_p;
        NULL,                  //		CampCamptureCB captureCB;
        NULL,
};

static void prvCamGc0406filter(void *recieve_data)
{
    uint8_t *data = (uint8_t *)recieve_data;
    int j = 0;
    for (int i = 0; i < gc0406Info.nPixcels; i = i + 4, j = j + 2)
    {
        data[j] = data[i];
        data[j + 1] = data[i + 2];
    }
}

static void prvCamReadReg(uint8_t addr, uint8_t *data, uint32_t len)
{
    sensorInfo_t *p = &gc0406Info;
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
    if (p->i2c_p != NULL)
    {
        drvI2cRead(p->i2c_p, &idAddress, data, len);
    }
    else
    {
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static void prvCamGc0406PowerOff(void)
{
    halPmuSwitchPower(HAL_POWER_CAMA, false, false);
    osiDelayUS(1000);
    halPmuSwitchPower(HAL_POWER_CAMD, false, false);
    osiDelayUS(1000);
}

static void prvCamWriteOneReg(uint8_t addr, uint8_t data)
{
    sensorInfo_t *p = &gc0406Info;
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
    if (p->i2c_p != NULL)
    {
        drvI2cWrite(p->i2c_p, &idAddress, &data, 1);
    }
    else
    {
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static void prvCsiCamIsrCB(void *ctx)
{
    static uint8_t pictureDrop = 0;
    static uint8_t prev_ovf = 0;
    static uint8_t cap_ovf = 0;
    sensorInfo_t *p = &gc0406Info;
    REG_CAMERA_IRQ_CAUSE_T cause;
    cameraIrqCause_t mask = {0, 0, 0, 0};
    cause.v = hwp_camera->irq_cause;
    hwp_camera->irq_clear = cause.v;
    OSI_LOGI(0x10009fe0, "cam gc0406 prvCsiCamIsrCB %d,%d,%d,%d", cause.b.vsync_f, cause.b.ovfl, cause.b.dma_done, cause.b.vsync_r);
    OSI_LOGI(0x10007d5b, "p->camCapStatus %d", p->camCapStatus);
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
            OSI_LOGI(0x10007d5c, "cam ovfl ");
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
                    OSI_LOGI(0x10007d5d, "cam p->captureDrops %d ", p->captureDrops);
                    OSI_LOGI(0x10007d5e, "cam pictureDrop %d ", pictureDrop);
                    if (pictureDrop < p->captureDrops)
                    {
                        mask.fend = 1;
                        drvCamSetIrqMask(mask);
                        p->camCapStatus = camCaptureState1;
                        pictureDrop++;
                    }
                    else
                    {
                        prvCamGc0406filter(p->capturedata);
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

            OSI_LOGI(0x10009fe1, "cam mask ovf %d, firstframe %d\n", prev_ovf, p->isFirstFrame);
            if (prev_ovf || p->isFirstFrame)
            {
                drvCamCmdSetFifoRst();
                drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->isCamIfcStart = true;
                p->isFirstFrame = false;
                prev_ovf = 0;
                hwp_camera->irq_clear |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
            }
            hwp_camera->irq_clear |= 1 << 0;
            mask.overflow = 1;
            mask.dma = 1;
            drvCamSetIrqMask(mask);

            p->camCapStatus = campPreviewState2;
        }

        break;
    case campPreviewState2:
        if (cause.b.ovfl)
        {
            OSI_LOGI(0x10009fe2, "cam irq ovfl \n");
            drvCamClrIrqMask();
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            prev_ovf = 1;
            p->camCapStatus = campPreviewState1;
            return;
        }
        if (cause.b.dma_done)
        {
            drvCamClrIrqMask();
            hwp_camera->irq_clear |= CAMERA_DMA_DONE | CAMERA_VSYNC_F;

            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]))
                {
                    //int64_t t1 = osiUpTimeUS();
                    prvCamGc0406filter(p->previewdata[p->page_turn]);
                    //int64_t t2 = osiUpTimeUS();
                    //OSI_LOGXI(OSI_LOGPAR_DDD, 0, "cam filter use %lld, %lld, %lld", (t2-t1), t2, t1);
                    OSI_LOGI(0x10009fe3, "cam irq preview release data %x\n", p->previewdata[p->page_turn]);
                    p->isFirstFrame = false;

#ifndef CONFIG_CAMERA_SINGLE_BUFFER
                    p->page_turn = 1 - p->page_turn;
#endif
                    osiSemaphoreRelease(p->cam_sem_preview);

                    OSI_LOGI(0x10009fe4, "cam irq drvCampStartTransfer %d\n", p->preview_page);
                    if (--p->preview_page > 0)
                    {
                        drvCamClrIrqMask();

                        mask.dma = 1;
                        mask.overflow = 1;
                        drvCamSetIrqMask(mask);

                        drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                        p->isCamIfcStart = true;
                        p->camCapStatus = campPreviewState2;
                    }
                    else
                    {
                        OSI_LOGI(0x10009fe5, "cam go to idle\n");
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
                    prev_ovf = 1;
                    OSI_LOGI(0x10007d62, "cam dma stop error");
                }
            }
        }
        break;
    default:
        break;
    }
}

void camGc0406GetId(uint8_t *data, uint8_t len)
{
    sensorInfo_t *p = &gc0406Info;
    if (p->isCamOpen)
    {
        if (data == NULL || len < 1)
            return;
        //prvCamWriteOneReg(0xf4, 0x1c);
        prvCamReadReg(0xf0, data, len - 1);
        osiDelayUS(2);
        prvCamReadReg(0xf1, data + 1, len - 1);
    }
}

static bool prvCamGc0406I2cOpen(uint32_t name, drvI2cBps_t bps)
{
    sensorInfo_t *p = &gc0406Info;
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

static void prvCamGc0406I2cClose()
{
    sensorInfo_t *p = &gc0406Info;
    if (p->i2c_p != NULL)
        drvI2cMasterRelease(p->i2c_p);
    p->i2c_p = NULL;
}

static void prvCamGc0406PowerOn(void)
{
    sensorInfo_t *p = &gc0406Info;
    halPmuSetPowerLevel(HAL_POWER_CAMD, p->dvdd_val);
    halPmuSwitchPower(HAL_POWER_CAMD, true, false);
    osiDelayUS(1000);
    halPmuSetPowerLevel(HAL_POWER_CAMA, p->avdd_val);
    halPmuSwitchPower(HAL_POWER_CAMA, true, false);
    osiDelayUS(1000);
}

static bool prvCamWriteRegList(const cameraReg_t *regList, uint16_t len)
{
    sensorInfo_t *p = &gc0406Info;
    uint16_t regCount;
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false};
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

void camGc0406Close(void)
{
    sensorInfo_t *p = &gc0406Info;
    if (p->isCamOpen)
    {
        osiReleaseClk(&gcCamCLK);
        drvCamSetPdn(true);
        osiDelayUS(1000);
        drvCamDisableMCLK();
        osiDelayUS(1000);
        prvCamGc0406PowerOff();
        drvCamSetPdn(false);
        prvCamGc0406I2cClose();
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

bool camGc0406CheckId(void)
{
    sensorInfo_t *p = &gc0406Info;
    uint8_t sensorID[2] = {0, 0};
    if (!p->isCamOpen)
    {
        drvCamSetPdn(true);
        osiDelayUS(1000);
        prvCamGc0406PowerOn();
        drvCamSetMCLK(p->sensorClk);
        osiDelayUS(1000);
        drvCamSetPdn(false);
        osiDelayUS(1000);
        drvCamSetPdn(true);
        osiDelayUS(1000);
        drvCamSetPdn(false);
        if (!prvCamGc0406I2cOpen(p->i2c_name, p->baud))
        {
            OSI_LOGE(0x10009fe6, "cam prvCamGc0406I2cOpen failed");
            return false;
        }
        if (!p->isCamOpen)
        {
            p->isCamOpen = true;
        }
        camGc0406GetId(sensorID, 2);
        OSI_LOGI(0x10007d67, "cam get id 0x%x,0x%x", sensorID[0], sensorID[1]);
        if ((p->sensorid[0] == sensorID[0]) && (p->sensorid[1] == sensorID[1]))
        {
            OSI_LOGI(0x10007d68, "check id successful");
            camGc0406Close();
            return true;
        }
        else
        {
            camGc0406Close();
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

static bool prvCamGc0406Rginit(sensorInfo_t *info)
{
    OSI_LOGI(0x10009fe7, "cam prvCamGC0406Rginit %d", info->sensorType);
    switch (info->sensorType)
    {
    case camCsi_In:
        if (!prvCamWriteRegList(RG_InitVgaMIPI2, sizeof(RG_InitVgaMIPI2) / sizeof(cameraReg_t)))
            return false;
        break;
    default:
        return false;
    }
    return true;
}

bool camGc0406Open(void)
{
    OSI_LOGI(0x10009fe8, "camGc0406Open");
    sensorInfo_t *p = &gc0406Info;
    osiRequestSysClkActive(&gcCamCLK);

    drvCamSetPdn(true);
    osiDelayUS(1000);
    prvCamGc0406PowerOn();
    drvCamSetMCLK(p->sensorClk);
    osiDelayUS(1000);
    drvCamSetPdn(false);
    osiDelayUS(1000);
    drvCamSetPdn(true);
    osiDelayUS(1000);
    drvCamSetPdn(false);
    if (!prvCamGc0406I2cOpen(p->i2c_name, p->baud))
    {
        OSI_LOGE(0x10009fe9, "cam prvCamGc0406I2cOpen fail");
        prvCamGc0406I2cClose();
        prvCamGc0406PowerOff();
        return false;
    }
    if (!prvCamGc0406Rginit(&gc0406Info))
    {
        OSI_LOGE(0x10009fea, "cam prvCamGc0406Rginit fail");
        prvCamGc0406I2cClose();
        prvCamGc0406PowerOff();
        return false;
    }
    drvCampRegInit(&gc0406Info);
    switch (p->sensorType)
    {
    case camCsi_In:
        drvCamSetIrqHandler(prvCsiCamIsrCB, NULL);
        break;
    default:
        break;
    }
    p->isCamOpen = true;
    return true;
}

void camGc0406GetSensorInfo(sensorInfo_t **pSensorInfo)
{
    OSI_LOGI(0x10009feb, "CamGc0406GetSensorInfo %08x", &gc0406Info);
    *pSensorInfo = &gc0406Info;
}

void camGc0406CaptureImage(uint32_t size)
{
    sensorInfo_t *p = &gc0406Info;
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
        drvCameraControllerEnable(true);
    }
}

void camGc0406PrevStart(uint32_t size)
{
    sensorInfo_t *p = &gc0406Info;
    OSI_LOGI(0x10009fec, "camGc0406PrevStart p->preview_page=%d ", p->preview_page);
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
        p->isFirstFrame = true;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void camGc0406PrevNotify(void)
{
    sensorInfo_t *p = &gc0406Info;
    uint32_t critical = osiEnterCritical();
    if (p->isCamOpen && !p->isStopPrev)
    {
        if (p->preview_page == 0)
        {
            cameraIrqCause_t mask = {0, 0, 0, 0};

            p->camCapStatus = campPreviewState1;
            p->preview_page++;
            p->isFirstFrame = true;
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

void camGc0406StopPrev(void)
{
    sensorInfo_t *p = &gc0406Info;
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

void camGc0406SetFalsh(uint8_t level)
{
}

void camGc0406Brightness(uint8_t level)
{
}

void camGc0406Contrast(uint8_t level)
{
}

void camGc0406ImageEffect(uint8_t effect_type)
{
}
void camGc0406Ev(uint8_t level)
{
}

void camGc0406AWB(uint8_t mode)
{
}

void camGc0406TestMode(bool on)
{
    sensorInfo_t *p = &gc0406Info;
    if (p->isCamOpen)
    {
        if (on)
        {
            prvCamWriteOneReg(0xfe, 0x00);
            prvCamWriteOneReg(0x8d, 0x03);
        }
        else
        {
            prvCamWriteOneReg(0xfe, 0x00);
            prvCamWriteOneReg(0x8d, 0x02);
        }
    }
}

bool camGc0406Reg(SensorOps_t *pSensorOpsCB)
{
    if (camGc0406CheckId())
    {
        pSensorOpsCB->cameraOpen = camGc0406Open;
        pSensorOpsCB->cameraClose = camGc0406Close;
        pSensorOpsCB->cameraGetID = camGc0406GetId;
        pSensorOpsCB->cameraCaptureImage = camGc0406CaptureImage;
        pSensorOpsCB->cameraStartPrev = camGc0406PrevStart;
        pSensorOpsCB->cameraPrevNotify = camGc0406PrevNotify;
        pSensorOpsCB->cameraStopPrev = camGc0406StopPrev;
        pSensorOpsCB->cameraGetSensorInfo = camGc0406GetSensorInfo;
        pSensorOpsCB->cameraTestMode = camGc0406TestMode;
        pSensorOpsCB->cameraSetAWB = camGc0406AWB;
        pSensorOpsCB->cameraSetBrightness = camGc0406Brightness;
        pSensorOpsCB->cameraSetContrast = camGc0406Contrast;
        pSensorOpsCB->cameraSetEv = camGc0406Ev;
        pSensorOpsCB->cameraSetImageEffect = camGc0406ImageEffect;
        pSensorOpsCB->cameraFlashSet = camGc0406SetFalsh;
        return true;
    }
    return false;
}
#endif
