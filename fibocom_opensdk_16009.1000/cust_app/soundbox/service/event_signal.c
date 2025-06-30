#include "event_signal.h"
#include "fibo_opencpu.h"
#include "fb_uart.h"

#include "network_service.h"


unsigned int  queue_event_signal_id=0;
int event_signal_msg=0;
int fb_event_signal_push(int msg)
{
    if(queue_event_signal_id!=0)
    {
        event_signal_msg=msg;
        return fibo_queue_put(queue_event_signal_id,&event_signal_msg,0);
    } 
    return -1;
}

int fb_event_signal_get(int *msg)
{
    if(queue_event_signal_id!=0)
    {
        return fibo_queue_get(queue_event_signal_id,&msg,0); 
    } 
    return -1;
}

UINT32  event_mutex_id = 0;

static void fb_event_signal_loop(void)
{
    int  event_signal_get_msg=0;

    while(1){
       
        fb_event_signal_get(&event_signal_get_msg);
        fibo_sem_wait(event_mutex_id);

        switch(event_signal_get_msg)
        {

            case CMD_CLOSE_LOG1:
                {
                    const UINT8 *cmd = (const UINT8 *)"at^tracectrl=1,0,0\r\n";
                    fibo_at_send((const UINT8 *)cmd,(UINT16)strlen((const char *)cmd));
                }

            break;
            case CMD_CGREG_CHECK:
                {
                    const UINT8 *cmd = (const UINT8 *)"AT+CGREG=2\r\n";
                    fibo_at_send((const UINT8 *)cmd,(UINT16)strlen((const char *)cmd));
                }

            break;
            case CMD_CEREG_CHECK:
                {
                   
                    const UINT8 *cmd = (const UINT8 *)"AT+CEREG=2\r\n";
                    fibo_at_send((const UINT8 *)cmd,(UINT16)strlen((const char *)cmd));
                }

            break;


            case CMD_CLOSE_LOG2:
                {
                  
                    const UINT8 *cmd = (const UINT8 *)"at^tracectrl=0,0,0\r\n";
                    fibo_at_send((const UINT8 *)cmd,(UINT16)strlen((const char *)cmd));
                }
        
            break;
            //1.2基线无此命令   1.3基线有     可缩短收发数据后进入休眠的时间
            // case CMD_LOCREL_TIME:
            //     {
            //         //user_log(0,"at^tracectrl=0,0,0\r\n");
            //         const UINT8 *open_spk = (const UINT8 *)"at+locrel=5\r\n";
            //         fibo_at_send((const UINT8 *)open_spk,(UINT16)strlen(open_spk));
            //     }
        
            // break;

            default:
                break;
        }
        
       fibo_sem_signal(event_mutex_id);   
    }
}



static void fb_event_signal_init(void)
{
    queue_event_signal_id = fibo_queue_create(50,1);
    event_mutex_id = fibo_sem_new(1);//mutex

}


static void fb_event_signal_task(void *param)
{ 
    fb_event_signal_init();
    while(1)
    {
        fb_event_signal_loop();
        fibo_taskSleep(1000);
    }

    fibo_thread_delete();
}



int fb_event_signal_enter(void)
{
    return fibo_thread_create(fb_event_signal_task,"_fb_event_signal_task", 1024*8*4, NULL, OSI_PRIORITY_NORMAL);
}

 