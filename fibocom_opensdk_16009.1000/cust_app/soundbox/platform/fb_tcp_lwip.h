#ifndef _FB_TCP_LWIP_
#define _FB_TCP_LWIP_
#include "fibo_opencpu.h"
#include "fibo_opencpu_comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define gmtime_r(a,b)       \
        do{                 \
            *(b)=*gmtime(a);\
        } while(0)


struct Timer {
    UINT32  end_time;
};

typedef struct Timer Timer;



UINT32 fb_get_SysRunTimeMs(void);
long HAL_GetUTCTime(void);
int ext_set_rtc_time(long time);

int ext_server_connect(const char *host, unsigned short  port,int socket_type);
int ext_tcp_disconnect(int fd);
int ext_tcp_write(int fd, const unsigned char *buf, unsigned int len, unsigned int timeout_ms, unsigned int *written_len);
int ext_tcp_read(int fd, unsigned char *buf, unsigned int len, unsigned int timeout_ms, unsigned int *read_len);


bool HAL_Timer_expired(Timer *timer);
void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms);
void HAL_Timer_countdown(Timer *timer, unsigned int timeout);
void HAL_Timer_init(Timer *timer);
#endif // !_FB_TCP_LWIP_