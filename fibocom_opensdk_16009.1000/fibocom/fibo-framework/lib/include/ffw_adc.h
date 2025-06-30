#ifndef _FFW_HAL_ADC_H_
#define _FFW_HAL_ADC_H_

typedef int hal_adc_channel_t;

void ffw_hal_adc_init(void);
int32_t ffw_hal_adc_get_data(hal_adc_channel_t channel, uint32_t scale_value);

#endif
