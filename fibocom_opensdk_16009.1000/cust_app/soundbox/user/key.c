
#include "key.h"
#include "voice_play.h"
#include "fb_config.h"


#if 0
#if IO_KEY_EN 

UINT8 Key_set_flag=0;
UINT8 Key_set_cnt=0;
UINT16 Key_set_out_time=0;


type_key_ctl power_key_ctl={NO_KEY,NO_KEY,0,0};
type_key_ctl key_ctl1={NO_KEY,NO_KEY,0,0};

type_key_ctl key_mode_ctl={NO_KEY,NO_KEY,0,0};
type_key_ctl key_vol_up_ctl={NO_KEY,NO_KEY,0,0};
type_key_ctl key_vol_down_ctl={NO_KEY,NO_KEY,0,0};
type_key_ctl key_play_ctl={NO_KEY,NO_KEY,0,0};

UINT32 timer_10ms_id;//按键扫描定时器ID

UINT32 key_msg_id=0;    //按键消息ID
int key_timer_flag=0;//按键定时器标志          1:已创建     0:未创建



#define key_msg_count	20
#define key_msg_size	1

//按键事件  +按键数量

static uint8_t key_deal_more(type_key_ctl *key_deal)
{
    uint8_t key_return = NO_KEY;
   // uint8_t key_tmp = NO_KEY;
    if (key_deal->key_val == NO_KEY || key_deal->key_val != key_deal->last_key_val) {
        if (key_deal->key_up < KEY_UP_TIMES) {
            key_deal->key_up++;
        } else {
            if (key_deal->key_cnt >= KEY_LONG_TIMES) {                //长按抬键     >150
			
                    key_return = key_deal->last_key_val | T_KEY_LONG_UP;
            } else if (key_deal->key_cnt >= KEY_SCAN_TIMES) {         //短按抬键  >=6
             
            		
                    key_return = key_deal->last_key_val | T_KEY_SHORT_UP;
            }
          //  key_tmp = key_deal->key_val;
            key_deal->last_key_val = key_deal->key_val;
            key_deal->key_cnt = 0;
        }
    }
	else 
		{
        key_deal->key_cnt++;
        if (key_deal->key_cnt == KEY_SCAN_TIMES) {                    //去抖
            key_deal->key_up = 0;
            key_return = key_deal->key_val;
        } else if (key_deal->key_cnt == KEY_LONG_TIMES) {             //长按

                key_return = key_deal->key_val | T_KEY_LONG;
        } else if (key_deal->key_cnt == KEY_LONG_HOLD_TIMES) 
        {        //连按

                key_return = key_deal->key_val | T_KEY_HOLD;
            key_deal->key_cnt = KEY_LONG_TIMES;
        }
     //   key_tmp = key_deal->key_val;
    }

    return key_return;
}



void timer_10ms_callback(void *arg)
{

    OSI_PRINTFI("houy debug timer_10ms_callback ");
#if IO_KEY_EN
    Io_key_scan();
#endif

}

static void Create_key_timer(void)
{
    OSI_PRINTFI("houy debug key timer flag = %d",key_timer_flag);
	if(key_timer_flag==0)
	{
		key_timer_flag=1;
		timer_10ms_id=fibo_timer_period_new(100,timer_10ms_callback,NULL);//10MS 定时器扫描按键消息
        OSI_PRINTFI("houy debug timer_10ms_id = %d",timer_10ms_id);
	}
}

static void Free_key_timer(void)
{
	if(key_timer_flag==1)
	{
		key_timer_flag=0;
		fibo_timer_free(timer_10ms_id);
	}
}

void Io_key_init(void)
{
      fibo_textTrace(" fides fn: %s line: %d\n", __func__, __LINE__);	
    // KEY_MODE_INIT();
	// KEY_VOL_DOWN_INIT();
	// KEY_VOL_UP_INIT();
	// fibo_setSleepMode(0); 
	//timer_10ms_id=fibo_timer_period_new(10,timer_10ms_callback,NULL);//10MS 定时器扫描按键消息
	Create_key_timer();

}


void Io_key_uninit(void)
{
    // fibo_setSleepMode(1); 
    Free_key_timer();
	
}






