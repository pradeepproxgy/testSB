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

static uint8_t *gbuff = NULL;
static void memory_destory(uint8_t **ptr)
{
    if ((ptr != NULL) && (*ptr != NULL))
    {
        free(*ptr);
        *ptr = NULL;
    }
}

static void Callplayend(void)
{
    memory_destory(&gbuff);
}

static void codec_thread_entry(void *param)
{
    fibo_textTrace("### 111 ####application thread enter, param 0x%x", param);

    fibo_taskSleep(9000);
    fibo_pmu_ctrl(2, true, true);//pa_ctrl pin 15 pmu
    fibo_pmu_setlevel(2,0);

    paSpec_t cfg = {NULL};
    cfg._init = NULL;
    cfg._open = NULL;
    cfg._close = NULL;
    cfg.mode = 1;
    cfg.pin_id = 15;
    fibo_external_pa_cfg(&cfg);//pa 配置

    codec_Spec_t codec_info_ptr = {NULL};
    codec_info_ptr.codeci2c_en = 0;
    codec_info_ptr.codec_init = NULL;
    codec_info_ptr.codec_output = NULL;
    codec_info_ptr.codec_input = NULL;
    codec_info_ptr.codec_close = NULL;

    codec_info_ptr.codecen_pinid = 90;
    codec_info_ptr.codecen_mode = 1;

    codec_info_ptr.i2s_sel = 2;
    codec_info_ptr.i2sin_en = 0;

    fibo_extern_codec_cfg(&codec_info_ptr);//codec 配置

    fibo_set_volume(AUDIO_PLAY_VOLUME, 7);

    const char fname[][128]={"/FFS/airply.mp3"};
    int32_t fd = 0;
    int32_t first_file_len = 0;
    int32_t bytes = 0;

    //fibo_audio_list_play(fname, 1); //播放音频1

    //fibo_audio_start_v2(1, "/FFS/airply.mp3", NULL); //播放音频2

    fd = vfs_open(fname[0], O_RDONLY);

    first_file_len = vfs_file_size(fname[0]);

    gbuff = (uint8_t *)malloc(first_file_len);
    if (NULL == gbuff)
    {
        OSI_PRINTFI("[%s %d] malloc failed", __FUNCTION__, __LINE__);
        return ;
    }

    bytes = vfs_read(fd, gbuff, first_file_len);
    if (bytes != first_file_len)
    {
        OSI_PRINTFI("[%s %d] read %s error", __FUNCTION__, __LINE__, fname[0]);
        vfs_close(fd);
        memory_destory(&gbuff);
        return ;
    }

    vfs_close(fd);

    fibo_audio_mem_start(3, gbuff, first_file_len, Callplayend); //播放音频3

    fibo_textTrace("### 222 ####application thread enter, param 0x%x", param);

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(codec_thread_entry, "codec_custhread", 1024*16, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

