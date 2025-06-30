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

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "key.h"
//#include "led.h"
#include "voice_play.h"

#include "mqtt_api.h"
#include "sequeue.h"
#include "network_service.h"
#include "aw87359.h"
#include "fb_config.h"
#include "event_signal.h"
#include "oc_mqtt_demo.h"
#include "oc_audio_demo.h"
//#include "drv_spi.h"

bool is_data_valid(char *temp, int *val, int *pval, uint32_t len)
{
	char ival[5],pvalue[2]={0};
	int ret=0;
	fibo_textTrace("i,pval original = %s",temp);
	ret = sscanf(temp, "%[^.].%[^\r\n]", ival, pvalue);
	fibo_textTrace("i,pval-%s,%s",ival,pvalue);
    	char *str1 = ival;
    	char *str2 = pvalue;

	for (uint32_t i = 0; i < strlen(ival); i++)
        {
                if (isdigit(*str1))
                {
                        // nothing
                }
                else
                {
                        fibo_textTrace("invalid data");
                        return 0;
                }
                str1++;
        }

	if (ret == 2)
	{
		for (uint32_t i = 0; i < strlen(pvalue); i++)
    		{
        	if (isdigit(*str2))
        	{
            		// nothing
       		}
       		else
       		{
            		fibo_textTrace("invalid data");
           		 return 0;
        	}
        	str2++;
   		 }
	}
    	*val = atoi(ival);
    	*pval = atoi(pvalue);
    
	if ((*val > 100000) || (*val <= 0))
    	{
		fibo_textTrace("invalid data");
		return 0;
    	}
    	return 1;
}

void play_ones_audio(int ones)
{
    switch (ones)
    {
    case 1:
        push_audio_play_list("onerss.mp3"); // play "1" audio
        break;
    case 2:
        push_audio_play_list("tworss.mp3"); //play "2" audio
        break;
    case 3:
        push_audio_play_list("threerss.mp3");
        break;
    case 4:
        push_audio_play_list("fourrss.mp3");
        break;
    case 5:
        push_audio_play_list("fiverss.mp3");
        break;
    case 6:
        push_audio_play_list("sixrss.mp3");
        break;
    case 7:
        push_audio_play_list("sevenrss.mp3");
        break;
    case 8:
        push_audio_play_list("eightrss.mp3");
        break;
    case 9:
        push_audio_play_list("ninerss.mp3");
        break;
    default:
        break;
    }
}

void play_tens_audio(int tens, int ones)
{
    if (tens == 1)
    {
        switch (ones)
        {
        case 0:
            push_audio_play_list("tenrss.mp3"); // play "10" audio
            break;
        case 1:
            push_audio_play_list("elevenrss.mp3"); // play "11" audio
            break;
        case 2:
            push_audio_play_list("twelverss.mp3");
            break;
        case 3:
            push_audio_play_list("thirteenrss.mp3");
            break;
        case 4:
            push_audio_play_list("fourteenrss.mp3");
            break;
        case 5:
            push_audio_play_list("fifteenrss.mp3");
            break;
        case 6:
            push_audio_play_list("sixteenrss.mp3");
            break;
        case 7:
            push_audio_play_list("seventeenrss.mp3");
            break;
        case 8:
            push_audio_play_list("eighteenrss.mp3");
            break;
        case 9:
            push_audio_play_list("nineteenrss.mp3");
            break;
        default:
            break;
        }
    }

    else if (tens >= 2)
    {
        switch (tens)
        {
        case 2:
            push_audio_play_list("twentyrss.mp3");
            break;
        case 3:
            push_audio_play_list("thirtyrss.mp3");
            break;
        case 4:
            push_audio_play_list("fortyrss.mp3");
            break;
        case 5:
            push_audio_play_list("fiftyrss.mp3");
            break;
        case 6:
            push_audio_play_list("sixtyrss.mp3");
            break;
        case 7:
            push_audio_play_list("seventyrss.mp3");
            break;
        case 8:
            push_audio_play_list("eightyrss.mp3");
            break;
        case 9:
            push_audio_play_list("ninetyrss.mp3");
            break;
        default:
            break;
        }
    }
    else
    {
        // nothing
    }
}

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

