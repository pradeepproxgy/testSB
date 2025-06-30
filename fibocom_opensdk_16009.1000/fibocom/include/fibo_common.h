#ifndef _FIBO_COMMON_H_
#define _FIBO_COMMON_H_
#include <ctype.h>
#include <stdio.h>
//#include "FreeRTOS.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "nvm.h"
#include <vfs.h>
#include "drv_gpio.h"
#include <cfw.h>
#include "at_command.h"
//#include "osi_api.h"

#ifdef CONFIG_FIBOCOM_BASE
#include "didi_app.h"
#endif

#define FIBOCOM_COMMON_NV_BUF_SIZE (512 * 24)
#define FIBOCOM_COMMON_SNV_BUF_SIZE (512 * 4) //  need reback
#define FIBOCOM_WAKELOCK_STACK_SIZE (512 * 4)
#define FIBOCOM_WAKELOCK_ENEVT_MAX 16
#define FIBOCOM_COMMON_NV_FNAME CONFIG_FS_AP_NVM_DIR "/fibo_common.nv"
#define FIBOCOM_COMMON_SNV_FNAME CONFIG_FS_AP_NVM_DIR "/fibo_sback.nv"
#define FIBOCOM_NV_NAME_MAX 40
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
#define HAL_WAKEUP_OUT_PIN_INDEX 20
#elif defined CONFIG_FIBOCOM_MC665
#define HAL_WAKEUP_OUT_PIN_INDEX 61
#else
#define HAL_WAKEUP_OUT_PIN_INDEX 0xff
#endif

#ifdef CONFIG_FIBOCOM_HALUO_AT
#define HAL_DRING_PIN_INDEX 16
#elif defined CONFIG_FIBOCUS_TAIBITE_FEATURE
#define HAL_DRING_PIN_INDEX 0xff
#elif defined CONFIG_MC665_CN_36_90_QDTIANYAN
#define HAL_DRING_PIN_INDEX 0xff
#elif defined CONFIG_FIBOCOM_MC669
#define HAL_DRING_PIN_INDEX 40
#elif defined CONFIG_FIBOCOM_MC660
#define HAL_DRING_PIN_INDEX 62
#elif defined CONFIG_FIBOCOM_MC661
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
#define HAL_DRING_PIN_INDEX 0xff
#else
#define HAL_DRING_PIN_INDEX 20
#endif
#else
#define HAL_DRING_PIN_INDEX 0xff
#endif

//define HAL_WAKEUP_IN_PIN_INDEX
#ifdef CONFIG_FIBOCOM_MC669
#define HAL_WAKEUP_IN_PIN_INDEX 49
#elif defined CONFIG_FIBOCOM_MC665
#define HAL_WAKEUP_IN_PIN_INDEX 41
#elif defined CONFIG_FIBOCOM_MC660
#define HAL_WAKEUP_IN_PIN_INDEX 1
#elif defined CONFIG_FIBOCUS_HAIKANG_FEATURE
#define HAL_WAKEUP_IN_PIN_INDEX 19
#else
#define HAL_WAKEUP_IN_PIN_INDEX 0xff
#endif

#ifdef CONFIG_FIBOCOM_MC669
#define HAL_DTR_PIN_INDEX 39
#elif defined CONFIG_FIBOCOM_MC661
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
#define HAL_DTR_PIN_INDEX 0xff
#elif defined CONFIG_MC661_CN_19_70_NIU
#define HAL_DTR_PIN_INDEX 21
#else
#define HAL_DTR_PIN_INDEX 19
#endif
#elif defined CONFIG_FIBOCOM_MC660
#define HAL_DTR_PIN_INDEX 66
#else
#define HAL_DTR_PIN_INDEX 0xff
#endif

//define FIBO_SIM1_DETECT_GPIO
#if defined (CONFIG_FIBOCOM_MC669) || defined (CONFIG_FIBOCOM_MG662)
#define FIBO_SIM1_DECECT_GPIO 9
#elif defined (CONFIG_MC665_CN_36_90_QDTIANYAN)
#define FIBO_SIM1_DECECT_GPIO 6
#elif defined (CONFIG_FIBOCOM_MC665) || defined (CONFIG_FIBOCOM_MC660)
#define FIBO_SIM1_DECECT_GPIO 0
#else
#define FIBO_SIM1_DECECT_GPIO 0xff
#endif

#ifdef CONFIG_FIBOCOM_MC669
#define FIBO_GTFMODE_PIN_INDEX 51
#elif defined CONFIG_FIBOCOM_MC660
#define FIBO_GTFMODE_PIN_INDEX 4
#else
#define FIBO_GTFMODE_PIN_INDEX 0xff
#endif

#ifdef CONFIG_FIBOCOM_HALUO_AT
#define FIBO_DCD_PIN_INDEX 0
#elif defined CONFIG_FIBOCOM_MC669
#define FIBO_DCD_PIN_INDEX 48
#elif defined CONFIG_FIBOCOM_MC661
#define FIBO_DCD_PIN_INDEX 21
#elif defined CONFIG_FIBOCOM_MC660
#define FIBO_DCD_PIN_INDEX 63
#else
#define FIBO_DCD_PIN_INDEX 0xff
#endif

