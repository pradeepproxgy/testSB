/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "drv_names.h"
int usbState = 0;

typedef enum
{
    DRV_SERIAL_EVENT_BROKEN = (1 << 3),      ///< usb port broken
    DRV_SERIAL_EVENT_READY = (1 << 4),       ///< usb port ready
    DRV_SERIAL_EVENT_OPEN = (1 << 5),        ///< open by host
    DRV_SERIAL_EVENT_CLOSE = (1 << 6),       ///< close by host
}usbEvent;


UINT32 usbdev_daemon_sem = 0;

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}
void recv_cb(atDevice_t *th, void *buf, size_t size, void *arg)
{
    fibo_textTrace("usbdev test: size=%d, data=%s", size, (char *)buf);
}

int UsbDev_Event (int arg)
{
    usbState = arg;
        fibo_textTrace("usbdev report msg  %d\n", usbState);
    switch (usbState)
    {
        case DRV_SERIAL_EVENT_BROKEN:
        {
            fibo_textTrace("the usbdev had broken!");
            break;
        }
        case DRV_SERIAL_EVENT_READY:
        {
            fibo_textTrace("the usbdev had ready!");
            break;
        }
        case DRV_SERIAL_EVENT_OPEN:
        {
            fibo_textTrace("the usbdev had opened! ");
            fibo_sem_signal(usbdev_daemon_sem);
            break;
        }
        case DRV_SERIAL_EVENT_CLOSE:
        {
            fibo_textTrace("the usbdev had closed!");
            break;
        }
        default:
        {
            fibo_textTrace("usbdev unkonw state!");
            usbState = 0;
            break;
        }
    }
    return 0;
}


static void usb_thread_entry(void *param)
{
    char *test = "test usb";
    fibo_textTrace("application thread enter, param 0x%x", param);
    uint8_t usbstatus=0;
    INT32 usbmode =0;

    atDevice_t *th;
    usbdev_daemon_sem = fibo_sem_new(0);
    th = FIBO_usbDevice_init(DRV_NAME_USRL_COM6, recv_cb);
    FIBO_usbDevice_State_report(th, (Report_UsbDev_Event)UsbDev_Event);
    fibo_taskSleep(5000);

    while(1)
    {
        if(usbState == DRV_SERIAL_EVENT_OPEN)
        {
            FIBO_usbDevice_send(th, (void *)test, strlen(test));
        }
        else if(usbState == DRV_SERIAL_EVENT_BROKEN)
        {
            FIBO_usbDevice_Deinit(th);
            th = FIBO_usbDevice_init(DRV_NAME_USRL_COM6, recv_cb);
            FIBO_usbDevice_State_report(th, (Report_UsbDev_Event)UsbDev_Event);
            fibo_textTrace("usbDev had broken %d", usbState);
            fibo_sem_wait(usbdev_daemon_sem);
        }
        else
        {
            fibo_textTrace("usbDev state %d", usbState);
        }

        usbstatus=fibo_get_Usbisinsert();
        fibo_textTrace("usbDev usbstatus=%d.",usbstatus);

        usbmode= fibo_get_usbmode();
        fibo_textTrace("usbDev usbmode=%d.",usbmode);
        fibo_taskSleep(2000);
    }
    FIBO_usbDevice_Deinit(th);

    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(usb_thread_entry, "usb_custhread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

