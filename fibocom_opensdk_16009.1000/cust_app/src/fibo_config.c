#include "osi_api.h"
#include "osi_log.h"
#include "hal_config.h"
#include "osi_sysnv.h"
#include "fibo_opencpu.h"
#include "fibocom.h"
#include "drv_hid.h"
#include "usb/usb_ch9.h"
#include "drv_usb.h"
#include "hal_iomux.h"

#define ITEM_INT_VAL(pval) (*((int *)pval))

fibo_config_t fibo_config;
int g_cust_remove_at1 = 0;
static bool share_ip_flag = false;
static char *ecm_host_ip = NULL;//format :"192.168.x.x"
fibo_keypad_info_t keypad_info;
uint32_t funpad_list[49];

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
    fibo_config.sim1_pin = 0;
    fibo_config.lpg_onoff = 1;
    for(int i = 0;i<PORT_MAX;i++)
        {
           fibo_config.uart_timer[i] = 1;
        }
    //customization
    fibo_config.ecm_auto_dial = 0;
    fibo_config.ecm_host_detect = 0;
    //save dumpinfo
    fibo_config.save_dumpinfo_en = true;

    //skip vbatsense
    fibo_config.skip_vbatsense =false;
    fibo_config.getCurbatVol = false;
     //usb string
    fibo_config.usb_custom_en =0;
    fibo_config.usb_vid =0;
    fibo_config.usb_pid =0;
    fibo_config.product =0;
    fibo_config.manufacturer=0;
    fibo_config.serialnum=0;
    fibo_config.usb_bcd =0;
    fibo_config.usb_hid_at_en=false;
    //usb printer mode
    fibo_config.usb_printer_en =false;
    /*MTC0789-59 begin wj add acm support*/
    fibo_config.usb_hid_acm_en =false;
    /*MTC0789-59 end wj add acm support*/
    //usb buff config
    /*The value of rx_dma_size >= 512 bytes, 
      and the value of rx_buf_size must be an integer multiple of rx_dma_size and at least twice.
      Similarly, the same to tx_dma_size and tx_buf_size.
      0 use default config.
      printer_buffcfg is used for DRV_NAME_USRL_COM5   DRV_NAME_ACM2
      usb_buffcfg is used for DRV_NAME_USRL_COM6   DRV_NAME_ACM3
    */
    fibo_config.printer_buffcfg.rx_buf_size=0;
    fibo_config.printer_buffcfg.rx_dma_size=0;
    fibo_config.printer_buffcfg.tx_buf_size=0;
    fibo_config.printer_buffcfg.tx_dma_size=0;
    fibo_config.printer_buffcfg.tx_wait_timeout=0;

    fibo_config.usb_buffcfg.rx_buf_size=0;
    fibo_config.usb_buffcfg.rx_dma_size=0;
    fibo_config.usb_buffcfg.tx_buf_size=0;
    fibo_config.usb_buffcfg.tx_dma_size=0;
    fibo_config.usb_buffcfg.tx_wait_timeout=0;

    fibo_exitCS(critical);

    fibo_config.ctzu_en = true;
    fibo_config.app_whole_reboot = true; //reboot module after upgrade
    fibo_config.app_delta_reboot = true;

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
bool fibo_get_usb_hidacm_enable(void)
{
    fibo_config_t *config = fibo_get_config();
    return config->usb_hid_acm_en;
}

bool fibo_get_share_ip_enable(void)
{
    OSI_PRINTFE("fibo_get_share_ip_enable is %d", share_ip_flag);
    return share_ip_flag;
}

char* fibo_get_ecm_host_ip(void)
{
    OSI_PRINTFE("fibo_get_ecm_host_ip is %s", ecm_host_ip);
    return ecm_host_ip;
}

fibo_usb_buffcfg_t *fibo_get_usb_buffcfg(void)
{
    fibo_config_t *config = fibo_get_config();
    return &(config->usb_buffcfg);
}
fibo_usb_buffcfg_t *fibo_get_printer_buffcfg(void)
{
    fibo_config_t *config = fibo_get_config();
    return &(config->printer_buffcfg);
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

fibo_keypad_info_t *fibo_get_keypad_enable(void)
{
    memset(&keypad_info,0,sizeof(fibo_keypad_info_t));
    keypad_info.enable_keypad = false;
    keypad_info.timeout = 0;
    return &keypad_info;
}

void fibo_iomux_set_init_config(uint32_t **fun_pad)
{
    memset(funpad_list, 0, sizeof(funpad_list));
    //If you remove the default configuration of the target pins in the osiboot stage,
    //fill in the pads corresponding to the target pins in the order of the funpad_list array.
    //Get the ball name of the pin in the gpio multiplexing table,
    //and find the pad corresponding to the ball name in hal_iomux_pin.h all pads.
    //The following example
    //funpad_list[0] = HAL_IOMUX_PAD_UART_1_RTS;
    //funpad_list[1] = HAL_IOMUX_PAD_I2C_M2_SCL;
    *fun_pad = funpad_list;
    return;
}


INT32 fibo_cus_fw_init()
{
    fibo_textTrace("fibo_cus_fw_init");
    if(OSI_BOOTMODE_NORMAL == osiGetBootMode())
    {
        return 1; //app will startup just by  1 ,if fw init fail ,maybe  need return 0 for discarding APP startup
    }

    return 1; //app will startup just by  1 
}