void Io_key_scan(void)  //10ms调用
{  

	UINT8 key_val=0;
   // UINT8 power_value=0;
    UINT8 key_mode_value=0;
	UINT8 key_vol_up_value=0;
	UINT8 key_vol_down_value=0;

#if 0
	power_key_ctl.key_val=NO_KEY;
    POWER_KEY_STA(power_value);
    if(power_value==0) {
        ///printf("111/r/n");
        power_key_ctl.key_val=T_KEY_POWER;
    }
    key_val=key_deal_more(&power_key_ctl);

    if(key_val!=NO_KEY){
        fibo_queue_put(key_msg_id,&key_val,0);
    }
#endif

//mode***************************

    key_mode_ctl.key_val=NO_KEY;
    KEY_MODE_STA(key_mode_value);
	
    if(key_mode_value == 0){
        key_mode_ctl.key_val=T_KEY_MODE;
    }
	
    key_val=key_deal_more(&key_mode_ctl);

    if(key_val!=NO_KEY){
        fibo_queue_put(key_msg_id,&key_val,0);
    }


    // vol_up*********************

	key_vol_up_ctl.key_val=NO_KEY;
    KEY_VOL_UP_STA(key_vol_up_value);
    if(key_vol_up_value == 0){
        key_vol_up_ctl.key_val=T_KEY_VOL_UP;
    }
	
    key_val=key_deal_more(&key_vol_up_ctl);

    if(key_val!=NO_KEY){
        fibo_queue_put(key_msg_id,&key_val,0);
    }
	

//vol_down******************************

	 key_vol_down_ctl.key_val=NO_KEY;
  
    KEY_VOL_DOWN_STA(key_vol_down_value);
    if(key_vol_down_value == 0){
        key_vol_down_ctl.key_val=T_KEY_VOL_DOWN;
    }
	
    key_val=key_deal_more(&key_vol_down_ctl);

    if(key_val!=NO_KEY){
        fibo_queue_put(key_msg_id,&key_val,0);
    }



}


uint8_t key_flag=0;

static void io_key_process(void)
{

    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    UINT8	KEY_MSG=0;

    fibo_queue_get(key_msg_id,&KEY_MSG,0);
    OSI_PRINTFI("houy debug key press handle");
    switch(KEY_MSG)
    {

        case  KLU_POWER:
 
            break;
        
        case  KH_POWER:
          
            break; 

        case KL_MODE:

            fb_printf(LEVEL_DEBUG,"zy_K_MODE\r\n");

            break;

        case KU_MODE :

            break;

        case K_VOL_UP:
        {
            int vol_val=0;
            fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);            
            vol_val=fibo_get_volume(AUDIO_PLAY_VOLUME);
            fb_printf(LEVEL_DEBUG,"zy_KU_VOL_UP:%d\r\n",vol_val);

            if(vol_val<VOLUME_MAX_VALUE)
            {
                vol_val++; 
                fb_printf(LEVEL_DEBUG,"zy_vol_val:%d\r\n",vol_val);    
		fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
                fibo_set_volume(AUDIO_PLAY_VOLUME,vol_val);  
		fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
               // push_audio_play_list("VOL_UP_CN.mp3");          
            }
            else
            {
               // push_audio_play_list("VOL_MAX_CN.mp3");  
            }
        }

            break;

        case K_VOL_DOWN:
        {
            int vol_val=0;
            fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__); 
            vol_val=fibo_get_volume(AUDIO_PLAY_VOLUME);
            fb_printf(LEVEL_DEBUG,"zy_KU_VOL_DOWN :%d\r\n",vol_val);

            if(vol_val>VOLUME_MIN_VALUE)
            {
                vol_val--;
                fb_printf(LEVEL_DEBUG,"zyvol_val:%d\r\n",vol_val);
		fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
                fibo_set_volume(AUDIO_PLAY_VOLUME,vol_val);  
		fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
               // push_audio_play_list("VOL_DOWN_CN.mp3");   
            }
            else
            {
                //push_audio_play_list("VOL_MIN_CN.mp3"); 
            }
        }
            break;

        default:
        
            break;
    }
           ////按键抬起后释放定时器
	UINT8 KEY_MSG_TEMP=KEY_MSG&0xE0;
	if((KEY_MSG_TEMP==T_KEY_SHORT_UP)||(KEY_MSG_TEMP==T_KEY_LONG_UP))
    {	
		Io_key_uninit();
	}	

}


void oc_gpioisrcallback(void *param)
{ 
    OSI_PRINTFI("houy debug key pressed");
    // Io_key_init();
    Create_key_timer();
}

oc_isr_t g_oc_ist_prt={
    .is_debounce=true,	 // debounce enabled
    .intr_enable=true,	//interrupt enabled, only for GPIO input
    .intr_level=false,	  // true for level interrupt, false for edge interrupt
    .intr_falling=false,	//falling edge or level low interrupt enabled
    .inte_rising=true,	//rising edge or level high interrupt enabled
    .callback= oc_gpioisrcallback,	  //interrupt callback
};

