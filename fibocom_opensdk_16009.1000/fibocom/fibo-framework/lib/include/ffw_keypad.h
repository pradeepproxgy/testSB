#ifndef LIB_INCLUDE_KEYPAD
#define LIB_INCLUDE_KEYPAD
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>
#include <stdint.h>

typedef signed long int32_t;
typedef signed char int8_t;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef int INT32;
typedef unsigned short UINT16;

typedef struct {
    int32_t key_id;
    int8_t long_or_short_press;
    int8_t press_or_release;
}keypad_info_t;

typedef void (*key_cb_t)(INT32 key, INT32 state, void *ctx);

typedef void (*key_callback)(void* param);

bool ffw_keypad_queue_init(uint32_t timeout);

bool ffw_keypad_queue_output(keypad_info_t *key);

uint8_t ffw_get_pwrkeystatus(void);

keypad_info_t ffw_get_pwrkeypad_status(void);

bool ffw_pwrkeypad_config(UINT32 timeout, bool Ispwroff);

bool ffw_keypad_msg_mod(bool en);

INT32 ffw_set_pwr_callback_ex(key_callback pwr_cb,UINT16 long_press,void *arg);

INT32 ffw_set_pwr_callback_v1(key_callback pwr_press_cb,key_callback pwr_release_cb,UINT16 long_press);

INT32 ffw_set_pwr_callback(key_callback pwr_cb,UINT16 long_press);

bool ffw_pwrkey_config(uint32_t timeout, bool Ispwroff);

INT32 ffw_key_register_callback(key_cb_t cb, void *ctx, UINT32 long_press_time);

INT32 ffw_key_release_callback(void);


extern bool fibo_hal_keypad_queue_init(uint32_t timeout);
extern bool fibo_hal_keypad_queue_output(keypad_info_t *key);
extern uint8_t fibo_hal_get_pwrkeystatus(void);
extern keypad_info_t fibo_get_pwrkeypad_status(void);
extern bool fibo_hal_pwrkey_config(uint32_t timeout, bool Ispwroff);
extern INT32 fibo_hal_set_pwr_callback(key_callback pwr_cb,UINT16 long_press);
extern INT32 fibo_hal_set_pwr_callback_ex(key_callback pwr_cb,UINT16 long_press,void *arg);
extern INT32 fibo_hal_set_pwr_callback_v1(key_callback pwr_press_cb,key_callback pwr_release_cb,UINT16 long_press);
extern bool fibo_hal_keypad_msg_mod(bool en);

extern INT32 fibo_hal_key_register_callback(key_cb_t cb, void *ctx, UINT32 long_press_time);
extern INT32 fibo_hal_key_release_callback(void);

#endif
