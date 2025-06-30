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

typedef struct {
    auStreamFormat_t format;
    uint8_t *audio_buff;
    uint32_t Wp;
    int32_t fd;
    int32_t size;
}RecordContext_t;
static RecordContext_t grecordctx = {};

static int RmAudiofile(const char *filename)
{
    int lRet=-2;
    char file_fold[8] = "/audio/";
    void *dir = fibo_file_opendir(file_fold);
    if (dir != NULL)
    {
        lRet = 0;
        struct fibo_dirent *ent;
        while ((ent = fibo_file_readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG)
            {
                if (NULL != strstr(ent->d_name,filename))
                {
                    fibo_file_delete(filename);
                    lRet=1;
                    break;
                }
            }
        }
        fibo_file_closedir(dir);
    }
    fibo_textTrace("audio recorder delete exist file %d .", lRet);
    return lRet;
}


static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void recorder_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    RecordContext_t *d = &grecordctx;
    d->format = AUSTREAM_FORMAT_AMRNB;
    d->size = 4096;
    d->audio_buff = NULL;
    d->Wp = 0;
    d->fd = 0;

    int ret = 0;
    int32_t leftspace = 0;
    int32_t Rp_pos = 0;

    //just for test
    int32_t filesize = 0;
    int32_t max_len = 4096*3;

    int test_count = 1;
    char Filename[32] = "/audio/audio_1.amr";

    while (test_count)
    {
        bool audio_status = fibo_audio_status_get();
        if (audio_status)
        {
            fibo_textTrace("audio busy now.");
            return;
        }

        /* to save record to audio_1.pcm */
        //char Filename[32] = "/audio/audio_1.pcm";


        d->audio_buff = (uint8_t *)fibo_malloc(4096);
        if (d->audio_buff == NULL)
        {
            fibo_textTrace("[%s-%d]malloc failed!", __FUNCTION__, __LINE__);
            break;
        }

        // add fs protect,when left 200KB size, can't start record.
        uint32_t availsize = fibo_file_get_free_size();
        if (availsize < (1024 * 100)*2)
        {
            fibo_textTrace("audio record don't have enough memory size, availsize=%d.",availsize);
            break;
        }
            fibo_textTrace("audio record memory size is enough, availsize=%d.",availsize);

        //rm exist file
        if (0 > RmAudiofile(Filename))
        {
            fibo_textTrace("delete file error.");
        }

        /* create new file to save record */
        d->fd = fibo_file_open(Filename, O_RDWR | O_CREAT | O_TRUNC);
        if (!d->fd)
        {
            fibo_textTrace("create file failed");
        }

        ret = fibo_audio_recorder_stream_start(d->format, d->audio_buff, &(d->Wp), d->size, 7);
        //ret = fibo_audio_recorder_stream_start(d->format, d->audio_buff, &(d->Wp), d->size);

        if (ret < 0)
        {
            fibo_textTrace("audio recorder stream start error %d", ret);

            fibo_audio_recorder_stream_stop();
            fibo_free(d->audio_buff);
            d->audio_buff = NULL;
            d->Wp = 0;

            return ;
        }

        fibo_textTrace("audio recorder stream start success");

        while (1)
        {
            if(Rp_pos != d->Wp)
            {
                filesize = fibo_file_get_size(Filename);
                if (filesize >= max_len)
                {
                    fibo_audio_recorder_stream_stop();
                    fibo_free(d->audio_buff);
                    d->audio_buff = NULL;
                    d->Wp = 0;
                    fibo_file_close(d->fd);
                    fibo_textTrace("audio recorder, filesize:%d, max_len:%d.", filesize, max_len);
                    break;
                }

                fibo_textTrace("audio recorder start to save pcm");
                if (Rp_pos < d->Wp)
                {
                    fibo_textTrace("audio recorder read position < write");
                    fibo_file_write(d->fd, &(d->audio_buff[Rp_pos]), (d->Wp - Rp_pos));
                    Rp_pos = d->Wp;
                }
                else
                {
                    fibo_textTrace("audio recorder read position > write");
                    leftspace = d->size - Rp_pos;
                    fibo_textTrace("audio recorder leftspace %d", leftspace);
                    fibo_file_write(d->fd, &(d->audio_buff[Rp_pos]), leftspace);
                    Rp_pos = 0;
                    fibo_file_write(d->fd, &(d->audio_buff[Rp_pos]), (d->Wp - Rp_pos));
                }
            }
            else
            {
                fibo_textTrace("audio recorder,There is no pcm data from mic,wait moment.");
                fibo_taskSleep(2);
                continue;
            }
        }
        fibo_textTrace("audio recorder test count %d\n", test_count);
        test_count--;
        fibo_taskSleep(200);
    }
    fibo_textTrace("audio recorder finished!\n");
    ret = fibo_audio_start_v2(1, Filename, NULL);
    fibo_textTrace("audio play:%d\n", ret);
    while(1)
    {
        fibo_taskSleep(200000);
    }
}

void * appimg_enter(void *param)
{
    fibo_textTrace("recorder application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(recorder_thread_entry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
