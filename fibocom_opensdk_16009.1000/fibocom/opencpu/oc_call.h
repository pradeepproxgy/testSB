#ifndef _OC_CALL_H_
#define _OC_CALL_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "fibo_opencpu_comm.h"

typedef struct
{
    uint8_t idx;
    uint8_t direction;
    uint8_t status;
    uint8_t multiparty;
    char dialnum[23];
}fibo_call_info;

//call
extern int fibo_voice_set_volte(uint8_t setvolte,uint8_t simId);
extern bool fibo_is_volte_enable(uint8_t simId);
extern int fibo_is_ims_register(uint8_t simId);
extern int fibo_voice_start_call(char *dial_num,uint8_t simId);
extern int fibo_voice_answer_call(uint8_t simId);
extern int fibo_voice_end_call(uint8_t simId);
extern int fibo_voice_get_call_info(uint8_t simId, uint8_t *count,fibo_call_info callinfo[AT_CC_MAX_NUM]);


#endif

