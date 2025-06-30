
#include "log_service.h"
#include "fb_uart.h"

#include "fibo_opencpu.h"



#if USER_LOG_EN
int fb_printf(const char *fmt, ...)
{
    int count = 0;
    char log_TxBuf[1024]={0};

    memset(log_TxBuf,0,sizeof(log_TxBuf));
    va_list ap;
    va_start(ap,fmt);
    count=vsprintf((char*)log_TxBuf,fmt,ap);
    va_end(ap);
    if(count<256-3)
    {
        OSI_PRINTFI("zy %s",log_TxBuf);
    }
    return count;
}
#endif

