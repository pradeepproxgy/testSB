#include "voice_play.h"
#include "stdint.h"
#include "fb_uart.h"
#include "stdio.h"
#include "sequeue.h"

// 1：PCM格式
// 2：WAV格式
// 3：MP3格式
// 4：AMR格式



AudioPlayInfo  en_audio_play={0};
sequeue play_list;

int8_t *ad_file_buf=NULL;
int8_t play_state_flag=0;
int voice_task_run=0;

void play_audio_callback(void)
{
    play_state_flag = 0;
}

int fb_play_audio(char *ad_file)
{
    int         len             = 256;
    static char system_str[256] = {0};
    int32_t     ret             = 0;
    
    UINT8 voices_type=3;    // 默认为MP3文件
    UINT8 ad_time_out=0;

    if (ad_file == NULL || strlen(ad_file) == 0) {
        return -1;
    }

    memset(system_str, 0, len);
    snprintf(system_str, len, "/voices/%s", ad_file);
    
    /* check if file exists */
    if(fibo_file_exist(system_str)!=1) 
    {
        return -1;
    }

    if(strstr(system_str,"mp3"))
    {
        voices_type=3;
    }
    else if(strstr(system_str,"wav"))
    {
        voices_type=2;
    }

    int32_t filesize=vfs_file_size(system_str);
    if (filesize<=0)
    {
        return -1;
    }
    ad_file_buf = (int8_t *)fibo_malloc((filesize+128)*sizeof(uint8_t));
    if(ad_file_buf==NULL)
    {
       return -1;
    }

    int fd = vfs_open(system_str, O_RDONLY);
    if (fd <=0)
    {
        if (ad_file_buf!=NULL)
        {
            fibo_free(ad_file_buf);
            ad_file_buf=NULL;
        }
        return -1;
    }

    int bytes = vfs_read(fd, ad_file_buf, filesize);
    if (bytes!=filesize)
    {
        vfs_close(fd);
        if (ad_file_buf!=NULL){
            fibo_free(ad_file_buf);
            ad_file_buf=NULL;
        }
        return-1;
    }

    vfs_close(fd);

    PA_UNMUTE();  
    play_state_flag=1;
    ret=fibo_audio_mem_play(voices_type,(UINT8 *) ad_file_buf, filesize, play_audio_callback);
    if (ret<0)
    {
        if (ad_file_buf != NULL)
        {
            fibo_free(ad_file_buf);
            ad_file_buf = NULL;
        }
        play_state_flag = 0;
        return -1;
    }

    while(play_state_flag)//Wait for ad  play complete
    {
        ad_time_out++;//60s超时退
        if(ad_time_out>(60*10)){
            play_state_flag = 0;

            break;    
        }
        fibo_taskSleep(100);
    }

    ad_time_out=0;
    if (ad_file_buf!=NULL){
        fibo_free(ad_file_buf); 
        ad_file_buf=NULL;
    }
    return 0;
}


AudioPlayInfo get_audio_play_state(void)
{
    return en_audio_play;
}

static void init_play_list(void)
{
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    SetNull(&play_list);
}

int push_play_list(AudioPlayInfo *play_info)
{
    fibo_textTrace("fn: %s line:text queued %d\n", __func__, __LINE__);
    return InQueue(&play_list,play_info); 
}

int pop_play_list(AudioPlayInfo *play_info)
{
    //出队
    return DeQueue(&play_list,play_info);
}

int push_tts_play_list(char *play_text)
{
     fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    AudioPlayInfo tts_play={0};
     fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    memset(&tts_play,0,sizeof(tts_play));
    tts_play.play_state=0;
    tts_play.play_type=1;
    memcpy(tts_play.play_text,play_text,strlen(play_text));
     fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    return push_play_list(&tts_play);//数据需要播放音频

}

int push_audio_play_list(char *play_text)
{
    AudioPlayInfo audio_play={0};

    memset(&audio_play,0,sizeof(audio_play));
    audio_play.play_state=0;
    audio_play.play_type=2;
    OSI_PRINTFI("property id push_audio_play_list[%s]",play_text);
    memcpy(audio_play.play_text,play_text,strlen(play_text));
    return push_play_list(&audio_play);//数据需要播放音频

}



void voice_play_loop(void)
{
	
    int ret=0;
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    AudioPlayInfo audio_play;
    memset(&audio_play,0,sizeof(AudioPlayInfo));
    while (1)
    {
        
        /* code */
        fibo_textTrace("fn: %s while enter line: %d\n", __func__, __LINE__);
        audio_play = get_audio_play_state(); //获取播放状态
       
        if (audio_play.play_state) {
            fibo_taskSleep(500);
            continue;
        }
        if(fibo_tts_is_playing())
        {
	fibo_textTrace("fn: %sPlaying TTS line: %d\n", __func__, __LINE__);
            fibo_taskSleep(500);
            continue;
        }
        //audio_play->play_name
        memset(&audio_play,0,sizeof(AudioPlayInfo));
        ret = pop_play_list(&audio_play);  //获取播放音频
        if(ret>0)
        {
            OSI_PRINTFI("property id  play_type[%d]",audio_play.play_type);
            if(audio_play.play_type==1) //tts
            {
		fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
                ret = fibo_tts_start((const UINT8 *)audio_play.play_text,0);

                //fibo_taskSleep(3*1000);
               OSI_PRINTFI("property id audio_play TTS ret[%d] play_text[%s]",ret,audio_play.play_text);

            }
            else if(audio_play.play_type==2) //audio
            {
		fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
                OSI_PRINTFI("property id  play_text[%s]",audio_play.play_text);
                ret = fb_play_audio(audio_play.play_text);//播放音频  目前此接口播放完成或超时后才会退出
                OSI_PRINTFI("property id fb_play_audio ret[%d]",ret);
                if(ret == 0)
                {
                   fibo_textTrace("fn: %sloop if line: %d\n", __func__, __LINE__);

                }
                else
                {
                    fibo_textTrace("fn: %sloop else line: %d\n", __func__, __LINE__);
                }
            }
            
            //执行完上报播放结果
        }
        fibo_taskSleep(500);

    }
    

}

/**
 * @brief voice_play_init() play voice init.
 *
 * @param[in] none.
 * 
 * @return  none.
 *
 */
#define AUDIO_FILE_PATH      "/voices/"

void voice_play_init(void)
{
    int ret = 0;
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    init_play_list();
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    ret = fibo_filedir_exist(AUDIO_FILE_PATH);
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    if(ret == -1)
    {
	fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
        ret = fibo_file_mkdir(AUDIO_FILE_PATH);
        if(ret < 0)
        {
	 fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
        }
    }

    PA_MUTE_INIT();
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    push_audio_play_list("DEVICE_POWER_UP.mp3");
    
  
}

/**
 * @brief voice_play_task()  play voice task
 *
 * @param[in] arg   play voice task param.
 *
 * @return  On success 0.
 *          On error, -1.
 */
void voice_play_task(void *arg)
{
     fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    voice_play_init();
    voice_task_run=1;
    while(voice_task_run)
    {
        voice_play_loop();
        fibo_taskSleep(1000);
    }
    voice_task_run=0;
    fibo_thread_delete();

}

int voice_play_enter(void)
{
	 fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
   // return fibo_thread_create(voice_play_task,"_audio_play_task", 1024*20, NULL, OSI_PRIORITY_BELOW_NORMAL);
}


