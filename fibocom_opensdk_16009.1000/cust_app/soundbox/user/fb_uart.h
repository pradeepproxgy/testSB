#ifndef _FB_UART_H_
#define	_FB_UART_H_

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include <stdint.h>
#include "string.h"
#include "fb_config.h"
#include "log_service.h"



extern uint8_t g_log_level;


typedef enum
{
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL
}LogLevelEnum;


int uart1_init(int baud);
int uart1_send_data(char *buff, int len);
int log_print(LogLevelEnum level, const char * fmt, ... );
int log_print_t(const char * fmt, ... );
int uart1_print(const char * fmt, ... );


int uart2_init(int baud);
int uart2_send_data(char *buff, int len);
int uart2_print(const char * fmt, ... );


INT32 uart3_init(void);
INT32 uart3_send_data(void);
int uart3_print(const char * fmt, ... );










#endif
