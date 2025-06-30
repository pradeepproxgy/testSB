/* Copyright (C) 2i018 RDA Technologies Limited and/or its affiliates("RDA").
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

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')
#define AUDIO_PATH_SIZE 300

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "oc_audio_demo.h"
//#include "cJSON.h"
//#include "pthread.h"


//va_list mqtt_received_arg;
//extern UINT32 mqtt_msge_received;

/*typedef enum
{
    AUSTREAM_FORMAT_UNKNOWN, ///< placeholder for unknown format
    AUSTREAM_FORMAT_PCM,     ///< raw PCM data
    AUSTREAM_FORMAT_WAVPCM,  ///< WAV, PCM inside
    AUSTREAM_FORMAT_MP3,     ///< MP3
    AUSTREAM_FORMAT_AMRNB,   ///< AMR-NB
    AUSTREAM_FORMAT_AMRWB,   ///< AMR_WB
} auStreamFormat_t;*/ 

// Global variables
/*char audioFilePath[AUDIO_PATH_SIZE];
sem_t audioSemaphore;
pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;*/

//void play_audio_function();

//void separate_digits(const char* value);

extern void test_printf(void);
static void audio_thread_entry(void *param);


#define TEST_FILE_NAME  "/prepack/received_rupees.mp3"
static uint8_t *Testbuff=NULL;

/* void process_message(const char* message) {
    cJSON *json = cJSON_Parse(message);
    if (json == NULL || !cJSON_IsArray(json)) {
        fibo_textTrace("Invalid JSON format\n");
        return;
    }
    
    cJSON *item = cJSON_GetArrayItem(json, 0);  // Assuming message is an array with one object
    if (item == NULL || !cJSON_IsObject(item)) {
        fibo_textTrace("Invalid JSON object\n");
        cJSON_Delete(json);
        return;
    }
    
    cJSON *status = cJSON_GetObjectItem(item, "status");
    if (status != NULL && cJSON_IsNumber(status) && status->valueint == 200) {
        cJSON *data = cJSON_GetObjectItem(item, "data");
        if (data != NULL && cJSON_IsObject(data)) {
            cJSON *message_field = cJSON_GetObjectItem(data, "message");
            if (message_field != NULL && cJSON_IsString(message_field) && strcmp(message_field->valuestring, "Payment received") == 0) {
                cJSON *value = cJSON_GetObjectItem(data, "value");
                if (value != NULL && cJSON_IsString(value)) {
                    // Separate and print each digit of the value
                    separate_digits(value->valuestring);
                }
            }
        }
    }

    cJSON_Delete(json);
}

// Separate and print each digit of the value
void separate_digits(const char* value) {
    fibo_textTrace("Separated digits: ");
    for (int i = 0; i < strlen(value); ++i) {
        if (value[i] != '.') {
            fibo_textTrace("[%c] ", value[i]);
        } else {
            fibo_textTrace("[.] ");
        }
    }
    fibo_textTrace("\n");
}*/

void Callplayend(void)
{
	fibo_textTrace("audiofile enters callplayend");
	if (Testbuff!=NULL)
	{
		free(Testbuff);
		Testbuff=NULL;
	}
	fibo_textTrace("testbuff data = %d", Testbuff);
}
static void StreamPlayTask(void *ctx)
{	
	//uint32_t Read=0;
	//uint32_t Write=0;
    int fd = vfs_open(TEST_FILE_NAME, O_RDONLY);
	if (fd <=0)
	{
		return;
	}
	fibo_textTrace("audiofile open");
    int32_t filesize=vfs_file_size(TEST_FILE_NAME);
	if (filesize<=0)
	{
		return;
	}
	fibo_textTrace("audiofile size=%d",filesize);
	//Write=filesize;	
    Testbuff = (uint8_t *)malloc((filesize+128)*sizeof(uint8_t));
	if (Testbuff==NULL)
	{
		return;
	}
	fibo_textTrace("audiofile buff is avalid");
	int bytes = vfs_read(fd, Testbuff, filesize);
	fibo_textTrace("audiofile read bytes=%d",bytes);
	if (bytes!=filesize)
	{
		return;
	}
    fibo_textTrace("audiofile fibo_audio_stream_play");
    vfs_close(fd);
    fibo_audio_mem_play(3, Testbuff, filesize, Callplayend);
    
    //fibo_audio_stream_play(AUSTREAM_FORMAT_PCM,buff,&Read,&Write,(filesize+128), 20);


    OSI_LOGD(0, "AT_CmdFunc_CAMTask osiThreadExit");
	fibo_textTrace("End of StreamPlayTask Function");
    osiThreadExit();

}

