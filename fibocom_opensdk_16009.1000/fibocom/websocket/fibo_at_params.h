#ifndef PLATEFORM_UNISOC_H
#define PLATEFORM_UNISOC_H

#include "ats_config.h"
#include "cfw.h"
#include "at_cfg.h"
#include "sockets.h"
#include "nvm.h"
#include "osi_api.h"
#include "osi_log.h"
#include "at_response.h"
#include "at_command.h"
#include "at_engine.h"
#include "at_engine_imp.h"
#include "ffw_result.h"
#include "ffw_plat_header.h"

#define fibo_at_cmd_t atCommand_t
#define fibo_at_params_t atCmdParam_t
#define fibo_get_params(index) (pParam->params[(index)])
#define fibo_atcmd_come(params)                             \
    do                                                      \
    {                                                       \
        g_atCmdEngine = params->engine;                     \
        FFW_LOG_INFO("g_atCmdEngine=%p", g_atCmdEngine); \
    } while (0)

#define FIBO_AT_CMD_SET AT_CMD_SET
#define FIBO_AT_CMD_READ AT_CMD_READ
#define FIBO_AT_CMD_TEST AT_CMD_TEST
#define FIBO_AT_CMD_EXEC AT_CMD_EXE

#define get_str_param_val_len(cmd, index, val, val_len) \
    do                                                  \
    {                                                   \
        int t = index++;                                \
        val = cmd->params[t]->value;                    \
        val_len = cmd->params[t]->length;               \
    } while (0);

#define get_str_param(cmd, index, str_max_len)                   \
    ({                                                           \
        char *ret;                                               \
        atCmdParam_t *s = cmd->params[index++];                  \
        bool paramRet = true;                                    \
        ret = (char *)atParamStr(s, &paramRet);                  \
        if (!paramRet || strlen(ret) > (str_max_len))            \
        {                                                        \
            FFW_LOG_INFO("get str params fail:%s", s->value); \
            fibo_atcmd_return_err(ERR_AT_CME_PARAM_INVALID);     \
            return;                                              \
        }                                                        \
        ret;                                                     \
    })

#define try_get_str_param(cmd, index, str_max_len)    \
    ({                                                \
        char *ret;                                    \
        atCmdParam_t *s = cmd->params[index];         \
        bool paramRet = true;                         \
        ret = (char *)atParamStr(s, &paramRet);       \
        if (!paramRet || strlen(ret) > (str_max_len)) \
        {                                             \
            ret = NULL;                               \
        }                                             \
        else                                          \
        {                                             \
            index++;                                  \
        }                                             \
        ret;                                          \
    })

#define get_range_int_param(cmd, index, min, max)                     \
    ({                                                                \
        int ret;                                                      \
        atCmdParam_t *s = cmd->params[index++];                       \
        bool paramRet = true;                                         \
        ret = atParamUintInRange(s, (min), (max), &paramRet);         \
        if (!paramRet)                                                \
        {                                                             \
            FFW_LOG_INFO("get rang int params fail:%s", s->value); \
            fibo_atcmd_return_err(ERR_AT_CME_PARAM_INVALID);          \
            return;                                                   \
        }                                                             \
        ret;                                                          \
    })

#define get_int_param(cmd, index)                                \
    ({                                                           \
        int ret;                                                 \
        atCmdParam_t *s = cmd->params[index++];                  \
        bool paramRet = true;                                    \
        ret = atParamUint(s, &paramRet);                         \
        if (!paramRet)                                           \
        {                                                        \
            FFW_LOG_INFO("get int params fail:%s", s->value); \
            fibo_atcmd_return_err(ERR_AT_CME_PARAM_INVALID);     \
            return;                                              \
        }                                                        \
        ret;                                                     \
    })

static atCmdEngine_t *g_atCmdEngine = NULL;
extern struct netif *netif_default;


static inline bool fibo_wireless_is_connection()
{
    ///wait 100ms for system set @c netif_default to NULL when wireless disconnect
    osiThreadSleep(100);
    return NULL != netif_default;
}

#define fibo_atcmd_get_params_count(p) (p)->param_count

static inline int fibo_atcmd_append_text(const char *text)
{
    int ret = FFW_R_SUCCESS;

    atCmdRespInfoText(g_atCmdEngine, text);

    return ret;
}

static inline int fibo_atcmd_append_ntext(const char *text, uint32_t n)
{
    int ret = FFW_R_SUCCESS;

    atCmdRespInfoNText(g_atCmdEngine, text, n);

    return ret;
}

static inline int fibo_atcmd_return_ok()
{
    int ret = FFW_R_SUCCESS;

    atCmdRespOK(g_atCmdEngine);

    return ret;
}

static inline int fibo_atcmd_return_err(int code)
{
    int ret = FFW_R_SUCCESS;

    atCmdRespCmeError(g_atCmdEngine, code);

    return ret;
}

#if 0
static inline int fibo_atcmd_write(void *buf, uint32_t len)
{
    atCmdWrite(g_atCmdEngine, buf, len);

    return RACOON_R_SUCCESS;
}

static inline int fibo_atcmd_flush()
{
    atCmdWriteFlush(g_atCmdEngine);
    return RACOON_R_SUCCESS;
}
#endif

static inline int fibo_atcmd_set_linemode()
{
    atCmdSetLineMode(g_atCmdEngine);
    return FFW_R_SUCCESS;
}

 static void atCmdRespOutputPrompt_ex(atCmdEngine_t *engine)
{
    atChannelSetting_t *chsetting = atCmdChannelSetting(engine);
    uint8_t rsp[5] = {chsetting->s3, chsetting->s4, '>', '\r','\n'};
    atCmdWrite(engine, rsp, 5);
    atCmdWriteFlush(engine);
}

static inline int fibo_atcmd_send_prompt()
{
    atCmdRespOutputPrompt_ex(g_atCmdEngine);
    return FFW_R_SUCCESS;
}

static inline int fibo_atcmd_set_bypassmode(int (*cb)(void *param, const void *data, size_t size), void *arg)
{
    atCmdSetBypassMode(g_atCmdEngine, (atCmdBypassCB_t)cb, arg);
    return FFW_R_SUCCESS;
}

static inline int fibo_atcmd_get_crlf(char *cr, char *lf)
{
    atChannelSetting_t *chsetting = atCmdChannelSetting(g_atCmdEngine);
    *cr = chsetting->s3;
    *lf = chsetting->s4;
    return FFW_R_SUCCESS;
}

static inline int fibo_atcmd_send_urc(const char *str)
{
    int ret = FFW_R_SUCCESS;

    CHECK(NULL == g_atCmdEngine, FFW_R_FAILED, "g_atCmdEngine is null");

    atCmdRespUrcText(g_atCmdEngine, str);

error:
    return ret;
}

static inline int fibo_atcmd_send_urc_len(const char *text, int len)
{
    int ret = FFW_R_SUCCESS;

    atCmdRespUrcNText(g_atCmdEngine, text, len);

    return ret;
}

#endif /* PLATEFORM_UNISOC_H */
