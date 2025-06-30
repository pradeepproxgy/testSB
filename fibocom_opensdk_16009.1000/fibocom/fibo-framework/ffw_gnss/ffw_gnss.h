#ifndef LIB_PLATEFORM_HD_GNSS_INCLUDE_HD_GNSS
#define LIB_PLATEFORM_HD_GNSS_INCLUDE_HD_GNSS

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef enum gnss_status_s
{
    GNSS_STATUS_POWEROFF = 0,                  ///已经下电
    GNSS_STATUS_POWERON_ING,                   ///正在上电中
    GNSS_STATUS_RUNNING,                       ///正常工作中
    GNSS_STATUS_ENTERGING_DOWNLOAD_BOOTLOADER, ///正在进入下载bootloader
    GNSS_STATUS_START_DOWNLOAD_BOOTLOADER,     ///开始下载bootloader
    GNSS_STATUS_SEND_BOOTLOADER_DATA,          ///发送BootLoader数据
    GNSS_STATUS_WAIT_BOOTLOADER_DATA_ACK,      ///等待BootLoader的确认
    GNSS_STATUS_WAIT_BOOTLOADER_LAST_ACK,      ///等待最后一个ACK
    GNSS_STATUS_WAIT_BOOTLOADER_RUN,           ///等待BOOTLOADER启动
    GNSS_STATUS_BOOTLOADER_FINISH,             ///bootloader下载完成
    GNSS_STATUS_ENTERGING_DOWNLOAD_FIRMWARE,   ///正在进入下载firmware
    GNSS_STATUS_START_DOWNLOAD_FIRMWARE,       ///开始下载firmware
    GNSS_STATUS_SEND_FIRMWARE_DATA,            ///发送firmware数据
    GNSS_STATUS_WAIT_FIRMWARE_DATA_ACK,        ///等待firmware的确认
    GNSS_STATUS_WAIT_FIRMWARE_LAST_ACK,        ///等待最后一个ACK
    GNSS_STATUS_FIRMWARE_FINISH,               ///firmware下载完成
    GNSS_STATUS_RESET_ING,                     ///RESET gnss
    GNSS_STATUS_INJECT_AGPS_ING,
} gnss_status_t;

typedef enum gnss_start_mode_s
{
    GNSS_START_NONE = 0,    ///关闭gnss
    GNSS_START_COLD,        ///冷启动
    GNSS_START_HOT,         ///热启动
}gnss_start_mode_t;
    
typedef enum gnss_satellite_e
{
// Reference Fibocom_AT命令手册_GNSS_V2.2.0
    GNSS_GPS_GLONASS = 0,
    GNSS_GPS_GALILEO,
    GNSS_GPS_QZSS,
    GNSS_GPS_BEIDOU_GALILEO,
    GNSS_GPS_BEIDOU_GLONASS,
    GNSS_GPS_BEIDOU_QZSS,  //5
    GNSS_GPS_GLONASS_GALILEO,
    GNSS_GPS_GALILEO_QZSS,
    GNSS_GPS_GLONASS_QZSS,
    GNSS_GPS_BEIDOU_GALILEO_GLONASS,
    GNSS_GPS_BEIDOU_GLONASS_QZSS, //10
    GNSS_GPS_BEIDOU_GALILEO_QZSS,
    GNSS_GPS_GALILEO_GLONASS_QZSS,
    GNSS_GPS_BEIDOU_GALILEO_GLONASS_QZSS,
    GNSS_GPS,
// Unisoc8850 have these
    GNSS_BEIDOU, //15
    GNSS_GPS_BDS,
    GNSS_GLONASS,
    GNSS_GALILEO,

    GNSS_BEIDOU_GALILEO,
    GNSS_GLONASS_GALILEO, //20

    GNSS_B1C,
    GNSS_B1C_GALILEO,
    GNSS_GPS_B1C,
    GNSS_GPS_B1C_GLONASS,
    GNSS_GPS_B1C_GALILEO,  //25
    GNSS_GPS_B1C_GLONASS_GALILEO,

    GNSS_NONE = 0xFF
} gnss_satellite_t;

