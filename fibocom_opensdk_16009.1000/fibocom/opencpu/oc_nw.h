#ifndef _OC_FIBO_NW_H_
#define _OC_FIBO_NW_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "fibo_opencpu_comm.h"

/*support network config name*/
#define FIBO_NW_ROAMING_STATUS          "ROAMING_STATUS"
#define FIBO_NW_LTE_TIMING_ADVANCE      "LTE_TIMING_ADVANCE"
#if 0 //need support LOCRELT, unisoc baseline ivykit_rtos_8850_rls_W24.12.4 support, need upgrade baseline
#define FIBO_NW_GTRRCREL                "GTRRCREL"
#endif
#define FIBO_NW_LTE_SIGNAL_QUALITY_URC      "LTE_SIGNAL_QUALITY_URC"


typedef struct nw_config_rrcrel_s
{
    UINT8 value;
    UINT8 mode;
    UINT8 frequency;
    UINT8 imm_release;
} fibo_nw_config_rrcrel_t;

typedef enum
{
    FIBO_SIGNAL_QUALITY_URC_INIT    = 0x00,   //dont urc
    FIBO_SIGNAL_QUALITY_URC_CONFIG1 = 0x01,   //only use rsrp_range urc
    FIBO_SIGNAL_QUALITY_URC_CONFIG2 = 0x02,   //only use sinr_range urc
    FIBO_SIGNAL_QUALITY_URC_CONFIG3 = 0x03,   //both use rsrp_range and sinr_range urc
    FIBO_SIGNAL_QUALITY_URC_DEFAULT = 0xFF,   //use default urc
}signal_quality_urc_config_type_t;

typedef struct nw_config_signal_quality_urc_s
{
    signal_quality_urc_config_type_t config;
    UINT8 rsrp_range;
    UINT8 sinr_range;
    UINT16 reserve;
} fibo_nw_config_signal_quality_urc_t;


/*
fibo_nw_get_config
support name:
name: ROAMING_STATUS         input_param: NULL                        output_param: uint8_t *
name: GTRRCREL               input_param: NULL                        output_param: fibo_nw_config_rrcrel_t *     ---not support
name: LTE_TIMING_ADVANCE     input_param: NULL                        output_param: int16_t *
*/
extern int fibo_nw_get_config(UINT8 simId, char *name, void *input_param, void *output_param);

/*
fibo_nw_set_config
name: GTRRCREL               input_param: fibo_nw_config_rrcrel_t *   output_param: NULL  
*/
extern int fibo_nw_set_config(UINT8 simId, char *name, void *input_param, void *output_param);


#endif