#define FIBO_HEADSET_PIN_INDEX 56
#define FIBO_COMMON_QUEUE_LEN 8
#define FIBO_AT_CMD_COMMON_RSP_LEN 64
#define FIBO_AT_CMD_ENLARGE_RSP_LEN 512
#define FIBO_FFS_DIR CONFIG_FS_SYS_MOUNT_POINT "FFS"
#define EV_FIBO_DTR_FALLING 0xB0000000
#define EV_FIBO_FTP_CLOSE 0xB0000001
typedef uint8_t (*temperature_callback)();

typedef struct
{
    uint8_t type;
    uint8_t rate;
    temperature_callback callback;
} st_mtsm;

typedef struct
{
    osiTimer_t *mtsm_event_timer;
    uint32_t period;
} mtsm_timer;

typedef struct
{
    uint16_t wakeup_mode;
    uint16_t wakeup_state;
} wakeup_cfg;

typedef struct
{
    uint16_t wrim_type0;
    uint16_t duration0;
    uint16_t wrim_type1;
    uint16_t duration1;
    uint16_t wrim_type2;
    uint16_t duration2;
} wrim_cfg;

typedef struct
{
    uint16_t lpm_mode;
    uint16_t lpm_submode;
} lpmmode;

typedef struct
{
    uint16_t delay_out;
    uint16_t delay_in;
    uint16_t sleep_again;
} gttime;

typedef enum
{
    NV_ITEM_LPG_SWITCH,
    NV_ITEM_LPMMODE,
    NV_ITEM_GTTIME,
    NV_ITEM_GTFMODE,
    NV_ITEM_LPG_CFG,
    NV_ITEM_KEYPAD_SWITCH,
    NV_ITEM_CHARGE_SWITCH,
    NV_ITEM_DUMP_CAUSE,
    NV_ITEM_OCP_LEVEL,
    NV_ITEM_SIM1_DET_GPIO,
    NV_ITEM_POWERON_NPI_BYPASS,
    NV_ITEM_MAX
} nv_item_index;

typedef struct
{
    nv_item_index nv_item_id;
} fibo_nv_cfg;

typedef struct
{
    bool lpg_switch;
    lpmmode lpm_mode;
    gttime gt_time;
    uint8_t gt_fmode;
    bool lpg_cfg;
    bool keypad_switch;
    bool charge_switch;
    bool bootcause_switch;
    uint8_t ocp_switch;
    uint8_t sim1_det_gpio;
    uint8_t poweron_npi_bypass;
} fibo_nv_data;

typedef void (*wakeupin_callback)(void* param);

typedef enum FIBO_COMMON_MSG
{
    FIBO_WAKEUP_MODEM_MSG,
    FIBO_SLEEP_MODEM_MSG,
    FIBO_WAKELOCK_RESET_MSG,
    FIBO_FMODE_ENTER_MSG,
    FIBO_FMODE_QUIT_MSG,
    FIBO_DTR_TRIGGER_MSG,
    FIBO_WAKEUPOUT_MSG,
    FIBO_HEADSET_PLUG_IN_MSG,
    FIBO_HEADSET_PLUG_OUT_MSG,
    FIBO_WAKEUP_TY_PININT_MSG,
    FIBO_SLEEP_TY_PININT_MSG,
    FIBO_START_SCAN_CODE_MSG,
    FIBO_STOP_SCAN_CODE_MSG,
    FIBO_COMMON_MSG_MAX
} fibo_common_msg_id;

typedef enum {
    FIBO_MBEDTLS_MD_NONE = 0,
    FIBO_MBEDTLS_MD_MD2,
    FIBO_MBEDTLS_MD_MD4,
    FIBO_MBEDTLS_MD_MD5,
    FIBO_MBEDTLS_MD_SHA1,
    FIBO_MBEDTLS_MD_SHA224,
    FIBO_MBEDTLS_MD_SHA256,
    FIBO_MBEDTLS_MD_SHA384,
    FIBO_MBEDTLS_MD_SHA512,
    FIBO_MBEDTLS_MD_RIPEMD160,
} fibo_mbedtls_md_type_t;

typedef struct
{
    fibo_common_msg_id msg_id;
} fibo_quene_msg_t;

typedef struct
{   
    uint16_t pin_index;
    uint16_t duration;
    bool is_high_wake;
    bool delay_send;
} wake_out_t;

int fibo_nvmReadItem(uint16_t nvid, void *buf, unsigned size);
int fibo_nvmWriteItem(uint16_t nvid, void *buf, unsigned size);
//void atCmdHandleMMAD(atCommand_t *cmd);
//void atCmdHandleMTSM(atCommand_t *cmd);
//void atCmdHandleTGPIO(atCommand_t *cmd);
//void atCmdHandleGPIO(atCommand_t *cmd);
void atCmdHandleWRIM(atCommand_t *cmd);
void atCmdHandleGTWAKE(atCommand_t *cmd);
void atCmdHandleGTLPMMODE(atCommand_t *cmd);
void atCmdHandleGTPMTIME(atCommand_t *cmd);
void atCmdHandleATS24(atCommand_t *cmd);

void fibo_wakelock_modem_switch(void);
bool fibo_wakesleepIsrDeinit(uint8_t pin_index);

void atCmdHandleKEYSWITCH(atCommand_t * cmd);
bool fibo_setChargeSwitch(bool is_switchoff);
#endif
