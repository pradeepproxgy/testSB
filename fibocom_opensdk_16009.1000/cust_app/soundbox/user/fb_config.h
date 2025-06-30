#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#include "fibo_opencpu.h"
#define PLAT_LOG(fmt, ...) OSI_PRINTFI("[Speaker] [%s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define APP_VERSIONS_INFO         "FR002001101"
#define APP_VERSION         "\"SPEAKER_APP_V1.2\""

//uart 
#define USER_LOG_EN                 1  //uart1   打印口 默认uart1 115200  需要先使能UART1 

#define NTP_UPDATE_TIME             1   // NTP同步网络时间
#define CLOUD_PLATFORM_EN           0   //网络平台任务  目前对接平台协议为 JT808
#define ALIYUN_PLATFORM_EN          1    //aliyun 物联网平台

//---------------------TTS 相关配置---------------------------
#define TTS_EN                      1

    #define TTS_BUF_MAX             60      //tts最大缓存数  
    #define TTS_MAX_DATA            100      //TTS缓存最大长度   
                                            //AT指令 16 byte 支付宝到帐 10byte  微信到帐8byte  

//-----------------------------power on--------


#define FAKE_POWR_ON                0        //假开机   用作拨码开关 off 以后  插入usb开机充电只显示充电灯 不出提示音


#define POWER_PIN                   30
#define POWER_ON()                  fibo_gpio_mode_set(POWER_PIN,1),\
                                    fibo_gpio_cfg(POWER_PIN,1),\
                                    fibo_gpio_set(POWER_PIN,1)  
#define POWER_OFF()                 fibo_gpio_set(POWER_PIN,0) 


//--------------------------battery charge --------

#define BAT_CHECK_EN                    1          //是否使用ADC检测 电池电量 "Whether to use ADC to detect battery level"

#define CHARGE_EN                       0          // 是否使用充电控制 "Whether to use charge control"

#define HTTP_EN                         1

#if CHARGE_EN

    // #define CHARGE_PIN               72
    // #define BAT_CHARGE_INIT()        fibo_gpio_mode_set(CHARGE_PIN,0),
    //                                  fibo_gpio_cfg(CHARGE_PIN,1),
    //                                  fibo_gpio_set(CHARGE_PIN,0)  
    // #define BAT_CHARGE_ON()          fibo_gpio_set(CHARGE_PIN,1) 
    // #define BAT_CHARGE_OFF()         fibo_gpio_set(CHARGE_PIN,0) 


//LP28053  STAT2  开漏输出   未插入时 为高    充满电 输出低

    #define CHARGE_STAT2_PIN            36
#define BAT_CHARGEING_INIT() 	 	    fibo_gpio_mode_set(CHARGE_STAT2_PIN,0),\
								 		fibo_gpio_cfg(CHARGE_STAT2_PIN,0),\
                                        fibo_gpio_pull_disable(CHARGE_STAT2_PIN)		 
	#define BAT_CHARGEING_UNINIT()	 
	#define BAT_CHARGEING_STA(value)  	fibo_gpio_get(CHARGE_STAT2_PIN,&value);

//LP28053  STAT1  开漏输出   充电 输出低    未充电始高   

    #define CHARGE_STAT1_PIN            35
	#define BAT_CHARGE_STAT1_INIT() 	fibo_gpio_mode_set(CHARGE_STAT1_PIN,0),\
								 	    fibo_gpio_cfg(CHARGE_STAT1_PIN,0)		 
	#define BAT_CHARGE_STAT1_UNINIT()
	#define BAT_CHARGE_STAT1_STA(value) fibo_gpio_get(CHARGE_STAT1_PIN,&value);




    #define POWER_CHARGE_PIN            120
	#define POWER_CHARGE_INIT()         fibo_gpio_mode_set(POWER_CHARGE_PIN,2),\
								 		fibo_gpio_cfg(POWER_CHARGE_PIN,0),\
                                        fibo_gpio_pull_up_or_down(POWER_CHARGE_PIN,true)		 
	#define POWER_CHARGE_UNINIT()	 
	#define POWER_CHARGE_STA(value)     fibo_gpio_get(POWER_CHARGE_PIN,&value);


#endif/*CHARGE_EN*/