void user_signal_process(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    OSI_PRINTFI("mqttapi RECV SIGNAL:%d", sig);
    char *topic;
    int8_t qos;
    char *message;
    uint32_t len;
    int ret = 1;
    int num = 0, pnum = 0, thousands = 0, hundreds = 0, tens = 0, ones = 0;
    char temp[300] = {0};

    switch (sig)
    {
    case GAPP_SIG_PDP_RELEASE_IND:

        break;

    case GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND: // PDP  异常

        break;

    case GAPP_SIG_PDP_ACTIVE_IND:

        break;

    case GAPP_SIG_CONNECT_RSP:
        // OSI_PRINTFI("mqttapi connect :%s", ret ? "ok" : "fail");

        break;

    case GAPP_SIG_CLOSE_RSP:

        break;

    case GAPP_SIG_SUB_RSP:

        break;

    case GAPP_SIG_UNSUB_RSP:

        break;

    case GAPP_SIG_PUB_RSP:

        break;

    case GAPP_SIG_INCOMING_DATA_RSP:
        topic = va_arg(arg, char *);
        qos = va_arg(arg, int);
        message = va_arg(arg, char *);
        len = va_arg(arg, uint32_t);
        strncpy(temp, message, len);
        fibo_textTrace("mqttapi recv message :topic:%s, qos=%d message=%s len=%d", topic, qos, temp, len);
       /* if (strcmp(topic, "ind/raj/bhi/sub") == 0)
        {
            if (is_data_valid(temp, &num, &pnum ,len))
            {
                push_audio_play_list("received_rupees.mp3");
                if (num == 100000)
                {
                    push_audio_play_list("onelakhrss.mp3"); // play "lakh" audio
                }
		else if(0 < num < 99999) 
                {
                    thousands = (num / 1000);
                    hundreds = (num / 100) % 10;
                    tens = (num / 10) % 10;
                    ones = num % 10;

                    if ((thousands > 0) && (thousands <= 99))
                    {
                        if (thousands >= 9)
                        {
                            int thousands_tens = (thousands / 10) % 10;
                            int thousands_ones = thousands % 10;
                            play_tens_audio(thousands_tens, thousands_ones);
                            if (thousands_tens != 1)
                            {
                                if ((thousands_ones > 0) && (thousands_ones <= 9))
                                {
                                    play_ones_audio(thousands_ones);
                                }
                            }
                        }
                        else
                        {
                            play_ones_audio(thousands);
                        }
                        push_audio_play_list("thousandrss.mp3"); // play "thousand" audio
                    }

                    if ((hundreds > 0) && (hundreds <= 9))
                    {
                        play_ones_audio(hundreds);
                        push_audio_play_list("hundredrss.mp3"); // play "hundreds" audio
                    }

                    if ((tens > 0) && (tens <= 9))
                    {
                        play_tens_audio(tens, ones);
                    }

                    if (tens != 1)
                    {
                        if ((ones > 0) && (ones <= 9))
                        {
                            play_ones_audio(ones);
                        }
                    }
		}	
		if ( 0 < pnum )
		{
     			int p_tens = (pnum / 10) % 10;
     			int p_ones = pnum % 10;
    			//paise
		        push_audio_play_list("paiserss.mp3");
		        fibo_textTrace("paise:");
			if ((p_tens > 0) && (p_tens <= 9))
			{
				play_tens_audio(p_tens, p_ones);
				fibo_textTrace("paise tens %d, %d",p_tens,p_ones);
			}

			if (p_tens != 1)
			{	
				if ((p_ones > 0) && (p_ones <= 9))
				{
					play_ones_audio(p_ones);
			                fibo_textTrace("paise ones %d",p_ones);
				}
			}
		}
            }
            else
            {
                push_audio_play_list("invaliddatarss.mp3");
            }
        }*/
        break;

    case GAPP_SIG_DNS_QUERY_IP_ADDRESS:

        break;

    case GAPP_SIG_PDP_ACTIVE_ADDRESS:

        break;

    case GAPP_SIG_MPING_RECV:

        break;

    case GAPP_SIG_MPINGSTAT_RECV:

        break;

    default:
        break;
    }
    fibo_taskSleep(100);
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{

    OSI_PRINTFI("FIBO <--%xs", buf);

    if (strstr((const char *)buf, "+SIM: Inserted")) // SIM
    {
        //
    }

    if (strstr((const char *)buf, "+SIM READY")) // SIM 已识别
    {
    }

    if (strstr((const char *)buf, "+SIM DROP") || strstr((const char *)buf, "+SIM: Removed")) // SIM 移动
    {
    }

#if TTS_EN
    /*  fibo_textTrace("TTS PREPROCESSOR ENABLED");
      char *playtext[] = {"Hello, this is a TTS test."};
      push_tts_play_list(playtext);
      fibo_textTrace("TTS play list push ok");
      voice_play_init();
      fibo_textTrace("TTS voice play ok");
      voice_play_task();
      voice_play_enter();
      fibo_textTrace("TTS voice play enter ok ");*/
    if (strstr((const char *)buf, "AT+GTTS=")) //
    {
        fibo_textTrace("TTS AT COMMAND with if OK");
        if (strstr((const char *)buf, "OK"))
        {
            fibo_textTrace("TTS AT COMMAND with nestedif OK");
            // TTS播放
        }
    }

    if (strstr((const char *)buf, "+TTS: END")) //
    {
        fibo_textTrace("TTS PREPROCESSOR END");

        // TTS播放完成
    }
#endif
}

static FIBO_CALLBACK_T user_callback =
    {
        .at_resp = at_res_callback,
        .fibo_signal = user_signal_process};

void app_init(void)
{
    int ret = 0;

    fibo_textTrace("zy_app versions:%s\r\n", APP_VERSIONS_INFO);

    // key init
    // #if IO_KEY_EN
    //     Io_key_init();
    // #endif

/*#if LED_EN
    // led init
    led_init();
    fibo_textTrace("MOTOR_LED");
    LOCK_MOTOR_INIT();
    fibo_textTrace("MOTOR_LED_1");
#endif*/

    ret = fibo_filedir_exist("/tmp/");
    if (ret < 0)
    {
        // 默然创建tmp路径 ，应用相关配置文件存放在里面  此路径为模块内部flash路径
        ret = fibo_file_mkdir("/tmp/");
     //   fb_printf("zy_mkdir /tmp/ Path  creat :%d\n", ret);
    }

    ret = fibo_filedir_exist("/voices/");
    if (ret < 0)
    {
        // 默然创建audio路径,音频相关文件放在此路径下
        ret = fibo_file_mkdir("/voices/");
      //  fb_printf("zy_mkdir /voices/ Path  creat :%d\n", ret);
    }

#if AW87359_EN
    AW87359_init();
#endif
}

// #if ALIYUN_PLATFORM_EN
// int ali_iot_enter(void);
// #endif

void app_main_enter(void *param)
{

#if VOICE_PLAY_EN
  // voice_play_enter();
    fibo_textTrace("Failed to create bluescreen");

#endif
    /*	int ret=0xff;
        ret = fibo_thread_create(play_audio_function, "play-audio", 1024 * 16, NULL, OSI_PRIORITY_NORMAL);
    if(ret < 0) {
            fibo_textTrace("Failed to create play audio function thread, error code %d",ret);
        } */

    network_enter(); // 网络维护  注网激活

    fb_event_signal_enter(); // 消息时间处理

/*#if IO_KEY_EN
    // 按键任务入口
    // fb_key_enter();
#endif*/
     fibo_textTrace("Soundbox mqtt connect before");
    // #if ALIYUN_PLATFORM_EN
    // ali_iot_enter();
    // #endif/*ALI_MQTT_EN*/
    create_mqtt_connect();
         fibo_textTrace("Soundbox mqtt connect after");

}

void *appimg_enter(void *param)
{
    const uint8_t *cmd = (const uint8_t *)"ATI\r\n";
    OSI_LOGI(0, "application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();

    fibo_at_send(cmd, strlen((const char *)cmd));

    app_init();
    app_main_enter(param);

    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}

