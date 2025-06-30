/* Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "osi_log.h"
#include "at_cfw.h"
#include "osi_sysnv.h"
#include "at_command.h"
#include "at_response.h"
#include "drv_config.h"
#include "diag_config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lwip/opt.h"
#include "at_cfw_log.h"
#ifdef CONFIG_FIBOCOM_BASE
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "drv_usb.h"
#endif
#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('A', 'T', 'S', BTOH(AT_LOG_MISC))

enum
{
    SYSNV_READ = 0,
    SYSNV_SET = 1,
    SYSNV_CLEAR = 2
};

enum
{
    SYSNV_TYPE_BOOL = 0,
    SYSNV_TYPE_INT = 1,
};

typedef struct
{
    void *value;
    int type;
    int min;
    int max;
    const char *name;
    const char *hint;
} sysnvItem_t;

#define ITEM_INT_VAL(pval) (*((int *)pval))
#define ITEM_BOOL_VAL(pval) (*((bool *)pval))

#ifdef CONFIG_FIBOCOM_BASE
extern bool fibo_get_usb_custom_enable(void);
extern bool fibo_get_usb_printer_enable(void);
extern bool fibo_get_usb_hidacm_enable(void);
#endif

static const char kSysnvBoolHint[] = "0.disable; 1.enable";
#if IP_NAT
static const char kSysnvNatCfgHint[] = "0 disable; 16bit config nat cid; 101 =0000000100100101 sim 0 cid(1,3,6 enable)(2,4,5,7,8 disable) sim1 cid(1 enable)(2,3,4,5,6,7,8 disable)";
#endif

#ifndef CONFIG_AT_CUS_SUPPORT
#ifdef CONFIG_AT_SELFREG_SUPPORT
static const char kSysnvSelfRegisterHint[] = "0.disable; 1.enable self-registration";
#endif
#endif

static const sysnvItem_t gAtSysnvMap[] = {
    {&gSysnvTraceEnabled, SYSNV_TYPE_BOOL, 0, 1, "ap_trace_en", kSysnvBoolHint},
    {&gSysnvDeepSleepEnabled, SYSNV_TYPE_BOOL, 0, 1, "deep_sleep_en", kSysnvBoolHint},
    {&gSysnvPsmSleepEnabled, SYSNV_TYPE_BOOL, 0, 1, "psm_sleep_en", kSysnvBoolHint},
#ifdef CONFIG_USB_SUPPORT
#if defined(CONFIG_SOC_8910)
#ifdef CONFIG_USB_ETHER_SUPPORT
    {&gSysnvUsbWorkMode, SYSNV_TYPE_INT, 0, 10, "usbmode",
     "0.Charger only; "
     "1.RDA (8910 ROM) serial; "
     "2.Eight serials; "
#ifdef CONFIG_USB_RNDIS_SUPPORT
     "3.RNDIS and serials; "
#endif
     "4.SPRD U2S Diag; "
     "5.ECM and serials; "
#ifdef CONFIG_USB_ACM_SUPPORT
     "6.ECM ACM[0-4], USL[2,4]; "
#endif
#ifdef CONFIG_USB_RNDIS_SUPPORT
     "7.RNDIS only; "
#endif
     "8.ECM only; "
#ifdef CONFIG_USB_MTP_SUPPORT
     "9.MTP only"
#endif
     "10.HID only"},
#else
    {&gSysnvUsbWorkMode, SYSNV_TYPE_INT, 0, 10, "usbmode",
     "0.Charger only; "
     "1.RDA (8910 ROM) serial; "
     "2.Eight serials;"
#ifdef CONFIG_USB_RNDIS_SUPPORT
     "3.RNDIS and serials; "
#endif
     "4.SPRD U2S Diag;"
#ifdef CONFIG_USB_ACM_SUPPORT
     "6.ECM ACM[0-4], USL[2,4]; "
#endif
#ifdef CONFIG_USB_RNDIS_SUPPORT
     "7.RNDIS only; "
#endif
#ifdef CONFIG_USB_MTP_SUPPORT
     "9.MTP only"
#endif
     "10.HID only"},
#endif
#elif defined(CONFIG_SOC_8850)
#ifdef CONFIG_USB_ETHER_SUPPORT
    {&gSysnvUsbWorkMode, SYSNV_TYPE_INT, 0, 11, "usbmode",
     "0.Charger only; 1.8850 ACM Serials; "
     "2.Four Serials; 3.RNDIS and Serials; "
     "4.NPI and CP log; 5.ECM and Serials; 6.ECM ACM and Serials;"
     "7.RNDIS only; 8.ECM only; 9.MTP only; 10.MSG only"
     "11.RNDIS/ECM Adaption;"},
#else
    {&gSysnvUsbWorkMode, SYSNV_TYPE_INT, 0, 10, "usbmode",
     "0.Charger only; 1.8850 ACM Serials; "
     "2.Four Serials;"
     "4.NPI and CP log;"
     "9.MTP only; 10.MSG only"},
#endif
#endif
#ifdef CONFIG_USB_HOST_SUPPORT
    {&gSysnvUsbMode, SYSNV_TYPE_BOOL, 0, 1, "usbmode_H_S", "0.USB Device; 1.USB Host"},
#endif
    {&gSysnvUsbDetMode, SYSNV_TYPE_INT, 0, 1, "usb_detect_mode", "0.Charger Detect; 1.Always On"},
#endif
#ifdef CONFIG_DIAG_DEVICE_USRL_SUPPORT
    {&gSysnvDiagDevice, SYSNV_TYPE_INT, 1, 2, "diag_device",
     "1.diag device uart; 2.diag device usb serial"},
#endif
    {&gSysnvSimHotPlug, SYSNV_TYPE_BOOL, 0, 1, "sim_hotplug", kSysnvBoolHint},
    {&gSysnvSimVoltTrigMode, SYSNV_TYPE_BOOL, 0, 1, "sim_hotplug_vol_trigmode", kSysnvBoolHint},
#if IP_NAT
    {&gSysnvNATCfg, SYSNV_TYPE_INT, 0, 65535, "nat_cfg", kSysnvNatCfgHint},
#endif
    {&gSysnvFstraceMask, SYSNV_TYPE_INT, 0, 65535, "fstrace",
     "[0]: aplog, [1] profile, [2] bsdump, [3] cplog, [4] zsplog, [5] moslog"},
    {&gSysnvPoffChargeEnable, SYSNV_TYPE_BOOL, 0, 1, "poff_charge_en", kSysnvBoolHint},
    {&gSysnvPm2GpioInd, SYSNV_TYPE_BOOL, 0, 1, "pm2_ind", kSysnvBoolHint},
    {&gSysnvPm3GpioInd, SYSNV_TYPE_BOOL, 0, 1, "pm3_ind", kSysnvBoolHint},
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
    {&gSysnvUart1RxPsmPullUp, SYSNV_TYPE_BOOL, 0, 1, "uart1_rx_psm_pullup", kSysnvBoolHint},
    {&gSysnvPm2BootEnabled, SYSNV_TYPE_BOOL, 0, 1, "psm_pm2boot", kSysnvBoolHint},
#endif
#ifndef CONFIG_AT_CUS_SUPPORT
#ifdef CONFIG_AT_SELFREG_SUPPORT
    {&gSysnvSelfRegisterEnabled, SYSNV_TYPE_INT, 0, 1, "self_register", kSysnvSelfRegisterHint},
#endif
#endif
#if !defined(CONFIG_SOC_8811) && !defined(CONFIG_SOC_8821)
    {&gSysnvLogOutput, SYSNV_TYPE_INT, 0, 1, "netinfo_logout", kSysnvBoolHint},
#endif
    {&gSysnvSysWdtFeedPeriod, SYSNV_TYPE_INT, 0, 65535, "sysWdt_feedPeriod"},
    {&gSysnvPmicWdtFeedPeriod, SYSNV_TYPE_INT, 0, 65535, "pmicWdt_feedPeriod"},
#if defined(CONFIG_SOC_8850)
    {&gSysnvImsLogOutput, SYSNV_TYPE_BOOL, 0, 1, "ims_logout", kSysnvBoolHint},
#endif
#if defined(CONFIG_WDT_ENABLE)
    {&gBsWatchdogReset, SYSNV_TYPE_BOOL, 0, 1, "WatchdogReset", kSysnvBoolHint},
#endif
    {&gSysnvNwRatMode, SYSNV_TYPE_INT, 2, 4, "network mode gsm or lte"},
};

const sysnvItem_t *prvNameGetItem(const char *name)
{
    for (unsigned i = 0; i < OSI_ARRAY_SIZE(gAtSysnvMap); ++i)
    {
        const sysnvItem_t *item = &gAtSysnvMap[i];
        if (strcmp(item->name, name) == 0)
            return item;
    }
    return NULL;
}

#ifdef CONFIG_USB_HOST_SUPPORT
sysnvItem_t *prvNameGetItemForUsbMode(const char *name)
{
    for (unsigned i = 0; i < OSI_ARRAY_SIZE(gAtSysnvMap); ++i)
    {
        sysnvItem_t *item = (sysnvItem_t *)&gAtSysnvMap[i];
        if (strcmp(item->name, name) == 0)
            return item;
    }
    return NULL;
}
#endif
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
int getLpuartSetMode(void)
{
    const char name[20] = "psm_pm2boot";
    int val = 0;
    const sysnvItem_t *item = prvNameGetItem(name);
    if (item == NULL)
    {
        OSI_LOGXE(OSI_LOGPAR_S, 0x100080b8, "at set sysnv %s not available", name);
    }
    if (item->type == SYSNV_TYPE_BOOL)
        return val = ITEM_BOOL_VAL(item->value) ? 1 : 0;

    return val;
}
#endif
void atCmdHandleSYSNV(atCommand_t *cmd)
{
    if (AT_CMD_SET == cmd->type)
    {
        bool paramok = true;
        uint8_t type = atParamUintInRange(cmd->params[0], 0, 2, &paramok);
        if (!paramok)
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
        if (gAtSetting.ipr == 0)
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_FAIL);
#endif
        if (type == SYSNV_CLEAR)
        {
            if (cmd->param_count != 1)
                RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);

            osiSysnvClear();
            RETURN_OK(cmd->engine);
        }

        const char *name = atParamStr(cmd->params[1], &paramok);
        if (!paramok)
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);

        const sysnvItem_t *item = prvNameGetItem(name);
        if (item == NULL)
        {
            OSI_LOGXE(OSI_LOGPAR_S, 0x100080b8, "at set sysnv %s not available", name);
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_ERROR);
        }

        if (type == SYSNV_READ)
        {
            if (cmd->param_count != 2)
                RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);

            int val = 0;
            if (item->type == SYSNV_TYPE_BOOL)
                val = ITEM_BOOL_VAL(item->value) ? 1 : 0;
            else if (item->type == SYSNV_TYPE_INT)
                val = ITEM_INT_VAL(item->value);
            else
                RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_FAIL);
            char resp[64];
            snprintf(resp, 64, "+SYSNV: \"%s\",%d", item->name, val);
            atCmdRespInfoText(cmd->engine, resp);
            RETURN_OK(cmd->engine);
        }
        else
        {
            if (item->type == SYSNV_TYPE_BOOL || item->type == SYSNV_TYPE_INT)
            {
                int val = atParamIntInRange(cmd->params[2], item->min, item->max, &paramok);
                if (!paramok)
                    RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_ERROR);
#ifdef CONFIG_FIBOCOM_BASE
                if (val ==1)
                {
                    halIomuxSetFunction(HAL_IOMUX_FUN_UART_4_RXD_PAD_UART_2_CTS);
                    halIomuxSetFunction(HAL_IOMUX_FUN_UART_4_TXD_PAD_UART_2_RTS);
                }
#endif
                if (item->type == SYSNV_TYPE_BOOL)
                    ITEM_BOOL_VAL(item->value) = !!val;
                else if (item->type == SYSNV_TYPE_INT)
                    ITEM_INT_VAL(item->value) = val;

#if defined(CONFIG_SOC_8910)
                if (strcmp(item->name, "usbmode") == 0)
                {
#ifndef CONFIG_USB_RNDIS_SUPPORT
                    if ((val == 3) || (val == 7))
                        RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
#endif
#ifndef CONFIG_USB_ACM_SUPPORT
                    if (val == 6)
                        RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
#endif
#ifndef CONFIG_USB_MTP_SUPPORT
                    if (val == 9)
                        RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
#endif
                }
#endif
                if (!osiSysnvSave())
                {
                    OSI_LOGE(0x100080b9, "AT+SYSNV fail to save");
                    RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_FAIL);
                }
                RETURN_OK(cmd->engine);
            }
            else
            {
                RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_FAIL);
            }
        }
        RETURN_OK(cmd->engine);
    }
    else if (AT_CMD_TEST == cmd->type)
    {
        char *resp = (char *)malloc(2048);
        for (unsigned i = 0; i < OSI_ARRAY_SIZE(gAtSysnvMap); ++i)
        {
            const sysnvItem_t *item = &gAtSysnvMap[i];
            snprintf(resp, 2048, "+SYSNV: \"%s\", \"%s\"", item->name, item->hint);
            atCmdRespInfoText(cmd->engine, resp);
        }
        free(resp);
        RETURN_OK(cmd->engine);
    }
    else
    {
        RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
    }
}

#ifdef CONFIG_FIBOCOM_BASE
int Getusbmode()
{
    char *name = "usbmode";
    const sysnvItem_t *item = prvNameGetItem(name);
    int val = ITEM_INT_VAL(item->value);
    return val;
}

void atCmdHandleGTUSBMODE(atCommand_t *cmd)
{
    bool paramok = true;
    char *name = "usbmode";

    uint8_t usbmaxnum=81;
    uint8_t list[9] ={2, 5, 3,DRV_8850USB_ELEC_ECM,DRV_8850USB_SERIALS_SPECIAL,DRV_USB_PRINTER,DRV_USB_PRINTER_AND_SERIALS,DRV_USB_HID_ACM_SERIALS,DRV_USB_HID_ACM};
    #ifdef CONFIG_FIBOCOM_SINGLE_SERIAL
    uint8_t usbminnum=72;
    #else
    uint8_t usbminnum=73;
    #endif

    const sysnvItem_t *item = prvNameGetItem(name);
    if (AT_CMD_SET == cmd->type)
    {
        int val = atParamIntInRange(cmd->params[0], usbminnum, usbmaxnum, &paramok);
        if (!paramok)
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_ERROR);
        if ((!fibo_get_usb_custom_enable())&&(val==76))
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
        #ifndef CONFIG_FIBOCOM_USB_SERIALS_SPECIAL
        if (val==77)
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
        #endif
        if ( (!fibo_get_usb_printer_enable()) && ((val==78)||(val==79)) )
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
        /*MTC0789-59 begin wj add acm support*/
        if ( (fibo_get_usb_hidacm_enable()) && ((val==78)||(val==79)) )
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
        
        if ( (!fibo_get_usb_hidacm_enable()) && ((val==80)||(val==81)) )
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
        /*MTC0789-59 end wj add acm support*/
        #ifdef CONFIG_FIBOCOM_SINGLE_SERIAL
         if (val==72)
             val=DRV_8850USB_SINGLE_SERIAL;
         else
             val = list[val - 73];
        #else
             val = list[val - 73];
        #endif

        
        ITEM_INT_VAL(item->value) = val;
        if (!osiSysnvSave())
        {
            OSI_LOGE(0, "AT+SYSNV fail to save");
            RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_FAIL);
        }
        RETURN_OK(cmd->engine);
    }
    else if(AT_CMD_READ == cmd->type)
    {
        int num = 0;
        int val = ITEM_INT_VAL(item->value);
        #ifdef CONFIG_FIBOCOM_SINGLE_SERIAL
        if (val==DRV_8850USB_SINGLE_SERIAL)
        {
           val=72;
           char respp[32];
           snprintf(respp, 32, "+GTUSBMODE: %d", val);
           atCmdRespInfoText(cmd->engine, respp);
           RETURN_OK(cmd->engine);
        }
        #endif
        for(num = 0; num < (usbmaxnum-72); num++)
        {
            if(list[num] == val)
                break;
        }
        val = num + 73;
        char resp[32];
        snprintf(resp, 32, "+GTUSBMODE: %d", val);
        atCmdRespInfoText(cmd->engine, resp);
        RETURN_OK(cmd->engine);
    }
    else if (AT_CMD_TEST == cmd->type)
    {
        char resp[64];
        uint8_t flag=0;
        if (fibo_get_usb_custom_enable())
        {
            flag=0x1;
        }
        else
        {
            flag=0x0;
        }
            
        #ifndef CONFIG_FIBOCOM_USB_SERIALS_SPECIAL
           flag|=0x0;
        #else
           flag|=0x2;
        #endif
        if (fibo_get_usb_printer_enable())
        {
            flag|=0x4;
        }
        else
        {
            flag|=0x0;
        }
        /*MTC0789-59 begin wj add acm support*/
        if (fibo_get_usb_hidacm_enable())
        {
            flag|=0x8;
        }
        else
        {
            flag|=0x0;
        }
        /*MTC0789-59 end wj add acm support*/
        switch(flag)
        {
              case 0x0:
              #ifdef CONFIG_FIBOCOM_SINGLE_SERIAL
              snprintf(resp, 64, "+GTUSBMODE: (72-75)");
              #else
              snprintf(resp, 64, "+GTUSBMODE: (73-75)");
              #endif
                      break;
              case 0x1:
              snprintf(resp, 64, "+GTUSBMODE: (73-76)");
                      break;
              case 0x2:
              snprintf(resp, 64, "+GTUSBMODE: (73-75,77)");
                      break;
              case 0x3:
              snprintf(resp, 64, "+GTUSBMODE: (73-77)");
                      break;
              case 0x4:
              snprintf(resp, 64, "+GTUSBMODE: (73-75,78-79)");
                      break;
              case 0x5:
              snprintf(resp, 64, "+GTUSBMODE: (73-76,78-79)");
                      break;
              case 0x6:
              snprintf(resp, 64, "+GTUSBMODE: (73-75,77-79)");
                      break;
              case 0x7:
              snprintf(resp, 64, "+GTUSBMODE: (73-79)");
                      break;
              /*MTC0789-59 begin wj add acm support*/
              case 0xC:
              snprintf(resp, 64, "+GTUSBMODE: (73-75,80-81)");
                      break;
              case 0xD:
              snprintf(resp, 64, "+GTUSBMODE: (73-76,80-81)");
                      break;
              case 0xE:
              snprintf(resp, 64, "+GTUSBMODE: (73-75,77,80-81)");
                      break;
              case 0xF:
              snprintf(resp, 64, "+GTUSBMODE: (73-77,80-81)");
                      break;
              /*MTC0789-59 end wj add acm support*/
        }
        atCmdRespInfoText(cmd->engine, resp);
        RETURN_OK(cmd->engine);
    }
    else
    {
        RETURN_CME_ERR(cmd->engine, ERR_AT_CME_EXE_NOT_SURPORT);
    }
}


void setNatCfgNv(uint32_t natCfg)
{
    char *name = {"nat_cfg"};
    const sysnvItem_t *item = prvNameGetItem(name);

    *((int *)item->value) = natCfg;                                //MTC0390-284

    if (!osiSysnvSave())
    {
        OSI_LOGE(0, "set nat_cfg 255 but save failed");
    }
}
#endif
