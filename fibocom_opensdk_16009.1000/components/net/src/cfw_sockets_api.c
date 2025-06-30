#include "osi_api.h"
#include "cfw_event.h"
#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#include "lwip/apps/sntp.h"
#include "api_msg.h"
#include "sockets.h"
#include "netutils.h"
#include "netmain.h"
#include "osi_log.h"
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
#include "socket_types.h"
#include "IN_Message.h"
#endif
#include "fibocom.h"
extern void *g_hCosMmiAdpTask;
extern void *g_hCosATTask;
extern osiThread_t *netThreadID;

#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
#define INFORM_STATE_DISABLE 0
#define INFORM_STATE_ENABLE 1
#define INFORM_STATE_SENDING 2
#define INFORM_STATE_SENT 3
#define INFORM_STATE_HANDLED 4
//extern uint32 SCI_SendSignal_ex(xSignalHeader sig_ptr, unsigned int revicer);
static void SCI_SendSignal_type(xSignalHeader sig_ptr, unsigned int revicer)
{
    if (revicer > 0 && revicer < 99)
    {
        SCI_SendSignal_ex(sig_ptr, revicer);
    }
    else
    {
        SCI_SendSignal(sig_ptr, (struct osiThread *)revicer);
    }
    return;
}
#endif
extern int32_t lwip_availableBuffer(int nSocket);
extern int32_t lwip_getAckedSize(int nSocket);
extern int32_t lwip_getSentSize(int nSocket);
extern int32_t lwip_getMss(int nSocket);
extern int32_t lwip_getRecvAvailSize(int nSocket);
extern int32_t lwip_getRecvBufSize(int nSocket);
extern int32_t lwip_getTcpState(int nSocket);
extern err_t netconn_gethostbyname_ext(const char *name, ip_addr_t *addr, u16_t simcid, u32_t taskHandle, u32_t func, u32_t param, u8_t addrtype);
extern err_t netconn_getallhostbyname_ext(const char *name, ip_addr_t *addr, u16_t simcid, u32_t taskHandle, u32_t func, u32_t param, u8_t addrtype, int *addr_count);
#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
extern err_t netconn_gethostbyname_specificserver_ext(const char *name, ip_addr_t *addr, u16_t simcid, u32_t taskHandle, char *dns_server, u32_t func, u32_t param, u8_t addrtype);
#endif
static void sys_post_event_to_APP(uint32_t nEventId, uint32_t nParam1, uint32_t nParam2, uint32_t nParam3);

#ifndef ONLY_AT_SUPPORT
extern void *g_TransportHandle;
#endif

#define MAX_SOCKET MEMP_NUM_NETCONN

//MIN_DATA_IND_INTERVAL_TIME 10ms
#define MIN_DATA_IND_INTERVAL_TIME 10
typedef struct _cfw_socket_map_t
{
    int socket_used;
    socket_status_t status;
    uint32_t userParam;
    uint32_t taskHandle;
    osiCallback_t func;
    osiNotify_t *dataIND_notify;
    uint32_t isSCISocket;
    uint32_t eventFlag;
    uint32_t is_inform_read;
} CFW_SOCKET_MAP_T;

static CFW_SOCKET_MAP_T cfw_socket_map[MAX_SOCKET + LWIP_SOCKET_OFFSET];

static void _dataIndNotify(void *ctx)
{
    int socket = (int)ctx;
    if (cfw_socket_map[socket].socket_used == 0)
        return;
    uint32_t taskHandle = cfw_socket_map[socket].taskHandle;
    osiCallback_t func = cfw_socket_map[socket].func;
    int32_t dataSize = lwip_getRecvAvailSize(socket);
    if (dataSize <= 0)
    {
        if (0 == dataSize)
        {
            int optval = -1;
            int optLen = sizeof(int);
            if (0 == CFW_TcpipSocketGetsockopt(socket, SOL_SOCKET, SO_TYPE, (void *)(&optval), &optLen))
            {
                if (SOCK_DGRAM == optval)
                {
                    uint8_t temp[2] = {0};
                    union sockaddr_aligned from_addr = {
                        0,
                    };
                    int fromLen = sizeof(from_addr);                                                            //union sockaddr_aligned addr;
                    CFW_TcpipSocketRecvfrom(socket, temp, 2, 0, (CFW_TCPIP_SOCKET_ADDR *)&from_addr, &fromLen); //consume pbuf
                }
            }
        }
        //sys_arch_printf("_dataIndNotify dataSize:%ld retrn", dataSize);
        return;
    }
    if (socket >= MAX_SOCKET + LWIP_SOCKET_OFFSET)
    {
        OSI_LOGI(0x100094f9, "_dataIndNotify socket:%d retrn", socket);
        return;
    }
    if (func == NULL)
    {
        if (0 == cfw_socket_map[socket].isSCISocket)
        {

            osiEvent_t ev;
            ev.id = EV_CFW_TCPIP_REV_DATA_IND;
            ev.param1 = socket;
            ev.param2 = dataSize;
            ev.param3 = cfw_socket_map[socket].userParam;
            OSI_LOGI(0x10007af6, "_dataIndNotify Notify:%x,event:%d,nParam1:%d,nParam2:%d,nParam3:%d", (unsigned int)taskHandle, (int)ev.id, (int)ev.param1, (int)ev.param2, (int)ev.param3);
            osiEventSend((osiThread_t *)taskHandle, &ev);
        }
        else
        {
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT

            if (((cfw_socket_map[socket].eventFlag & AS_READ) != 0) &&
                ((cfw_socket_map[socket].is_inform_read == INFORM_STATE_ENABLE) || (cfw_socket_map[socket].is_inform_read == INFORM_STATE_HANDLED)))
            {
                SOCKET_READ_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_READ_EVENT_IND_SIG_T));
                if (sig == NULL)
                {
                    OSI_LOGE(0x1000a0fe, "_dataIndNotify Notify: malloc error");
                    return;
                }
                cfw_socket_map[socket].is_inform_read = INFORM_STATE_SENDING;
                sig->SignalCode = SOCKET_READ_EVENT_IND;
                sig->SignalSize = sizeof(SOCKET_READ_EVENT_IND_SIG_T);
                sig->Sender = (BLOCK_ID)cfw_socket_map[socket].taskHandle;
                sig->socket_id = socket;

                SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[socket].taskHandle);
                //sys_post_event_to_APP(SOCKET_READ_EVENT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
                cfw_socket_map[socket].is_inform_read = INFORM_STATE_SENT;
            }

#endif
        }
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        if (ev == NULL)
        {
            OSI_LOGE(0x1000a0fe, "_dataIndNotify Notify: malloc error");
            return;
        }
        ev->id = EV_CFW_TCPIP_REV_DATA_IND;
        ev->param1 = socket;
        ev->param2 = dataSize;
        ev->param3 = cfw_socket_map[socket].userParam;
        OSI_LOGI(0x10007af6, "_dataIndNotify Notify:%x,event:%d,nParam1:%d,nParam2:%d,nParam3:%d", (unsigned int)taskHandle, (int)ev->id, (int)ev->param1, (int)ev->param2, (int)ev->param3);

        func((void *)ev);
    }
}

