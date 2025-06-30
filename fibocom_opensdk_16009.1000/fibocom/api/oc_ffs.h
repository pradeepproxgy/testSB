#ifndef OPENCPU_API_OC_FFS
#define OPENCPU_API_OC_FFS
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

typedef int INT32;
typedef unsigned char UINT8;
typedef unsigned int UINT32;

// Struct of dirent, def for fibo_file_readdir:
typedef struct fibo_dirent
{
    unsigned char   d_type;     // type of file
    char            d_name[256];// file name
} fibo_dirent;
//..

// Struct of file status, def for fibo_file_stat/fstat:
typedef struct fibo_stat
{
    short           st_dev;     // device num
    unsigned long   st_ino;     // inode
    int             st_mode;    // file mode
    unsigned short  st_nlink;   // nlink num
    unsigned short  st_uid;     // userid
    unsigned short  st_gid;     // user group id
    short           st_rdev;    // rdev num
    long            st_size;    // file size
//    long            st_atime;   // access time
//    long            st_mtime;   // modify time
//    long            st_ctime;   // change time
} fibo_stat;
//..

// Struct of file system status, def for fibo_file_get_free_size/fibo_file_get_total_size:
typedef struct fibo_sysstat
{
    unsigned long f_bsize;   // file system block size
    unsigned long f_frsize;  // fragment size
    unsigned long f_blocks;  // size of fs in f_frsize units
    unsigned long f_bfree;   // free blocks in fs
    unsigned long f_bavail;  // free blocks avail to non-superuser
    unsigned long f_files;   // total file nodes in file system
    unsigned long f_ffree;   // free file nodes in fs
    unsigned long f_favail;  // avail file nodes in fs
    unsigned long f_fsid;    // file system id
    unsigned long f_flag;    // mount flags
    unsigned long f_namemax; // maximum length of filenames
} fibo_sysstat;
//..

// File open flag, def for fibo_file_open:
#ifndef O_RDONLY
#define O_RDONLY    00000000
#endif

#ifndef O_WRONLY
#define O_WRONLY    00000001
#endif

#ifndef O_RDWR
#define O_RDWR      00000002
#endif

#ifndef O_CREAT
#define O_CREAT     00000100
#endif

#ifndef O_TRUNC
#define O_TRUNC     00001000
#endif

#ifndef O_APPEND
#define O_APPEND    00002000
#endif

#ifndef O_EXCL
#define O_EXCL      00000200
#endif
//..

// File seek position, def for fibo_file_seek:
#ifndef SEEK_SET
#define SEEK_SET    0    // set file offset to offset
#endif
#ifndef SEEK_CUR
#define SEEK_CUR    1    // set file offset to current plus offset
#endif
#ifndef SEEK_END
#define SEEK_END    2    // set file offset to EOF plus offset
#endif
//..

// FFS Apis:
/**
 * open and possibly create a file
 *
 * \param [in] path: file path
 * \param [in] flags: open flags
 * \    - <O_RDONLY, O_WRONLY, O_RDWR>[|<O_TRUNC, O_APPEND>][|<O_CREAT>[|<O_EXCL>]]
 * \return
 *      - file descriptor on success
 *      - -1 on error
 */
INT32 fibo_file_open(const char *path, INT32 flags);

/**
 * close a file descriptor
 *
 * \param [in] fd: file descriptor
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_close(INT32 fd);

/**
 * read from a file descriptor
 *
 * \param [in] fd: file descriptor
 * \param [out] buff: buf for read
 * \param [in] size: byte count
 * \return
 *      - the number of bytes read on success
 *      - -1 on error
 */
INT32 fibo_file_read(INT32 fd, UINT8 *buff, UINT32 count);

/**
 * write to a file descriptor
 *
 * \param [in] fd: file descriptor
 * \param [in] buf: buf for write
 * \param [in] size: byte count
 * \return
 *      - the number of bytes written on success
 *      - -1 on error
 */
INT32 fibo_file_write(INT32 fd, UINT8 *buff, UINT32 count);

/**
 * reposition read/write file offset
 *
 * \param [in] fd: file descriptor
 * \param [in] offset: offset according to the directive whence
 * \param [in] opt: SEEK_SET, SEEK_CUR, SEEK_END
 * \return
 *      - offset location as measured in bytes from the beginning of the file on success
 *      - -1 on error
 */
INT32 fibo_file_seek(INT32 fd, INT32 offset, UINT8 opt);

/**
 * change the name or location of a file
 *
 * \param [in] oldpath: old file path
 * \param [in] newpath: new file path
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_rename(const char *oldpath, const char *newpath);

/**
 * delete a directory
 *
 * \param [in] path: directory path name
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_rmdir(const char *path);

/**
 * create a directory
 *
 * \param [in] path: directory path name
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_mkdir(const char *path);

/**
 * to confirm whether the file is exist
 *
 * \param [in] path: file path
 * \return
 *      - 1 on exist
 *      - -1 on non-exist
 */
