
#include "led.h"
#include "fb_config.h"

#if LED_EN

_LED_STRUCT led_struct;

void led_scan(void) //100ms  定时器调用
{

#if LED_RED_EN                 
//red
    if(led_struct.red_flash_flag){
        led_struct.red_led_cnt++;
        if(led_struct.red_led_cnt<=led_struct.red_on_time){
            if(led_struct.red_flag==0){
                led_struct.red_flag=1;
                LED_R_ON();
            }        
        }
        else{  
            if(led_struct.red_flag==1){
                led_struct.red_flag=0;
                LED_R_OFF();  
            }

            if(led_struct.red_led_cnt>(led_struct.red_off_time+led_struct.red_on_time)){
                led_struct.red_led_cnt=0;
            }
        }
    }
#endif

#if LED_BLUE_EN
    //blue
    if(led_struct.blue_flash_flag){
        led_struct.blue_led_cnt++;
        if(led_struct.blue_led_cnt<=led_struct.blue_on_time){
            if(led_struct.blue_flag==0){
                led_struct.blue_flag=1;
                LED_B_ON();
            }        
        }
        else{  
            if(led_struct.blue_flag==1){
                led_struct.blue_flag=0;
                LED_B_OFF();  
            }

            if(led_struct.blue_led_cnt>(led_struct.blue_off_time+led_struct.blue_on_time)){
                led_struct.blue_led_cnt=0;
            }
        }
    }
#endif
#if LED_GREEN_EN
    //green
    if(led_struct.green_flash_flag){
        led_struct.green_led_cnt++;
        if(led_struct.green_led_cnt<=led_struct.green_on_time){
            if(led_struct.green_flag==0){
                led_struct.green_flag=1;
                LED_G_ON();
            }        
        }
        else{  
            if(led_struct.green_flag==1){
                led_struct.green_flag=0;
                LED_G_OFF();  
            }

            if(led_struct.green_led_cnt>(led_struct.green_off_time+led_struct.green_on_time)){
                led_struct.green_led_cnt=0;
            }
        }
    }
#endif
}

UINT32 timer_100ms_id=0;//按键扫描定时器ID
UINT8 led_timer_flag=0;//按键定时器标志          1:已创建     0:未创建


void timer_100ms_callback(void *arg)
{
    led_scan();
}

static void Create_led_timer(void)
{
    if(led_timer_flag==0)
    {
        led_timer_flag=1;
        timer_100ms_id=fibo_timer_period_new(100,timer_100ms_callback,NULL);//10MS 定时器扫描按键消息
    }
}

static void Free_led_timer(void)
{
    
    if((led_struct.red_flash_flag==0)&&(led_struct.blue_flash_flag==0)&&(led_struct.green_flash_flag==0))
    {
        if(led_timer_flag==1)
        {
            led_timer_flag=0;
            fibo_timer_free(timer_100ms_id);
        }
    }
}

void led_io_all_off(void)
{
#if LED_RED_EN
    LED_R_OFF();
#endif

#if LED_BLUE_EN
    LED_B_OFF();
#endif
    
#if LED_GREEN_EN
    LED_G_OFF();
#endif
}



//on_time      led开启时间    步长为led_scan调用速度  送
//off_time     led关闭时间   步长为led_scan调用速度
void led_set(_LED_COLOUR led_colour,UINT16 on_time,UINT16 off_time)  //
{

    led_io_all_off();
    switch(led_colour)
    {
        case LED_RED:
           
            led_struct.red_on_time=on_time; 
            led_struct.red_off_time=off_time;
            led_struct.red_led_cnt=0;
            led_struct.red_flag=0;
            //LED_R_OFF(); 
            break;

        case LED_BLUE: 
     
            led_struct.blue_on_time=on_time; 
            led_struct.blue_off_time=off_time;
            led_struct.blue_led_cnt=0;
            led_struct.blue_flag=0;
           // LED_B_OFF(); 
            break;

        case LED_GREEN: 
       
            led_struct.green_on_time=on_time; 
            led_struct.green_off_time=off_time;
            led_struct.green_led_cnt=0;
            led_struct.green_flag=0;
            //LED_G_OFF(); 
        break;
       
    }
}

//只设置需要设置的led参数 不影响其他 
void led_only_one_set(_LED_COLOUR led_colour,UINT16 on_time,UINT16 off_time)  //
{

    
    switch(led_colour)
    {
        case LED_RED:
           
            led_struct.red_on_time=on_time; 
            led_struct.red_off_time=off_time;
            led_struct.red_led_cnt=0;
            led_struct.red_flag=0;
            break;

        case LED_BLUE: 
     
            led_struct.blue_on_time=on_time; 
            led_struct.blue_off_time=off_time;
            led_struct.blue_led_cnt=0;
            led_struct.blue_flag=0;
            break;

        case LED_GREEN: 
       
            led_struct.green_on_time=on_time; 
            led_struct.green_off_time=off_time;
            led_struct.green_led_cnt=0;
            led_struct.green_flag=0;
        break;
       
    }
}

