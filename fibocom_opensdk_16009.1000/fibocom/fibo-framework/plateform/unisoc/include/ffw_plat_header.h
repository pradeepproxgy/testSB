#ifndef LIB_PLATEFORM_UNISOC_INCLUDE_FFW_PLAT_HEADER
#define LIB_PLATEFORM_UNISOC_INCLUDE_FFW_PLAT_HEADER

#include <unistd.h>
#include <sys/time.h>
#include <vfs.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <errno.h>
#include "fupdate.h"

/**
 * thread priority
 *
 * The definition is independent of implementation. Though some
 * implementation will use larger value for higher priority and
 * others will use smaller value for highe priority, this enum will
 * use larger value for higher priority.
 *
 * \p OSI_PRIORITY_IDLE and \p OSI_PRIORITY_HISR are reserved, can't
 * be used.
 *
 */
typedef enum
{
    FFW_OSI_PRIORITY_IDLE = 1, // reserved
    FFW_OSI_PRIORITY_LOW = 8,
    FFW_OSI_PRIORITY_BELOW_NORMAL = 16,
    FFW_OSI_PRIORITY_NORMAL = 24,
    FFW_OSI_PRIORITY_ABOVE_NORMAL = 32,
    FFW_OSI_PRIORITY_HIGH = 40,
    FFW_OSI_PRIORITY_REALTIME = 48,
    FFW_OSI_PRIORITY_HISR = 56, // reserved
} ffw_osi_priority_t;

/// 文件分隔符的定义
#define FFW_FS_SEP "/"

/// FFW操作文件的文件基目录
#define FFW_BASE_DIRNAME "/fibocom"


/// select fd set的定义
#define ffw_fd_set fd_set

/// 文件操作句柄类型的定义
typedef int32_t ffw_fs_handle_t;

/// 无效文件句柄的定义
#define FFW_FS_INVALID_HANDLE -1


/**
* @brief 打印日志函数
* 
* @param fmt 格式化字符串
* @param ... 
 */
void ffw_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

int ffw_rat_get_cid_by_context(int context_id);

extern char *strcasestr (const char *, const char *);

#endif /* LIB_PLATEFORM_UNISOC_INCLUDE_FFW_PLAT_HEADER */
