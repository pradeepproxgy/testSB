#ifndef EDC2C806_DB94_4163_9193_A6478F4F669F
#define EDC2C806_DB94_4163_9193_A6478F4F669F

/**
* @file ffw_ftpc.h
* @author sundq (sundaqing@fibocom.com)
* @brief FTP客户端实现
* @version 0.1
* @date 2020-12-27
* 
* @copyright Copyright (c) 2020
* 
 */

#include <stdio.h>
#include <stdint.h>

#include "ffw_network.h"
#ifdef TLS_ENABLE
#include "ffw_network_tls.h"
#endif
#include "ffw_transport_poll.h"
#include "ffw_transport_interface.h"
#include "ffw_os.h"
#include "ffw_list.h"
#include "ffw_buffer.h"
#include "ffw_rbuffer.h"

/// The max length of ftp command
#define FFW_FTPC_MAX_CMD_LEN (1024)

/// The max event report to user once
#define FFW_FTPC_MAX_EVENT (4)

/// port mode
#define FFW_FTP_MODE_PORT (1)

/// PASV mode
#define FFW_FTP_MODE_PASV (2)

/// ipv4
#define FFW_FTP_DATA_IPV4 (1)

/// ipv6
#define FFW_FTP_DATA_IPV6 (2)

/// type of ffw_ftpcopt_s
typedef struct ffw_ftpcopt_s ffw_ftpcopt_t;

/// type of ffw_ftpc_s
typedef struct ffw_ftpc_s ffw_ftpc_t;

/**
* @brief 命令状态
* 
 */
typedef enum
{
    FTP_STOP,    /* do nothing state, stops the state machine */
    FTP_WAIT220, /* waiting for the initial 220 response immediately after
                  a connect */
    FTP_AUTH,
    FTP_USER,
    FTP_PASS,
    FTP_ACCT,
    FTP_PBSZ,
    FTP_PROT,
    FTP_CCC,
    FTP_PWD,
    FTP_SYST,
    FTP_NAMEFMT,
    FTP_QUOTE, /* waiting for a response to a command sent in a quote list */
    FTP_RETR_PREQUOTE,
    FTP_STOR_PREQUOTE,
    FTP_POSTQUOTE,
    FTP_CWD,       /* change dir */
    FTP_MKD,       /* if the dir didn't exist */
    FTP_MDTM,      /* to figure out the datestamp */
    FTP_TYPE,      /* to set type when doing a head-like request */
    FTP_LIST_TYPE, /* set type when about to do a dir list */
    FTP_RETR_TYPE, /* set type when about to RETR a file */
    FTP_STOR_TYPE, /* set type when about to STOR a file */
    FTP_SIZE,      /* get the remote file's size for head-like request */
    FTP_RETR_SIZE, /* get the remote file's size for RETR */
    FTP_STOR_SIZE, /* get the size for STOR */
    FTP_REST,      /* when used to check if the server supports it in head-like */
    FTP_RETR_REST, /* when asking for "resume" in for RETR */
    FTP_STOR_REST, /* when asking for "resume" in for STOR */
    FTP_PORT,      /* generic state for PORT, LPRT and EPRT, check count1 */
    FTP_PRET,      /* generic state for PRET RETR, PRET STOR and PRET LIST/NLST */
    FTP_PASV,      /* generic state for PASV and EPSV, check count1 */
    FTP_LIST,      /* generic state for LIST, NLST or a custom list command */
    FTP_RETR,
    FTP_STOR,
    FTP_QUIT,
    FTP_RNFR,
    FTP_RNTO,
    FTP_STAT,
    FTP_DELETE,
    FTP_LAST /* never used */
} ffw_ftpstat_t;

/**
* @brief 运行过程中给用户上报的事件
* 
 */
