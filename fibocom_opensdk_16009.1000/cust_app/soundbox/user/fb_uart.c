
#include "fb_uart.h"
#include "fibo_opencpu.h"

uint8_t g_log_level = 0;
//UartOptionStruct *g_logHanle;
const char *log_level_string[LEVEL_FATAL + 1] = {
    "[DEBUG]",
    "[INFO]",
    "[WARN]",
    "[ERROR]",
    "[FATAL]",
};

typedef enum
{
        UART1=0,
        UART2,
        UART3
}_UART_PORT;



int log_print_t(const char * fmt, ...)
{  
    // uint16_t count = 0;
    // uint8_t log_TxBuf[1024];
    // va_list ap;

    // va_start(ap,fmt);
    // count += sprintf((char *)&log_TxBuf[count],"[%d]",fb_get_SysRunTimeMs());
    // count+=vsprintf((char*)&log_TxBuf[count],fmt,ap);
    // va_end(ap);

    return 0;//fibo_hal_uart_put(UART1, UART_TxBuf,count);

}
/*************************UART1 ********************/
int log_print(LogLevelEnum level, const char * fmt, ... )
{  
    uint16_t count = 0;
    uint8_t UART_TxBuf[1024];
    va_list ap;
  
    if(level < g_log_level)
    { 
      return 0;
    }
    va_start(ap,fmt);
    count += sprintf((char *)&UART_TxBuf[count],"%s",log_level_string[level]);
    count+=vsprintf((char*)&UART_TxBuf[count],fmt,ap);
    va_end(ap);

    return fibo_hal_uart_put(UART1, UART_TxBuf,count);

}

int uart1_print(const char * fmt, ... )
{  
    uint16_t count = 0;
    uint8_t UART_TxBuf[1024];
    va_list ap;
 
    va_start(ap,fmt);
    count+=vsprintf((char*)&UART_TxBuf[count],fmt,ap);
    va_end(ap);

    return fibo_hal_uart_put(UART1, UART_TxBuf,count);

}


UINT16 uart_util_remove_char(UINT8 *pdata, UINT16 len,char c)
{
    UINT16 idxr = 0;
    UINT16 idxw = 0;
    for (idxr=0; idxr<len; idxr++)
    {
        if (c != pdata[idxr])
        {
            pdata[idxw++] = pdata[idxr];
        }
    }

    len = idxw;

    for (; idxw<idxr; idxw++)
    {
        pdata[idxw] = 0;
    }

    return len;
}


//UART1 recv callback
static void uart1_recv_callback(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg)
{

    OSI_PRINTFI("uartapi recv uart_port=%d len=%d, data=%s", uart_port, len, (char *)data);
}

int uart1_send_data(char *buff, int len)
{
    return fibo_hal_uart_put(UART1, (UINT8 *)buff,(UINT32)len);
}

int uart1_init(int baud)
{
    int port = UART1;//0:uart1   1:uart2
    hal_uart_config_t drvcfg = {0};

    drvcfg.baud = baud;
    drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
    drvcfg.cts_enable=false;           
    drvcfg.rts_enable=false;
    return fibo_hal_uart_init(port, &drvcfg,uart1_recv_callback, NULL);
}

/*************************UART2 ********************/
//UART2recv callback
static void uart2_recv_callback(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg)
{

    OSI_PRINTFI("uartapi recv uart_port=%d len=%d, data=%s", uart_port, len, (char *)data);
}

int uart2_send_data(char *buff, int len)
{
   return fibo_hal_uart_put(UART2,(UINT8 *) buff,(UINT32)len);
}

int uart2_init(int baud)
{
    int port = UART2;//0:uart1   1:uart2 
    hal_uart_config_t drvcfg = {0};

    drvcfg.baud = baud;
    drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
    drvcfg.cts_enable=false;           
    drvcfg.rts_enable=false;

    return fibo_hal_uart_init(port, &drvcfg, uart2_recv_callback, NULL);

}

int uart2_print(const char * fmt, ... )
{  
    int count = 0;
    uint8_t UART_TxBuf[1024];
    va_list ap;

    va_start(ap,fmt);
    count+=vsprintf((char*)&UART_TxBuf[count],fmt,ap);
    va_end(ap);

    return fibo_hal_uart_put(UART2, UART_TxBuf,count);
}


/*************************UART3 ********************/
//UART3 recv callback
static void uart3_recv_callback(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg)
{

    OSI_PRINTFI("uartapi recv uart_port=%d len=%d, data=%s", uart_port, len, (char *)data);
}

INT32 uart3_send_data(void)
{
    hal_uart_port_t port = UART3;
    INT32 ret = 0xff;

    UINT8 data_to_send[] = "#DS,2,*/r/n";
    UINT32 len = sizeof(data_to_send); // Exclude the null terminator
    ret = fibo_hal_uart_put(port, &data_to_send[0], len);
    fb_printf("uartapi_send_data ret: %d\r\n", ret);
    fibo_hal_uart_data_to_send(port);
    return ret;
}

INT32 uart3_init(void)
{
    hal_uart_port_t port = UART3;//0:uart1   1:uart2
    hal_uart_config_t drvcfg = {0};
    uint32_t baud = 9600;
    INT32 ret = 0xff;

    drvcfg.baud = baud;
    drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
    drvcfg.cts_enable=false;
    drvcfg.rts_enable=false;
    drvcfg.recv_timeout=200;
    fibo_textTrace("uartapi_init_check_ok");
    ret = fibo_hal_uart_init(port, &drvcfg, uart3_recv_callback, NULL);
    fb_printf("uartapi_init ret: %d\r\n", ret);
    return ret;
}

int uart3_print(const char * fmt, ... )
{  
    int count = 0;
    uint8_t UART_TxBuf[1024];
    va_list ap;

    va_start(ap,fmt);
    count+=vsprintf((char*)&UART_TxBuf[count],fmt,ap);
    va_end(ap);

    return fibo_hal_uart_put(UART3, UART_TxBuf,count);
}