//**********音量设置***************************
// set volume value 
#define  VOLUME_DEFAULT_VALUE           10    //0-7   0：不是静音Not muted
#define  VOLUME_MAX_VALUE               10
#define  VOLUME_MIN_VALUE               0


//********** key 按键配置**********************
#define IO_KEY_EN                       1   //io 中断

#if IO_KEY_EN

    #define POWER_KEY_PIN               21
    #define POWER_KEY_INIT()            fibo_gpio_mode_set(POWER_KEY_PIN,1),\
                                        fibo_gpio_cfg(POWER_KEY_PIN,0)
                                         

    #define POWER_KEY_STA(value)        fibo_gpio_get(POWER_KEY_PIN,&value);

//mode
    #define KEY_MODE_PIN                65
    #define KEY_MODE_INIT()      	    fibo_gpio_mode_set(KEY_MODE_PIN,0),\
                                	    fibo_gpio_cfg(KEY_MODE_PIN,0)        
    #define KEY_MODE_UNINIT()    
    #define KEY_MODE_STA(value)  	    fibo_gpio_get(KEY_MODE_PIN,&value);

//vol_up
	 #define KEY_VOL_UP_PIN 			 5
	 #define KEY_VOL_UP_INIT() 	 		fibo_gpio_mode_set(KEY_VOL_UP_PIN,1),\
							fibo_gpio_cfg(KEY_VOL_UP_PIN,0),\
				       			fibo_textTrace("fides gpio_mode_set was succeed for volume up")			
	 #define KEY_VOL_UP_UNINIT()	 
	 #define KEY_VOL_UP_STA(value)  	fibo_gpio_get(KEY_VOL_UP_PIN,&value);


 //vol_down
	 #define KEY_VOL_DOWN_PIN 			6
	 #define KEY_VOL_DOWN_INIT() 	 	fibo_gpio_mode_set(KEY_VOL_DOWN_PIN,1),\
						fibo_gpio_cfg(KEY_VOL_DOWN_PIN,0),\
						fibo_textTrace("fides gpio_mode_set was succeed for volume down") 			
	 #define KEY_VOL_DOWN_UNINIT()	 
	 #define KEY_VOL_DOWN_STA(value)  	fibo_gpio_get(KEY_VOL_DOWN_PIN,&value);
// //play
//Quec vol up

/*	#define VOL_UP_BUTTON 				64
	#define VOL_UP_INIT()             fibo_gpio_mode_set(VOL_UP_BUTTON,0),\
                                          fibo_gpio_cfg(VOL_UP_BUTTON,0)

        #define VOL_DOWN_BUTTON                           62
        #define VOL_DOWN_INIT()           fibo_gpio_mode_set(VOL_DOWN_BUTTON,0),\
                                          fibo_gpio_cfg(VOL_DOWN_BUTTON,4)
*/
// 	 #define KEY_PLAY_PIN 				29
// 	 #define KEY_PLAY_INIT() 	 		fibo_gpio_mode_set(KEY_PLAY_PIN,1),
// 								 		fibo_gpio_cfg(KEY_PLAY_PIN,0)		 
// 	 #define KEY_PLAY_UNINIT()	 
// 	 #define KEY_PLAY_STA(value)  		fibo_gpio_get(KEY_PLAY_PIN,&value);


#endif /*IO_KEY_EN*/

//--------- LED配置----------------
#define LED_EN                      1 

#if LED_EN   //io模式// 输出设置 //输出低

#define LED_RED_EN                  1
#define LED_BLUE_EN                 1
#define LED_GREEN_EN                1

#if LED_RED_EN
//red
    #define LED_R_PIN               54
    #define LED_R_INIT()            fibo_gpio_mode_set(LED_R_PIN,0),\
                                    fibo_gpio_cfg(LED_R_PIN,1),\
                                    fibo_gpio_set(LED_R_PIN,0)


    #define LED_R_ON()              fibo_gpio_set(LED_R_PIN,1)
    #define LED_R_OFF()             fibo_gpio_set(LED_R_PIN,0)
#endif


    #define DAC_CTRL_PIN               68
    #define DAC_CTRL_INIT()         fibo_gpio_mode_set(DAC_CTRL_PIN,0),\
                                    fibo_gpio_cfg(DAC_CTRL_PIN,1),\
                                    fibo_gpio_set(DAC_CTRL_PIN,1)