static void fb_key_init(void)
{


	//mode ***********
	fibo_gpio_isr_init(KEY_MODE_PIN,&g_oc_ist_prt);

	//up ***********
	fibo_gpio_isr_init(KEY_VOL_UP_PIN,&g_oc_ist_prt);

	//down ***********
	fibo_gpio_isr_init(KEY_VOL_DOWN_PIN,&g_oc_ist_prt);

	key_msg_id=fibo_queue_create(key_msg_count, key_msg_size);
    OSI_PRINTFI("houy debug key msg id : %d\r\n",key_msg_id);
}


static int fb_key_task_loop()
{
    io_key_process();
    return 0;
}

static void fb_key_task(void *param)
{

    fb_key_init();

    while(1)
    {
        fb_key_task_loop();
    } 

    fibo_thread_delete();

}
int fb_key_enter(void)
{
    return fibo_thread_create(fb_key_task,"_fb_key_task", 1024*32, NULL, OSI_PRIORITY_NORMAL);
}


#endif /*IO_KEY_EN*/
#endif

#include "fb_config.h"
#include "key.h"
#include "voice_play.h"

#if IO_KEY_EN

UINT8 Key_set_flag = 0;
UINT8 Key_set_cnt = 0;
UINT16 Key_set_out_time = 0;

type_key_ctl power_key_ctl = {NO_KEY, NO_KEY, 0, 0};
type_key_ctl key_ctl1 = {NO_KEY, NO_KEY, 0, 0};

type_key_ctl key_mode_ctl = {NO_KEY, NO_KEY, 0, 0};
type_key_ctl key_vol_up_ctl = {NO_KEY, NO_KEY, 0, 0};
type_key_ctl key_vol_down_ctl = {NO_KEY, NO_KEY, 0, 0};
type_key_ctl key_play_ctl = {NO_KEY, NO_KEY, 0, 0};

UINT32 timer_10ms_id;//按键扫描定时器ID

UINT32 key_msg_id = 0;  // 按键消息ID
int key_timer_flag = 0; // 按键定时器标志          1:已创建     0:未创建


#define key_msg_count 20
#define key_msg_size 1
void *g_period_timer = NULL;


// 按键事件  +按键数量
//  return 0xff 按键抖动处理 未触发按键功能，退出释放定时器
static uint8_t key_deal_more(type_key_ctl *key_deal)
{
    uint8_t key_return = NO_KEY;
    // uint8_t key_tmp = NO_KEY;
    if (key_deal->key_val == NO_KEY || key_deal->key_val != key_deal->last_key_val)
    {
        PLAT_LOG("key_deal->key_cnt if = %d", key_deal->key_cnt);
        if (key_deal->key_up < KEY_UP_TIMES)
        {
            key_deal->key_up++;
        }
        else
        {
            if (key_deal->key_cnt >= KEY_LONG_TIMES)
            { // 长按抬键     >150

                key_return = key_deal->last_key_val | T_KEY_LONG_UP;
            }
            else if (key_deal->key_cnt >= KEY_SCAN_TIMES)
            { // 短按抬键  >=6

                key_return = key_deal->last_key_val | T_KEY_SHORT_UP;
            }
            // else if (key_deal->key_cnt < KEY_SCAN_TIMES)
            // {
            //     key_return = 0xff; //消抖未触发
            // }
            //  key_tmp = key_deal->key_val;
            key_deal->last_key_val = key_deal->key_val;
            key_deal->key_cnt = 0;
        }
    }
    else
    {
        PLAT_LOG("key_deal->key_cnt else = %d", key_deal->key_cnt);
        key_deal->key_cnt++;
        if (key_deal->key_cnt == KEY_SCAN_TIMES)
        { // 去抖
            key_deal->key_up = 0;
            key_return = key_deal->key_val;
        }
        else if (key_deal->key_cnt == KEY_LONG_TIMES)
        { // 长按

            key_return = key_deal->key_val | T_KEY_LONG;
        }
        else if (key_deal->key_cnt == KEY_LONG_HOLD_TIMES)
        { // 连按

            key_return = key_deal->key_val | T_KEY_HOLD;
            key_deal->key_cnt = KEY_LONG_TIMES;
        }
        //   key_tmp = key_deal->key_val;
    }

    return key_return;
}

void timer_10ms_callback(void *arg)
{
       fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
#if IO_KEY_EN
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    Io_key_scan();
#endif
}

void test_timer_period()
{
    g_period_timer = fibo_timer_create(timer_10ms_callback, NULL, false);
    fibo_timer_start(g_period_timer,50, true);
}


