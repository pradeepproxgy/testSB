#ifndef __MASS_STORAGE_H__
#define __MASS_STORAGE_H__

#include "ux_host_class_storage.h"

#define FILEX_THREAD_STACK_SIZE (4 * 1024)
#define MASSSTORAGE_MEDIA_DEVICE_MAX_NUM UX_HOST_CLASS_STORAGE_MAX_MEDIA

typedef enum
{
    MASSSTORAGE_MEDIA_DIR = 0,
    MASSSTORAGE_ROOT_DIR,
    MASSSTORAGE_FOLDER_DIR,

} MASSSTORAGE_OPERATION_DIR_TYPE;

typedef struct
{
    FX_MEDIA *current_media;
    MASSSTORAGE_OPERATION_DIR_TYPE current_dir_type; //dir depth

} MASSSTORAGE_OPERATION;

typedef struct
{
    //void (*open_notify)(UX_MEDIA *media);
    //void (*close_notify)(UX_MEDIA *media);
    int media_num;
    MASSSTORAGE_OPERATION operation;
    UX_THREAD *filex_thread;
    struct list_head media_list; //mass storage device list head
} MEDIA_INFO;

typedef enum
{
    FILE_CREATE,
    //FILE_READ,
    //FILE_WRITE,
} FILEX_MSG_TYPE;

typedef struct
{
    FILEX_MSG_TYPE msg_type;
    UX_THREAD *thread_src; //which thread message send from
    void *ctx;
} FILEX_MSG;
#if 0
typedef struct
{
    void *engine;
    char data[0];
}FILEX_AT_MSG;
#endif

void mass_storage_init(void);
MEDIA_INFO *media_info_get(void);
UINT filex_msg_send(UX_THREAD *thread_dst, UX_CALLBACK callback, FILEX_MSG_TYPE type,
                    bool thread_src, VOID *ctx);

void filex_thread_callback(void *ctx);
#if 0
void create_file_handler(FILEX_MSG *msg);
void read_file_handler(FILEX_MSG *msg);
void write_file_handler(FILEX_MSG *msg);
#endif
int massstorage_media_check(void);
int massstorage_media_is_mounted(FX_MEDIA *media);
FX_MEDIA *massstorage_media_get_by_name(char *media_name);

#endif
