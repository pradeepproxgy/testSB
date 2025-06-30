#ifndef __MTP_H__
#define __MTP_H__

#include "libmtp.h"

#ifdef OSI_LOCAL_LOG_TAG
#undef OSI_LOCAL_LOG_TAG
#endif
#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('M', 'T', 'P', 'H')

#define MTP_LOG_LEN 256

#ifdef OSI_LOCAL_LOG_LEVEL
#undef OSI_LOCAL_LOG_LEVEL
#endif
#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_VERBOSE

static inline void MTP_LOG(int level, const char *fmt, ...)
{
    if (OSI_LOCAL_LOG_LEVEL < level)
        return;

    char str[MTP_LOG_LEN] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(str, MTP_LOG_LEN, fmt, ap);
    va_end(ap);

    //OSI_LOGXE(OSI_LOGPAR_S, 0, "MTP: %s",str);
    __OSI_LOGX(level, OSI_LOGPAR_S, 0, "MTP: %s", str);
}

#define MTP_LOGE(fmt, ...)                                          \
    do                                                              \
    {                                                               \
        MTP_LOG(OSI_LOG_LEVEL_ERROR, "ERROR: " fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_LOGW(fmt, ...)                                        \
    do                                                            \
    {                                                             \
        MTP_LOG(OSI_LOG_LEVEL_WARN, "WARN: " fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_LOGI(fmt, ...)                               \
    do                                                   \
    {                                                    \
        MTP_LOG(OSI_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_LOGD(fmt, ...)                                \
    do                                                    \
    {                                                     \
        MTP_LOG(OSI_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_LOGV(fmt, ...)                                  \
    do                                                      \
    {                                                       \
        MTP_LOG(OSI_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_DEV_LOGI(fmt, ...)                                          \
    do                                                                  \
    {                                                                   \
        MTP_LOG(OSI_LOG_LEVEL_INFO, "device info:" fmt, ##__VA_ARGS__); \
    } while (0)

#define MTP_HOST_THREAD_STACK_SIZE (4 * 1024)

typedef enum
{
    MTP_HOST_SUCCESS = 0,
    MTP_HOST_MSG_ERROR = 0x101,
    MTP_HOST_INVALID_THREAD_SRC,

} MTP_HOST_ERRORNUM;

typedef enum
{
    MTP_NONE = 0,
    MTP_ROOT_DIR,
    MTP_DEVICE_DIR,
    MTP_STORAGE_DIR,
    MTP_FOLDER_DIR,

} MTP_OPERATION_DIR_TYPE;

typedef struct
{
    LIBMTP_mtpdevice_t *device;
    LIBMTP_devicestorage_t *storage;
    MTP_OPERATION_DIR_TYPE current_dir_type; //dir depth
    u32 current_dir_id;

} MTP_OPERATION;

typedef struct
{
    MTP_OPERATION operation;
    UX_THREAD *mtp_host_thread;
    struct list_head device_list; //mtp device list head
} MTP_HOST;

typedef enum
{
    MTP_HOST_IDLE = 0,
    MTP_HOST_START,
    MTP_HOST_STOP,
    MTP_EVENT_PROCESS,
    MTP_GET_FRIENDLY_NAME,
    MTP_SET_FRIENDLY_NAME,
    MTP_OPEN_DIR,
    MTP_CLOSE_DIR,
    MTP_GET_FILE,
    MTP_SEND_FILE,
    MTP_CREATE_FOLDER,
    MTP_FILE_RENAME,
    MTP_DELETE_FILE,
    MTP_READ_FILE,
} MTP_HOST_MSG_TYPE;

typedef struct
{
    MTP_HOST_MSG_TYPE msg_type;
    UX_THREAD *thread_src; //which thread message send from
    void *ctx;
} MTP_HOST_MSG;

typedef struct
{
    void *engine;
    char data[0];
} MTP_AT_MSG;

void mtp_init(void);
UINT mtp_start(void *para);
UINT mtp_stop(void *para);
UINT mtp_start_process(void *para);
UINT mtp_stop_process(void *para);
UINT mtp_host_msg_send(UX_THREAD *thread_dst, UX_CALLBACK callback, MTP_HOST_MSG_TYPE type,
                       bool thread_src, VOID *ctx);
void mtp_host_callback(void *ctx);
UINT mtp_event_process(void *para);
MTP_HOST *mtp_host_info_get(void);
void friendly_name_handler(int state, MTP_HOST_MSG *msg);
void open_directory_handler(MTP_HOST_MSG *msg);
void get_file_from_device_handler(MTP_HOST_MSG *msg);
void send_file_to_device_handler(MTP_HOST_MSG *msg);
void create_new_folder_handler(MTP_HOST_MSG *msg);
void file_rename_handler(MTP_HOST_MSG *msg);
void delete_file_handler(MTP_HOST_MSG *msg);
void close_directory_handler(MTP_HOST_MSG *msg);
void read_file_handler(MTP_HOST_MSG *msg);

#endif
