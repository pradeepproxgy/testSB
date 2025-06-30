#include "osi_api.h"
#include "osi_log.h"
#include "hal_config.h"
#include "osi_sysnv.h"
#include "fibo_opencpu.h"

#define ITEM_INT_VAL(pval) (*((int *)pval))

fibo_config_t fibo_config;
int g_cust_remove_at1 = 0;
static bool share_ip_flag = false;
void fibo_set_config(fibo_config_t *fibo_config)
{
    return;
}

fibo_config_t *fibo_get_config(void)
{
    UINT32 critical;
    critical =fibo_enterCS();
    memset(&fibo_config,0,sizeof(fibo_config_t));
    fibo_config.gSysnvUsbWorkMode = ITEM_INT_VAL(&gSysnvUsbWorkMode);
    fibo_config.mqtt_num = 2;
    fibo_config.sim1_dect = 19;

    //customization
    fibo_config.ecm_auto_dial = 0;
    fibo_config.ecm_host_detect = 0;
    //save dumpinfo
    fibo_config.save_dumpinfo_en = true;

    //skip vbatsense
    fibo_config.skip_vbatsense =false;
    fibo_config.getCurbatVol = false;
    //usb printer mode
    fibo_config.usb_printer_en =false;
    //usb buff config
    /*The value of rx_dma_size >= 512 bytes, 
      and the value of rx_buf_size must be an integer multiple of rx_dma_size and at least twice.
      Similarly, the same to tx_dma_size and tx_buf_size.
      0 use default config.
    */
    fibo_config.usb_buffcfg.rx_buf_size=0;
    fibo_config.usb_buffcfg.rx_dma_size=0;
    fibo_config.usb_buffcfg.tx_buf_size=0;
    fibo_config.usb_buffcfg.tx_dma_size=0;
    fibo_config.usb_buffcfg.tx_wait_timeout=0;

    fibo_exitCS(critical);

    fibo_config.ctzu_en = true;

    return (fibo_config_t *)&fibo_config;
}

bool fibo_get_usb_custom_enable(void)
{
    fibo_config_t *config = fibo_get_config();
    if (config->usb_custom_en ==1)
       return true;
    else
       return false;
}

bool fibo_get_usb_printer_enable(void)
{
    fibo_config_t *config = fibo_get_config();
    return config->usb_printer_en;
}

bool fibo_get_share_ip_enable(void)
{
        OSI_PRINTFE("fibo_get_share_ip_enable is %d", share_ip_flag);
        return share_ip_flag;
}
fibo_usb_buffcfg_t *fibo_get_usb_buffcfg(void)
{
    fibo_config_t *config = fibo_get_config();
    return &(config->usb_buffcfg);
}

#ifndef CONFIG_FIBOCOM_ALIPAY_SDK
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
    if (file != NULL && func != NULL && failedexpr != NULL)
    {
        OSI_LOGXE(OSI_LOGPAR_SSIS, 0, "assertion \"%s\" failed: %s:%d in file \"%s\"",
        failedexpr, func, line, file);
    }
    osiPanicPosix();
}
#endif

