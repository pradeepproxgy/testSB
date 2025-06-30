#ifndef __ES8311_HAL_
#define __ES8311_HAL_

#include "hal_chip.h"
#include "hal_config.h"
#include "drv_names.h"
#include "drv_i2c.h"
#include "audio_config.h"

#if defined(CONFIG_CHIP_8850_V3_BOARD) || defined(CONFIG_CHIP_8850_V4_BOARD)
#ifndef CONFIG_FIBOCOM_BASE
#define USE_CODEC_ES8311
#define CONFIG_CODEC_GAIN
#endif
#endif

#if defined(CONFIG_FIBOCOM_BASE)
#define USE_CODEC_ES8311
#endif

#ifdef USE_CODEC_ES8311

void es8311_cfg_close();
void es8311_cfg_output(uint16_t hpFlag, uint16_t dac, uint16_t mixgain);
void es8311_cfg_input(uint16_t hpFlag, uint16_t pga, uint16_t adc, uint16_t mute);
void es8311_cfg_mute(uint16_t muteFlag);
void es8311_cfg_init(uint32_t sampleRate, uint32_t mode, bool isSlave);
void aw87390_PA_init(uint16_t gain);
void aw87390_PA_close();

#endif

#endif
