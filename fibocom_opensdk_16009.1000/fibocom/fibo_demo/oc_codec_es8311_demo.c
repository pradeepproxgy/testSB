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

#define ES8311_I2C_ADDR (0x18)

static uint8_t *gbuff = NULL;
uint32_t sampleRate = 8000; //采样率
uint32_t mode = 1;//play
bool isSlave = 1;
uint16_t hpFlag = 0; //spk
i2c_Handle i2c_handle = 0;


typedef struct
{
    uint8_t addr;
    uint8_t data;
} extDevReg_t;

//This is an example for ES8311.You can modify them.
static const extDevReg_t g_audInitRegList[] =
    {
        {0x45, 0x00},
        {0x01, 0x30},
        {0x02, 0xa0}, //MCLK prediv:6-1  mult:1,8K
        {0x03, 0x10}, //single speed
        {0x16, 0x24}, //ADC
        {0x04, 0x10}, //DAC_OSR
        {0x05, 0x00}, //DIV_ADCCLK DIV_DACCLK
        {0x06, 0x15}, //DIV_BCK:4,codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x05}, //256LRCK=MCLK  salve:inactive
        {0x08, 0xff}, //256LRCK=MCLK  salve:inactive
        {0x09, 0x0c}, //SDP_IN:IIS 16BIT
        {0x0a, 0x0c}, //SDP_OUT:IIS 16BIT

        {0x0b, 0x00}, //Power UP STAGE
        {0x0c, 0x00}, //Power UP STAGE
        {0x10, 0x03}, //AVDD:3V3, power saving
        {0x11, 0x7b}, //internal use, power saving
        {0x13, 0x00},
};

//This is an example for ES8311.You can modify them.
//g_audCloseRegList:  register list when closing.
static const extDevReg_t g_audstandbyRegList[] =
    {
        {0x32, 0x00}, //DAC VOLUME:-95.5DB
        {0x17, 0x00}, //ADC VOLUME:-95.5DB
        {0x0E, 0xFF}, //POWER DOWN ANALOG PGA/ADC
        {0x12, 0x02}, //POWER DOWN DAC
        {0x14, 0x00}, //NO INPUT,PGA:0DB
        {0x0D, 0xFA}, //POWER DOWN CIRCUITS
        {0x15, 0x00}, //ADC
        {0x37, 0x08}, //DAC EQ BYPASS
        {0x02, 0x10}, //pre_div mult
        {0x00, 0x00},
        {0x00, 0x1F}, //reset
        {0x01, 0x30},
        {0x01, 0x00}, //clk off
        {0x45, 0x00},
};

//This is an example for ES8311.You can modify them and add lists,such as
//8k,11.025k,12k,16k,22.05k,24k,32k,44.1k,48k.

static const extDevReg_t g_aud8kRegList[] =
    {
        {0x02, 0xa0}, //MCLK prediv:6-1  mult:1,8K
        {0x06, 0x15}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x05}, //1536LRCK=MCLK
        {0x08, 0xff}, //1536LRCK=MCLK
};

static const extDevReg_t g_aud11kRegList[] =
    {
        {0x02, 0x60}, //MCLK prediv:4-1  mult:1,11.025K,12K
        {0x06, 0x0f}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x03}, //1024LRCK=MCLK
        {0x08, 0xff}, //1024LRCK=MCLK
};

static const extDevReg_t g_aud12kRegList[] =
    {
        {0x02, 0x60}, //MCLK prediv:4-1  mult:1,11.025K,12K
        {0x06, 0x0f}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x03}, //1024LRCK=MCLK
        {0x08, 0xff}, //1024LRCK=MCLK
};

static const extDevReg_t g_aud16kRegList[] =
    {
        {0x02, 0x40}, //MCLK prediv:3-1  mult:1,16K
        {0x06, 0x0b}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x02}, //768LRCK=MCLK
        {0x08, 0xff}, //768LRCK=MCLK
};

static const extDevReg_t g_aud22kRegList[] =
    {
        {0x02, 0x20}, //MCLK prediv:2-1  mult:1,22.05K,24K
        {0x06, 0x07}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x01}, //512LRCK=MCLK
        {0x08, 0xff}, //512LRCK=MCLK
};

static const extDevReg_t g_aud24kRegList[] =
    {
        {0x02, 0x20}, //MCLK prediv:2-1  mult:1,22.05K,24K
        {0x06, 0x07}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x01}, //512LRCK=MCLK
        {0x08, 0xff}, //512LRCK=MCLK
};