static uint32_t getRequestTaskHandle()
{
    void *task = (void *)osiThreadCurrent();
#if 0
#ifndef ONLY_AT_SUPPORT
    if (task == g_hCosMmiAdpTask || task == g_TransportHandle)
        return g_hCosMmiAdpTask;
#else
    if (task == g_hCosMmiAdpTask)
        return g_hCosMmiAdpTask;
#endif
    if (task == g_hCosATTask)
        return g_hCosATTask;
#ifdef ONLY_AT_SUPPORT
    if (g_hCosATTask != 0)
        return g_hCosATTask;
#endif
    return g_hCosMmiAdpTask;
#endif
    return (uint32_t)task;
}

static void sys_post_event_to_APP(uint32_t nEventId, uint32_t nParam1, uint32_t nParam2, uint32_t nParam3)
{
    int socket = nParam1;
    if (socket > MAX_SOCKET + LWIP_SOCKET_OFFSET - 1)
    {
        OSI_LOGI(0x10009827, "sys_post_event_to_APP socket error %d", socket);
        return;
    }
#if LWIP_RFACK
    if (EV_CFW_TCPIP_DATA_RF_ACKED_IND == nEventId)
    {
        socket = socket & 0xff;
    }
#endif
    if (socket <= 0 || cfw_socket_map[socket].socket_used == 0 || cfw_socket_map[socket].socket_used != socket)
    {
        return;
    }
    uint32_t taskHandle = cfw_socket_map[socket].taskHandle;
    osiCallback_t func = cfw_socket_map[socket].func;

    OSI_LOGI(0x100075ac, "sys_post_event_to_APP:%x,event:%d,nParam1:%d,nParam2:%d,nParam3:%d", (unsigned int)taskHandle, (int)nEventId, (int)nParam1, (int)nParam2, (int)nParam3);

    if (func == NULL)
    {
        osiEvent_t ev;
        ev.id = nEventId;
        ev.param1 = nParam1;
        ev.param2 = nParam2;
        ev.param3 = nParam3;
        osiEventSend((osiThread_t *)taskHandle, &ev);
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = nEventId;
        ev->param1 = nParam1;
        ev->param2 = nParam2;
        ev->param3 = nParam3;
        bool ret = osiThreadCallback((osiThread_t *)taskHandle, func, (void *)ev);
        if (false == ret)
        {
            OSI_LOGI(0x10007af7, "sys_post_event_to_APP:osiThreadCallback failed");
            free(ev);
        }
    }

    return;
}

bool sys_post_event_to_APS(void *msg, uint32_t msg_type)
{
    if (netThreadID != NULL)
    {
        osiEvent_t ev = {0};
        ev.id = EV_INTER_APS_TCPIP_REQ;
        ev.param1 = (uint32_t)msg;
        return osiEventSend(netThreadID, &ev);
    }
    return false;
}

void sys_post_DNSevent_to_APP(uint32_t nEventId, uint32_t nParam1, uint32_t nParam2,
                              uint8_t nType, uint8_t nFlag, uint16_t reserved,
                              uint32_t hTask, osiCallback_t func, void *pParam)
{
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x100075ad, "sys_post_DNSevent_to_APP nEventId=%d,task=%x,func=0X%x", (int)nEventId, (unsigned int)hTask, (unsigned int)func);
    if (func == NULL)
    {
        CFW_EVENT cfw_event = {
            .nEventId = nEventId,
            .nParam1 = nParam1,
            .nParam2 = nParam2,
            .nFlag = nFlag,
            .nType = nType,
            .nUTI = reserved,
        };
        osiEventSend((osiThread_t *)hTask, (osiEvent_t *)&cfw_event);
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = nEventId;
        ev->param1 = nParam1;
        ev->param2 = nParam2;
        ev->param3 = (uint32_t)pParam;
        osiThreadCallback((osiThread_t *)hTask, func, (void *)ev);
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return;
}

