
#include "fb_tcp_lwip.h"
#include "log_service.h"

int ext_server_connect(const char *host, unsigned short port,int socket_type)
{
    int             fd = 0;
    int             ret = 0;
    GAPP_TCPIP_ADDR_T addr;
    ip_addr_t addr_dns;
    UINT8 temp_ip[50];
    UINT8 cid_status=0;
    int NODELAY=1;

    memset(temp_ip, 0, sizeof(temp_ip));
    ret = fibo_PDPStatus(1,temp_ip,&cid_status,0);
    if(cid_status==0)
    {
        //no active 
        return -1;
    }


    fb_printf("zy nhost[%s] port[0x%x]\r\n",host,port);

    memset(&addr,0,sizeof(addr));
    memset(&addr_dns,0,sizeof(addr_dns));
    ret=fibo_getHostByName((char *)host,&addr_dns,1, 0);
    if(ret<0)
    {
        // Log_e("get hostname fail\n");
        fb_printf("get hostname fail\n");
        return -1;
    }

    fd=fibo_sock_create(socket_type);
    addr.sin_port=htons(port);
    addr.sin_addr.u_addr.ip4.addr=addr_dns.u_addr.ip4.addr;
    addr.sin_addr.type=addr_dns.type;

    fibo_sock_setOpt(fd,SOL_SOCKET,SO_KEEPALIVE,&NODELAY,sizeof(NODELAY));

    fb_printf("sin_port:0x%x, type:%d, ip4:0x%x \r\n",\
            addr.sin_port,addr.sin_addr.type,addr.sin_addr.u_addr.ip4.addr);

    ret =fibo_sock_connect(fd,&addr);
    fb_printf("fibo_sock_connect :%d\r\n",ret);  
  
    if(ret<0){
        fb_printf("socket connet fail socketid[%d]\r\n",fd);
        return -1;  
    }

    fb_printf("socket connet success socketid[%d]\r\n",fd);  
    return fd;
}


int ext_tcp_disconnect(int fd)
{
    if(fibo_sock_close(fd)==0){
        fb_printf("user_socket_close id:%d\n",fd);
        return 0;  
    }  
    return -1;
}


int ext_tcp_write(int fd, const unsigned char *buf, unsigned int len, unsigned int timeout_ms, unsigned int *written_len)
{
    int ret=0;

    ret=fibo_sock_send((INT32)fd,(UINT8*)buf,(UINT16)len);
    *written_len = (size_t)ret;
    if(ret<0)
    {
        ret=-1;;
    }


    return ret;
           
}


// return  :    >0 success   recv len   
//              = 0  timeout 
//              = -1 error
//        
int ext_tcp_read(int fd, unsigned char *buf, unsigned int len, unsigned int timeout_ms, unsigned int *read_len)
{
    int            ret, err_code;
    unsigned int   len_recv;

    struct timeval timeout;
    int error =0;

    len_recv = 0;
    err_code = 0;

    fd_set readfds;
    fd_set errorfds;
    fd_set writeset;

    do {
        FD_ZERO(&readfds);
        FD_ZERO(&errorfds);
        FD_ZERO(&writeset);
        FD_SET(fd, &writeset);
        FD_SET(fd, &readfds);
        FD_SET(fd, &errorfds);
      
        timeout.tv_sec  = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        fibo_taskSleep(50);
        ret = fibo_sock_lwip_select(fd+1, &readfds, NULL, &errorfds, &timeout);
        if (ret > 0) {
        if(FD_ISSET(fd, &errorfds))	{
                    //abnormally
                
            ret = fibo_sock_getOpt(fd,(INT32)SOL_SOCKET,(INT32)SO_ERROR,&error,(INT32 *) &len);
            fb_printf("[%s-%d] sys_sock_recv	ret = %d\n", __FUNCTION__, __LINE__, ret);
            err_code = -3;
            break;
    
        }
        else if(FD_ISSET(fd, &readfds)){
                    /*noral handle*/
            ret = fibo_sock_recv(fd,buf + len_recv,len);
            //fb_printf("fibo_sock_recv ret:%d\n",ret);
            if(ret>0){
                len_recv += ret;
            }
            else if (ret<=0) {
                fb_printf("ext_tcp_read error ret:%d\n",ret);
                len_recv=0;
                err_code = -1;
                break;
            }
        }
        } else if (0 == ret) {
            err_code =0;  //timeout
            break;
        }
        else {
            fb_printf("select-recv error: %s", strerror(errno));
            err_code =-1;
            break;
        }
    } 
    while (0);

    *read_len = (size_t)len_recv;

    return len_recv>0?len_recv:err_code;
}