static const extDevReg_t g_aud32kRegList[] =
    {
        {0x02, 0x48}, //MCLK prediv:3-1  mult:2,32K
        {0x06, 0x05}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x01}, //256LRCK=MCLK
        {0x08, 0x7f}, //256LRCK=MCLK
};

static const extDevReg_t g_aud44kRegList[] =
    {
        {0x02, 0x00}, //MCLK prediv:1-1  mult:1,44.1K,48K
        {0x06, 0x03}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x00}, //256LRCK=MCLK
        {0x08, 0xff}, //256LRCK=MCLK
};

static const extDevReg_t g_aud48kRegList[] =
    {
        {0x02, 0x00}, //MCLK prediv:1-1  mult:1,44.1K,48K
        {0x06, 0x03}, //DIV_BCK:codecÎªmasterÊ±²ÅÓÐÓÃ
        {0x07, 0x00}, //256LRCK=MCLK
        {0x08, 0xff}, //256LRCK=MCLK
};

static void memory_destory(uint8_t **ptr)
{
    if ((ptr != NULL) && (*ptr != NULL))
    {
        free(*ptr);
        *ptr = NULL;
    }
}

static void ff_i2c_init()
{
    int result = 0;
    static HAL_I2C_BPS_T bsp1_1 = {1, 0};//i2c1,100k

    fibo_gpio_mode_set(56, 0);  // mc665-i2c2
    fibo_gpio_mode_set(52, 0);  // mc665-i2c2

    result = fibo_i2c_open(bsp1_1, &i2c_handle);
    fibo_textTrace("i2c handle is %p", i2c_handle);
    fibo_textTrace("test_i2c open result %d", result);
}

static void ff_i2c_close()
{
    int result = 0;

    result = fibo_i2c_close(&i2c_handle);
    fibo_textTrace("test_i2c close result = %d", result);

}

