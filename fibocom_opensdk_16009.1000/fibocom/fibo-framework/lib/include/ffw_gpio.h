#ifndef __FFW_HAL_GPIO_H_
#define __FFW_HAL_GPIO_H_
#include "drv_gpio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef enum
{
    HAL_GPIO_DATA_LOW,
    HAL_GPIO_DATA_HIGH,
    HAL_GPIO_DATA_MAX
} hal_gpio_data_t;

typedef void (*ISR_callback)(void* param);

typedef struct
{
    drvGpio_t *drv_gpio;
    drvGpioConfig_t *drv_isr;
    hal_gpio_data_t level;
    ISR_callback isr_callback;
} hal_gpioisr_t;

typedef struct
{
    uint16_t pin_index;
    uint16_t gpio_index;
    volatile uint32_t *key_reg;
    volatile uint32_t *key_cfg_reg;
    uint8_t default_mode;
    int  gpio_mode;
    bool is_high_resistance;
}pin_map_t;

typedef enum
{
    /** force pull, with neither pull up nor pull down */
    FORCE_PULL_NONE = 1,
    /** force pull down */
    FORCE_PULL_DOWN,
    /** force pull up */
    FORCE_PULL_UP,
    /** force pull up with weakest strength (largest resistance) */
    FORCE_PULL_UP_1 = FORCE_PULL_UP,
    /** force pull up with smaller resistance */
    FORCE_PULL_UP_2,
    /** force pull up with smaller resistance */
    FORCE_PULL_UP_3,
} hal_IomuxPullType_t;

typedef struct
{
    uint16_t pin;
    hal_IomuxPullType_t pinType;
} ffw_pin_mode;

int ffw_hal_getpinindex(uint16_t gpio_id);
int ffw_hal_getgpioindex(uint16_t pin_id);
bool ffw_hal_gpioModeInit(uint16_t pin_id);
bool ffw_hal_gpioModeSet(uint16_t  pin_id, uint8_t mode);
bool ffw_hal_gpioModerelease(uint16_t pin_id);
bool ffw_hal_gpioSetDir(uint16_t pin_id, drvGpioMode_t inout);
bool ffw_hal_gpioGetLevel(uint16_t pin_id, hal_gpio_data_t *level);
bool ffw_hal_gpioSetLevel(uint16_t pin_id, hal_gpio_data_t out_level);
bool ffw_hal_gpioIsrDeInit(uint8_t pin_id);
bool ffw_hal_gpioIsrInit(uint8_t  pin_id, hal_gpioisr_t *gpio_Isr);
bool ffw_hal_gpio_pull_up_or_down(uint16_t pin_id,bool is_pull_up);
bool ffw_hal_gpio_pull_high_resistance(uint16_t pin_id, bool is_resistance);
bool ffw_hal_i2s_mclk_unset(uint8_t i2s_num);
bool ffw_hal_i2s_pin_iomux_set(uint8_t i2s_num, uint8_t sdati_sel);
void ffw_hal_huada_gnss_reset(void);
bool ffw_hal_setdriving(uint16_t pin_id,uint8_t driving);
bool ffw_hal_gpioPullCtrl(ffw_pin_mode pMode,bool is_sleepvoid,void *reserve);
#endif