typedef enum
{
    ///执行命令错误事件
    FFW_FTP_CMD_FAILED = 0,

    /// 登录成功
    FFW_FTP_LOGIN_OK,

    /// 登录失败
    FFW_FTP_LOGIN_FAIL,

    /// PWD命令执行成功
    FFW_FTP_PWD_OK,

    /// PWD命令执行失败
    FFW_FTP_PWD_FAILED,

    /// ACCOUNT命令执行成功
    FFW_FTP_ACCOUNT_OK,

    /// PORT命令执行成功
    FFW_FTP_PORT_OK,

    /// 数据通道连接成功
    FFW_FTP_DATA_CONNECTTED,

    /// 数据通道连接失败
    FFW_FTP_DATA_CONNECTTED_FAILED,

    ///收到数据
    FFW_FTP_DATA,

    ///收到数据,缓存下来了，通知读取
    FFW_FTP_DATA_NOTIFY,

    ///收到STAT数据,从命令通道接收
    FFW_FTP_STAT_DATA,

    ///长时间没有收取数据超时
    FFW_FTP_DATA_TIMEOUT,

    ///开始接收或者发送数据
    FFW_FTP_DATA_TRANS_START,

    ///数据接收或者发送发生错误
    FFW_FTP_DATA_ERROR,

    ///数据接收或者发送完毕
    FFW_FTP_DATA_END,

    ///消息事件，只要收到服务器的回应，这个事件就是上报原始消息内容
    FFW_FTP_MSG,

    ///发送给服务器的命令内容
    FFW_FTP_SEND,

    ///命令通道连接失败
    FFW_FTP_CONNECT_FAIL,

    ///命令通道断开
    FFW_FTP_CMD_DISCONNECT,

    ///命令执行完毕
    FFW_FTP_CMD_FINISH,

    ///接收数据超时
    FFW_FTP_DATA_RECV_TIMEOUT,

    /// 获取文件大小事件成功
    FFW_FTP_CMD_SIZE_OK,

    /// 获取文件大小事件失败
    FFW_FTP_CMD_SIZE_FAIL,

    /// 发送数据流控关闭，可以继续发送数据
    FFW_FTP_SEND_AVA,

    /// 最大事件
    FFW_FTP_END
} ftpevent_t;

/**
* @brief FTP的数据处理模式
* 
 */
typedef enum
{
    /// FTP直接上报收到的文件数据
    FFW_FTP_DATA_MODE_REPORT = 0,

    /// FTP会将收到的数据先缓存下来，等待调用接口读取
    FFW_FTP_DATA_MODE_CACHE,
} ffw_ftp_mode_t;

/**
* @brief FTP客户端运行过程中的切换状态
* 
 */
typedef enum
{
    ///空闲状态
    FFW_FTP_STATUS_CMD_END = 0,

    /// 执行命令开始
    FFW_FTP_STATUS_CMD_START,

    /// PASV模式下表示TCP连接已经发起，等待三次握手完成；PORT模式表示listen已经完成，等待服务连接
    FFW_FTP_STATUS_DATA_TUNNEL_OPENING,

    /// 150收到，但是数据通道还没有连接完成
    FFW_FTP_STATUS_DATA_TUNNEL_WAIT,

    /// 226收到，但是数据通道还没有连接完成
    FFW_FTP_STATUS_DATA_RECV_WAIT,

    /// 226收到，才开始接accept并收数据
    FFW_FTP_STATUS_DATA_TRANS_START1,

    /// 数据通道已经建立完成，但是命令通道的150还没有收到
    FFW_FTP_STATUS_DATA_TUNNEL_WAIT_150,

    /// 数据通道已经建立完成，命令通道150已经收到，开始数据传输
    FFW_FTP_STATUS_DATA_TRANS_START,

    /// 数据已经传送完毕，等待命令通道通知结果，226或者426
    FFW_FTP_STATUS_DATA_TRANS_ENDING,

    /// 数据已经传送失败，等待命令通道通知结果，226或者426
    FFW_FTP_STATUS_DATA_TRANS_ERROR_ENDING,

    /// 接收数据超时
    FFW_FTP_STATUS_DATA_TRANS_TIMEOUT,
} ftp_status_t;

/**
* @brief FTP事件上报回调函数类型
* @param c FTP句柄
* @param evt 事件ID,类型为 #ftpevent_t
* @param data 事件对应的数据
* @param len @c data的长度
* @param arg 回调的用户数据，此数据是在 ffw_ftpc_init() 初始化时 @ref ffw_ftpcopt_s.arg 中设定的
* 
 */
typedef void (*ftpevent_cb_t)(ffw_ftpc_t *c, ftpevent_t evt, uint8_t *data, uint32_t len, void *arg);

/**
* @brief 初始化时使用的选项结构体
* 
 */
struct ffw_ftpcopt_s
{
    /// 用户名
    char *username;

    /// 密码
    char *password;

    /// 账号
    char *account;

    /// 数据通道源端口
    uint16_t data_port;

    /// 目的地址
    char *dst_addr;

    /// 命令通道源端口
    uint16_t src_port;

    /// 目的端口
    uint16_t dst_port;

    ///事件上报回调函数
    ftpevent_cb_t cb;

    /// 使用的网卡，可以是网卡名字或者网卡句柄
    ffw_handle_t netif;

    /// 对应 struct ffw_net_handler_s 中的 id字段，用户获取socket或者dns的配置
    int sock_idx;

    ///事件上报回调数据
    void *arg;
};

/**
* @brief FTP客户端定义
* 
 */
struct ffw_ftpc_s
{

    /// 是否已经初始化
    bool init;

    /// 数据通道连接
    ffw_transport_t t;

    /// 当前FTP客户端的状态
    ffw_ftpstat_t stat;

    /// 初始化数据
    ffw_ftpcopt_t opt;

    /// 是否使用TLS
    bool tls;

