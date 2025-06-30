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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('T', 'T', 'S', 'Y')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static UINT32 g_fibo_ble_queue = 0; 

extern void test_printf(void);

typedef struct{
    int msg_id;
    int uuid_index;
    int len;
    char *data; 
}fibo_ble_q_t;

fibo_ble_q_t g_fibo_ble_q;

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_textTrace("sig=%d", sig);
    memset(&g_fibo_ble_q,0,sizeof(fibo_ble_q_t));
    g_fibo_ble_q.msg_id = sig;
    switch (sig)
    {
        case GAPP_SIG_BLE_READ_REQ_IND:   //357
        {
            int index = (int )va_arg(arg, int);
            fibo_textTrace("read req:uuid_index=%d",index);
            g_fibo_ble_q.uuid_index = index;
        }
        break;
        case GAPP_SIG_BLE_RECV_DATA_IND:   //358
        {
            int uuid_index = (int )va_arg(arg, int);
            int len = (int )va_arg(arg, int);
            char *data = (char *)va_arg(arg, char*);
            fibo_textTrace("recv data:uuid_index=%d,len=%d,data=%s",uuid_index, len, data);
            g_fibo_ble_q.uuid_index = uuid_index;
            g_fibo_ble_q.len = len;
            if(g_fibo_ble_q.len > 0)
            {
                g_fibo_ble_q.data = malloc(g_fibo_ble_q.len + 1);
                if(g_fibo_ble_q.data == NULL)
                {
                    fibo_textTrace("malloc failed");
                    break;
                }
                memcpy(g_fibo_ble_q.data, data, len);
                g_fibo_ble_q.data[len] = '\0';
            }
        }
        break;	
        default:
        {
            break;
        }
    }
    fibo_queue_put(g_fibo_ble_queue, (void *)&g_fibo_ble_q, 0);	
    return;
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback};

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();
    
    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

void fibo_spp_data_recv(int data_len,char *data)
{
    fibo_textTrace("bledemo application data:%s, data_len: %d",data, data_len);
    return;
}

static void ble_thread_entry(void *param)
{
    INT32 ret = 0;
    fibo_taskSleep(20000);
    fibo_textTrace("application thread enter, param 0x%x", param);
    fibo_ble_recv_data((ble_data_recv_callback)fibo_spp_data_recv);
    
    fibo_taskSleep(2000);
    fibo_ble_onoff(1);
    fibo_taskSleep(2000);
    
    //close broadcast
    fibo_ble_enable_dev(0);
    fibo_taskSleep(2000);
    
    //set_dev_param
    fibo_ble_set_dev_param(6,80,1600,0,0,0,7,0,NULL);
    fibo_taskSleep(2000);
    
    //set_dev_data
    char adv_data[50]={0};
    //1.data format 1
    memcpy(adv_data,"04FF12345606FF0120003045",24);//test OK
    fibo_ble_set_dev_data(12,adv_data); // set broadcast data
    fibo_taskSleep(2000);
    
    //open broadcast
    fibo_ble_enable_dev(1);
    ret = fibo_ble_data_request(12, "AT+GTUUID?\r\n", 10);
    fibo_textTrace("AT+GTUUID? ret=%d",ret);
    //get addr
    uint8_t addr_type = 0;
    char addr_test[20] = {0};
    fibo_ble_read_addr(addr_type, addr_test);
    fibo_textTrace("bledemo addr_test %s",addr_test);
    fibo_taskSleep(2000);
    fibo_ble_uuid_config(0, "AA5C49D204A34071A0B535853EB08307");
    
    ret = fibo_ble_set_gtuuid(0, "1023", 0xFF);
    fibo_textTrace("gtuuid 0:1023 ret=%d",ret); 		
    
    //peer connecting
    int *state=malloc(2);
    while(1)
    {
        fibo_taskSleep(2000);
        fibo_ble_get_connect_state(state);
        fibo_textTrace("bledemo state %d",*state);
        if(*state)
        {        
            ret = fibo_ble_gtsend_data(1, 10, "abcdefghij");
            fibo_textTrace("gtsend 1 ret=%d",ret);
            fibo_taskSleep(2000);
        }
        break;
    }

}

static void fibo_ble_task(void *param)
{
    while(1)
    {
        fibo_ble_q_t value;
        memset(&value, 0, sizeof(fibo_ble_q_t));
        fibo_queue_get(g_fibo_ble_queue, (void *)&value, 0);
        int event = value.msg_id;
        int uuid_index = value.uuid_index;
        fibo_textTrace("event=%d, uuid_index=%d", event, uuid_index);
        switch(event)
        {
            case GAPP_SIG_BLE_CONNECT_IND:
            {
                fibo_textTrace("ble connect ok");
            }
            break;
            case GAPP_SIG_BLE_READ_REQ_IND:
            {
                if(uuid_index == 2 || uuid_index == 4)
                {
                    fibo_ble_response_to_read(19, "123456789012345678\0");
                }
                else
                {
                    fibo_ble_response_to_read(6, "12345\0");
                }
            }
            break;
            case GAPP_SIG_BLE_RECV_DATA_IND: //send indicator
            {
                fibo_textTrace("uuid_index=%d, len=%d,data=%s",value.uuid_index, value.len, value.data);
            }
            break;
            case GAPP_SIG_BLE_DISCONNECT_IND: //send notification
            {
                fibo_textTrace("ble disconnect ok");
            }
            break;
            default:
                fibo_textTrace("[%s:%d]", __FUNCTION__, __LINE__);
            break;
        }
    }
    return;
    }

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();
    if(g_fibo_ble_queue == 0)
    {
        g_fibo_ble_queue = fibo_queue_create(20, sizeof(fibo_ble_q_t));
    }
    fibo_thread_create(ble_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    fibo_thread_create(fibo_ble_task, "fibo_ble_task", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
