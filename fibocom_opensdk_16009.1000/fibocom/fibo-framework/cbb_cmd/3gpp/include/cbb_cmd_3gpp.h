
#ifndef __CBB_CMD_3GPP_H__
#define __CBB_CMD_3GPP_H__

#include "ffw_rat.h"

typedef struct
{
  uint8_t mode;
  uint8_t frequency;
  uint8_t value;
  uint8_t setflag;
}fibo_rrc_locrel_cfg__t;


extern void fibo_locrel_deal_rel(uint8_t nSimID);

extern void fibo_locrel_deal_con(uint8_t nSimID);

extern void fibo_set_gtrrcrelcfg(uint8_t simId, uint8_t value, uint8_t mode, uint8_t frequency);

extern void fibo_get_gtrrcrelcfg(uint8_t simId, uint8_t *value, uint8_t *mode, uint8_t *frequency);

extern int32_t cbb_cmd_gtrrcrelcfg_exe(void);
extern int32_t cbb_cmd_gtrrcrelcfg(const uint8_t value, const uint8_t mode, const uint8_t frequency);
extern int32_t cbb_cmd_gtrrcrelcfg_read(void *engine);

extern int32_t cbb_cmd_gtsetrrm_set(uint8_t enable,void *engine);
extern int32_t cbb_cmd_gtsetrrm_read(uint8_t *enable,void *engine);

extern void fibo_set_rejcause(uint8_t type, uint8_t reject_cause,uint8_t nSim);
extern int32_t cbb_cmd_gtrejcause_set(uint8_t enable_report);
extern int32_t cbb_cmd_gtrejcause_read(reject_code_t *rejcause);




#endif


