#ifndef FFW_HAL_PWM_H
#define FFW_HAL_PWM_H
#include <stdint.h>
#include <stdbool.h>

bool ffw_pwm_open(int32_t index);
void ffw_pwm_close(int32_t index);
void ffw_pwm_close_all(void);
void ffw_pwm_config(int32_t index, uint16_t duty, uint16_t arr, uint16_t psc);
void ffw_pwm_config_lowpower(int32_t index, uint16_t duty, uint16_t arr, uint16_t psc);
void ffw_pwm_config_irq(int32_t index, uint16_t duty, uint16_t arr, uint16_t psc, void* callback);
void ffw_pwm_load_duty(int32_t index, uint16_t duty);
void ffw_pwm_irq_disable(int32_t index);

#endif