    /// cmd tunnel status
    ftp_status_t status;

    ///是否已经登录
    bool login;

    /// 是否已经中断
    bool abort;

    /// 接收数据的原始缓冲区
    char cmd_buff[FFW_FTPC_MAX_CMD_LEN];

    /// 接收数据的BUFFER
    ffw_buffer_t cmd_recv;

    /// 发送FTP CMD列表
    struct list_head cmd_list;

    /// 数据通道连接
    ffw_transport_t conn_t;

    /// 如果位PASV模式，在为侦听连接
    ffw_transport_t passive_t;

    ///下载偏移量
    uint32_t download_offset;
    ///当前下载文件的大小
    uint32_t file_size;
    ///当前下载的大小
    uint32_t download_data_size;
    ///总共需要下载的大小,如果为-1，则表示下载全部文件
    uint32_t download_size;

    ///同步执行时，用来保存执行结果
    int result;

    /// 信号量
    ffw_handle_t sem;

    /// 互斥锁
    ffw_handle_t lock;

    /// 缓存的事件
    char evt_notify[FFW_FTPC_MAX_EVENT];

    /// 缓存的事件个数
    int evt_count;

    /// 如果处理数据模式为 ::FFW_FTP_DATA_MODE_CACHE, 数据先缓存在这里
    ffw_rbuffer_t rxbuffer;

    /// 数据处理方式
    ffw_ftp_mode_t mode;

    /// 如果 ffw_ftp_mode_t::mode 的值为 ::FFW_FTP_DATA_MODE_CACHE 则这里指定缓存的大小，否则这个值没意义
    int32_t buff_size;

    /// 接收数据的超时时间，如果 @c xon 为true，则会停止接收数据，此为停止接收数据的超时时间，单位毫秒
    uint32_t recvtimeout;

    /// 是否需要按照摩托罗拉的方式编码，之后上报数据。
    bool encode;

    /// 计时器句柄
    ffw_handle_t timer;

    /// 记录解码数据的状态，处理摩托罗拉时的编码 [0x10 0x03 --> 0x03]  [0x10 0x10 --> 0x10]
    /// 0 表示当前为转义字符 1表示正常字符
    uint8_t decode_status;

    /// 是否流控
    bool xoff;
};

/**
* @brief 设置FTP的模式，支持 FFW_FTP_MODE_PORT 和 FFW_FTP_MODE_PASV
* 
* @param mode FFW_FTP_MODE_PORT or FFW_FTP_MODE_PASV
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_set_mode(int mode);

/**
* @brief 初始化一个FTP句柄
* 
* @param c 待初始化的FTP句柄
* @param opt 初始化选项，详情参考 @c ffw_ftpcopt_t
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_init(ffw_ftpc_t *c, ffw_ftpcopt_t *opt);

/**
* @brief FTP登录，如果在AT线程，sync参数必须为false
* 
* @param c FTP句柄
* @param tls 是否使用TLS
* @param sync 是否同步
* @param timeout 如果为同步，则此为超时时间，单位为毫秒
* @return int 
 */
int ffw_ftpc_login(ffw_ftpc_t *c, bool tls, int32_t timeout, bool sync);

/**
* @brief 获取目录列表
* 
* @param c FTP句柄
* @param dir_path 目录
* @param sync 是否同步
* @param timeout 如果为同步，则此为超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_list(ffw_ftpc_t *c, const char *dir_path, bool sync, int32_t timeout);

/**
* @brief 获取目录的状态，此命令为特殊命令，不通用
* 
* @param c FTP句柄
* @param dir_path 目录
* @param sync 是否同步
* @param timeout 如果为同步，则此为超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_stat(ffw_ftpc_t *c, const char *dir_path, bool sync, int32_t timeout);

/**
* @brief 下载文件
* 
* @param c FTP句柄
* @param file_path 文件路径
* @param offset 偏移量
* @param size 下载大小，如果小于0，则表示下载整个文件
* @param type 0: A 1: I
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_retr(ffw_ftpc_t *c, const char *file_path, uint32_t offset, int32_t size, int type);

/**
* @brief 下载文件
* 
* @param c FTP句柄
* @param file_path 文件路径
* @param offset 偏移量
* @param size 下载大小，如果小于0，则表示下载整个文件
* @param type 0: A 1: I
* @param cached 是否是缓存模式
* @param buff_size 如果是缓存模式，则需要设置缓存的大小
* @param timeout 如果是缓存模式，读取数据的超时事件，单位为毫秒
* @param encode 是否上报数据钱进行摩托路罗拉编码
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_retr1(ffw_ftpc_t *c, const char *file_path, uint32_t offset, int32_t size, int type, bool cached, int32_t buff_size, int32_t timeout, bool encode);

/**
* @brief 上传文件，此函数是异步函数，在处理过程中会通过在 ffw_ftpc_init() 初始化时 @ref ffw_ftpcopt_s.cb 设定
*        回调函数上报 #ftpevent_t 事件
*        - ::FFW_FTP_DATA
*        - ::FFW_FTP_DATA_TRANS_START
*        - ::FFW_FTP_DATA_ERROR
*        - ::FFW_FTP_DATA_END
*        - ::FFW_FTP_CMD_FINISH
* 
* @param c FTP句柄
* @param file_path 文件路径 
* @param offset 下载文件的偏移量，此时会发送REST命令
* @param type 文件类型 0: 表示文本类型 1: 表示二进制类型
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
*/
int ffw_ftpc_stor(ffw_ftpc_t *c, const char *file_path, uint32_t offset, int type);