#if LED_BLUE_EN
//blue
    #define LED_B_PIN               53
    #define LED_B_INIT()            fibo_gpio_mode_set(LED_B_PIN,0),\
                                    fibo_gpio_cfg(LED_B_PIN,1),\
                                    fibo_gpio_set(LED_B_PIN,0) 

    #define LED_B_ON()              fibo_gpio_set(LED_B_PIN,1)
    #define LED_B_OFF()             fibo_gpio_set(LED_B_PIN,0)
#endif

#if LED_GREEN_EN
//green
    #define LED_G_PIN               55
    #define LED_G_INIT()            fibo_gpio_mode_set(LED_G_PIN,0),\
                                    fibo_gpio_cfg(LED_G_PIN,0),\
                                    fibo_gpio_set(LED_G_PIN,0) 

    #define LED_G_ON()              fibo_gpio_set(LED_G_PIN,1)
    #define LED_G_OFF()             fibo_gpio_set(LED_G_PIN,0)

#endif
#endif/*LED_EN*/



//功放*相关配置**************************************
#define VOICE_PLAY_EN               1   //使能voice播放 支持audio文件播放、TTS播放 Enable voice playback, supporting audio file playback and TTS playback.
#define AW87359_EN                  0   // AW87359 iic 控制 control


//IO控制   PA mute 控制 
#if 0

#define MUTE_PIN                    1
#define PA_MUTE_INIT()              fibo_gpio_mode_set(MUTE_PIN,0),\
                                    fibo_gpio_cfg(MUTE_PIN,1),\
                                    fibo_gpio_set(MUTE_PIN,0) 
//#define PA_MUTE_INIT()            fibo_external_PA_enable(55)     //
#define PA_MUTE()                   fibo_gpio_set(MUTE_PIN,0),fibo_taskSleep(50)
#define PA_UNMUTE()                 fibo_gpio_set(MUTE_PIN,1),fibo_taskSleep(100)

#else

#define MUTE_PIN                    56
#define PA_MUTE_INIT()              fibo_external_PA_enable(MUTE_PIN);
//#define PA_MUTE_INIT()            fibo_external_PA_enable(55)     //
#define PA_MUTE()               
#define PA_UNMUTE()                 fibo_gpio_set(MUTE_PIN,1),fibo_taskSleep(100)    


#endif


#define LOCK_INFO_EN                      1  //锁信息使能 包含：锁控制 状态获取 车轮转速统计
//lock control  io

//电机锁
#define LOCK_MOTOR_PIN              5
#define LOCK_MOTOR_INIT()           fibo_gpio_mode_set(LOCK_MOTOR_PIN,0),\
                                    fibo_gpio_cfg(LOCK_MOTOR_PIN,1),\
                                    fibo_gpio_set(LOCK_MOTOR_PIN,0) 

#define LOCK_MOTOR_OPEN()           fibo_gpio_set(LOCK_MOTOR_PIN,1)
#define LOCK_MOTOR_CLOSE()          fibo_gpio_set(LOCK_MOTOR_PIN,0)
#define LOCK_MOTOR_STA(value)       fibo_gpio_get(LOCK_MOTOR_PIN,&value)

//电池锁
#define LOCK_BAT_PIN                5
#define LOCK_BAT_INIT()             fibo_gpio_mode_set(LOCK_BAT_PIN,0),\
                                    fibo_gpio_cfg(LOCK_BAT_PIN,1),\
                                    fibo_gpio_set(LOCK_BAT_PIN,0) 

#define LOCK_BAT_OPEN()             fibo_gpio_set(LOCK_BAT_PIN,1)
#define LOCK_BAT_CLOSE()            fibo_gpio_set(LOCK_BAT_PIN,0)
#define LOCK_BAT_STA(value)         fibo_gpio_get(LOCK_BAT_PIN,&value)

//唐泽锁
#define LOCK_TANGZE_PIN              5
#define LOCK_TANGZE_INIT()          fibo_gpio_mode_set(LOCK_TANGZE_PIN,0),\
                                    fibo_gpio_cfg(LOCK_TANGZE_PIN,1),\
                                    fibo_gpio_set(LOCK_TANGZE_PIN,0) 

#define LOCK_TANGZE_OPEN()          fibo_gpio_set(LOCK_TANGZE_PIN,1)
#define LOCK_TANGZE_CLOSE()         fibo_gpio_set(LOCK_TANGZE_PIN,0)
#define LOCK_TANGZE_STA(value)      fibo_gpio_get(LOCK_TANGZE_PIN,&value)