void led_only_red_set(UINT16 on_time,UINT16 off_time)
{

    led_only_one_set(LED_RED,on_time,off_time);
    led_struct.red_flash_flag=1;
    Create_led_timer();
}

void led_only_blue_set(UINT16 on_time,UINT16 off_time)
{

    led_only_one_set(LED_BLUE,on_time,off_time);
    led_struct.blue_flash_flag=1;
    Create_led_timer();
}

void led_only_green_set(UINT16 on_time,UINT16 off_time)
{
    led_only_one_set(LED_GREEN,on_time,off_time);
    led_struct.green_flash_flag=1;
    Create_led_timer();
}


void led_only_red_on(void)
{
    
    Free_led_timer();
    led_only_one_set(LED_RED,0xFFFF,0);
    led_struct.red_flash_flag=0;
    LED_R_ON();
}

void led_only_blue_on(void)
{
  
    Free_led_timer();
    led_only_one_set(LED_BLUE,0xFFFF,0);
    led_struct.blue_flash_flag=0;
    LED_B_ON();
   
}


void led_only_green_on(void)
{
    Free_led_timer();
   
    led_only_one_set(LED_GREEN,0xFFFF,0);
    led_struct.green_flash_flag=0;
    #if LED_GREEN_EN
        LED_G_ON();
    #endif
}


void led_only_red_off(void)
{
    Free_led_timer();
    led_only_one_set(LED_RED,0,0xFFFF);
    led_struct.red_flash_flag=0;
    #if LED_RED_EN
        LED_R_OFF();
    #endif
}

void led_only_blue_off(void)
{
    Free_led_timer();
    led_only_one_set(LED_BLUE,0,0xFFFF);
    led_struct.blue_flash_flag=0;
    #if LED_BLUE_EN
    LED_B_OFF();
    #endif
}

void led_only_green_off(void)
{
    Free_led_timer();
    led_only_one_set(LED_GREEN,0,0xFFFF);
    led_struct.green_flash_flag=0;
    #if LED_GREEN_EN
    LED_G_OFF();
    #endif
}



void led_red_set(UINT16 on_time,UINT16 off_time)
{

    led_set(LED_RED,on_time,off_time);
    led_set(LED_GREEN,0,0xFFFF);
    led_set(LED_BLUE,0,0xFFFF);
    led_struct.red_flash_flag=1;
    led_struct.blue_flash_flag=0;
    led_struct.green_flash_flag=0;
    Create_led_timer();
}


void led_blue_set(UINT16 on_time,UINT16 off_time)
{

    led_set(LED_BLUE,on_time,off_time);
    led_set(LED_GREEN,0,0xFFFF);
    led_set(LED_RED,0,0xFFFF);
    led_struct.red_flash_flag=0;
    led_struct.blue_flash_flag=1;
    led_struct.green_flash_flag=0;
    Create_led_timer();
}

void led_green_set(UINT16 on_time,UINT16 off_time)
{
    led_set(LED_GREEN,on_time,off_time);
    led_set(LED_RED,0,0xFFFF);
    led_set(LED_BLUE,0,0xFFFF);
    led_struct.red_flash_flag=0;
    led_struct.blue_flash_flag=0;
    led_struct.green_flash_flag=1;
    Create_led_timer();

}


void led_red_on(void)
{
    
    Free_led_timer();
    led_set(LED_RED,0xFFFF,0);
    led_struct.red_flash_flag=0;
    #if LED_RED_EN
    LED_R_ON();
    #endif
}

void led_blue_on(void)
{
  
    Free_led_timer();
    led_set(LED_BLUE,0xFFFF,0);
    led_struct.blue_flash_flag=0;
    #if LED_BLUE_EN
    LED_B_ON();
    #endif
   
}


void led_green_on(void)
{
    Free_led_timer();
    led_set(LED_GREEN,0xFFFF,0);
    led_struct.green_flash_flag=0;
    #if LED_GREEN_EN
    LED_G_ON();
    #endif
}


void led_red_off(void)
{
    Free_led_timer();
    led_set(LED_RED,0,0xFFFF);
    led_struct.red_flash_flag=0;
    #if LED_RED_EN
    LED_R_OFF();
    #endif
}

void led_blue_off(void)
{
    Free_led_timer();
    led_set(LED_BLUE,0,0xFFFF);
    led_struct.blue_flash_flag=0;
    #if LED_BLUE_EN
    LED_B_OFF();
    #endif
}

void led_green_off(void)
{
    Free_led_timer();
    led_set(LED_GREEN,0,0xFFFF);
    led_struct.green_flash_flag=0;
    #if LED_GREEN_EN
    LED_G_OFF();
    #endif
}


void led_all_off(void)
{
    led_red_off();
    led_blue_off();
    led_green_off(); 
}


void led_all_on(void)
{  
    led_red_on();
    led_blue_on();
    led_green_on();

}


void led_init(void)
{ 
    memset(&led_struct,0,sizeof(led_struct));
    fibo_lpg_switch(0);
    LED_R_INIT();
    LED_B_INIT();
    led_all_off();
    // led_only_red_set(1,1);
    led_only_blue_set(5,5);

}

#endif
