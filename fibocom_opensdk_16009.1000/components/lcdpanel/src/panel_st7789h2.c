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

// #define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG

#include "drv_lcd_defs.h"
#include "osi_api.h"
#include "osi_log.h"
#include "drv_config.h"

#define LCD_CMD_READ_ID (0x04)

static void prvSt7789h2SetDir(void *d, drvLcdDirection_t dir)
{
    // MY MX MV ML BGR MH x x
    uint8_t ctrl = 0x0; // default value except MY/MX/MV
    if (dir & 1)
        ctrl |= 0x80;
    if (dir & 2)
        ctrl |= 0x40;
    if (dir & 4)
        ctrl |= 0x20;

    drvLcdWriteCmd(d, 0x36); // memory access control
    drvLcdWriteData(d, ctrl);
}

static void prvSt7789h2Init(void *d)
{
    OSI_LOGI(0x100089cb, "lcd:    _st7789h2Init ");
    const drvLcdPanelDesc_t *desc = drvLcdGetDesc(d);
    drvLcdWriteCmd(d, 0x11); //Sleep out

    drvLcdWriteCmd(d, 0x36); // Memory data access control
    prvSt7789h2SetDir(d, desc->dir);

    drvLcdWriteCmd(d, 0x3a);  // interface pixedl format
    drvLcdWriteData(d, 0x05); //RGB565, 16 bits, 65k rgb?
    if (desc->line_mode == DRV_LCD_SPI_3WIRE_2LANE)
    {
        drvLcdWriteCmd(d, 0xe7);
        drvLcdWriteData(d, 0x10); //enable two dataline
    }
    drvLcdWriteCmd(d, 0xb2);  // porch setting
    drvLcdWriteData(d, 0x0c); //default:0x0c	0x7f// back porch
    drvLcdWriteData(d, 0x0c); //default:0x0c	0x7f//front porch
    drvLcdWriteData(d, 0x00); // Disable seperate porch control
    drvLcdWriteData(d, 0x33); // B/F porch in idle mode
    drvLcdWriteData(d, 0x33); // B/F porch in partial mode?

    drvLcdWriteCmd(d, 0xb7);  // gate control
    drvLcdWriteData(d, 0x35); // vgh = 13.26v, vgl = -10.43v

    drvLcdWriteCmd(d, 0xbb);  // VCOM setting
    drvLcdWriteData(d, 0x35); // VCOM=1.425v

    drvLcdWriteCmd(d, 0xc0); // LCM setting?
    drvLcdWriteData(d, 0x2c);

    drvLcdWriteCmd(d, 0xc2); // VDV VRH command enable??
    drvLcdWriteData(d, 0x01);

    drvLcdWriteCmd(d, 0xc3);  // VRH set
    drvLcdWriteData(d, 0x0b); // VAP(GVDD) = 4.1+

    drvLcdWriteCmd(d, 0xc4); // VDV set
    drvLcdWriteData(d, 0x20);

    drvLcdWriteCmd(d, 0xc6);  // frame rate control
    drvLcdWriteData(d, 0x0b); // 0x1f, 39hz; 0x0f, 60hz; 0x0B,69hz

    drvLcdWriteCmd(d, 0xd0);  // power control 1
    drvLcdWriteData(d, 0xa4); // fixed
    drvLcdWriteData(d, 0xa1); // AVDD=6.8, AVCL=-4.8, VDS=2.3

    drvLcdWriteCmd(d, 0xe0); // positive voltage gamma control
    drvLcdWriteData(d, 0xd0);
    drvLcdWriteData(d, 0x00);
    drvLcdWriteData(d, 0x02);
    drvLcdWriteData(d, 0x07);
    drvLcdWriteData(d, 0x0b);
    drvLcdWriteData(d, 0x1a);
    drvLcdWriteData(d, 0x31);
    drvLcdWriteData(d, 0x54);
    drvLcdWriteData(d, 0x40);
    drvLcdWriteData(d, 0x29);
    drvLcdWriteData(d, 0x12);
    drvLcdWriteData(d, 0x12);
    drvLcdWriteData(d, 0x12);
    drvLcdWriteData(d, 0x17);

    drvLcdWriteCmd(d, 0xe1); // negative voltage gamma control
    drvLcdWriteData(d, 0xd0);
    drvLcdWriteData(d, 0x00);
    drvLcdWriteData(d, 0x02);
    drvLcdWriteData(d, 0x07);
    drvLcdWriteData(d, 0x05);
    drvLcdWriteData(d, 0x25);
    drvLcdWriteData(d, 0x2d);
    drvLcdWriteData(d, 0x44);
    drvLcdWriteData(d, 0x45);
    drvLcdWriteData(d, 0x1c);
    drvLcdWriteData(d, 0x18);
    drvLcdWriteData(d, 0x16);
    drvLcdWriteData(d, 0x1c);
    drvLcdWriteData(d, 0x1d);

    //Set_Tear_Scanline
    if (desc->fmark_enabled)
    {
        //Set_Tear_Scanline
        drvLcdWriteCmd(d, 0x44);
        drvLcdWriteData(d, 0x00);
        drvLcdWriteData(d, 0x08);

        //enable fmark ST
        drvLcdWriteCmd(d, 0x35);
        drvLcdWriteData(d, 0x01);
    }
    drvLcdWriteCmd(d, 0x29);
    drvLcdWriteCmd(d, 0x2c);
}

