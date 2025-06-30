/*****************************************************************
*   Copyright (C), 2022, Shenzhen Fibocom Wireless Inc
*                  All rights reserved
*
*   FileName    : 
*   Author      : Charles Shi
*   Created     : 2022-01-05
*   Description : 
*****************************************************************/
#ifndef __FIBO_ALIPAY_ADAPT_H
#define __FIBO_ALIPAY_ADAPT_H

/*--------------------------------------*
 *              Head File               *
 *--------------------------------------*/
#include "alipay_macro_def.h"
#include "alipay_iot_coll_biz.h"
#include "alipay_iot_signer.h"
#include "ant_log.h"
#include "alipay_iot_sdk.h"
#include "ant_os.h"
#include "ant_event.h"
#include "ant_clib.h"
#include "ant_mem.h"
#include "ffw_log.h"

/*--------------------------------------*
 *             Enum Definition          *
 *--------------------------------------*/
typedef enum
{
    LOCAL_MSG_TIMER_EXPIRED = 998,
    LOCAL_MSG_SDK_START
} E_ALIPAY_IOT_SDK_LOCAL_MSG;

typedef enum
{
    ALI_SIGN_QRCODE = 0x01,
    ALI_SIGN_FACE_TOKEN,
    ALI_SIGN_RECV_QRCODE,
    ALI_SIGN_DELEGATED,
} E_FIBO_ALI_SIGN_MODE;

/*--------------------------------------*
 *            Macro Definition          *
 *--------------------------------------*/

#define STATIC static

#define FIBO_ALI_SIGN_DATA_MAX (128)
#define FIBO_ALI_SIGN_AMOUNT_MAX (32)

#define ALI_SDK_QUEUE_L1 (10)
#define ALI_SDK_QUEUE_L2 (20)

#define ALI_SDK_QUEUE_RECORD_NUM_MAX (ALI_SDK_QUEUE_L1 + ALI_SDK_QUEUE_L2)

/* log */
#define LOG_TAG "ANT_IOT"
#define ANT_IOT_LOGI(format, ...)                                                \
    do                                                                           \
    {                                                                            \
        ffw_log("[%s: %d]-" format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

#define ANT_IOT_LOGD(format, ...)                                                \
    do                                                                           \
    {                                                                            \
        ffw_log("[%s: %d]-" format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

#define ANT_IOT_LOGE(format, ...)                                                 \
    do                                                                            \
    {                                                                             \
        ffw_log("[%s: %d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

/*--------------------------------------*
 *          Struct Definition           *
 *--------------------------------------*/
/* Alipay IoT SDK Product Info */
typedef struct _S_FIBO_ALI_SDK_PROFILE_INFO
{
    alipay_sdk_profile_info_struct profile;
    bool is_set;
} S_FIBO_ALI_SDK_PROFILE_INFO;

/* Alipay IoT SDK transaction data */
typedef struct _S_FIBO_ALI_SDK_TRANS_DATA
{
    alipay_iot_transaction_data_struct data;
} S_FIBO_ALI_SDK_TRANS_DATA;

/* Alipay IoT SDK action data */
typedef struct _S_FIBO_ALI_SDK_ACT_DATA
{
    alipay_iot_action_data_struct data;
} S_FIBO_ALI_SDK_ACT_DATA;

/* Alipay IoT SDK production info */
typedef struct _S_FIBO_ALI_SDK_PD_INFO
{
    alipay_iot_production_info_struct pd_info;
} S_FIBO_ALI_SDK_PD_INFO;

/* Alipay IoT SDK sys time update ind */
typedef struct _S_FIBO_ALI_SDK_SYS_TIME_UPDATE_IND
{
    alipay_sdk_sys_time_update_ind_struct update_ind;
} S_FIBO_ALI_SDK_SYS_TIME_UPDATE_IND;

/* Alipay IoT SDK thread */
typedef struct _S_FIBO_ALI_SDK_THREAD
{
    int thread_id;
    ant_sem sem;
} S_FIBO_ALI_SDK_THREAD;

/* Alipay IoT Sign info */
typedef struct _S_FIBO_ALI_SIGN_INFO
{
    E_FIBO_ALI_SIGN_MODE sign_mode;
    char sign_data[FIBO_ALI_SIGN_DATA_MAX];
    char amount[FIBO_ALI_SIGN_AMOUNT_MAX];
} S_FIBO_ALI_SIGN_INFO;

/*--------------------------------------*
 *           Global Variable            *
 *--------------------------------------*/
extern S_FIBO_ALI_SDK_PROFILE_INFO g_alipay_sdk_profile;
extern int g_iot_sdk_user_tid;
extern S_FIBO_ALI_SDK_THREAD ali_sdk_task_th;

/*--------------------------------------*
 *            Extern Function           *
 *--------------------------------------*/
extern char *fibo_alipay_sdk_get_sdk_version(void);
extern char *fibo_alipay_sdk_get_sdk_build_time(void);
extern int fibo_alipay_sdk_set_profile(S_FIBO_ALI_SDK_PROFILE_INFO *p_ali_profile);
extern int fibo_alipay_sdk_manage_sdk_close(int iot_sdk_tid);
extern char *fibo_alipay_sdk_get_biztid(void);
extern void fibo_alipay_sdk_free_biztid(char *p_biztid);
extern int fibo_alipay_sdk_sign(S_FIBO_ALI_SIGN_INFO *p_sign_info, char *p_result, int *p_len_result);
extern int fibo_alipay_sdk_send_transaction_data(S_FIBO_ALI_SDK_TRANS_DATA *p_tans_data);
extern int fibo_alipay_sdk_send_production_info(S_FIBO_ALI_SDK_PD_INFO *p_pd_info);
extern int fibo_alipay_sdk_send_action_data(S_FIBO_ALI_SDK_ACT_DATA *p_act_data);
extern int fibo_alipay_sdk_task_send_message(int target_id, int src_id, int msg_id, void *data);
extern void fibo_alipay_sdk_msg_task(void *para);
extern void fibo_alipay_sdk_set_terminal_id(const char *p_buf);
extern char *fibo_alipay_sdk_get_terminal_id(void);

#endif