/**
* @brief 通过FTP数据通道发送数据，此函数必须在 @c ffw_ftpc_wait_data_tunnel 函数返回之后才能执行, 或者等待 @c FFW_FTP_DATA_TRANS_START 事件。
* 
* @param c FTP句柄
* @param data 数据
* @param len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_send_data(ffw_ftpc_t *c, uint8_t *data, uint32_t len);

/**
 * @brief 通过FTP数据通道发送摩托罗拉编码时的数据，此函数必须在 @c ffw_ftpc_wait_data_tunnel 函数返回之后才能执行, 或者等待 @c FFW_FTP_DATA_TRANS_START 事件。
 * 
 * @param c FTP句柄
 * @param data 数据
 * @param len 数据长度
 * @param end 是否最后一包
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_send_data_decode(ffw_ftpc_t *c, uint8_t *data, uint32_t len, bool *end);

/**
* @brief 在上传文件时，如果文件数据已经发送完毕，最后需要调用这个接口
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_send_data_finish(ffw_ftpc_t *c);

/**
* @brief 销毁FTP客户端实例
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_deinit(ffw_ftpc_t *c);

/**
* @brief 等待FTP命令执行完毕
* 
* @param c FTP句柄
* @param timeout 超时时间，单位毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_TIMEOUT: 超时
 */
int ffw_ftpc_wait_cmd_finish(ffw_ftpc_t *c, int32_t timeout);

/**
* @brief 等待数据通道打开完成，此时就可以发送数据了。
* 
* @param c FTP句柄
* @param timeout 超时时间，单位毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_TIMEOUT: 超时
 */
int ffw_ftpc_wait_data_tunnel(ffw_ftpc_t *c, int32_t timeout);

/**
 * @brief 将事件转换为字符串
 * 
 * @param evt 事件
 * @return const char* 事件对应的字符串 
 */
const char *ffw_ftpc_evt_str(ftpevent_t evt);

/**
 * @brief 读取FTP缓冲区中的数据，只有数据模式为 ::FFW_FTP_DATA_MODE_CACHE 才可以调用
 * 
 * @param c FTP句柄
 * @param size 缓冲区大小
 * @param cb 回调函数,数据通过这个函数送出
 * @param arg 回调上下文
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_read_data(ffw_ftpc_t *c, uint32_t size, int (*cb)(void *data, int size, void *arg), void *arg);

/**
* @brief 关闭数据通道
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_close_data_tunnel(ffw_ftpc_t *c);

/**
* @brief 执行任意一个FTP命令
* 
* @param c FTP句柄
* @param cmd FTR 命令
* @param sync 是否同步执行
* @param timeout 如果为同步，则此为超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_command(ffw_ftpc_t *c, const char *cmd, bool sync, uint32_t timeout);

/**
* @brief 重命名文件
* 
* @param c FTP句柄
* @param from 源文件名
* @param to 目的文件名
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_rename(ffw_ftpc_t *c, const char *from, const char *to);

/**
* @brief 获取FTP当前工作目录,上报 ::FFW_FTP_PWD_OK 事件
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_pwd(ffw_ftpc_t *c);

/**
* @brief 获取文件大小,上报 ::FFW_FTP_SIZE_OK 事件
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_filesize(ffw_ftpc_t *c, const char *filename);

/**
 * @brief 改变当前工作目录
 * 
 * @param c FTP句柄
 * @param dirname 目标工作目录
 * @return int 
 */
int ffw_ftpc_cwd(ffw_ftpc_t *c, const char *dirname);

/**
* @brief 获取文件大小,上报 ::FFW_FTP_SIZE_OK 事件
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_delfile(ffw_ftpc_t *c, const char *filename);

/**
* @brief 判断FTP是否登录
* 
* @param c FTP句柄
* @return true 已登录
* @return false 未登录
 */
bool ffw_ftpc_is_login(ffw_ftpc_t *c);

/**
* @brief 终端下载过程
* 
* @param c FTP句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ftpc_download_abort(ffw_ftpc_t *c);

#endif /* EDC2C806_DB94_4163_9193_A6478F4F669F */
