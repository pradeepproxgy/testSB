/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fibo_opencpu.h"
#include "oc_ffs.h"

#define THREAD_STACK_SIZE 16 * 1024


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('F', 'F', 'S', ' ')
#ifndef FIBO_FFS_DIR
#define FIBO_FFS_DIR CONFIG_FS_SYS_MOUNT_POINT "FFS"
#endif

#define st_test
#define send_recv_test

#define demo_filepath CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest1"
#define demo_2filepath CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest2"
#define demo_filename CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest1" "/testfile.txt"
#define demo_2filename CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest2" "/testfile.txt"
#define demo_1filerename CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest1" "/renamefile1.txt"
#define demo_2filerename CONFIG_FS_SYS_MOUNT_POINT "/fsdemotest1" "/renamefile2.txt"

#define demo_non_error       0b00000000
#define demo_sys_err         0b00000001
#define demo_dir_err         0b00000010
#define demo_file_err        0b00000100
#define demo_comp_err        0b00001000
#define demo_sys_unfinished  0b00010000
#define demo_dir_unfinished  0b00100000
#define demo_file_unfinished 0b01000000
#define demo_comp_unfinished 0b10000000

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void ffs_thread_entry(void *param)
{
    fibo_taskSleep(2000);

    fibo_textTrace("application thread enter, param 0x%x", param);

    unsigned char err = demo_non_error;
    INT32 ret = -1;

#ifdef send_recv_test
    bool send_recv_flag = true;
    unsigned char *demo_buffer_send = (unsigned char *)malloc(10 * sizeof(unsigned char));
    unsigned char *demo_buffer_recv = (unsigned char *)malloc(10 * sizeof(unsigned char));

    do
    {
        if (NULL != demo_buffer_send)
        {
            memset(demo_buffer_send, 0xFF, 10);
        }
        else
        {
            send_recv_flag = false;
            fibo_textTrace("Parameter demo_buffer_send init failed.\r\n");
            break;
        }
        if (NULL != demo_buffer_recv)
        {
            memset(demo_buffer_recv, 0x0, 10);
        }
        else
        {
            send_recv_flag = false;
            fibo_textTrace("Parameter demo_buffer_recv init failed.\r\n");
            break;
        }
    }while(0);
#endif

#ifdef st_test
    bool statcmp = true;
    struct fibo_stat *st1 = (struct fibo_stat *)malloc(sizeof(fibo_stat));
    struct fibo_stat *st2 = (struct fibo_stat *)malloc(sizeof(fibo_stat));
#endif

    INT32 demo_filesize = -1;
    fibo_textTrace("\r\n");
    fibo_textTrace("==================================================\r\n");
    fibo_textTrace("\r\n");
    fibo_textTrace("Following test opencpu: FFS\r\n");
    fibo_textTrace("\r\n");
    fibo_textTrace("=========== 1/4, File System Apis Test ===========\r\n");
    fibo_textTrace("\r\n");

    do
    {
        if (-1 != (ret = fibo_file_get_total_size()))
        {
            fibo_textTrace("File system total size: %d\r\n", ret);
        }
        else
        {
            fibo_textTrace("Get total size failed.\r\n");
            err |= demo_sys_err;
            err |= demo_sys_unfinished;
            break;
        }
        if (-1 != (ret = fibo_file_get_free_size()))
        {
            fibo_textTrace("File system free size: %d\r\n", ret);
        }
        else
        {
            fibo_textTrace("Get free size failed.\r\n");
            err |= demo_sys_err;
            break;
        }
    }while(0);

    fibo_textTrace("\r\n");
    fibo_textTrace("========= 1/4, File System Apis Finished =========\r\n");
    fibo_taskSleep(100);
    fibo_textTrace("\r\n");
    fibo_textTrace("============ 2/4, Directory Apis Test ============\r\n");
    fibo_textTrace("\r\n");
    do
    {
        fibo_textTrace("Following mkdir1...\r\n");
        if (1 == (fibo_file_dir_exist(demo_filepath)))
        {
            fibo_textTrace("Dir exist, re-mkdir1...\r\n");
            if (-1 == (fibo_file_rmdir(demo_filepath)))
            {
                fibo_textTrace("Exec rmdir1 failed!(1/2)\r\n");
                fibo_textTrace("Will delete exist file and retry...\r\n");
                fibo_file_delete(demo_filename);
                fibo_file_delete(demo_1filerename);
                fibo_file_delete(demo_2filerename);
                if (-1 == (fibo_file_rmdir(demo_filepath)))
                {
                    fibo_textTrace("Exec rmdir1 failed!(2/2)\r\n");
                    err |= demo_dir_err;
                    err |= demo_dir_unfinished;
                    break;
                }

            }
        }
        if (-1 == (fibo_file_mkdir(demo_filepath)))
        {
            fibo_textTrace("Exec mkdir1 failed!\r\n");
            err |= demo_dir_err;
            err |= demo_dir_unfinished;
            break;
        }
        else
        {
            fibo_textTrace("Exec mkdir1 success!\r\n");
        }
        fibo_textTrace("Following mkdir2...\r\n");
        if (1 == (fibo_file_dir_exist(demo_2filepath)))
        {
            fibo_textTrace("Dir exist, re-mkdir2...\r\n");
            if (-1 == (fibo_file_rmdir(demo_2filepath)))
            {
                fibo_textTrace("Exec rmdir2 failed!(1/2)\r\n");
                fibo_textTrace("Will delete exist file and retry...\r\n");
                fibo_file_delete(demo_2filename);
                if (-1 == (fibo_file_rmdir(demo_2filepath)))
                {
                    fibo_textTrace("Exec rmdir2 failed!(2/2)\r\n");
                    err |= demo_dir_err;
                    err |= demo_dir_unfinished;
                    break;
                }

            }
        }
        if (-1 == (fibo_file_mkdir(demo_2filepath)))
        {
            fibo_textTrace("Exec mkdir2 failed!\r\n");
            err |= demo_dir_err;
            err |= demo_dir_unfinished;
            break;
        }
        else
        {
            fibo_textTrace("Exec mkdir2 success!\r\n");
        }

        void *ffs_p;
        if (NULL ==(ffs_p = fibo_file_opendir(demo_filepath)))
        {
            fibo_textTrace("Exec opendir failed.\r\n");
            err |= demo_dir_err;
            err |= demo_dir_unfinished;
            break;
        }
        fibo_textTrace("Exec opendir success.\r\n");
        if (-1 == (ret = fibo_file_open(demo_filename, (O_CREAT|O_RDWR))))
        {
            fibo_textTrace("Create file failed.\r\n");
            err |= demo_dir_err;
        }
        else
        {
            fibo_textTrace("Create file success.\r\n");
            fibo_file_close(ret);

            if (NULL == fibo_file_readdir(ffs_p))
            {
                fibo_textTrace("Exec readdir failed.\r\n");
                err |= demo_dir_err;
            }
            else
            {
                fibo_textTrace("Exec readdir success.\r\n");
            }

            if (-1 == (ret = fibo_file_telldir(ffs_p)))
            {
                fibo_textTrace("Exec telldir failed.\r\n");
                fibo_textTrace("Will not check seekdir.\r\n");
                err |= demo_dir_err;
                err |= demo_dir_unfinished;
            }
            else
            {
                fibo_textTrace("Exec telldir success.\r\n");
                fibo_file_seekdir(ffs_p, ret);
            }
        }
        fibo_file_delete(demo_filename);
        fibo_file_closedir(ffs_p);
        fibo_textTrace("Exec closedir finished.\r\n");
        if (1 == fibo_file_dir_exist(demo_filepath))
        {
            fibo_textTrace("Confirm file exist.\r\n");
        }
        else
        {
            fibo_textTrace("Dir exist stat error.\r\n");
            err |= demo_dir_err;
        }
    }while(0);
    fibo_textTrace("\r\n");
    fibo_textTrace("========== 2/4, directory apis finished ==========\r\n");
    fibo_taskSleep(100);
    fibo_textTrace("\r\n");
    fibo_textTrace("============== 3/4, file apis test ===============\r\n");
    fibo_textTrace("\r\n");
    do
    {

        fibo_textTrace("Following create file...\r\n");
        if (1 == fibo_file_exist(demo_filename))
        {
            fibo_textTrace("File exist, will delete file.\r\n");
            if (!fibo_file_delete(demo_filename))
            {
                fibo_textTrace("Delete file failed.\r\n");
                err |= demo_file_err;
                err |= demo_file_unfinished;
                break;
            }
        }
        INT32 fd = -1;
        if (0 > (fd = fibo_file_open(demo_filename,(O_CREAT|O_RDWR))))
        {
            fibo_textTrace("File open failed, fd = %d\r\n", fd);
            err |= demo_file_err;
            err |= demo_file_unfinished;
            break;
        }
        fibo_textTrace("File open success, fd = %d\r\n", fd);

#ifdef send_recv_test
        if (send_recv_flag)
        {
            if (0 >= (ret = fibo_file_write(fd, demo_buffer_send, 10)))
            {
                fibo_textTrace("Write file failed.\r\n");
                err |= demo_file_err;
                err |= demo_file_unfinished;
                break;
            }
        }
        else
        {
            fibo_textTrace("Init err, will not check write.\r\n");
        }
#endif

        fibo_file_fsync(fd);
        fibo_textTrace("File synced.\r\n");
        if (-1 == fibo_file_seek(fd, 0, SEEK_SET))
        {
            fibo_textTrace("Seek file failed.\r\n");
            err |= demo_file_err;
            err |= demo_file_unfinished;
        }
#ifdef send_recv_test
        else if (send_recv_flag)
        {
            fibo_textTrace("Seek file success, exec file read.\r\n");
            ret = fibo_file_read(fd, demo_buffer_recv, 10);
            if (ret >= 0)
            {
                fibo_textTrace("Read %d byte(s), contains: %s.\r\n", ret, demo_buffer_recv);
            }
            else
            {
                fibo_textTrace("Read file failed.\r\n");
                err |= demo_file_err;
            }
        }
        else
        {
            fibo_textTrace("Init err, will not check read.\r\n");
        }
#else
        else
        {
            fibo_textTrace("Seek file success.\r\n");
        }
#endif

#ifdef st_test
        if (-1 == fibo_file_fstat(fd, st1))
        {
            fibo_textTrace("Get file fstat failed.\r\n");
            statcmp = false;
            err |= demo_file_err;
        }
        else
        {
            fibo_textTrace("Get file fstat success,size = %d.\r\n",st1->st_size);
        }
#endif
        ret = fibo_file_feof(fd);
        fibo_textTrace("Current feof: %d.\r\n", ret);
        ret = fibo_file_ftell(fd);
        fibo_textTrace("Current ftell: %d.\r\n", ret);
        if(-1 == (fibo_file_close(fd)))
        {
            fibo_textTrace("Close file failed.\r\n");
            err |= demo_file_err;
        }
        else
        {
            fibo_textTrace("Close file success.\r\n");
        }

#ifdef st_test
        if (-1 == fibo_file_stat(demo_filename, st2))
        {
            fibo_textTrace("Get file stat failed.\r\n");
            statcmp = false;
            err |= demo_file_err;
        }
        else
        {
            fibo_textTrace("Get file stat success.\r\n");
        }
#endif
        if (-1 == (demo_filesize = fibo_file_get_size(demo_filename)))
        {
            fibo_textTrace("Get file size failed.\r\n");
            err |= demo_file_err;
        }
        else
        {
            fibo_textTrace("Get file size success, file size: %d.\r\n", demo_filesize);
        }

        fibo_file_close(fibo_file_open(demo_1filerename, (O_CREAT|O_RDWR)));
        ret = (fibo_file_rename(demo_1filerename, demo_2filerename));
        if (ret == -1)
        {
            fibo_textTrace("Rename file failed.\r\n");
            err |= demo_file_err;
            if (-1 == fibo_file_delete(demo_1filerename))
            {
                fibo_textTrace("Delete file failed.\r\n");
            }
            else
            {
                fibo_textTrace("Delete file success.\r\n");
            }
        }
        else
        {
            fibo_textTrace("Rename file success.\r\n");
            if (-1 == fibo_file_delete(demo_2filerename))
                {
                    fibo_textTrace("Delete file failed.\r\n");
                    err |= demo_file_err;
                }
            else
            {
                fibo_textTrace("Delete file success.\r\n");
            }
        }
    }while(0);
    fibo_textTrace("\r\n");
    fibo_textTrace("============ 3/4, File Apis Finished =============\r\n");
    fibo_taskSleep(100);
    fibo_textTrace("\r\n");
    fibo_textTrace("============ 4/4, Comprehensive Test =============\r\n");
    fibo_textTrace("\r\n");

    do
    {
#ifdef st_test
        if (statcmp)
        {
            ret = memcmp(st1, st2, sizeof(stat));
            if (0 != ret)
            {
                fibo_textTrace("Stat compare: stat and fstat values are not match.\r\n");
                err |= demo_comp_err;
            }
            else
            {
                fibo_textTrace("Stat compare: stat and fstat values matched.\r\n");
            }
            if (-1 != demo_filesize)
            {
                ret = (st1->st_size == demo_filesize ? 1 : 0);
                if (ret)
                {
                    fibo_textTrace("Stat compare: fstat and size values matched.\r\n");
                }
                else
                {
                    fibo_textTrace("Stat compare: fstat and size values are not match.\r\n");
                    err |= demo_comp_err;
                }
                ret = (st2->st_size == demo_filesize ? 1 : 0);
                if (ret)
                {
                    fibo_textTrace("Stat compare: stat and size values matched.\r\n");
                }
                else
                {
                    fibo_textTrace("Stat compare: stat and size values are not match.\r\n");
                    err |= demo_comp_err;
                }
            }
            else
            {
                fibo_textTrace("There occurred error(s) in get_size, statcmp will not be checked.\r\n");
            }
        }
        else
        {
            fibo_textTrace("There occurred error(s) in stat/fstat, statcmp will not be checked.\r\n");
            err |= demo_comp_err;
            err |= demo_comp_unfinished;
        }
#endif

#ifdef send_recv_test
        if (0 != memcmp(demo_buffer_recv, demo_buffer_send, 10))
        {
            fibo_textTrace("Test write/read file not matched.\r\n");
            err |= demo_comp_err;
        }
        else
        {
            fibo_textTrace("Test write/read file matched.\r\n");
        }
#endif
    }while(0);

    fibo_textTrace("\r\n");
    fibo_textTrace("========== 4/4, Comprehensive Finished ===========\r\n");
    fibo_taskSleep(100);
    fibo_textTrace("\r\n");
    fibo_textTrace("================ Result Exporting ================\r\n");
    fibo_textTrace("\r\n");
    fibo_textTrace("Demo status:0x%x\r\n", err);
    fibo_textTrace("\r\n");

    if (demo_sys_err == (err & demo_sys_err))
    {
        fibo_textTrace("Sys part contains err.\r\n");
    }
    else
    {
        fibo_textTrace("Sys part completed.\r\n");
    }
    if (demo_dir_err == (err & demo_dir_err))
    {
        fibo_textTrace("Dir part contains err.\r\n");
    }
    else
    {
        fibo_textTrace("Dir part completed.\r\n");
    }
    if (demo_file_err == (err & demo_file_err))
    {
        fibo_textTrace("File part contains err.\r\n");
    }
    else
    {
        fibo_textTrace("File part completed.\r\n");
    }
    if (demo_comp_err == (err & demo_comp_err))
    {
        fibo_textTrace("Comp part contains err.\r\n");
    }
    else
    {
        fibo_textTrace("Comp part completed.\r\n");
    }
    if (demo_sys_unfinished == (err & demo_sys_unfinished))
    {
        fibo_textTrace("Sys part is not finished.\r\n");
    }
    else
    {
        fibo_textTrace("Sys part is fully coveraged.\r\n");
    }
    if (demo_dir_unfinished == (err & demo_dir_unfinished))
    {
        fibo_textTrace("Dir part is not finished.\r\n");
    }
    else
    {
        fibo_textTrace("Dir part is fully coveraged.\r\n");
    }
    if (demo_file_unfinished == (err & demo_file_unfinished))
    {
        fibo_textTrace("File part is not finished.\r\n");
    }
    else
    {
        fibo_textTrace("File part is fully coveraged.\r\n");
    }
    if (demo_comp_unfinished == (err & demo_comp_unfinished))
    {
        fibo_textTrace("Comp part is not finished.\r\n");
    }
    else
    {
        fibo_textTrace("Comp part is fully coveraged.\r\n");
    }
    fibo_textTrace("\r\n");
    if (demo_non_error == err)
    {
        fibo_textTrace("Test all done, no error contains.\r\n");
    }
    else
    {
        fibo_textTrace("It would have error(s) in this test, please check.\r\n");
    }
    fibo_textTrace("\r\n");
    fibo_textTrace("============== result exported done ==============\r\n");
    fibo_textTrace("\r\n");
    fibo_textTrace("Following finished opencpu: FFS\r\n");
    fibo_textTrace("\r\n");
    fibo_textTrace("==================================================\r\n");

    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    if (0 == fibo_thread_create(ffs_thread_entry, "demo_thread", THREAD_STACK_SIZE, NULL, OSI_PRIORITY_BELOW_NORMAL))
    {
        fibo_textTrace("Create thread ok.\r\n");
    }
    else
    {
        fibo_textTrace("Create thread failed!\r\n");
    }
    
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