//keep for debug
static void prvAudWriteOneReg(uint8_t addr, uint8_t data)
{
    drvI2cSlave_t idAddress = {ES8311_I2C_ADDR, addr, 0, false, true};

    if (i2c_handle != 0)
    {
        fibo_textTrace("es8311 i2c write reg1 %x,%x", addr, data);
        fibo_i2c_Write(i2c_handle, idAddress, &data, 1);
        osiDelayUS(10);
    }
    else
    {
        fibo_textTrace("i2c is not open");
    }
}
static bool es8311_WriteRegList(const extDevReg_t *regList, uint16_t len)
{
    uint16_t regCount;
    drvI2cSlave_t wirte_data = {ES8311_I2C_ADDR, 0, 0, false, true};

    if (i2c_handle != 0)
    {
        for (regCount = 0; regCount < len; regCount++)
        {
            fibo_textTrace("es8311 i2c write reg %x,%x", regList[regCount].addr, regList[regCount].data);
            wirte_data.addr_data = regList[regCount].addr;
            uint8_t data = regList[regCount].data;
            if (fibo_i2c_Write(i2c_handle, wirte_data, &data, 1) == 0)
            {
                 osiDelayUS(10);
                if (regList[regCount].addr == 0x0d && regList[regCount].data == 0x01)
                {
                    osiDelayUS(50000);
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        fibo_textTrace("i2c is not open");
    }

    return true;
}

void es8311_cfg_init()
{
    ff_i2c_init();

    fibo_textTrace("codec config start !!!");
    {
        //init
        es8311_WriteRegList(g_audInitRegList, sizeof(g_audInitRegList) / sizeof(extDevReg_t));

        if (isSlave)
        {
            prvAudWriteOneReg(0x00, 0x80);
        }
        else
        {
            prvAudWriteOneReg(0x00, 0xc0);
        }

        prvAudWriteOneReg(0x0d, 0x01); //START UP VMID,power ctrl
        prvAudWriteOneReg(0x01, 0x3f); //MCLKÒýÊ±ÖÓ

        if (mode == 0) //record
        {
            prvAudWriteOneReg(0x0e, 0x02); //PGA/ADC ENABLE
            //prvAudWriteOneReg(0x0f, 0x84);
            prvAudWriteOneReg(0x0f, 0x44);

            prvAudWriteOneReg(0x15, 0x10); //adc vc ramp rate,fade in and fade out
            prvAudWriteOneReg(0x1b, 0x05); //adc hpf coeff
            prvAudWriteOneReg(0x1c, 0x65); //adc eq bypass
        }
        else if (mode == 1)//play
        {
            //prvAudWriteOneReg(0x0a, 0x4c); //SDP_OUT:IIS 16BIT,mute
            prvAudWriteOneReg(0x0e, 0x62);
            //prvAudWriteOneReg(0x0f, 0x7b);
            prvAudWriteOneReg(0x0f, 0x44);

            prvAudWriteOneReg(0x04, 0x20); //DAC_OSR
            prvAudWriteOneReg(0x37, 0x08); //dac vc ramp rate,fade in and fade out:0x48
        }
        else if (mode == 2)
        {
            //prvAudWriteOneReg(0x16, 0x24);
            prvAudWriteOneReg(0x04, 0x20);
            prvAudWriteOneReg(0x0e, 0x02);
            prvAudWriteOneReg(0x0f, 0x44);

            prvAudWriteOneReg(0x15, 0x10);
            prvAudWriteOneReg(0x1b, 0x05);
            prvAudWriteOneReg(0x1c, 0x65);
            prvAudWriteOneReg(0x37, 0x08);
        }

        //samplerate
        switch (sampleRate)
        {
        case 8000:
            es8311_WriteRegList(g_aud8kRegList, sizeof(g_aud8kRegList) / sizeof(extDevReg_t));
            break;
        case 11025:
            es8311_WriteRegList(g_aud11kRegList, sizeof(g_aud11kRegList) / sizeof(extDevReg_t));
            break;
        case 12000:
            es8311_WriteRegList(g_aud12kRegList, sizeof(g_aud12kRegList) / sizeof(extDevReg_t));
            break;
        case 16000:
            es8311_WriteRegList(g_aud16kRegList, sizeof(g_aud16kRegList) / sizeof(extDevReg_t));
            break;
        case 22050:
            es8311_WriteRegList(g_aud22kRegList, sizeof(g_aud22kRegList) / sizeof(extDevReg_t));
            break;
        case 24000:
            es8311_WriteRegList(g_aud24kRegList, sizeof(g_aud24kRegList) / sizeof(extDevReg_t));
            break;
        case 32000:
            es8311_WriteRegList(g_aud32kRegList, sizeof(g_aud32kRegList) / sizeof(extDevReg_t));
            break;
        case 44100:
            es8311_WriteRegList(g_aud44kRegList, sizeof(g_aud44kRegList) / sizeof(extDevReg_t));
            break;
        case 48000:
            es8311_WriteRegList(g_aud48kRegList, sizeof(g_aud48kRegList) / sizeof(extDevReg_t));
            break;
        default:
            es8311_WriteRegList(g_aud8kRegList, sizeof(g_aud8kRegList) / sizeof(extDevReg_t));
            break;
        }
    }
}

void es8311_cfg_close()
{
    fibo_textTrace("codec close !!!");
    es8311_WriteRegList(g_audstandbyRegList, sizeof(g_audstandbyRegList) / sizeof(extDevReg_t));
    ff_i2c_close();
}
void es8311_cfg_output()
{
    if (hpFlag == 1)
    {
        prvAudWriteOneReg(0x13, 0x10);
    }
    else
    {
        prvAudWriteOneReg(0x13, 0x00);
    }

    prvAudWriteOneReg(0x31, 0x00);
    prvAudWriteOneReg(0x12, 0x00);
    prvAudWriteOneReg(0x32, 0xbf);
    prvAudWriteOneReg(0x37, 0x08); //DAC RAMPRATE
}

void es8311_cfg_input()
{
    uint16_t REG14 = ((1 << 4) | 7);
    prvAudWriteOneReg(0x14, REG14);
    prvAudWriteOneReg(0x17, 0xbf);  //0-0xff:-95.5-32db
}

static void codec_thread_entry(void *param)
{
    fibo_textTrace("### 111 ####application thread enter, param 0x%x", param);

    fibo_taskSleep(9000);

    codec_Spec_t codec_info_ptr = {NULL};
    codec_info_ptr.codeci2c_en = 1;
    codec_info_ptr.codec_init = es8311_cfg_init;
    codec_info_ptr.codec_output = es8311_cfg_output;
    codec_info_ptr.codec_input = es8311_cfg_input;
    codec_info_ptr.codec_close = es8311_cfg_close;

    codec_info_ptr.codecen_pinid = 0;
    codec_info_ptr.codecen_mode = 0;

    codec_info_ptr.i2s_sel = 2;
    codec_info_ptr.i2sin_en = 1;

    fibo_extern_codec_cfg(&codec_info_ptr);//codec 配置

    fibo_set_volume(AUDIO_PLAY_VOLUME, 7);

    const char fname[][128]={"/FFS/airply.mp3"};

    fibo_audio_list_play(fname, 1); //播放音频1

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(codec_thread_entry, "codec_custhread", 1024*32, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

