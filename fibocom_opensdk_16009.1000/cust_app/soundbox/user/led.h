#ifndef _LED_H_
#define _LED_H_


#include "fibo_opencpu.h"
#include "fb_uart.h"

#if LED_EN

typedef struct {


    //red
    UINT16 red_led_cnt;
    UINT16 red_on_time;
    UINT16 red_off_time;
    UINT8 red_flag;   //led 开关状态  1：打开  0：关闭  
    UINT8 red_flash_flag;   //led 开关状态  1：闪烁 0：不闪烁
    //blue
     UINT16 blue_led_cnt;
    UINT16 blue_on_time;
    UINT16 blue_off_time;
    UINT8 blue_flag;   //led 开关状态  1：打开  0：关闭  
    UINT8 blue_flash_flag;   //led 开关状态  1：闪烁 0：不闪烁
    //green
    UINT16 green_led_cnt;
    UINT16 green_on_time;
    UINT16 green_off_time;
    UINT8 green_flag;   //led 开关状态  1：打开  0：关闭  
    UINT8 green_flash_flag;   //led 开关状态  1：闪烁 0：不闪烁

} _LED_STRUCT;



typedef enum{
    LED_RED = 1,
    LED_GREEN,
    LED_BLUE
  

}_LED_COLOUR;



void led_scan(void);
void led_all_off(void);
void led_all_on(void);
void led_init(void);

void led_red_on(void);
void led_blue_on(void);
void led_green_on(void);

void led_red_off(void);
void led_blue_off(void);
void led_green_off(void);

void led_all_off(void);
void led_all_on(void);

void led_set(_LED_COLOUR led_colour,UINT16 on_time,UINT16 off_time);
void led_red_set(UINT16 on_time,UINT16 off_time);
void led_blue_set(UINT16 on_time,UINT16 off_time);
void led_green_set(UINT16 on_time,UINT16 off_time);

void led_only_red_set(UINT16 on_time,UINT16 off_time);

void led_only_blue_set(UINT16 on_time,UINT16 off_time);

void led_only_green_set(UINT16 on_time,UINT16 off_time);


void led_only_red_on(void);

void led_only_blue_on(void);

void led_only_green_on(void);

#endif
#endif // !_LED_H_
 