static void Create_key_timer(void)
{
//	timer_10ms_id = 0;
	fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
	if(key_timer_flag==0)
	{
		key_timer_flag=1;
		fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
//		timer_10ms_id=fibo_timer_period_new(100,timer_10ms_callback,NULL);//10MS 定时器扫描按键消息
 		test_timer_period();
                fibo_textTrace("fides fn: %s timer_100ms_id=%d line: %d\n", __func__,timer_10ms_id,__LINE__);
	}
}

static void Free_key_timer(void)
{
	if(key_timer_flag==1)
	{
		key_timer_flag=0;
		fibo_timer_free(timer_10ms_id);
	}
}

void Io_key_init(void)
{
	fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);

    KEY_MODE_INIT();
    KEY_VOL_DOWN_INIT();
    KEY_VOL_UP_INIT();
}

void Io_key_uninit(void)
{
    Free_key_timer();
}

void Io_key_scan(void) // 10ms调用
{
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    static int key_time_cnt = 0;
    UINT8 key_val = 0;
    // UINT8 power_value=0;
    UINT8 key_mode_value = 0;
    UINT8 key_vol_up_value = 0;
    UINT8 key_vol_down_value = 0;

#if 0
	power_key_ctl.key_val=NO_KEY;
    POWER_KEY_STA(power_value);
    if(power_value==0) {
        ///printf("111/r/n");
        power_key_ctl.key_val=T_KEY_POWER;
    }
    key_val=key_deal_more(&power_key_ctl);

    if(key_val!=NO_KEY){
        fibo_queue_put(key_msg_id,&key_val,0);
    }
#endif
    // mode***************************
    key_mode_ctl.key_val = NO_KEY;
    KEY_MODE_STA(key_mode_value);

    if (key_mode_value == 1)
    {
        key_mode_ctl.key_val = T_KEY_MODE;
    }

    key_val = key_deal_more(&key_mode_ctl);
    PLAT_LOG("key_val mode value = %d", key_val);

    if (key_val != NO_KEY)
    {
        fibo_queue_put(key_msg_id, &key_val, 0);
    }

    // vol_up*********************
    key_vol_up_ctl.key_val = NO_KEY;
    KEY_VOL_UP_STA(key_vol_up_value);
    if (key_vol_up_value == 1)
    {
        key_vol_up_ctl.key_val = T_KEY_VOL_UP;
    }

    key_val = key_deal_more(&key_vol_up_ctl);
    PLAT_LOG("key_val up value = %d", key_val);

    if (key_val != NO_KEY)
    {
        fibo_queue_put(key_msg_id, &key_val, 0);
    }

    // vol_down******************************
    key_vol_down_ctl.key_val = NO_KEY;

    KEY_VOL_DOWN_STA(key_vol_down_value);
    if (key_vol_down_value == 1)
    {
        key_vol_down_ctl.key_val = T_KEY_VOL_DOWN;
    }

    key_val = key_deal_more(&key_vol_down_ctl);
    PLAT_LOG("key_val down value = %d", key_val);

    if (key_val != NO_KEY)
    {
        fibo_queue_put(key_msg_id, &key_val, 0);
    }

    if ((key_vol_down_ctl.key_val == NO_KEY) && (key_vol_up_ctl.key_val == NO_KEY) && (key_mode_ctl.key_val == NO_KEY))
    {
        key_time_cnt++;
        if (key_time_cnt > KEY_UP_TIMES + 5)
        {
            PLAT_LOG("no key available");
            key_time_cnt = 0;
            // free timer
            // Free_key_timer();
            Io_key_uninit();
        }
    }
    else
    {
        key_time_cnt = 0;
    }
    PLAT_LOG("key mode value = %d, volume up value = %d, volume down value = %d", key_mode_value, key_vol_up_value, key_vol_down_value);
}

uint8_t key_flag = 0;

uint8_t volume_list[10] = {0, 10, 20, 30, 40, 60, 70, 80, 90, 100};

uint8_t get_volume_level(uint8_t volume_vla)
{
    uint8_t i;
    for (i = 0; i < 10; i++)
    {
        if (volume_vla == volume_list[i])
        {
            return i;
        }
    }
    return 3;
}