static int fb_gettimeofday(struct timeval *tv, void *tz)
{
    uint64_t us;
    us=osiUpTimeUS();
    
    tv->tv_usec=us%1000000;
    tv->tv_sec=us/1000000;
    return 0;
}

UINT32 fb_get_SysRunTimeMs(void)
{

    struct timeval tv;
    UINT32 time_ms;

    fb_gettimeofday(&tv,NULL);
    time_ms=(tv.tv_sec*1000)+(tv.tv_usec/1000);
    return time_ms;
}

long HAL_GetUTCTime(void)
{
#define xMINUTE     (60             ) //1分的秒数
#define xHOUR       (60*xMINUTE) //1小时的秒数
#define xDAY        (24*xHOUR   ) //1天的秒数
#define xYEAR       (365*xDAY   ) //1年的秒数
    hal_rtc_time_t  tc_time;
    fibo_getRTC(&tc_time);

    // HAL_Printf("zy_rtc get year:%d   month:%d  day:%d ",tc_time.year,tc_time.month,tc_time.day);
    // HAL_Printf("zy_rtc get hour:%d   min:%d  sec:%d ",tc_time.hour,tc_time.min,tc_time.sec);
    static unsigned int  month[12]={
        /*01月*/xDAY*(0),
        /*02月*/xDAY*(31),
        /*03月*/xDAY*(31+28),
        /*04月*/xDAY*(31+28+31),
        /*05月*/xDAY*(31+28+31+30),
        /*06月*/xDAY*(31+28+31+30+31),
        /*07月*/xDAY*(31+28+31+30+31+30),
        /*08月*/xDAY*(31+28+31+30+31+30+31),
        /*09月*/xDAY*(31+28+31+30+31+30+31+31),
        /*10月*/xDAY*(31+28+31+30+31+30+31+31+30),
        /*11月*/xDAY*(31+28+31+30+31+30+31+31+30+31),
        /*12月*/xDAY*(31+28+31+30+31+30+31+31+30+31+30)
    };
    long  seconds = 0;
    unsigned int  year = 0;
    year = tc_time.year+2000-1970;       //不考虑2100年千年虫问题
    seconds = xYEAR*year + xDAY*((year+1)/4);  //前几年过去的秒数
    seconds += month[tc_time.month-1];      //加上今年本月过去的秒数
    if( (tc_time.month > 2) && (((year+2)%4)==0) )//2008年为闰年
        seconds += xDAY;            //闰年加1天秒数
    seconds += xDAY*(tc_time.day-1);         //加上本天过去的秒数
    seconds += xHOUR*tc_time.hour;           //加上本小时过去的秒数
    seconds += xMINUTE*tc_time.min;       //加上本分钟过去的秒数
    seconds += tc_time.sec;               //加上当前秒数<br>　seconds -= 8 * xHOUR;
    seconds -= 8 * xHOUR;


   // HAL_Printf("zy_rtc get seconds:%d ",seconds);
    return seconds;
}




int ext_set_rtc_time(long time)
{
   
    hal_rtc_time_t  tc_time;

    struct tm tm;
    time_t lt;
//北京时间  8×60×60 =28800
// Log_i("zy_rtc  ext_set_system_time:%d",time);
    lt=time+28800;
    gmtime_r(&lt,&tm);
    tc_time.year=(tm.tm_year+1900)%100;
   
    tc_time.month=tm.tm_mon+1;
    tc_time.day=tm.tm_mday;
    tc_time.hour=tm.tm_hour;
    tc_time.min=tm.tm_min;
    tc_time.sec=tm.tm_sec;

    fb_printf("zy_rtc set year:%d   month:%d  day:%d ",tc_time.year,tc_time.month,tc_time.day);
    fb_printf("zy_rtc set hour:%d   min:%d  sec:%d ",tc_time.hour,tc_time.min,tc_time.sec);
   
    return fibo_setRTC(&tc_time);
}


bool HAL_Timer_expired(Timer *timer)
{
    uint32_t now_ts;

    now_ts =fb_get_SysRunTimeMs();

    return (now_ts > timer->end_time) ? true : false;
}

void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms)
{
    timer->end_time = fb_get_SysRunTimeMs();
    timer->end_time += timeout_ms;
}

void HAL_Timer_countdown(Timer *timer, unsigned int timeout)
{
    timer->end_time = fb_get_SysRunTimeMs();
    timer->end_time += timeout * 1000;
}

int HAL_Timer_remain(Timer *timer)
{
    return (int)(timer->end_time - fb_get_SysRunTimeMs());
}

void HAL_Timer_init(Timer *timer)
{
    timer->end_time = 0;
}

