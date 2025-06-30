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

#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"
SPIHANDLE spiFd;
uint8_t FlashId[4] = {0};
uint8_t dummydata [4] = {0};  // JedecdeviceID+ 3 dummydata
SPI_IOC spi_ioc = {8, 1000000, SPI_I2C_CS0, SPI_CS_ACTIVE_LOW};

static void hello_world_demo(void *param)
{
/*
    29--cs1
    30--di-1    miso
    31--dio_0    mosi
    32--clk
    33--cs0 不能配置复用cs0
*/
    //fibo_gpio_mode_set(29,4); //用cs1 SPI_I2C_CS1
    //fibo_gpio_mode_set(30,4);
    //fibo_gpio_mode_set(31,4);
    //fibo_gpio_mode_set(32,4);
    //fibo_pmu_ctrl(1, true, true);  // open sd power domain
    //fibo_pmu_setlevel(1, 0);  // set sd 0=1.8V 1=3.2

/*
    37--cs0
    38--dio_0 mosi
    39--miso
    40--clk
*/
    fibo_gpio_mode_set(37,2);
    fibo_gpio_mode_set(38,2);
    fibo_gpio_mode_set(39,2);
    fibo_gpio_mode_set(40,2);
    
    dummydata[0] = 0x9f;
    drvSpiConfig_t cfg = {0};
    cfg.name = DRV_NAME_SPI2;
    cfg.inputEn = true;
    cfg.baud = 1000000;
    cfg.cpol = SPI_CPOL_LOW;
    cfg.cpha = SPI_CPHA_1Edge;
    cfg.input_sel = SPI_DI_1;
    cfg.transmode = SPI_DIRECT_POLLING;
    cfg.cs_polarity0 = SPI_CS_ACTIVE_LOW;
    cfg.framesize = 8;
    //The clock source must be greater than or equal to spi baud*2
    cfg.clk_source = CLK_SOURCE_XTAL26;
    //CLK_SOURCE_RTC32,        32K  clk_source
    //CLK_SOURCE_XTAL26,       26M  clk_source
    //CLK_SOURCE_RC26_26,      26M  clk_source
    //CLK_SOURCE_GNSSPLL_133m, 133M clk_source GNSS used
    //CLK_SOURCE_APLL_167,     167M clk_source

    fibo_spi_open(cfg, &spiFd);
    //fibo_spi_xfer(spiFd, spi_ioc,dummydata, FlashId, 4);
    //fibo_spi_send(spiFd, spi_ioc, dummydata, 4);
    fibo_spi_send_rcv(spiFd, spi_ioc, dummydata, FlashId, 4);
    fibo_textTrace("testspi read id %02x, %02x, %02x, %02x", FlashId[0], FlashId[1], FlashId[2], FlashId[3]); 



    fibo_thread_delete();
}



int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);


    fibo_thread_create(hello_world_demo, "spi_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
