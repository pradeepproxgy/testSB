
#ifndef _OC_SOCKET_H_
#define _OC_SOCKET_H_

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <sys/types.h>
#include "fibo_opencpu_comm.h"

#include "ffw_socket.h"
typedef struct sockaddr      fibo_sockaddr_t;
typedef struct ffw_addrinfo fibo_addrinfo_t;

typedef void (*key_callback)(void* param);
typedef struct
{
    uint8_t result;
    int64_t time;
}fibo_ntp_rsp;

typedef struct
{
    int32_t send_count;
    int32_t recv_count;
    int32_t lost_count;
    int32_t min_rtt;
    int32_t max_rtt;
    int32_t total_rtt;
    int32_t rtt_avg;
}fibo_ping_rsp;

typedef struct
{
    uint8_t ping_mode;
    char *ip;
    uint16_t total_num;
    uint16_t packet_len;
    uint8_t ping_ttl;
    uint32_t ping_tos;
    int32_t ping_time_out;
    key_callback cb;
}fibo_ping_para;

typedef struct
{
    char *host_name;
    uint16_t host_port;
    int ip_type;
    char *dnsServerIp1;
    char *dnsServerIp2;
    fibo_addrinfo_t **peer_addr;
    int32_t timeout;
}fibo_addrinfo_para;

#if 1
extern void fibo_sock_freeaddrinfo(fibo_addrinfo_t *ai);
extern int fibo_sock_accept_v2(int s, struct sockaddr *addr, uint32_t *addrlen);
extern int fibo_sock_bind_v2(int s, const struct sockaddr *name, uint32_t namelen);
extern int fibo_sock_shutdown(int s, int how);
extern int fibo_sock_getpeername(int s, struct sockaddr *name, uint32_t *namelen);
extern int fibo_sock_getsockname(int s, struct sockaddr *name, uint32_t *namelen);
extern int fibo_sock_setsockopt(int s, int level, int optname, const void *optval, uint32_t optlen);
extern int fibo_sock_getsockopt(int s, int level, int optname, void *optval, uint32_t *optlen);
extern int fibo_sock_close_v2(int s);
extern int fibo_sock_connect_v2(int s, const struct sockaddr *name, uint32_t namelen);
extern int fibo_sock_listen_v2(int s, int backlog);
extern ssize_t fibo_sock_recv_v2(int s, void *mem, size_t len, int flags);
extern ssize_t fibo_sock_recvfrom_v2(int s, void *mem, size_t len, int flags, struct sockaddr *from, uint32_t *fromlen);
extern ssize_t fibo_sock_send_v2(int s, const void *dataptr, size_t size, int flags);
extern ssize_t fibo_sock_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, uint32_t tolen);
extern int fibo_sock_socket(int domain, int type, int protocol);
extern int fibo_sock_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
extern ssize_t fibo_sock_read(int s, void *mem, size_t len);
extern ssize_t fibo_sock_write(int s, const void *dataptr, size_t size);
extern int fibo_sock_fcntl(int s, int cmd, int val);
extern int fibo_sock_ioctl(int s, long cmd, void *argp);
extern void* fibo_sock_getDefaultNetif();

extern char *fibo_sock_ip6addr_ntoa_r(const ip6_addr_t *addr, char *buf, int buflen);
extern char *fibo_sock_ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen);
extern int fibo_sock_ip6addr_aton(const char *cp, ip6_addr_t *addr);
extern int fibo_sock_ip4addr_aton(const char *cp, ip4_addr_t *addr);

extern INT32 fibo_errno_get(void);
extern INT32 fibo_sock_ntp(char *addr, int port, CFW_SIM_ID nSimID, key_callback cb);
extern INT32 fibo_mping_v2(fibo_ping_para *paras);
extern INT32 fibo_get_addrinfo(fibo_addrinfo_para *paras);

extern INT32 fibo_dns_get(uint8_t simid, uint8_t cid, char *priv4, char *secv4, char *priv6, char *secv6);
extern INT32 fibo_get_global_DNS(char *pri_dns, char *sec_dns);
extern INT32 fibo_set_global_DNS(const char *pri_dns, const char *sec_dns);

extern int fibo_get_NetInfo(uint32_t *pNetInfo, uint8_t DataSize);
#endif

#if 1//for old API
typedef struct
{
    UINT16 sin_port; /* save as network sequence */
    struct ip_addr sin_addr;
} GAPP_TCPIP_ADDR_T;

/* these three macro for the para of fibo_sock_create use */
#define GAPP_IPPROTO_TCP 0
#define GAPP_IPPROTO_UDP 1
#define GAPP_IPPROTO_SEC 2

#define FIBO_DEFAULT_PDP_TYPE_IPV4       1
#define FIBO_DEFAULT_PDP_TYPE_IPV6       2
#define FIBO_DEFAULT_PDP_TYPE_IPV4V6     3

extern INT32 fibo_sock_create(INT32 nProtocol);
extern INT32 fibo_sock_create_ex(int domain, int type, int protocol);
extern INT32 fibo_get_socket_error(void);
extern INT32 fibo_sock_available_buffer(INT32 sock);
extern INT32 fibo_sock_close(INT32 sock);
extern INT32 fibo_sock_connect(INT32 sock, GAPP_TCPIP_ADDR_T *addr);
extern INT32 fibo_sock_listen(INT32 sock);
extern INT32 fibo_sock_bind(INT32 sock, GAPP_TCPIP_ADDR_T *addr);
extern INT32 fibo_sock_accept(INT32 sock, GAPP_TCPIP_ADDR_T *addr);
extern INT32 fibo_sock_send(INT32 sock, UINT8 *buff, UINT16 len);
extern INT32 fibo_sock_send2(INT32 sock, UINT8 *buff, UINT16 len, GAPP_TCPIP_ADDR_T *dest);
extern INT32 fibo_sock_recv(INT32 sock, UINT8 *buff, UINT16 len);
extern INT32 fibo_sock_recvFrom(INT32 sock, UINT8 *buff, UINT16 len, GAPP_TCPIP_ADDR_T *src);
extern INT32 fibo_getHostByName(char *hostname, ip_addr_t *addr, uint8_t nCid, CFW_SIM_ID nSimID);
extern INT32 fibo_sock_setOpt(INT32 sock, INT32 level, INT32 optname, const void *optval, INT32 optlen);
extern INT32 fibo_sock_getOpt(INT32 sock, INT32 level, INT32 optname, void *optval, INT32 *optlen);
extern INT32 fibo_sock_lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,struct timeval *timeout);
extern INT32 fibo_sock_lwip_fcntl(int s, int cmd, int val);
#endif
extern INT32 fibo_sock_bind_local_info(int32_t sockid, int family, uint16_t src_port);
extern INT32 fibo_default_pdp_type(void);
typedef void (*host_async_callback)(int result, fibo_addrinfo_t *addrinfo, void *arg);
extern INT32 fibo_get_host_by_url(uint8_t nSyncType, fibo_addrinfo_para *paras, host_async_callback host_cb);
#endif