void sys_post_ICMPevent_to_APP(uint32_t nsocketId, uint32_t nParam1, uint32_t nParam2)
{
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    uint32_t nDLC = cfw_socket_map[nsocketId].userParam;
    uint32_t taskHandle = cfw_socket_map[nsocketId].taskHandle;
    osiCallback_t func = cfw_socket_map[nsocketId].func;
    OSI_LOGI(0x100075ae, "sys_post_ICMPevent_to_APP:%x,func:%x,nParam1:%d,nParam2:%d,nsocketId:%d", (unsigned int)taskHandle, (unsigned int)func, (int)nParam1, (int)nParam2, (int)nsocketId);
    if (func == NULL)
    {
        osiEvent_t ev;
        ev.id = EV_CFW_ICMP_DATA_IND;
        ev.param1 = nParam1;
        ev.param2 = nParam2;
        ev.param3 = nDLC;
        osiEventSend((osiThread_t *)taskHandle, &ev);
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = EV_CFW_ICMP_DATA_IND;
        ev->param1 = nParam1;
        ev->param2 = nParam2;
        ev->param3 = nDLC;
        osiThreadCallback((osiThread_t *)taskHandle, func, (void *)ev);
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return;
}

#if LWIP_RFACK
err_t lwip_rfack_event(void *arg, u32_t seqno, u16_t acked)
{
    struct netconn *conn = (struct netconn *)arg;

    OSI_LOGI(0x10009828, "lwip_rfack_event conn=0x%p,seqno: %d,acked: %d", conn, seqno, acked);
    if (conn == NULL)
        return -1;
    if (conn->rf_ack_callback != NULL)
    {
        conn->rf_ack_callback(conn->rf_ack_param, seqno, acked);
    }
    if (conn->callback_arg.socket <= 0 || cfw_socket_map[conn->callback_arg.socket].socket_used == 0 || cfw_socket_map[conn->callback_arg.socket].socket_used != (conn->callback_arg.socket))
    {
        return 0;
    }
    u32_t ack_socket = ((conn->callback_arg.socket) & 0xff) | (acked << 16);

    sys_post_event_to_APP(EV_CFW_TCPIP_DATA_RF_ACKED_IND, ack_socket, seqno, cfw_socket_map[conn->callback_arg.socket].userParam);
    return 0;
}
#endif


#if defined(CONFIG_FIBOCOM_BASE)
typedef void (*event_callback_t)(int sock, int event, u16_t size, void *arg);

extern void ffw_at_thread_callback(void (*cb)(void *arg), void *arg, bool sync);

typedef struct cb_para_s
{
    event_callback_t fun;

    int sock;
    int event;
    uint16_t size;
    void* arg;
}cb_para_t;

void socket_evt_cb_ex(void *arg)
{
    cb_para_t* s = (cb_para_t*)(arg);
    if(s->fun != NULL)
    {
        s->fun(s->sock, s->event, s->size, s->arg);
    }

    if(arg)
    {
        free(arg);
    }
}

/* socket_evt_cb will call cb in at thread, it registered at ffw_socket_evt_callback */
osiThread_t *atEngineGetThreadId(void);
void socket_evt_cb(event_callback_t fun, int sock, int event, uint16_t size, void* arg)
{
    cb_para_t *t = NULL;
    t = malloc(sizeof(cb_para_t));
    OSI_PRINTFI("[fibocom] socket_evt_cb, sock=%d, event=%d, size=%d", sock, event, size);
    if(t != NULL)
    {
        t->fun = fun;
        t->sock = sock;
        t->event = event;
        t->size = size;
        t->arg = arg;
        if (atEngineGetThreadId() != osiThreadCurrent())
        {
            ffw_at_thread_callback(socket_evt_cb_ex, (void *)t, false);
        }
        else
        {
            socket_evt_cb_ex(t);
        }

    }
}

#endif

#if LWIP_TCP
err_t lwip_tcp_event(void *arg, struct tcp_pcb *pcb,
                     enum lwip_event lwip_event,
                     struct pbuf *p,
                     u16_t size,
                     err_t err)
#else
err_t lwip_tcp_event(void *arg, void *pcb,
                     enum lwip_event lwip_event,
                     struct pbuf *p,
                     u16_t size,
                     err_t err)
#endif
{
    struct netconn *conn = (struct netconn *)arg;

    OSI_LOGI(0x100075af, "lwip_tcp_event conn=0x%p,lwip_event:%d,pcb:0x%p", conn, (int)lwip_event, pcb);

#if defined(CONFIG_FIBOCOM_BASE)//fibo20220307
    OSI_PRINTFI("lwip_tcp_event conn=0x%p,lwip_event:%d,pcb:0x%p", conn, (int)lwip_event, pcb);

    if (pcb != NULL)
    {
        switch (lwip_event)
        {
        case LWIP_EVENT_SENT:
            if (pcb->event_callback != NULL)
            {
                //FFW_SOCKET_EVT_SEND_ACK
                #if 0
                pcb->event_callback(0, 4, size, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 4/*FFW_SOCKET_EVT_SEND_ACK*/, size, pcb->arg);
                #endif
            }
            break;

        case LWIP_EVENT_RECV:
            // if (p == NULL)
            // {
            //     if (pcb->event_callback != NULL)
            //     {
            //         #if 0
            //         pcb->event_callback(0, 3, 0, pcb->arg);
            //         #else
            //         socket_evt_cb(pcb->event_callback, 0, 3/*FFW_SOCKET_EVT_FIN_RST*/, 0, pcb->arg);
            //         #endif
            //     }
            // }
            break;

        case LWIP_EVENT_CLOSE_FIN_SEND:
            if (pcb->event_callback != NULL)
            {
                #if 0
                pcb->event_callback(0, 0, 0, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 0/*FFW_SOCKET_EVT_FIN_SEND*/, 0, pcb->arg);
                #endif
            }
            break;

        case LWIP_EVENT_CLOSE_2MSL:
            if (pcb->event_callback != NULL)
            {
                #if 0
                pcb->event_callback(0, 2, 0, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 2/*FFW_SOCKET_EVT_FIN_2MSL*/, 0, pcb->arg);
                #endif
            }
            break;

        case LWIP_EVENT_CLOSE_RST:
            if (pcb->event_callback != NULL)
            {
                #if 0
                pcb->event_callback(0, 3, 0, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 3/*FFW_SOCKET_EVT_FIN_RST*/, 0, pcb->arg);
                #endif
            }
            break;

        case LWIP_EVENT_CLOSE_FIN_ACK:
            if (pcb->event_callback != NULL)
            {
                #if 0
                pcb->event_callback(0, 1, 0, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 1/*FFW_SOCKET_EVT_FIN_ACK*/, 0, pcb->arg);
                #endif
            }
            break;

        case LWIP_EVENT_CLOSE_FIN_RECV:
            if (pcb->event_callback != NULL)
            {
                #if 0
                pcb->event_callback(0, 1, 0, pcb->arg);
                #else
                socket_evt_cb(pcb->event_callback, 0, 5/*FFW_SOCKET_EVT_FIN*/, 0, pcb->arg);
                #endif
            }
            break;

        default:
            OSI_LOGI(0x100075b3, "lwip_tcp_event unprocessed event:%d", lwip_event);
            break;
        }
    }
#endif

    if (conn != NULL && pcb == NULL)
    {
        //OSI_LOGI(0x100075b0, "lwip_tcp_event UDP event");
    }
#if LWIP_TCP
    else
    {
        //OSI_LOGI(0x100075b1, "lwip_tcp_event pcb->is_for_socket=%d", pcb->is_for_socket);
        if (conn == NULL || !pcb->is_for_socket)
            return err;
    }
#endif
    if (lwip_event == LWIP_EVENT_SENT)
        conn->acked_size += size;

#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    //OSI_LOGI(0x100075b2, "lwip_tcp_event sockedid=%d,socket_used=%d,acked_size=%d", (int)conn->callback_arg.socket, (int)cfw_socket_map[conn->callback_arg.socket].socket_used, (int)conn->acked_size);
    if (conn->callback_arg.socket <= 0 || cfw_socket_map[conn->callback_arg.socket].socket_used == 0 || cfw_socket_map[conn->callback_arg.socket].socket_used != (conn->callback_arg.socket))
    {
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return err;
    }
    if (0 == cfw_socket_map[conn->callback_arg.socket].isSCISocket)
    {
        switch (lwip_event)
        {
        case LWIP_EVENT_ACCEPT:
            sys_post_event_to_APP(EV_CFW_TCPIP_ACCEPT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
            break;
        case LWIP_EVENT_SENT:
            sys_post_event_to_APP(EV_CFW_TCPIP_SOCKET_SEND_RSP, conn->callback_arg.socket, size, cfw_socket_map[conn->callback_arg.socket].userParam);
            break;
        case LWIP_EVENT_RECV:
            if (NULL == p)
        {
            if (lwip_getRecvAvailSize(conn->callback_arg.socket) != 0 && cfw_socket_map[conn->callback_arg.socket].dataIND_notify != NULL)
            {
                osiNotifyTrigger(cfw_socket_map[conn->callback_arg.socket].dataIND_notify);
            }
            cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_CLOSING;
            sys_post_event_to_APP(EV_CFW_TCPIP_CLOSE_IND, conn->callback_arg.socket, netconn_err(conn), cfw_socket_map[conn->callback_arg.socket].userParam);
            #ifdef CONFIG_FIBOCOM_BASE
            OSI_PRINTFI("fibocom--EV_CFW_TCPIP_CLOSE_IND");
            #endif
        }
        else
        {
            if (cfw_socket_map[conn->callback_arg.socket].dataIND_notify != NULL)
                osiNotifyTrigger(cfw_socket_map[conn->callback_arg.socket].dataIND_notify);
        }
        break;
    case LWIP_EVENT_CONNECTED:
        cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_CONNECTED;
        sys_post_event_to_APP(EV_CFW_TCPIP_SOCKET_CONNECT_RSP, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
        break;
    case LWIP_EVENT_POLL:
        break;
    case LWIP_EVENT_ERR:
        sys_post_event_to_APP(EV_CFW_TCPIP_ERR_IND, conn->callback_arg.socket, err, cfw_socket_map[conn->callback_arg.socket].userParam);
        break;

    default:
        OSI_LOGI(0x100075b3, "lwip_tcp_event unprocessed event:%d", lwip_event);
        break;
        }
    }
    else
    {
        switch (lwip_event)
        {
        case LWIP_EVENT_ACCEPT:
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
            if (0 != (cfw_socket_map[conn->callback_arg.socket].eventFlag & AS_ACCEPT))
            {
                SOCKET_ACCEPT_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_ACCEPT_EVENT_IND_SIG_T));
                sig->SignalCode = SOCKET_ACCEPT_EVENT_IND;
                sig->SignalSize = sizeof(SOCKET_ACCEPT_EVENT_IND_SIG_T);
                sig->Sender = (BLOCK_ID)cfw_socket_map[conn->callback_arg.socket].taskHandle;
                sig->socket_id = conn->callback_arg.socket;
                //sig->error_code = 0;
                SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[conn->callback_arg.socket].taskHandle);

                //sys_post_event_to_APP(SOCKET_ACCEPT_EVENT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
            }
#endif

            break;
        case LWIP_EVENT_SENT:
            OSI_LOGI(0x1000a96c, "lwip_tcp_event LWIP_EVENT_SENT event");
            break;
        case LWIP_EVENT_RECV:
            if (NULL == p)
            {
                if (lwip_getRecvAvailSize(conn->callback_arg.socket) != 0 && cfw_socket_map[conn->callback_arg.socket].dataIND_notify != NULL)
                {
                    //osiNotifyTrigger(cfw_socket_map[conn->callback_arg.socket].dataIND_notify);
                }

#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
                if (0 != (cfw_socket_map[conn->callback_arg.socket].eventFlag & AS_CLOSE))
                {
                    cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_CLOSING;
                    SOCKET_CONNECTION_CLOSE_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_CONNECTION_CLOSE_EVENT_IND_SIG_T));
                    sig->SignalCode = SOCKET_CONNECTION_CLOSE_EVENT_IND;
                    sig->SignalSize = sizeof(SOCKET_CONNECTION_CLOSE_EVENT_IND_SIG_T);
                    sig->Sender = (BLOCK_ID)cfw_socket_map[conn->callback_arg.socket].taskHandle;
                    sig->socket_id = conn->callback_arg.socket;
                    sig->error_code = 0;
                    SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[conn->callback_arg.socket].taskHandle);
                    //sys_post_event_to_APP(SOCKET_CONNECTION_CLOSE_EVENT_IND, conn->callback_arg.socket, netconn_err(conn), cfw_socket_map[conn->callback_arg.socket].userParam);
                }
#endif
            }
            else
            {

#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
                OSI_LOGI(0x1000a96d, "lwip_tcp_event scisocket  is_inform_read: %d,socket %d", cfw_socket_map[conn->callback_arg.socket].is_inform_read, conn->callback_arg.socket);

                if (((cfw_socket_map[conn->callback_arg.socket].eventFlag & AS_READ) != 0) &&
                    ((cfw_socket_map[conn->callback_arg.socket].is_inform_read == INFORM_STATE_ENABLE) || (cfw_socket_map[conn->callback_arg.socket].is_inform_read == INFORM_STATE_HANDLED)))
                {
                    if (lwip_getRecvAvailSize(conn->callback_arg.socket) > 0)
                    {
                        cfw_socket_map[conn->callback_arg.socket].is_inform_read = INFORM_STATE_SENDING;
                        SOCKET_READ_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_READ_EVENT_IND_SIG_T));
                        sig->SignalCode = SOCKET_READ_EVENT_IND;
                        sig->SignalSize = sizeof(SOCKET_READ_EVENT_IND_SIG_T);
                        sig->Sender = (BLOCK_ID)cfw_socket_map[conn->callback_arg.socket].taskHandle;
                        sig->socket_id = conn->callback_arg.socket;
                        //sig->error_code = 0;
                        SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[conn->callback_arg.socket].taskHandle);
                        //sys_post_event_to_APP(SOCKET_READ_EVENT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
                        cfw_socket_map[conn->callback_arg.socket].is_inform_read = INFORM_STATE_SENT;
                        if (lwip_getRecvAvailSize(conn->callback_arg.socket) > (lwip_getRecvBufSize(conn->callback_arg.socket) >> 1))
                        {
                            SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T));
                            sig->SignalCode = SOCKET_READ_BUFFER_STATUS_EVENT_IND;
                            sig->SignalSize = sizeof(SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T);
                            sig->Sender = (BLOCK_ID)cfw_socket_map[conn->callback_arg.socket].taskHandle;
                            sig->socket_id = conn->callback_arg.socket;
                            //sig->error_code = 0;
                            SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[conn->callback_arg.socket].taskHandle);
                            //sys_post_event_to_APP(SOCKET_READ_BUFFER_STATUS_EVENT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
                        }
                    }
                }