INT32 fibo_file_exist(const char *path);

/**
 * get the file size
 *
 * \param [in] path: file path
 * \return
 *      - size of file
 *      - -1 on error
 */
INT32 fibo_file_get_size(const char *path);

/**
 * delete the file, only used for file which size is less than 2M.
 *
 * \param [in] path: file path
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_delete(const char *path);

/**
 * truncate the file
 *
 * \param [in] path: file path
 * \param [in] length: file size after truncated
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_truncate(const char *path, int32_t length);

/**
 * get current file system free size
 *
 * \return
 *      - current free space
 *      - -1 on error
 */
INT32 fibo_file_get_free_size(void);

/**
 * add for xiaoniu
 * get the file size
 *
 * \param [in] path: file path
 * \returns
 *      - size of file
 *      - -1 on error
 */
INT32 fibo_file_getSize(const char *path);

/**
 * add for xiaoniu
 * get current file system free size
 *
 * \return
 *      - current free space
 *      - -1 on error
 */
INT32 fibo_file_getFreeSize(void);


/**
 * get current file system total size
 *
 * \return
 *      - current total space
 *      - -1 on error
 */
INT32 fibo_file_get_total_size(void);

/**
 * get file status
 *
 * \param [in] path: file path
 * \param [out] st: file status struct
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_stat(const char *path, struct fibo_stat *st);

/**
 * synchronize file status
 *
 * \param [in] fd: file descriptor
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_fsync(int fd);

/**
 * same as fibo_file_delete
 *
 * \param [in] path: file path
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_unlink(const char *path);

/**
 * open a directory
 *
 * \param [in] path: directory path
 * \return
 *      - pointer of directory on success
 *      - NULL on error
 */
void *fibo_file_opendir(const char *path);

/**
 * read a directory
 *
 * \param [in] *pdir: pointer of directory
 * \return
 *      - fibo_dirent * struct on success
 *      - NULL on error
 */
struct fibo_dirent *fibo_file_readdir(void *pdir);

/**
 * tell a directory
 *
 * \param [in] *pdir: pointer of directory returned by fibo_file_opendir
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_telldir(void *pdir);

/**
 * set the position of the next fibo_file_readdir call in the directory stream
 *
 * \param [in] *pdir: pointer of directory returned by fibo_file_opendir
 * \param [in] loc: location returned by fibo_file_telldir
 */
void fibo_file_seekdir(void *pdir, long loc);

/**
 * get file status
 *
 * \param [in] fd: file descriptor
 * \param [out] st: file status struct
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_fstat(int fd ,struct fibo_stat *st);

/**
 * compare file position with EOF
 *
 * \param [in] fd: file descriptor
 * \return
 *      - 1 on success, current position is not EOF
 *      - 0 on success, current position is EOF
 *      - -1 on error
 */
INT32 fibo_file_feof(int fd);

/**
 * tell the current filepos
 *
 * \param [in] fd: file descriptor
 * \param [out] st: file status struct
 * \return
 *      - offset location as measured in bytes from the beginning of the file on success
 *      - -1 on error
 */
INT32 fibo_file_ftell(int fd);

/**
 * confirm whether the directory is exist
 *
 * \param [in] path: directory path
 * \return
 *      - 0 on exist
 *      - -1 on non-exist
 */
INT32 fibo_file_dir_exist(const char *path);

/**
 * close a directory
 *
 * \param [in] *pdir: pointer of directory returned by fibo_file_opendir
 * \return
 *      - 0 on success
 *      - -1 on error
 */
INT32 fibo_file_closedir(void *pdir);

/**
 * verify a path is a file or directory
 *
 * \param [in] *file_path: file or directory path
 * \return
 *      - 1 on directory
 *      - 0 on file
 *      - -1 on error
 */
INT32 fibo_file_differdir(const char * file_path);

/**
 * get the file list
 *
 * \param [in] *dir: directory path
 * \param [in] cb: callback
 * \param [in] *args: callback context
 * \return
 *      - 1 on directory
 *      - 0 on file
 *      - -1 on error
 */
INT32 fibo_get_file_list(char * dir, void(* cb)(const char * filepath, size_t size, void * arg), void * args);
INT32 fibo_file_unzip(const char * file_path, const char * zip_path);

#if 1 //def fibocom_ffs_customization
INT32 fibo_file_getFreeSize_ex(const char *dir_path);
INT32 fibo_file_getTotalSize_ex(const char *dir_path);
#endif
//..


#endif
