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

#include "fibo_opencpu.h"
#include "oc_uart.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


void uart_recv(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg)
{
    fibo_textTrace("uartapi recv uart_port=%d len=%d, data=%s", uart_port, len, (char *)data);
}

static void uart_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    hal_uart_config_t drvcfg = {0};
    drvcfg.baud = 115200;
    drvcfg.parity = HAL_UART_NO_PARITY;
    drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    drvcfg.tx_buf_size = UART_TX_BUF_SIZE;

    int port = 2;
    fibo_gpio_mode_set(50, 1);
    fibo_gpio_mode_set(51, 1);
    uint8_t send_test[] = "uart terst";
    fibo_hal_uart_init(port, &drvcfg, uart_recv, NULL);
    for (size_t i = 0; i < 3; i++)
    {
        fibo_hal_uart_put(port, send_test, sizeof(send_test) - 1);
        fibo_taskSleep(10 * 1000);
    }
    fibo_textTrace("application thread exit, param 0x%x", param);
    fibo_hal_uart_deinit(port);

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(uart_thread_entry, "uart_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