#endif
            }
            break;
        case LWIP_EVENT_CONNECTED:

#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
            if (0 != (cfw_socket_map[conn->callback_arg.socket].eventFlag & AS_CONNECT))
            {
                cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_CONNECTED;
                SOCKET_CONNECT_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_CONNECT_EVENT_IND_SIG_T));
                sig->SignalCode = SOCKET_CONNECT_EVENT_IND;
                sig->SignalSize = sizeof(SOCKET_CONNECT_EVENT_IND_SIG_T);
                sig->Sender = (BLOCK_ID)cfw_socket_map[conn->callback_arg.socket].taskHandle;
                sig->socket_id = conn->callback_arg.socket;
                sig->error_code = 0;
                SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[conn->callback_arg.socket].taskHandle);
                //sys_post_event_to_APP(SOCKET_CONNECT_EVENT_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
            }
#endif

            break;
        case LWIP_EVENT_POLL:
            break;
        case LWIP_EVENT_ERR:
            //sys_post_event_to_APP(EV_CFW_TCPIP_ERR_IND, conn->callback_arg.socket, err, cfw_socket_map[conn->callback_arg.socket].userParam);
            break;

        default:
            OSI_LOGI(0x100075b3, "lwip_tcp_event unprocessed event:%d", lwip_event);
            break;
        }
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return err;
}

void CFW_PostCloseRSP(struct netconn *conn)
{
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x100075b4, "CFW_PostCloseRSP sockedid=%d, socket_used=%d", (int)conn->callback_arg.socket, (int)cfw_socket_map[conn->callback_arg.socket].socket_used);
    if (conn->callback_arg.socket <= 0 || cfw_socket_map[conn->callback_arg.socket].socket_used == 0 || cfw_socket_map[conn->callback_arg.socket].socket_used != (conn->callback_arg.socket))
    {
        OSI_LOGI(0x100075b5, "CFW_PostCloseRSP can not send EV_CFW_TCPIP_SOCKET_CLOSE_RSP");
        OSI_LOGI(0x100075b4, "CFW_PostCloseRSP sockedid=%d, socket_used=%d", (int)conn->callback_arg.socket, (int)cfw_socket_map[conn->callback_arg.socket].socket_used);
        if (conn->callback_arg.socket > 0 && conn->callback_arg.socket < MAX_SOCKET + LWIP_SOCKET_OFFSET)
        {
            cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_COLSED;
        }
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return;
    }
    if (1 == cfw_socket_map[conn->callback_arg.socket].isSCISocket)
    {
        OSI_LOGI(0x1000a96e, "CFW_PostCloseRSP sci socket id=%d", (int)conn->callback_arg.socket);
        cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_COLSED;
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return;
    }
    cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_COLSED;
    sys_post_event_to_APP(EV_CFW_TCPIP_SOCKET_CLOSE_RSP, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
}

