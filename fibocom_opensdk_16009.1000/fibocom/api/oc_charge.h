#ifndef COMPONENTS_FIBOCOM_OPENCPU_OC_CHARGE_H
#define COMPONENTS_FIBOCOM_OPENCPU_OC_CHARGE_H

#include "fibo_opencpu_comm.h"

typedef void (*charge_handler_t)(void *arg);

typedef struct
{
    int vol_type;
    int threshold_vol;
    charge_handler_t handler;
    void *arg;
} fiboVolDetectParams_t;

typedef struct
{
    UINT32 end_vol;   // charger end_vol
    UINT32 chg_cur;   // max charger current
    UINT32 rechg_vol; // re_charger start vol
    UINT32 shut_vol;  // shut_down vol
    UINT32 warn_vol;  // low power warning vol
} fiboChargeParams_t;

typedef enum
{
    // Charge message.
    FIBO_CHARGE_CAP_IND = 0x1, // Notify the battery's capacity

    FIBO_CHARGE_START_IND, // start the charge process.
    FIBO_CHARGE_END_IND,   // the charge ended.

    FIBO_CHARGE_WARNING_IND,  // the capacity is low, should charge.
    FIBO_CHARGE_SHUTDOWN_IND, // the capacity is very low and must shutdown.

    FIBO_CHARGE_CHARGE_FINISH,     // the charge has been completed.
    FIBO_CHARGE_CHARGE_DISCONNECT, // the charge be disconnect
    FIBO_CHARGE_CHARGE_FAULT,      // the charge fault, maybe the voltage of charge is too low.

    FIBO_CHARGE_MSG_MAX_NUM
} FIBO_CHARGE_MSG_E;

typedef void (*fiboChargeNoticeCB_t)(UINT32 msg);

INT32 fibo_charge_enable(bool enable);

INT32 fibo_charge_onoff(bool on);

BOOL fibo_charge_is_full(void);

INT32 fibo_charge_config_set(fiboChargeParams_t *chg_param_t);

INT32 fibo_charge_notice_cb_set(fiboChargeNoticeCB_t cb);

INT32 fibo_charge_vbat_vol_get(void);

INT32 fibo_get_threshold_vol(void);

INT32 fibo_vol_threshold_trigger_callback(int vol_type, int threshold_vol, charge_handler_t handler, void *arg);

INT32 fibo_charge_vbat_vol_percent_get(void);

INT32 fibo_charge_current_get(void);

INT32 fibo_getVchgVol(void);

INT32 fibo_getChargeStatus(void);
#endif // COMPONENTS_FIBOCOM_OPENCPU_OC_CHARGE_H