static void io_key_process(void)
{

    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    int ret = 0;
    UINT8 KEY_MSG = 0;
    // bool audio_state;
    // bool tts_state;

    ret = fibo_queue_get(key_msg_id, &KEY_MSG, 0);

    fibo_textTrace("fides fn: %s queue get=%d line: %d\n", __func__,ret, __LINE__);
    // audio_state = fibo_audio_status_get();
    // tts_state = fibo_tts_is_playing();

    switch (KEY_MSG)
    {

    case KLU_POWER:

        break;

    case KH_POWER:

        break;

        /*************************modo key************************/
    case KL_MODE:
        PLAT_LOG("key_event mode long pressed");
        break;

    case KU_MODE:
        PLAT_LOG("key_event mode short press up");
        break;

    case K_MODE:
        PLAT_LOG("key_event mode key pressed");

        break;

    case KLU_MODE:
        PLAT_LOG("key_event mode key long press up");
        break;
        /*************************VOLUME UP key************************/
    case K_VOL_UP:
    {
        int vol_val=0;
        char buf[3];
        vol_val=fibo_get_volume(AUDIO_PLAY_VOLUME);
        PLAT_LOG("zy_KU_VOL_UP:%d\r\n",vol_val);

        if(vol_val<VOLUME_MAX_VALUE)
        {
            vol_val++;
	  //  print_volume(vol_val); 
            PLAT_LOG("zy_vol_val:%d\r\n",vol_val);  
            //buf[]=vol_val;
	    
            fibo_set_volume(AUDIO_PLAY_VOLUME,vol_val);  
            sprintf(buf,"%02d",vol_val);
	   // print_volume(buf);
	  //  push_audio_play_list("VOL_UP_CN.mp3");          
        }
        else
        {
           // push_audio_play_list("VOL_MAX_CN.mp3");  
        }
    }
    break;
        /*************************VOLUME DOWN key************************/
    case K_VOL_DOWN:
    {
        int vol_val=0;
	char buf[3];
        vol_val=fibo_get_volume(AUDIO_PLAY_VOLUME);
        PLAT_LOG("zy_KU_VOL_DOWN :%d\r\n",vol_val);

        if(vol_val>VOLUME_MIN_VALUE)
        {
            vol_val--;
	   // print_volume(vol_val);
            PLAT_LOG("zyvol_val:%d\r\n",vol_val);
            fibo_set_volume(AUDIO_PLAY_VOLUME,vol_val);
	   sprintf(buf,"%02d",vol_val);  
           // print_volume(buf);
	   // push_audio_play_list("VOL_DOWN_CN.mp3");   
        }
        else
        {
            //push_audio_play_list("VOL_MIN_CN.mp3"); 
        }
    }
    break;

    default:

        break;
    }
}

void oc_gpioisrcallback(void *param)
{
    // Io_key_init();
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    Create_key_timer();
}

static void fb_key_init(void)
{
     int ret=0, ret1=0;
      fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    oc_isr_t g_oc_ist_prt = {
        .is_debounce = true,            // debounce enabled
        .intr_enable = true,            // interrupt enabled, only for GPIO input
        .intr_level = false,            // true for level interrupt, false for edge interrupt
        .intr_falling = false,           // falling edge or level low interrupt enabled
        .inte_rising = true,           // rising edge or level high interrupt enabled
        .callback = oc_gpioisrcallback, // interrupt callback
    };

    // mode ***********
    fibo_gpio_isr_init(KEY_MODE_PIN, &g_oc_ist_prt);

    // up ***********
    ret = fibo_gpio_isr_init(KEY_VOL_UP_PIN, &g_oc_ist_prt);

    fibo_textTrace("fides fn: %s ret=%d line: %d\n", __func__,ret,__LINE__);

    // down ***********
    ret1 = fibo_gpio_isr_init(KEY_VOL_DOWN_PIN, &g_oc_ist_prt);

    fibo_textTrace("fides fn: %s ret1=%d line: %d\n", __func__,ret1,__LINE__);
    key_msg_id = fibo_queue_create(key_msg_count, key_msg_size);
    
    fibo_textTrace("fides fn: %s key_msg_id=%d line: %d\n", __func__,key_msg_id,__LINE__);
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
}

static int fb_key_task_loop()
{
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    io_key_process();
    return 0;
}

static void fb_key_task(void *param)
{
    PLAT_LOG("key task enter");
    fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
    fb_key_init();

    while (1)
    {
        fibo_textTrace("fides fn: %s line: %d\n", __func__, __LINE__);
        fb_key_task_loop();
    }

    fibo_thread_delete();
}

int fb_key_enter(void)
{
    // fibo_set_volume_external(AUDIO_PLAY_VOLUME, 100);
    return fibo_thread_create(fb_key_task, "_fb_key_task", 1024 * 10, NULL, OSI_PRIORITY_NORMAL);
}

#endif /*IO_KEY_EN*/