void CFW_PostConnectRSP(struct netconn *conn)
{
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x100075b6, "CFW_PostConnectRSP sockedid=%d, socket_used=%d", (int)conn->callback_arg.socket, (int)cfw_socket_map[conn->callback_arg.socket].socket_used);
    if (conn->callback_arg.socket <= 0 || cfw_socket_map[conn->callback_arg.socket].socket_used == 0 || cfw_socket_map[conn->callback_arg.socket].socket_used != (conn->callback_arg.socket))
    {
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return;
    }
    cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_CONNECTED;

    if (0 == cfw_socket_map[conn->callback_arg.socket].isSCISocket)
    {
        sys_post_event_to_APP(EV_CFW_TCPIP_SOCKET_CONNECT_RSP, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
}

void CFW_PostBearerLosingInd(struct netconn *conn)
{

    if (conn == NULL)
    {
        OSI_LOGI(0x100075b7, "CFW_PostBearerLosingInd conn is NULL");
        return;
    }
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x100075b8, "CFW_PostBearerLosingInd sockedid=%d, socket_used=%d", (int)conn->callback_arg.socket, (int)cfw_socket_map[conn->callback_arg.socket].socket_used);
    if (conn->callback_arg.socket <= 0 || cfw_socket_map[conn->callback_arg.socket].socket_used == 0 || cfw_socket_map[conn->callback_arg.socket].socket_used != (conn->callback_arg.socket))
    {
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return;
    }
    cfw_socket_map[conn->callback_arg.socket].status = CFW_SOCKET_BEARER_LOSING;

    if (0 == cfw_socket_map[conn->callback_arg.socket].isSCISocket)
    {
        sys_post_event_to_APP(EV_CFW_TCPIP_BEARER_LOSING_IND, conn->callback_arg.socket, 0, cfw_socket_map[conn->callback_arg.socket].userParam);
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
}

uint32_t CFW_TcpipInetAddr(const char *cp)
{
    return inet_addr((const char *)cp);
}

int32_t CFW_TcpipAvailableBuffer(SOCKET nSocket)
{
    return lwip_availableBuffer(nSocket);
}

uint16_t CFW_TcpipGetRecvAvailable(SOCKET nSocket)
{
    int availsize = 0;
    int ret = lwip_ioctl(nSocket, FIONREAD, &availsize);
    OSI_LOGI(0x100094fa, "CFW_TcpipGetRecvAvailable recv_avail %d ret %d", availsize, ret);
    if (ret <= 0)
        return availsize;
    return ret;
}

extern int errno;
uint32_t CFW_TcpipGetLastError(void)
{
    return errno;
}

SOCKET CFW_TcpipSocket(uint8_t nDomain, uint8_t nType, uint8_t nProtocol)
{
    return CFW_TcpipSocketEX(nDomain, nType, nProtocol, 0, 0);
}

void CFW_TcpipSocketSetISSCI(SOCKET socket)
{
    cfw_socket_map[socket].isSCISocket = 1;
    return;
}

SOCKET CFW_TcpipSocketasyncSelect(SOCKET socket, osiThread_t *task_id, uint32_t event)
{
    int ret = 0;
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
    cfw_socket_map[socket].eventFlag = event;
    cfw_socket_map[socket].taskHandle = (uint32_t)task_id;

    OSI_LOGI(0x1000a96f, "CFW_TcpipSocketasyncSelect0 eventFlag:%d", cfw_socket_map[socket].eventFlag);
    OSI_LOGI(0x1000a970, "CFW_TcpipSocketasyncSelect0 is_inform_read:%d ,socket %d", cfw_socket_map[socket].is_inform_read, socket);
    if (((cfw_socket_map[socket].eventFlag & AS_READ) != 0))
    {

        if (INFORM_STATE_SENDING == cfw_socket_map[socket].is_inform_read)
        {
            OSI_LOGI(0x1000a971, "SOCKET_READ_EVENT_IND_SIG_T is already send");
        }
        else
        {

            if (cfw_socket_map[socket].is_inform_read == INFORM_STATE_DISABLE)
                cfw_socket_map[socket].is_inform_read = INFORM_STATE_ENABLE;
            if (lwip_getRecvAvailSize(socket) > 0)
            {
                cfw_socket_map[socket].is_inform_read = INFORM_STATE_SENDING;
                SOCKET_READ_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_READ_EVENT_IND_SIG_T));
                if (sig == NULL)
                {
                    OSI_LOGE(0x1000a972, "SOCKET_READ_EVENT_IND_SIG_T *sig: malloc error");
                    return -1;
                }
                sig->SignalCode = SOCKET_READ_EVENT_IND;
                sig->SignalSize = sizeof(SOCKET_READ_EVENT_IND_SIG_T);
                sig->Sender = (BLOCK_ID)cfw_socket_map[socket].taskHandle;
                sig->socket_id = socket;
                //sig->error_code = 0;
                SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[socket].taskHandle);
                //sys_post_event_to_APP(SOCKET_READ_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
                cfw_socket_map[socket].is_inform_read = INFORM_STATE_SENT;
                if (lwip_getRecvAvailSize(socket) > (lwip_getRecvBufSize(socket) >> 1))
                {
                    SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T *sig = malloc(sizeof(SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T));
                    sig->SignalCode = SOCKET_READ_BUFFER_STATUS_EVENT_IND;
                    sig->SignalSize = sizeof(SOCKET_READ_BUFFER_STATUS_EVENT_IND_SIG_T);
                    sig->Sender = (BLOCK_ID)cfw_socket_map[socket].taskHandle;
                    sig->socket_id = socket;
                    //sig->error_code = 0;
                    SCI_SendSignal_type((xSignalHeader)sig, (unsigned int)cfw_socket_map[socket].taskHandle);
                    //sys_post_event_to_APP(SOCKET_READ_BUFFER_STATUS_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
                }
            }
        }
    }
    else
    {
        cfw_socket_map[socket].is_inform_read = INFORM_STATE_DISABLE;
    }
    OSI_LOGI(0x1000a973, "CFW_TcpipSocketasyncSelect end is_inform_read:%d", cfw_socket_map[socket].is_inform_read);
/*
    if ((cfw_socket_map[socket].eventFlag & AS_ACCEPT) != 0)
    {
        sys_post_event_to_APP(SOCKET_ACCEPT_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
    }

    if ((cfw_socket_map[socket].eventFlag & AS_CONNECT) != 0)
    {
        sys_post_event_to_APP(SOCKET_CONNECT_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
    }

    if ((cfw_socket_map[socket].eventFlag & AS_CLOSE) != 0)
    {
        sys_post_event_to_APP(SOCKET_CONNECTION_CLOSE_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
    }

    if ((cfw_socket_map[socket].eventFlag & AS_FULLCLOSED) != 0)
    {
        sys_post_event_to_APP(SOCKET_FULL_CLOSED_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
    }

    if ((cfw_socket_map[socket].eventFlag & AS_WRITE) != 0)
    {
        sys_post_event_to_APP(SOCKET_WRITE_EVENT_IND, socket, 0, cfw_socket_map[socket].userParam);
    }*/
