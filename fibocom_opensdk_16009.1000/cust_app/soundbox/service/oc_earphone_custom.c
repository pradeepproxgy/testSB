#include "osi_api.h"
#include "osi_log.h"
#include "app_image.h"
#include "oc_earphone_custom.h"
#include "fibo_opencpu.h"
#include "sequeue.h"

#define MAX_FILES 40 // Maximum number of MP3 files

AudioPlayInfo  en_audio={0};

// static int current_file_index = 0;  // Track current file index
sequeue play_audio;

static uint8_t *Testbuff = NULL;
int8_t play_state_flag=0;

int voice_task_run=0;

static const char *mp3_files[MAX_FILES] = {
    "/FFS/receivedrupees.mp3",
    "/FFS/onerss.mp3",
    "/FFS/tworss.mp3",
    "/FFS/threerss.mp3",
    "/FFS/fourrss.mp3",
    "/FFS/fiverss.mp3",
    "/FFS/sixrss.mp3",
    "/FFS/sevenrss.mp3",
    "/FFS/eightrss.mp3",
    "/FFS/ninerss.mp3",
    "/FFS/tenrss.mp3",
    "/FFS/elevenrss.mp3",
    "/FFS/twelverss.mp3",
    "/FFS/thirteenrss.mp3",
    "/FFS/fourteenrss.mp3",
    "/FFS/fifteenrss.mp3",
    "/FFS/sixteenrss.mp3",
    "/FFS/seventeenrss.mp3",
    "/FFS/eighteenrss.mp3",
    "/FFS/nineteenrss.mp3",
    "/FFS/twentyrss.mp3",
    "/FFS/thirtyrss.mp3",
    "/FFS/fortyrss.mp3",
    "/FFS/fiftyrss.mp3",
    "/FFS/sixtyrss.mp3",
    "/FFS/seventyrss.mp3",
    "/FFS/eightyrss.mp3",
    "/FFS/ninetyrss.mp3",
    "/FFS/hundredrss.mp3",
    "/FFS/thousandrss.mp3",
    "/FFS/paiserss.mp3",
    "/FFS/invaliddatarss.mp3",
    "/FFS/onelakhrss.mp3",
    "/FFS/power.mp3",
    "/FFS/PLEASE_INSERT_SIM.mp3",
    "/FFS/NET_CONNECT_FAIL.mp3",
    "/FFS/NET_CONNECTED.mp3"
};

AudioPlayInfo get_audio_play_state(void)
{
    return en_audio;
}

// void play_next_file(void); // Forward declaration
static void init_mp3_play_list(void)
{
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    SetNull(&play_audio);
}

int push_mp3_play_list(AudioPlayInfo *audio_info)
{
    fibo_textTrace("fn: %s line:play info queued %d\n",__func__, __LINE__);
    return InQueue(&play_audio, audio_info);
}

int pop_mp3_play_list(AudioPlayInfo *audio_info)
{
    fibo_textTrace("fn: %s line:play info dequeued %d\n",__func__, __LINE__);
    return DeQueue(&play_audio, audio_info);
}

// Callback function after each file playback ends
void Callplayend(void)
{
	play_state_flag = 0;
        if (Testbuff != NULL)
    {
        fibo_free(Testbuff);
        Testbuff = NULL;
    }
	
}
int push_index(uint8_t index)
{
    AudioPlayInfo audio_play={0};

    memset(&audio_play,0,sizeof(audio_play));
    audio_play.play_state=0;
    audio_play.play_type=2;
    //OSI_PRINTFI("property id push_audio_play_list[%s]",play_text);
    memcpy(&audio_play.play_text[0],&index,sizeof(uint8_t));
    return push_mp3_play_list(&audio_play);
}

void audio_play_loop(void)
{
	int ret=0;
    fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
    AudioPlayInfo audio_play;
    memset(&audio_play,0,sizeof(AudioPlayInfo));
   while(1){    
 fibo_textTrace("fn: %s while enter line: %d\n", __func__, __LINE__);
 fibo_textTrace("audio_play_loop_while.");
        audio_play = get_audio_play_state(); //获取播放状态

                if (audio_play.play_state) {
            fibo_taskSleep(500);
            continue;
        }

                //audio_play->play_name
        memset(&audio_play,0,sizeof(AudioPlayInfo));
            ret = pop_mp3_play_list(&audio_play);
            if (ret > 0)
            {
             if(audio_play.play_type==2) //audio
            {
                fibo_textTrace("Before_mp3play_call_Value: %d\n", audio_play.play_text[0]);
                play_audio_mp3_file(audio_play.play_text[0]);
		fibo_textTrace("After_mp3play_call_Value: %d\n", audio_play.play_text[0]);
            }
            else{
                // nothing
            }
            }
            fibo_taskSleep(500);
      }

}
// Function to play the current MP3 file
void play_audio_mp3_file(uint8_t current_file_index)
{
	 fibo_textTrace("current_file_index=%d", current_file_index);
    const char *file_name = mp3_files[current_file_index];
	 int32_t ret = 0;
	 UINT8 ad_time_out=0;
    if (1 == fibo_file_exist(file_name))
    {
        fibo_textTrace("File exists: %s", file_name);

        int32_t file_len = fibo_file_get_size(file_name);
        INT32 fd = fibo_file_open(file_name, O_RDONLY);

        Testbuff = (unsigned char *)fibo_malloc(file_len);
	fibo_textTrace("Testbuff_Before_memset=%d",*Testbuff);
        memset(Testbuff, 0, file_len);
	fibo_textTrace("Testbuff_After_memset=%d",*Testbuff);
        fibo_file_read(fd, Testbuff, file_len);
	fibo_textTrace("Testbuff_After_file_read=%d",*Testbuff);
        fibo_file_close(fd);
        play_state_flag=1;
        ret=fibo_audio_mem_start(AUSTREAM_FORMAT_MP3, Testbuff, file_len, Callplayend);
 //       ret=fibo_audio_mem_start(2, Testbuff, file_len, Callplayend);

	fibo_textTrace("fib_audio_mem_start ret value =%d",ret);
	if (ret<0)
    {
        if (Testbuff != NULL)
        {
            fibo_free(Testbuff);
            Testbuff = NULL;
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
    if (Testbuff!=NULL){
        fibo_free(Testbuff);
        Testbuff=NULL;
    }
	
    }
    else
    {
        fibo_textTrace("File does not exist: %s", file_name);
   }
    return 0;
}

// Main demo function
static void audio_play_thread(void *arg)
{
   fibo_textTrace("Application thread started.");
    
    init_mp3_play_list();
    

    voice_task_run=1;
    while(voice_task_run)
    {
	 fibo_textTrace("voice_task_run_while.");
         audio_play_loop();
	fibo_taskSleep(1000);
    }
        voice_task_run=0;
    fibo_thread_delete();
}

void audio_play_enter(void)
{
    fibo_textTrace("audioplay_enter._suc");
  fibo_thread_create(audio_play_thread, "mythread", 10 * 1024, NULL, OSI_PRIORITY_NORMAL);
}