typedef enum
{
    FFW_GNSS_UPGRADE_RESULT = 0,
    FFW_GNSS_UPGRADE_PROCESS
} ffw_gnss_upgrade_evt_t;

typedef void (*ffw_gnss_upgrade_callback_t)(ffw_gnss_upgrade_evt_t evt, int data, char *version, void *arg);

/**
 * @brief GNSS vendor define
 * 
 */
typedef struct ffw_gnss_vendor_s
{
    /// vendor id
    uint8_t vendor_id;

    /// 供应商
    const char *vendor;

    /// 获取固件版本的命令
    uint8_t cmd[64];

    /// 命令长度
    int cmdlen;

    /// 开启GNSS,类似上电
    int (*start)(uint8_t mode);

    /// 关闭GNSS，类似下电
    int (*stop)(void);

    /// 给GNSS发送命令
    int (*send_cmd)(const void *cmd, int len);

    /// 解析版本号函数，返回true表示解析成功，false表示失败
    bool (*parse_version)(uint8_t *data, int datalen, char **version);

    /// 开始升级固件
    int (*upgrade_firmware)(uint8_t uart_port, const char *bootloaderfile, const char *firewarefile, uint32_t firewarelen, ffw_gnss_upgrade_callback_t cb, void *arg);

    /// 进入下载bootloader模式
    bool (*check_bootloader_download_mode)(uint8_t *data, int datalen);

    /// 检查数据的ACK
    bool (*check_data_ack)(uint8_t *data, int datalen);

    /// BOOTLOADER是否启动
    bool (*check_bootloader_run)(uint8_t *data, int datalen);

    /// 是否进入固件下载模式
    bool (*check_firmware_download_mode)(uint8_t *data, int datalen);

    /// 检查AGPS的ACK
    bool (*check_agps_ack)(uint8_t *data, int datalen);

    /// 获取增量观测数据
    int (*get_rtcm)(uint8_t *data, int len, int *start, int *rtcm_len);

    /// 解析下载的agps数据，并注入到gnss中
    int (*parse_agps_data_chunk)(const uint8_t *data, int len, int (*cb)(const void *data, int len, void *arg), void *arg);

    ///是否使用
    bool enable;

    /// 设置卫星命令
    int (*set_satellite)(gnss_satellite_t satellite);

    /// 获取卫星设置
    int (*get_satellite)();

    /// 设置上报频率命令
    int (* set_freq)(int freq);

    ///  设置AGNSS命令;
    int (* set_agnss)(int mode, char *host, int host_len, int port, uint8_t srv_type);

    ///  获取AGNSS设置模式和GNSS启动模式，后续需要获取其他GNSS信息的可以在这个接口添加
    int (* get_agnss)(uint8_t *agnss_mode, uint8_t *gnss_start_mode, char **host, int *port, uint8_t *srv_type);

} ffw_gnss_vendor_t;

/// 上报NMEA语句函数类型
typedef void (*nmea_report_t)(void *arg);

/// rtk发送命令的回调函数
typedef void (*rtk_send_t)(uint8_t *buf, int len);

/// 上报NMEA语句回调函数
typedef void (*rtk_recv_t)(const uint8_t *buf, int len);

/// 上报rtk状态的回调函数
typedef void (*rtk_status_t)(int status);

/**
* @brief 
* 
* @return int 
 */
int ffw_gnss_init();

/**
* @brief 
* 
* @param data 
* @param len 
* @param send_cb 
* @param arg 
* @return int 
 */
int ffw_gnss_inject_agps_data(const uint8_t *data, int len);

/**
* @brief 
* 
* @param data 
* @param len 
* @return int 
 */
int ffw_gnss_check_ack(uint8_t *data, int len);