#endif
    return ret;
}
SOCKET CFW_TcpipSocketEX(uint8_t nDomain, uint8_t nType, uint8_t nProtocol, osiCallback_t func, uint32_t nUserParam)
{
    int socket = lwip_socket(nDomain, nType, nProtocol);
    if (socket > 0)
    {
        OSI_ASSERT(cfw_socket_map[socket].socket_used == 0, "socket in used");
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        cfw_socket_map[socket].isSCISocket = 0;
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
        cfw_socket_map[socket].is_inform_read = INFORM_STATE_DISABLE;
#endif
        cfw_socket_map[socket].status = CFW_SOCKET_NONE;
        cfw_socket_map[socket].socket_used = socket;
        cfw_socket_map[socket].userParam = nUserParam;
        cfw_socket_map[socket].taskHandle = getRequestTaskHandle();
        cfw_socket_map[socket].func = func;
        cfw_socket_map[socket].dataIND_notify = osiNotifyCreate(osiThreadCurrent(), _dataIndNotify, (void *)socket);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        uint32_t NBIO = 1;
        lwip_ioctl(socket, FIONBIO, &NBIO);
    }
    return socket;
}

void CFW_TcpipSocketSetParam(SOCKET nSocket, osiCallback_t func, uint32_t nUserParam)
{
    if (nSocket > 0)
    {
        OSI_ASSERT(cfw_socket_map[nSocket].socket_used == nSocket, "socket in used");
        cfw_socket_map[nSocket].userParam = nUserParam;
        cfw_socket_map[nSocket].func = func;
    }
}

uint32_t CFW_TcpipSocketConnect(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *pName, uint8_t nNameLen)
{
    if (cfw_socket_map[nSocket].socket_used == nSocket)
        cfw_socket_map[nSocket].status = CFW_SOCKET_CONNECTING;
    int ret = lwip_connect(nSocket, (struct sockaddr *)pName, nNameLen);
    return (ret == 0 || errno == EINPROGRESS) ? 0 : SOCKET_ERROR;
}
int CFW_TcpipSocketSend(SOCKET nSocket, uint8_t *pData, uint16_t nDataSize, uint32_t nFlags)
{
    int ret = lwip_send(nSocket, pData, nDataSize, nFlags);
    if (ret == -1 && (errno == EWOULDBLOCK || errno == EINPROGRESS))
        ret = 0;
    return (ret == -1) ? SOCKET_ERROR : ret;
}

#if LWIP_RFACK
int CFW_TcpipSocketSendWithSeqno(SOCKET nSocket, uint8_t *pData, uint16_t nDataSize, uint32_t nFlags, uint32_t seqno)
{
    int ret = lwip_send_with_seqno(nSocket, pData, nDataSize, nFlags, seqno);
    if (ret == -1 && (errno == EWOULDBLOCK || errno == EINPROGRESS))
        ret = 0;
    return (ret == -1) ? SOCKET_ERROR : ret;
}
#endif

uint32_t CFW_TcpipSocketRecv(SOCKET nSocket, uint8_t *pData, uint16_t nDataSize, uint32_t nFlags)
{
    return CFW_TcpipSocketRecvfrom(nSocket, pData, nDataSize, nFlags, NULL, NULL);
}

uint32_t CFW_TcpipSocketRecvfrom(SOCKET nSocket, void *pMem, int nLen, uint32_t nFlags, CFW_TCPIP_SOCKET_ADDR *from, int *fromlen)
{
    int ret = lwip_recvfrom(nSocket, pMem, nLen, nFlags, (struct sockaddr *)from, (socklen_t *)fromlen);
    if (ret == -1 && errno == EWOULDBLOCK)
        ret = 0;

#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
    if (lwip_getRecvAvailSize(nSocket) == 0)
        cfw_socket_map[nSocket].is_inform_read = INFORM_STATE_HANDLED;
#endif
    return (ret == -1) ? SOCKET_ERROR : ret;
}

int CFW_TcpipSocketClose(SOCKET nSocket)
{
    if (cfw_socket_map[nSocket].socket_used == nSocket)
        cfw_socket_map[nSocket].status = CFW_SOCKET_CLOSING;
    int ret = lwip_close(nSocket);
    OSI_LOGI(0x100075b9, "CFW_TcpipSocketClose:lwip_close ,ret=%d", ret);
    if (!ret && (cfw_socket_map[nSocket].socket_used <= 0 || cfw_socket_map[nSocket].socket_used > MAX_SOCKET))
    {
        OSI_LOGI(0x100075bb, "invalid socket");
        return ret;
    }

    if (!ret)
    {
        OSI_LOGI(0x100075bc, "CFW_TcpipSocketClose set socket_used to 0,ret=%d", ret);
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
#ifdef CONFIG_NET_SCI_NETIF_SUPPORT
        cfw_socket_map[nSocket].is_inform_read = INFORM_STATE_DISABLE;
#endif
        cfw_socket_map[nSocket].socket_used = 0;
        cfw_socket_map[nSocket].userParam = 0;
        cfw_socket_map[nSocket].taskHandle = 0;
        cfw_socket_map[nSocket].func = 0;
        osiNotifyDelete(cfw_socket_map[nSocket].dataIND_notify);
        cfw_socket_map[nSocket].dataIND_notify = NULL;
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
    }
    return ret;
}

uint32_t CFW_TcpipSocketIoctl(SOCKET nSocket, int cmd, void *argp)
{
    return lwip_ioctl(nSocket, cmd, argp);
}

uint32_t CFW_TcpipSocketShutdownOutput(SOCKET nSocket, int how)
{
    return lwip_shutdown(nSocket, how);
}
uint32_t CFW_TcpipSocketSendto(SOCKET nSocket, void *pData, uint16_t nDataSize, uint32_t nFlags, CFW_TCPIP_SOCKET_ADDR *to, int tolen)
{
    int ret = lwip_sendto(nSocket, pData, nDataSize, nFlags, (struct sockaddr *)to, tolen);
    if (ret == -1 && (errno == EWOULDBLOCK || errno == EINPROGRESS))
        ret = 0;
    return (ret == -1) ? SOCKET_ERROR : ret;
}

#if LWIP_RFACK
uint32_t CFW_TcpipSocketSendtoWithSeqno(SOCKET nSocket, void *pData, uint16_t nDataSize, uint32_t nFlags, CFW_TCPIP_SOCKET_ADDR *to, int tolen, uint32_t seqno)
{
    int ret = lwip_sendto_with_seqno(nSocket, pData, nDataSize, nFlags, (struct sockaddr *)to, tolen, seqno);
    if (ret == -1 && (errno == EWOULDBLOCK || errno == EINPROGRESS))
        ret = 0;
    return (ret == -1) ? SOCKET_ERROR : ret;
}
#endif

uint32_t CFW_TcpipSocketBind(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *pName, uint8_t nNameLen)
{
    int ret = lwip_bind(nSocket, (struct sockaddr *)pName, nNameLen);
    if (ret == -1 && errno == EINPROGRESS)
        ret = 0;
    return (ret == -1) ? SOCKET_ERROR : ret;
}