static void prvSt7789h2BlitPrepare(void *d, drvLcdDirection_t dir, const drvLcdArea_t *roi)
{
    OSI_LOGD(0x100089cc, "St7789h2 dir/%d roi/%d/%d/%d/%d", dir, roi->x, roi->y, roi->w, roi->h);

    prvSt7789h2SetDir(d, dir);

    uint16_t left = roi->x;
    uint16_t right = drvLcdAreaEndX(roi);
    uint16_t top = roi->y;
    uint16_t bot = drvLcdAreaEndY(roi);

    drvLcdWriteCmd(d, 0x2a);                 // set hori start , end (left, right)
    drvLcdWriteData(d, (left >> 8) & 0xff);  // left high 8 b
    drvLcdWriteData(d, left & 0xff);         // left low 8 b
    drvLcdWriteData(d, (right >> 8) & 0xff); // right high 8 b
    drvLcdWriteData(d, right & 0xff);        // right low 8 b

    drvLcdWriteCmd(d, 0x2b);               // set vert start , end (top, bot)
    drvLcdWriteData(d, (top >> 8) & 0xff); // top high 8 b
    drvLcdWriteData(d, top & 0xff);        // top low 8 b
    drvLcdWriteData(d, (bot >> 8) & 0xff); // bot high 8 b
    drvLcdWriteData(d, bot & 0xff);        // bot low 8 b

    drvLcdWriteCmd(d, 0x2c); // recover memory write mode
}

static uint32_t prvSt7789h2ReadId(void *d)
{
    uint8_t id[4] = {0, 0, 0, 0};
    drvLcdReadData(d, LCD_CMD_READ_ID, id, 4);

    uint32_t dev_id = (id[3] << 16) | (id[2] << 8) | id[1];
    OSI_LOGI(0x100089cd, "St7789h2 read id: 0x%08x", dev_id);
    return dev_id;
}

static bool prvSt7789h2Probe(void *d)
{
    const drvLcdPanelDesc_t *desc = drvLcdGetDesc(d);

    OSI_LOGI(0x100089ce, "St7789h2 probe");
    return prvSt7789h2ReadId(d) == desc->dev_id;
}

#ifdef CONFIG_FIBOCUS_1081_FEATURE
const drvLcdPanelDesc_t gLcdSt7789h2Desc = {
    .ops = {
        .probe = prvSt7789h2Probe,
        .init = prvSt7789h2Init,
        .blit_prepare = prvSt7789h2BlitPrepare,
    },
    .name = "St7789h2",
    .dev_id = 0x858552,
    .reset_us = 20 * 1000,
    .init_delay_us = 100 * 1000,
    .width = 240,
    .height = 320,
    .out_fmt = DRV_LCD_OUT_FMT_16BIT_RGB565,
    .dir = DRV_LCD_DIR_XINV,
    .line_mode = DRV_LCD_SPI_4WIRE,
    .fmark_enabled = false,
    .fmark_delay = 0x18000,
    .freq = 30 * 1000000,
    .frame_us = (unsigned)(1000000 / 28.0),
};
#else
const drvLcdPanelDesc_t gLcdSt7789h2Desc = {
    .ops = {
        .probe = prvSt7789h2Probe,
        .init = prvSt7789h2Init,
        .blit_prepare = prvSt7789h2BlitPrepare,
    },
    .name = "St7789h2",
    .dev_id = 0x858552,
    .reset_us = 20 * 1000,
    .init_delay_us = 100 * 1000,
    .width = 240,
    .height = 320,
    .out_fmt = DRV_LCD_OUT_FMT_16BIT_RGB565,
    .dir = DRV_LCD_DIR_XINV,
    .line_mode = DRV_LCD_SPI_3WIRE_2LANE,
#ifdef CONFIG_8850_FPGA_BOARD
    .fmark_enabled = false,
#else
    .fmark_enabled = true,
#endif
    .fmark_delay = 0x18000,
#ifdef CONFIG_8850_FPGA_BOARD
    .freq = 10 * 1000000,
    .frame_us = (unsigned)(11200000 / 28.0),
#else
    .freq = 50 * 1000000,
    .frame_us = (unsigned)(1000000 / 28.0),
#endif
};
#endif