/**
* @brief 
* 
* @param url 
* @return int 
 */
int ffw_gnss_set_agps_url(const char *url, const char *name, const char *password);

/**
* @brief 
* 
* @param url 
* @param name 
* @param password 
* @return int 
 */
int ffw_gnss_get_agps_url(const char **url, const char **name, const char **password);

/**
* @brief 
* 
* @param on 
* @return int 
 */
int ffw_gnss_power(uint8_t on);

/**
* @brief 
* 
* @return int 
 */
int ffw_gnss_power_on();

/**
 * @brief 检查是否开始下载GNSS firmware
 * 
 * @param data gnss上报的数据
 * @param datalen 数据长度
 * @return true 
 * @return false 
 */
bool ffw_gnss_check_down_firmware(uint8_t *data, int datalen);

/**
 * @brief 检查bootloader是否启动
 * 
 * @param data gnss上报的数据
 * @param datalen 数据长度
 * @return true 
 * @return false 
 */
bool ffw_gnss_check_bootloader_run(uint8_t *data, int datalen);

/**
 * @brief 检查数据的ACK
 * 
 * @param data gnss上报的数据
 * @param datalen 数据长度
 * @return true 
 * @return false 
 */
bool ffw_gnss_check_data_ack(uint8_t *data, int datalen);

/**
 * @brief 检查是否进入下载bootloader状态
 * 
 * @param data gnss上报的数据
 * @param datalen 数据长度
 * @return true 进入下载bootloader模式
 * @return false 未进入下载bootloader模式
 */
bool ffw_gnss_check_bootloader_download(uint8_t *data, int datalen);

/**
 * @brief 尝试获取GNSS固件的版本号
 * 
 * @param data 上报的数据
 * @param datalen 数据长度
 * @param[out] version 版本号
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_get_version(uint8_t *data, int datalen, char **version);

/**
 * @brief 等待GNSS状态变化
 * 
 * @param timeout 
 * @param target_status 
 * @return true 
 * @return false 
 */
bool ffw_gnss_wait_status(int timeout, gnss_status_t target_status);

/**
 * @brief 设置GNSS的状态
 * 
 * @param status 状态
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_set_status(gnss_status_t status);

/**
 * @brief 获取GNSS的状态
 * 
 * @return gnss_status_t GNSS状态
 */
gnss_status_t ffw_gnss_get_status();

/**
 * @brief 获取所有NMEA语句
 * 
 * @param[out] data 输出数据的缓冲区 
 * @param len @c data的长度
 * @param len_out 输出数据的长度
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_get_nmea(uint8_t *data, int len, int *len_out);

/**
 * @brief 获取GNSS上报的定位消息中指定NMEA格式的所有对应NMEA格式的语句
 * 
 * @param nmea_name NMEA语句种类
 * @param[out] data 输出数据的缓冲区 
 * @param len @c data的长度
 * @param len_out 输出数据的长度
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_get_nmea_byname(const char *nmea_name, uint8_t *data, int len, int *len_out);

/**
 * @brief 把GNSS上报的数据发送给GNSS数据处理模块
 * 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_gnss_push_data(uint8_t *data, uint16_t len);

/**
 * @brief 升级gnss固件
 * 
 * @param uart_port 串口号
 * @param bootloaderfile bootloader的文件 
 * @param firewarefile 固件的文件
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_upgrade_firmware(uint8_t uart_port, const char *bootloaderfile, const char *firewarefile, uint32_t fireware_len);

/**
* @brief 
* 
* @param bootloaderfile 
* @param firewarefile 
* @param fireware_len 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_gnss_upgrade_firmware_async(const char *bootloaderfile, const char *firewarefile, uint32_t fireware_len, ffw_gnss_upgrade_callback_t cb, void *arg);

/**
* @brief 解析AGPS的数据，并用回调函数返回
*
* 
* @param data 
* @param len 
* @param send_cb 
* @param arg 
* @return int 
 */
