#ifndef DC6FC293_058C_406F_B473_1170B51410F4
#define DC6FC293_058C_406F_B473_1170B51410F4

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "drv_uart.h"

#define UART_INSTANCE_NUM (6)
#define UART_WAIT_TX_DONE_TIMEOUT (500)
#define UART_WATI_RX_DONE_TIMEOUT (10)
#define DRV_UART_EVENT_RX_TIMEOUT DRV_UART_EVENT_TX_COMPLETE << 1

/**
* @brief Callback for recv uart data
*
 */
typedef void (*ffw_uart_input_callback_t)(int uart_port, uint8_t *data, uint16_t len, void *arg);

// type define struct uart_instance_s
typedef struct uart_instance_s uart_instance_t;
struct uart_instance_s
{
    int uart_port;
    int name;
    drvUart_t *drv;
    uint32_t pending_event;
    drvUartCfg_t drvcfg;
    osiWork_t *work;
    bool init;
    uint8_t *recv_buf;
    uint32_t recv_len;
    uint32_t recv_buf_len;
    uint32_t rx_buf_size;
    ffw_uart_input_callback_t recv_cb;
    osiTimer_t *recv_timer;
    uint32_t recv_timeout;
    void *arg;
    void *sem;
};

/// type define struct ffw_uart_config_s
typedef struct ffw_uart_config_s ffw_uart_config_t;

/// type define enum ffw_uart_data_bits_s
typedef enum ffw_uart_data_bits_s ffw_uart_data_bits_t;

/// type define enum racon_uart_stop_bits_s
typedef enum ffw_uart_stop_bits_s ffw_uart_stop_bits_t;

/// rx buffer size
#define UART_RX_BUF_SIZE (4 * 1024)

/// tx buffer size
#define UART_TX_BUF_SIZE (4 * 1024)

/// data bit define
enum ffw_uart_data_bits_s
{
    FFW_UART_DATA_BITS_7 = 7,
    FFW_UART_DATA_BITS_8 = 8
};

/// stop bit define
enum ffw_uart_stop_bits_s
{
    FFW_UART_STOP_BITS_1 = 1,
    FFW_UART_STOP_BITS_2 = 2
};

/// parity bit define
typedef enum
{
    FFW_UART_NO_PARITY,   ///< No parity check
    FFW_UART_ODD_PARITY,  ///< Parity check is odd
    FFW_UART_EVEN_PARITY, ///< Parity check is even
} ffw_uart_parity_t;

/**
 * @brief 串口配置定义
 * 
 */
struct ffw_uart_config_s
{
    /// baudrate
    uint32_t baud; 

    /// data bits                    
    ffw_uart_data_bits_t data_bits; 

    /// stop bits
    ffw_uart_stop_bits_t stop_bits;

    /// parity
    ffw_uart_parity_t parity;

    /// < enable cts or not
    bool cts_enable;

    /// < enable rts or not
    bool rts_enable;

    /// < rx buffer size
    size_t rx_buf_size;

    /// < tx buffer size
    size_t tx_buf_size;

    /// recv data timeout in ms
    uint32_t recv_timeout;
};

/**
 * @brief 打开一个串口
 * 
 * @param uart_port 串口号
 * @param uart_config 串口配置
 * @param recv_cb 接收数据回调函数
 * @param arg 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_uart_init(int uart_port, ffw_uart_config_t *uart_config, ffw_uart_input_callback_t recv_cb, void *arg);


/**
 * @brief 打开一个串口，此时接收数据通过 @c ffw_uart_get 获取
 * 
 * @param uart_port 串口号
 * @param uart_config 串口配置
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_uart_init1(uint8_t uart_port, ffw_uart_config_t *uart_config);


/**
 * @brief 从串口接收数据，只能在使用 @c racoon_uart_init1 初始化时使用
 * 
 * @param uart_port 串口号
 * @param buf 缓冲区
 * @param len 缓冲区的长度
 * @param timeout 超时时间，单位ms，如果为0表示直接返回，如果为-1，表示一直超时
 * @return int 已接收的数据长度
 * @retval -1 发送失败
 */
int ffw_uart_get(uint8_t uart_port, uint8_t *buf, uint32_t len, int32_t timeout);

/**
 * @brief 从串口发送数据
 * 
 * @param uart_port 串口号
 * @param buff 数据
 * @param len 数据长度
 * @return int 已发送的数据长度
 * @retval -1 发送失败
 */
int ffw_uart_put(int uart_port, uint8_t *buff, uint32_t len);

/**
 * @brief 等待缓冲区的数据发送完毕
 * 
 * @param uart_port 串口号
 * @return int 
 * @retval FFW_R_SUCCESS: 发送完毕
 */
int ffw_uart_wait_send_finish(int uart_port);

/**
 * @brief 关闭串口
 * 
 * @param uart_port 串口号
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_uart_deinit(int uart_port);

#endif /* DC6FC293_058C_406F_B473_1170B51410F4 */
