#ifndef __ES8374_HAL_
#define __ES8374_HAL_

#include "hal_chip.h"
#include "hal_config.h"
#include "drv_names.h"
#include "drv_i2c.h"

void es8374_cfg_master_init(uint32_t sampleRate, uint32_t mode);
void es8374_cfg_slave_init(uint32_t sampleRate, uint32_t mode);
void es8374_cfg_close();
void es8374_cfg_output(uint16_t hpFlag, uint16_t dac, uint16_t mixgain);
void es8374_cfg_input(uint16_t hpFlag, uint16_t pga, uint16_t adc, uint16_t mute);
void es8374_cfg_mute(uint16_t muteFlag);
void es8374_cfg_init(uint32_t sampleRate, uint32_t mode, bool isSlave);

#endif