uint32_t CFW_TcpipSocketListen(SOCKET nSocket, uint32_t backlog)
{
    return lwip_listen(nSocket, backlog);
}
uint32_t CFW_TcpipSocketAccept(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *addr, uint32_t *addrlen)
{
    if (nSocket > MAX_SOCKET + LWIP_SOCKET_OFFSET - 1)
    {
        OSI_LOGI(0x100075bd, "CFW_TcpipSocketAccept nSocket error %d", nSocket);
        return SOCKET_ERROR;
    }
    int socket = lwip_accept(nSocket, (struct sockaddr *)addr, (socklen_t *)addrlen);
#if LWIP_TCP
    if (socket > 0)
    {
        int32_t nRecvAvail = 0;
        int32_t nTcpState = 0;
        OSI_ASSERT(cfw_socket_map[socket].socket_used == 0, "invalid socket");
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        cfw_socket_map[socket].socket_used = socket;
        cfw_socket_map[socket].userParam = cfw_socket_map[nSocket].userParam;
        cfw_socket_map[socket].taskHandle = cfw_socket_map[nSocket].taskHandle;
        cfw_socket_map[socket].func = cfw_socket_map[nSocket].func;
        cfw_socket_map[socket].dataIND_notify = osiNotifyCreate(osiThreadCurrent(), _dataIndNotify, (void *)socket);
        if (cfw_socket_map[nSocket].isSCISocket == 1)
        {
            cfw_socket_map[socket].isSCISocket = 1;
        }
        else
        {
            cfw_socket_map[socket].isSCISocket = 0;
        }
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        nRecvAvail = lwip_getRecvAvailSize(socket);
        if (nRecvAvail > 0) // get data befor accept
        {
            if (0 == cfw_socket_map[socket].isSCISocket)
            {
                sys_post_event_to_APP(EV_CFW_TCPIP_REV_DATA_IND, socket, nRecvAvail, cfw_socket_map[socket].userParam);
            }
        }
        nTcpState = lwip_getTcpState(socket);
        if (nTcpState > ESTABLISHED) // closed by remote befor accept
        {
            if (0 == cfw_socket_map[socket].isSCISocket)
            {
                sys_post_event_to_APP(EV_CFW_TCPIP_CLOSE_IND, socket, 0, cfw_socket_map[socket].userParam);
            }
        }
    }
#endif
    return socket;
}
uint32_t CFW_TcpipSocketAcceptEx(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *addr, uint32_t *addrlen, osiCallback_t func, uint32_t nUserParam)
{
    if (nSocket > MAX_SOCKET + LWIP_SOCKET_OFFSET - 1)
    {
        sys_arch_printf("CFW_TcpipSocketAccept nSocket error %d", nSocket);
        return SOCKET_ERROR;
    }
    int socket = lwip_accept(nSocket, (struct sockaddr *)addr, (socklen_t *)addrlen);
#if LWIP_TCP
    if (socket > 0)
    {
        int32_t nRecvAvail = 0;
        int32_t nTcpState = 0;
        OSI_ASSERT(cfw_socket_map[socket].socket_used == 0, "invalid socket");
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        cfw_socket_map[socket].socket_used = socket;
        cfw_socket_map[socket].userParam = nUserParam;
        cfw_socket_map[socket].taskHandle = getRequestTaskHandle();
        cfw_socket_map[socket].func = func;
        cfw_socket_map[socket].dataIND_notify = osiNotifyCreate(osiThreadCurrent(), _dataIndNotify, (void *)socket);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        nRecvAvail = lwip_getRecvAvailSize(socket);
        if (nRecvAvail > 0) // get data befor accept
        {
            sys_post_event_to_APP(EV_CFW_TCPIP_REV_DATA_IND, socket, nRecvAvail, cfw_socket_map[socket].userParam);
        }
        nTcpState = lwip_getTcpState(socket);
        if (nTcpState > ESTABLISHED) // closed by remote befor accept
        {
            sys_post_event_to_APP(EV_CFW_TCPIP_CLOSE_IND, socket, 0, cfw_socket_map[socket].userParam);
        }
    }
#endif
    return socket;
}

uint32_t CFW_TcpipSocketGetsockname(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *pName, int *pNameLen)
{
    return lwip_getsockname(nSocket, (struct sockaddr *)pName, (socklen_t *)pNameLen);
}
uint32_t CFW_TcpipSocketGetpeername(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *pName, int *pNameLen)
{
    return lwip_getpeername(nSocket, (struct sockaddr *)pName, (socklen_t *)pNameLen);
}

// this method will blocked until dns sucess/fail when getRequestTaskHandle return NULL,
// this method will use system dns_servers when cid or sim is not valid
uint32_t CFW_Gethostbyname(char *hostname, ip_addr_t *addr, uint8_t nCid, CFW_SIM_ID nSimID)
{
    int ret = netconn_gethostbyname_ext(hostname, addr, nSimID << 8 | nCid, getRequestTaskHandle(), 0, 0, NETCONN_DNS_IPUN);
    if (ret == ERR_OK || ret == ERR_LOCAL_OK)
        return RESOLV_COMPLETE;
    else if (ret == ERR_INPROGRESS)
        return RESOLV_QUERY_QUEUED;
    else
        return RESOLV_QUERY_INVALID;
}

// this method will blocked until dns sucess/fail when func is NULL Or getRequestTaskHandle return NULL ,
// this method will use system dns_servers when cid or sim is not valid
uint32_t CFW_GethostbynameEX(char *hostname, ip_addr_t *addr, uint8_t nCid, CFW_SIM_ID nSimID,
                             osiCallback_t func, void *param)
{
    uint32_t taskHandle = getRequestTaskHandle();
    if (func == NULL)
        taskHandle = 0;
    int ret = netconn_gethostbyname_ext(hostname, addr, nSimID << 8 | nCid, taskHandle, (u32_t)func, (u32_t)param, NETCONN_DNS_IPUN);
    OSI_LOGI(0x10007af8, "ret=%d", ret);
    if (ret == ERR_OK || ret == ERR_LOCAL_OK)
        return RESOLV_COMPLETE;
    else if (ret == ERR_INPROGRESS)
        return RESOLV_QUERY_QUEUED;
    else
        return RESOLV_QUERY_INVALID;
}
#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
// this method will blocked until dns sucess/fail when func is NULL Or getRequestTaskHandle return NULL ,
// this method will use system dns_servers when cid or sim is not valid
/*resolve hostname by specific server*/
uint32_t CFW_Gethostbyname_SpecificserverEX(char *hostname, ip_addr_t *addr, uint8_t nCid, CFW_SIM_ID nSimID,
                                            osiCallback_t func, void *param, char *dns_server, u8_t addrtype)
{
    uint32_t taskHandle = getRequestTaskHandle();
    if (func == NULL)
        taskHandle = 0;
    int ret = netconn_gethostbyname_specificserver_ext(hostname, addr, nSimID << 8 | nCid, taskHandle, dns_server, (u32_t)func, (u32_t)param, addrtype);
    OSI_LOGI(0x10007af8, "ret=%d", ret);
    if (ret == ERR_OK || ret == ERR_LOCAL_OK)
        return RESOLV_COMPLETE;
    else if (ret == ERR_INPROGRESS)
        return RESOLV_QUERY_QUEUED;
    else
        return RESOLV_QUERY_INVALID;
}
#endif
// this method will blocked until dns sucess/fail when func is NULL Or getRequestTaskHandle return NULL ,
// this method will use system dns_servers when cid or sim is not valid
uint32_t CFW_GetallhostbynameEX(char *hostname, ip_addr_t *addr, uint8_t nCid, CFW_SIM_ID nSimID,
                                osiCallback_t func, void *param, int *addr_num)
{
    uint32_t taskHandle = getRequestTaskHandle();
    int addr_count = 0;
    if (func == NULL)
        taskHandle = 0;
    int ret = netconn_getallhostbyname_ext(hostname, addr, nSimID << 8 | nCid, taskHandle, (u32_t)func, (u32_t)param, NETCONN_DNS_IPUN, &addr_count);
    OSI_LOGI(0x10007af8, "ret=%d", ret);
    if (ret == ERR_OK || ret == ERR_LOCAL_OK)
    {
        if (addr_num != NULL)
            *addr_num = addr_count;
        return RESOLV_COMPLETE;
    }
    else if (ret == ERR_INPROGRESS)
    {
        return RESOLV_QUERY_QUEUED;
    }
    else
    {
        return RESOLV_QUERY_INVALID;
    }
}