//提示音   TTS  AUDIO 二选一
#define   TTS_WARNING_TONE          0   //TTS 播放提示音    1:AT   2: API  
#define   AUDIO_PLAY_EN             0   //AUDIO 播放提示音



/***************location config*********************/


#define LCD_POWER_PIN               3
#define LCD_POWER_INIT()            fibo_gpio_mode_set(LCD_POWER_PIN,0),\
                                    fibo_gpio_cfg(LCD_POWER_PIN,1),\
                                    fibo_gpio_set(LCD_POWER_PIN,0) 

#define LCD_POWER_ON()              fibo_gpio_set(LCD_POWER_PIN,1)
#define LCD_POWER_OFF()             fibo_gpio_set(LCD_POWER_PIN,0)

#if 0
#define POWER_CHARGE_PIN            120
#define POWER_CHARGE_INIT() 	 	fibo_gpio_mode_set(POWER_CHARGE_PIN,2),\
                                    fibo_gpio_cfg(POWER_CHARGE_PIN,0),\
                                    fibo_gpio_pull_up_or_down(POWER_CHARGE_PIN,true)		 
#define POWER_CHARGE_UNINIT()	 
#define POWER_CHARGE_STA(value)     fibo_gpio_get(POWER_CHARGE_PIN,&value);

#define LCD_REST_PIN                120
#define LCD_REST_INIT()             fibo_gpio_pull_disable(LCD_REST_PIN),\
                                    fibo_gpio_mode_set(LCD_REST_PIN,2),\
                                    fibo_gpio_pull_up_or_down(LCD_REST_PIN,true),\
                                    fibo_gpio_cfg(LCD_REST_PIN,1),\
                                    fibo_gpio_set(LCD_REST_PIN,1) 

#define LCD_REST_EN()               fibo_gpio_set(LCD_REST_PIN,1)
#define LCD_REST_DIS()              fibo_gpio_set(LCD_REST_PIN,1)
#endif

#define SPI2_SDA_PIN                142
#define SPI2_SDA_INIT()             fibo_gpio_mode_set(SPI2_SDA_PIN,1)
                                  

#define SPI2_SCL_PIN                141
#define SPI2_SCL_INIT()             fibo_gpio_mode_set(SPI2_SCL_PIN,1)
                                   

#define SPI1_SDA_PIN                41
#define SPI1_SDA_INIT()             fibo_gpio_mode_set(SPI1_SDA_PIN,0)
                                  

//#define SPI1_SCL_PIN                42
//#define SPI1_SCL_INIT()             fibo_gpio_mode_set(SPI1_SCL_PIN,0)

#define LED_RED_PIN                   25
#define REDLED_INIT()               fibo_gpio_mode_set(LED_RED_PIN,0),\
                                    fibo_gpio_cfg(LED_RED_PIN,1),\
				    fibo_gpio_set(LED_RED_PIN,1)

#define LED_GREEN_PIN                 16
#define GREENLED_INIT()             fibo_gpio_mode_set(LED_GREEN_PIN,0),\
                                    fibo_gpio_cfg(LED_GREEN_PIN,1),\
                                    fibo_gpio_set(LED_GREEN_PIN,0)

#define REDLED_ON()                 fibo_gpio_set(LED_RED_PIN,1)

#define REDLED_OFF()                fibo_gpio_set(LED_RED_PIN,0)
                
#define GREENLED_ON()               fibo_gpio_set(LED_GREEN_PIN,1)

#define GREENLED_OFF()              fibo_gpio_set(LED_GREEN_PIN,0)
					
#define TX_PIN                      138
#define UART_ENABLE_SEGMENT()       fibo_gpio_mode_set(TX_PIN,0),\
                                    fibo_gpio_cfg(TX_PIN,1)


/***************other config*********************/
#define EXT_FLASH_EN                0   //是否挂载外部flash  //GD25LQ64C    
#define EXT_BLE_EN                  0//外挂ble模块使能 默认通过uart通信

//tracker 移植新加宏定义

 #define DEV_INFO_SAVE_FS           0   //设备信息储存    需要修改储存到模块内部flash 或外挂flash 

 #define EXT_SERSON_EN              0   //传感器
 
 #endif
