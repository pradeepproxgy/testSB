#ifndef _KEY_H_
#define _KEY_H_

#include "fibo_opencpu.h"
#include "fb_uart.h"
#include "fb_config.h"


#if IO_KEY_EN 

typedef struct {

    UINT8 key_val;  				 //当前按键值
    UINT8 last_key_val;            //上一个按键值
    UINT8 key_cnt;                 //按键值持续计数
    UINT8 key_up;                  //抬键计数
} type_key_ctl;


//GPIO3
    // fibo_gpio_mode_set(25,0);      // gpio口复用状态   复用功能设置，
	// fibo_gpio_cfg(25,0);          //  配置为输出 0为输入，1为输出
    // fibo_gpio_set(25,1);s
   // fibo_thread_create(prvThreadEntry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);


#define IO_KEY1_PIN         17



#define KEY0_STA()   //nrf_gpio_pin_read(KEY2_PIN)
#define KEY2_STA()  // nrf_gpio_pin_read(KEY2_PIN)


#define KEY_SCAN_TIMES          6           //按键防抖的扫描次数
#define KEY_UP_TIMES            6           //抬键防抖的扫描次数
#define KEY_LONG_TIMES          150         //长按键的次数
#define KEY_HOLD_TIMES          50          //连接的频率次数
#define KEY_LONG_HOLD_TIMES     (KEY_LONG_TIMES + KEY_HOLD_TIMES)


#define T_KEY_SHORT             0x00    //000 00000--0
#define T_KEY_SHORT_UP          0x80    //100 00000--4
#define T_KEY_LONG              0xA0    //101 00000--5
#define T_KEY_LONG_UP           0xC0    //110 00000--6
#define T_KEY_HOLD              0xE0    //111 00000--7


#define NO_KEY                  0
#define T_KEY_1                 0x1
#define T_KEY_2                 0x2
#define T_KEY_3                 0x3
#define T_KEY_4                 0x4
#define T_KEY_5                 0x5
#define T_KEY_6                 0x6
#define T_KEY_7                 0x7
#define T_KEY_8                 0x8
#define T_KEY_9                 0x9
#define T_KEY_9                 0x9
#define T_KEY_10                0xa
#define T_KEY_11                0xb
#define T_KEY_12                0xc
#define T_KEY_13                0xd
#define T_KEY_14                0xe
#define T_KEY_15                0xf
#define T_KEY_16                0x10
#define T_KEY_17                0x11
#define T_KEY_18                0x12
#define T_KEY_19                0x13
#define T_KEY_20                0x14
#define T_KEY_21                0x15
#define T_KEY_22                0x16
#define T_KEY_23                0x17
#define T_KEY_24                0x18
#define T_KEY_25                0x19
#define T_KEY_26                0x1a
#define T_KEY_27                0x1b
#define T_KEY_28                0x1c
#define T_KEY_29                0x1d
#define T_KEY_30                0x1e
#define T_KEY_31                0x1f





//功能键
#define T_KEY_NUM1          T_KEY_1    
#define T_KEY_NUM2          T_KEY_2 
#define T_KEY_POWER         T_KEY_3
#define T_KEY_VOL_UP        T_KEY_4
#define T_KEY_VOL_DOWN      T_KEY_5
#define T_KEY_PLAY        	T_KEY_6
#define T_KEY_MODE        	T_KEY_7



/*****************************************************************************
 * Module    : 按键长短按配置
 *****************************************************************************/
//按键详细定义 -> K_: Key Short Down;   KU_: Key Short Up;   KL_: Key Long Down;   KLU_: Key Long Up;   KH_: Key Hold;
#define K_NUM1                      (T_KEY_NUM1 | T_KEY_SHORT)
#define KU_NUM1                     (T_KEY_NUM1 | T_KEY_SHORT_UP)
#define KL_NUM1                     (T_KEY_NUM1 | T_KEY_LONG)
#define KLU_NUM1                    (T_KEY_NUM1 | T_KEY_LONG_UP)
#define KH_NUM1                     (T_KEY_NUM1 | T_KEY_HOLD)

#define K_NUM2                 	 	(T_KEY_NUM2 | T_KEY_SHORT)
#define KU_NUM2                 	(T_KEY_NUM2 | T_KEY_SHORT_UP)
#define KL_NUM2                		(T_KEY_NUM2 | T_KEY_LONG)
#define KLU_NUM2                	(T_KEY_NUM2 | T_KEY_LONG_UP)
#define KH_NUM2                 	(T_KEY_NUM2 | T_KEY_HOLD)



#define K_POWER                 	(T_KEY_POWER | T_KEY_SHORT)
#define KU_POWER                	(T_KEY_POWER | T_KEY_SHORT_UP)
#define KL_POWER               		(T_KEY_POWER | T_KEY_LONG)
#define KLU_POWER                	(T_KEY_POWER | T_KEY_LONG_UP)
#define KH_POWER                	(T_KEY_POWER | T_KEY_HOLD)




#define K_VOL_UP                	(T_KEY_VOL_UP | T_KEY_SHORT)
#define KU_VOL_UP                	(T_KEY_VOL_UP | T_KEY_SHORT_UP)
#define KL_VOL_UP                   (T_KEY_VOL_UP | T_KEY_LONG)
#define KLU_VOL_UP                	(T_KEY_VOL_UP | T_KEY_LONG_UP)
#define KH_VOL_UP                 	(T_KEY_VOL_UP | T_KEY_HOLD)

#define K_VOL_DOWN                	(T_KEY_VOL_DOWN | T_KEY_SHORT)
#define KU_VOL_DOWN                	(T_KEY_VOL_DOWN | T_KEY_SHORT_UP)
#define KL_VOL_DOWN                 (T_KEY_VOL_DOWN | T_KEY_LONG)
#define KLU_VOL_DOWN                (T_KEY_VOL_DOWN | T_KEY_LONG_UP)
#define KH_VOL_DOWN                	(T_KEY_VOL_DOWN | T_KEY_HOLD)

#define K_MODE               	(T_KEY_MODE | T_KEY_SHORT)
#define KU_MODE              	(T_KEY_MODE | T_KEY_SHORT_UP)
#define KL_MODE                 (T_KEY_MODE | T_KEY_LONG)
#define KLU_MODE                (T_KEY_MODE | T_KEY_LONG_UP)
#define KH_MODE               	(T_KEY_MODE | T_KEY_HOLD)



void Io_key_init(void);
void Io_key_uninit(void);
void Io_key_scan(void);

int fb_key_enter(void);
#endif/* IO_KEY_EN */

#endif

