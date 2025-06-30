#ifndef COMPONENTS_FIBOCOM_OPENCPU_OC_ADC_H
#define COMPONENTS_FIBOCOM_OPENCPU_OC_ADC_H
#include "ffw_adc.h"

/**
* @brief 
* 
* @return INT32 
 */
INT32 fibo_hal_adc_init(void);


/**
* @brief 
* 
* @return INT32 
 */
INT32 fibo_hal_adc_deinit(void);


/**
* @brief 
* 
* @param channel 
* @param data 
* @return INT32 
 */
INT32 fibo_hal_adc_get_data(hal_adc_channel_t channel, UINT32 scale_value);
INT32 fibo_hal_adc_get_data_polling(hal_adc_channel_t channel, UINT32 *data);

#endif /* COMPONENTS_FIBOCOM_OPENCPU_OC_ADC_H */

INT32 fibo_setSleepMode(UINT8 time);