int audio_test(void *param)
{
    OSI_LOGI(0, "audio image enter, param");
    osiThreadCreate("audplay", StreamPlayTask, NULL, OSI_PRIORITY_NORMAL, 2048, 4);
    return 0;
}

static void audio_thread_entry(void *param)
{
	fibo_taskSleep(10 *1000);
	fibo_textTrace("audiofile entered pvrThreadEntry");
	audio_test(NULL);
	OSI_LOGI(0,"fibo_audio_play application thread enter, param 0x%x", param);
	INT8 amrtest[] = "happy.amr";
	INT8 mp3test[] = "received_rupees.mp3";
	
	INT8 wavtest[] = "play.wav";
	INT32 ret;
	INT32 type = 1;

#if 0
    char fname[][128] = {
        //"/FFS/0.mp3",
        "/prepack/1.mp3",
        //"/FFS/2.mp3",
    };
    int32_t num = 0;

    num = sizeof(fname) / sizeof(fname[0]);

    fibo_audio_list_play(fname, num);
#endif

fibo_taskSleep(10 *1000);
	
	/* *.mp3 format audio file play and stop demo*/
	/* *.mp3 format audio file pause and resume please Refer to the above code*/
	fibo_taskSleep(3 *1000);
	fibo_textTrace("audiofile start playing");
	ret = fibo_audio_play(1,TEST_FILE_NAME); // fibo_audio_play(type,mp3test);
	fibo_textTrace("audiofile played");
	osiDebugEvent(0x00001193);
//
	        fibo_textTrace("pvrThreadEntry function Ends_1");

	if(ret < 0)
	{
		osiDebugEvent(0x00001194);
		fibo_textTrace("audiofile play failed");
	}
	
	       // fibo_textTrace("pvrThreadEntry function Ends_2");

	fibo_taskSleep(3 *1000);
	       // fibo_textTrace("pvrThreadEntry function Ends_3");

	ret = fibo_audio_stop();

	       // fibo_textTrace("pvrThreadEntry function Ends_4");

	if(ret < 0)
	{
		osiDebugEvent(0x00001195);
		fibo_textTrace("audiofile play stopped");
	}
	

	fibo_textTrace("pvrThreadEntry function Ends");
	fibo_thread_delete();
}


void create_audio_play(){

fibo_textTrace("TTS play audio : ENTERED");

push_audio_play_list("received_rupees.mp3");
char  tts_buff[100] = {0};
uint8_t rx_len=0; 
char zfb_buf[]="0X31"; //CAN
char wx_buf[]="e5beaee4bfa1e588b0e8b4a6";//微信到帐
char  RMB_BUF[]="e58583";//
//UINT16 gb_char[] ="e58583";
//UINT16* unicode_char[100] = {0};// Variable to store the converted Unicode character
//INT32 ret;

	rx_len=strlen(zfb_buf)+strlen(RMB_BUF)+1;
        HAL_Snprintf(tts_buff,rx_len,"%s%s",zfb_buf,RMB_BUF);

    if(rx_len<=100)
    {
        if(push_tts_play_list(tts_buff)<0)
        {
            fibo_textTrace("TTS play audio : InQueue failed\r\n");
        }
    }
    else
    {
        fibo_textTrace("TTS play audio : Receive data to long max:80 Byte rx_len:%d\r\n",rx_len);
    } 

    fibo_textTrace("TTS play audio :EXIT"); 

}



/*void play_audio_function()
{
	char *topic;
        int8_t qos;
        char *message;
        uint32_t len;
	while(1)
	{
		  fibo_sem_wait(mqtt_msge_received);
		topic = va_arg(mqtt_received_arg, char *);
              qos = va_arg(mqtt_received_arg, int);
              message = va_arg(mqtt_received_arg, char *);
              len = va_arg(mqtt_received_arg, uint32_t);
              char temp[300]={0};
              strncpy(temp,message,len);
              fibo_textTrace("mqttapi recv message :topic:%s, qos=%d message=%s len=%d", topic, qos, temp, len);
              if(strcmp(topic, "ind/raj/bhi/sub")==0){
                      fibo_textTrace("audiofile =%s",temp);
                	   create_audio_play();
                      fibo_textTrace("audiofile  played");}
		memset(&mqtt_received_arg,0,sizeof(va_list));*/
	     /*  const char* message = "[{"
                          "\"status\": 200,"
                          "\"data\": {"
                          "\"value\": \"2345.34\","
                          "\"currency\": \"Rupees\","
                          "\"message\": \"Payment received\""
                          "}"
                          "}]";
    
    			process_message(message); */
//	}
//  }	     