int CFW_TcpipSocketConnectEx(SOCKET nSocket, CFW_TCPIP_SOCKET_ADDR *pName, uint8_t nNameLen, CFW_SIM_ID nSimID)
{
    int ret = lwip_connect(nSocket, (struct sockaddr *)pName, nNameLen);
    return (ret == 0 || errno == EINPROGRESS) ? 0 : SOCKET_ERROR;
}

int CFW_TcpipSocketGetsockopt(SOCKET nSocket, int level, int optname, void *optval, int *optlen)
{
    return lwip_getsockopt(nSocket, level, optname, optval, (socklen_t *)optlen);
}

int CFW_TcpipSocketSetsockopt(SOCKET nSocket, int level, int optname, const void *optval, int optlen)
{
    return lwip_setsockopt(nSocket, level, optname, optval, optlen);
}

int32_t CFW_TcpipSocketGetAckedSize(SOCKET nSocket)
{
    return lwip_getAckedSize(nSocket);
}

int32_t CFW_TcpipSocketGetSentSize(SOCKET nSocket)
{
    return lwip_getSentSize(nSocket);
}

socket_status_t CFW_TcpipSocketGetStatus(SOCKET nSocket)
{
    if (nSocket >= LWIP_SOCKET_OFFSET && nSocket < MAX_SOCKET + LWIP_SOCKET_OFFSET)
    {
        return cfw_socket_map[nSocket].status;
    }
    return CFW_SOCKET_NONE;
}

int32_t CFW_TcpipSocketGetMss(SOCKET nSocket)
{
    return lwip_getMss(nSocket);
}

void CFW_TcpipSocketDataNotify(SOCKET nSocket)
{
    if (nSocket > MAX_SOCKET + LWIP_SOCKET_OFFSET - 1 || cfw_socket_map[nSocket].socket_used == 0)
    {
        OSI_LOGI(0x100075be, "CFW_TcpipSocketDataNotify nSocket error %d", nSocket);
        return;
    }
    if (cfw_socket_map[nSocket].socket_used == nSocket && cfw_socket_map[nSocket].dataIND_notify != NULL)
    {
        osiNotifyTrigger(cfw_socket_map[nSocket].dataIND_notify);
    }
    return;
}

int32_t CFW_TcpipSocketGetRecvAvailSize(SOCKET nSocket)
{
    return lwip_getRecvAvailSize(nSocket);
}

#ifdef CONFIG_LWIP_SNTP_SUPPORT
extern osiThread_t *netGetTaskID(void);
CFW_SNTP_CONFIG *CFW_SntpInit(void)
{
    CFW_SNTP_CONFIG *conf = (CFW_SNTP_CONFIG *)malloc(sizeof(CFW_SNTP_CONFIG));
    if (conf != NULL)
    {
        conf->cServer = NULL;
        conf->iRetry = 3;
        conf->iSyncMode = SNTP_OPMODE_POLL;
        conf->bAutoUpdate = 0;
        conf->fCallback = NULL;
        conf->nSimCid = 0;
        conf->fCallback_ctx = NULL;
    }

    return conf;
}

sntp_status_t CFW_SntpStart(CFW_SNTP_CONFIG *SntpConfig)
{
    if (SntpConfig == NULL || SntpConfig->cServer == NULL || strlen(SntpConfig->cServer) == 0)
        return CFW_SNTP_PARAM_INVALID;

    if (sntp_enabled())
        return CFW_SNTP_SYNCING;

    sntp_setoperatingmode(SntpConfig->iSyncMode);
    sntp_setservername(0, SntpConfig->cServer);
    sntp_setdonecb(SntpConfig->fCallback);
    sntp_setdonecb_ctx(0, SntpConfig->fCallback_ctx);
    sntp_setsimcid(0, SntpConfig->nSimCid);
    osiThreadCallback(netGetTaskID(), (osiCallback_t)sntp_init, NULL);

    return CFW_SNTP_READY;
}

void CFW_SntpStop(CFW_SNTP_CONFIG *SntpConfig)
{
    if (SntpConfig != NULL)
    {
        memset(SntpConfig, 0, sizeof(CFW_SNTP_CONFIG));
        free(SntpConfig);
    }

    sntp_setdonecb(NULL);
    sntp_setservername(0, NULL);
    sntp_setdonecb_ctx(0, NULL);

    osiThreadCallback(netGetTaskID(), (osiCallback_t)sntp_stop, NULL);
}
#endif
bool CFW_get_Netif_dataCountBySimCid(uint16_t simID, uint16_t CID, uint16_t uType, uint16_t uDataType, uint32_t *loadsize)
{
    int iRet = false;
    struct netif *netif = NULL;

    if (loadsize == NULL)
        goto LEAVE;
    else
        *loadsize = 0;

    netif = getGprsGobleNetIf(simID, CID);
    if (netif == NULL)
        goto LEAVE;

    switch (uDataType)
    {
    case LWIP_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32LwipULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32LwipDLSize;
        }
        break;
    case PPP_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32PPPULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32PPPDLSize;
        }
        break;
    case RNDIS_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32RndisULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32RndisDLSize;
        }
        break;
    default:
        OSI_LOGI(0x100075bf, "unknown Data Type uDataType: %d\n", uDataType);
        break;
    }
    iRet = true;
    OSI_LOGI(0x100075c0, "loadsize: %ld\n", *loadsize);
LEAVE:
    return iRet;
}

bool CFW_get_Netif_dataCount(struct netif *netif, uint16_t uType, uint16_t uDataType, uint32_t *loadsize)
{
    int iRet = false;

    if (loadsize == NULL)
        goto LEAVE;
    else
        *loadsize = 0;

    if (netif == NULL)
        goto LEAVE;

    switch (uDataType)
    {
    case LWIP_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32LwipULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32LwipDLSize;
        }
        break;
    case PPP_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32PPPULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32PPPDLSize;
        }
        break;
    case RNDIS_DATA:
        if (uType == UPLOAD)
        {
            *loadsize = netif->u32RndisULSize;
        }
        else if (uType == DOWNLOAD)
        {
            *loadsize = netif->u32RndisDLSize;
        }
        break;
    default:
        OSI_LOGI(0x100075bf, "unknown Data Type uDataType: %d\n", uDataType);
        break;
    }
    iRet = true;
    OSI_LOGI(0x100075c0, "loadsize: %ld\n", *loadsize);
LEAVE:
    return iRet;
}
