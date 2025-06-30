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

extern void test_printf(void);

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    OSI_LOGI(0, "test");
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
#if 0
    char adv_data[50]={0};
    //1.data format 1
    memcpy(adv_data,"04FF12345606FF0120003045",24);//test OK
    fibo_ble_set_dev_data(12,adv_data); // set broadcast data
#endif
    //2.data format 2
    char adv_data[50]={0x08,0xFF,0x01,0x02,0x1A,0x2B,0x00,0x20,0x60};//test OK
    //len	 |type	|data
    //0x08	 |0xFF	|0x01021A2B002060
    fibo_ble_set_dev_data(9, adv_data);
    fibo_taskSleep(2000);

    //open broadcast
    fibo_ble_enable_dev(1);

    //get version
    char version_test[10]={0};
    fibo_ble_get_ver(version_test);
    fibo_textTrace("bledemo version_test %s",version_test);
    fibo_taskSleep(2000);

    //set_read_name
    char name_t1[10]="weiweiwei";
    char name_t2[10]={0};
    char name_t3[10]={0};
    fibo_ble_set_read_name(1, (uint8_t *)name_t2,0);
    fibo_textTrace("bledemo old name %s",name_t2);
    fibo_taskSleep(2000);
    fibo_ble_set_read_name(0, (uint8_t *)name_t1,0);
    fibo_taskSleep(2000);
    fibo_ble_set_read_name(1, (uint8_t *)name_t3,0);
    fibo_textTrace("bledemo new name %s",name_t3);
    fibo_taskSleep(2000);

    fibo_ble_data_request(21, "AT+NAME=fibobletest\r\n", 2);
    fibo_ble_data_request(10, "AT+NAME?\r\n", 1);   
    fibo_ble_data_request(10, "AT+NAME?\r\n", 2);    
    fibo_ble_data_request(13, "AT+CONNADD?\r\n", 2);
    ret = fibo_ble_data_request(9, "AT+SCAN\r\n", 20);
    fibo_textTrace("AT+SCAN ret=%d",ret);
    ret = fibo_ble_data_request(13, "AT+SCAN=100\r\n", 101);
    fibo_textTrace("AT+SCAN=100 ret=%d",ret);

    //get addr
    uint8_t addr_type = 0;
    char addr_test[20] = {0};
    fibo_ble_read_addr(addr_type, addr_test);
    fibo_textTrace("bledemo addr_test %s",addr_test);
    fibo_taskSleep(2000);

    fibo_ble_uuid_config(0,"AA5C49D204A34071A0B535853EB08307");
    fibo_taskSleep(2000);
    //peer connecting

    int *state=malloc(2);
    while(1)
    {
        fibo_taskSleep(2000);
        fibo_ble_get_connect_state(state);
        fibo_textTrace("bledemo state %d",*state);
        if(*state)
        {
            //get_peer_mtu
            int peer_mtu[2]={0};
            fibo_ble_get_peer_mtu(peer_mtu);
            fibo_textTrace("bledemo peer_mtu %d",*peer_mtu);
            fibo_taskSleep(2000);

            //set_get mtu
            int mtu[2]={0};
            int mtu_t[2]={0};
            int peer_mtu_t[2]={0};
            peer_mtu_t[0] =100;
            //peer_mtu_t[0] = (int)(*peer_mtu+20);
            fibo_ble_set_read_mtu(1, mtu);
            fibo_taskSleep(2000);
            fibo_textTrace("bledemo old mtu %d",*mtu);
            fibo_ble_set_read_mtu(0,peer_mtu_t);
            fibo_taskSleep(2000);
            fibo_ble_set_read_mtu(1, mtu_t);
            fibo_textTrace("bledemo new mtu %d",*mtu_t);
            fibo_taskSleep(2000);

            //send data
            char sdata[50]={0};
            memcpy(sdata,"0123456789abc",13);
            for(int i=0; i<5; i++)
            {
                fibo_ble_send_data(13, sdata);
                fibo_taskSleep(2000);
            }

            char data[50]={0x01,0x04,0x04,0x04,0x7B,0x00,0x00,0x8A,0xAD};
            for(int i=0; i<5; i++)
            {
                fibo_ble_send_data(9, data);
                fibo_taskSleep(2000);
            }
            fibo_ble_disconnect(0); 
            fibo_taskSleep(50000);

            //receive data
            break;
        }
    }

    fibo_thread_delete();
}


void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();
    fibo_thread_create(ble_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