int ffw_gnss_agps_data(uint8_t *data, int len, int (*cb)(void *data, int len, void *arg), void *arg);

/**
 * @brief 查找rtcm
 * 
 * @param data 数据
 * @param len 数据长度
 * @param start rtcm 起始下标
 * @param rtcm_len rtcm长度
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_get_rtcm(uint8_t *data, int len, int *start, int *rtcm_len);

/**
 * @brief 
 * 
 * @param nmea_names 
 * @param on 
 * @return int 
 */
int ffw_gnss_nmea_config(char *nmea_names, bool on);

/**
* @brief 
* 
* @param buf 
* @param buf_size 
* @return int 
 */
int ffw_gnss_nmea_get_config(char *buf, int32_t buf_size);

/**
 * @brief 获取GNSS固件版本好
 * 
 * @param version 版本号
 * @param len @c version 的长度
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_version(char *version, int len);

/**
 * @brief 设置GNSS卫星信息
 * 
 * @param satellite 卫星类型
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_satellite(gnss_satellite_t satellite);

/**
 * @brief 设置GNSS卫星信息
 * 
 * @return gnss_satellite_t 
 */
gnss_satellite_t ffw_gnss_get_satellite();
/**
 * @brief 设置GNSS上报频率
 * 
 * @param freq 卫星上报频率
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_frequency(int freq);

/**
 * @brief 初始化RTK
 * 
 * @param send 发送命令回调函数
 * @param recv 接受NMEA回调函数
 * @param status_noitfy 上报状态的函数
 * @param ... 不确定参数
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_rtk_init(rtk_send_t send, rtk_recv_t recv, rtk_status_t status_noitfy, ...);

/**
 * @brief 启动rtk
 * 
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_rtk_start();

/**
 * @brief 停止rtk
 * 
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_rtk_stop();

/**
 * @brief 注入rtk数据
 * 
 * @param data 数据
 * @param len 长度
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_rtk_inject_data(uint8_t *data, int len);

/**
 * @brief rtk上报的最后一个NMEA
 * 
 * @return const char* 
 */
const char *ffw_rtk_last_nmea();

/**
 * @brief 
 * 
 * @return ffw_gnss_vendor_t* 
 */
ffw_gnss_vendor_t *ffw_get_gnss_vendor();

/**
 * @brief 设置AGNSS的模式
 * 
 * @param mode AGNSS的模式，0:默认关闭AGNSS，1:打开AGNSS
 * @param host AGNSS服务器地址
 * @param host_len AGNSS服务器地址长度
 * @param port AGNSS服务器的端口
 * @pram srv_type: AGNSS服务器类型，0：HTTP，1：FTP
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_set_agnss(int mode, char *host, int host_len, int port, uint8_t srv_type);

/**
 * @brief 获取GNSS的模式, agnss的模式和gnss启动模式
 * 
 * @param [out] agnss_mode AGNSS的模式
 * @param [out] gnss_start_mode gnss启动模式
 * @param [out] host 设置的AGNSS服务器地址
 * @param [out] port AGNSS服务器端口号
 * @param [out] srv_type AGNSS服务器类型，HTTP还是FTP服务器
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
int ffw_gnss_get_agnss(uint8_t *agnss_mode, uint8_t *gnss_start_mode, char **host, int *port,  uint8_t *srv_type);

/**
 * @brief 清除nmea语句
 * 
 * @return int 
 * @retval FFW_R_SUCCESS 成功
 * @retval FFW_R_FAILED 失败
 */
void ffw_gnss_clear_nmea(void);

int ffw_gnss_cmd_send(const void *cmd, int len);

/**
 * @brief 获取当前的GNSS厂商
 * 
 * @return void * 
 * @retval GNSS厂商指针
 */
const void *ffw_gnss_get_current_vendor(void);


#endif /* LIB_PLATEFORM_HD_GNSS_INCLUDE_HD_GNSS */
