#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_
#include "fibo_opencpu.h"

typedef enum
{
    HTTP_DEFAULT_STA        =  0xff,    //初始化状态
    HTTP_DOWLOAD_SUCCESS    =  0,       //文件下载成功
    HTTP_DOWLOAD_FAIL       = -1,       //文件下载失败
    HTTP_POST_FAIL          = -2,       // post 连接返回失败
    HTTP_FIND_FILE          = -3,       //找到文件路径
    HTTP_NO_FIND_FILE       = -4,       //未找到文件路径
}HTTP_STATUS_TYPE_;


/**
 * @brief http_download_file()  http文件下载
 *
 * @param[in] url                   : http下载地址.
 * @param[in] local_file_path       ：保存本地路径
 * @param[out] download_file_size   ：下载文件大小
 *
 * @return  0：success.
 *          -1:queue full.
 */

int http_download_file(char *url,char *local_file_path,int *download_file_size);

void agnss_http_enter(void);
int  gps_data_update(void);

/**
 * @brief get_agnss_http_status()  获取agnss下载状态
 *
 * @param[in] none.
 * 
 * @return  0：success.
 *         -1: fail.
 *       具体查看 HTTP_STATUS_TYPE_ 类型
 */
HTTP_STATUS_TYPE_  get_agnss_http_status(void);

/**
 * @brief set_agnss_http_status()  设置agnss下载状态
 *
 * @param[in] 具体查看 HTTP_STATUS_TYPE_ 类型.
 * 
 * @return  none.
 */
void set_agnss_http_status(HTTP_STATUS_TYPE_ agnss_sta);

/**
 * @brief agnss_http_task()  agnss下载任务 下载完成后会输入星历
 *
 * @param[in] param.
 * 
 * @return  none.
 */
void agnss_http_task(void *param);

#endif /* _HTTP_SERVICE_H_*/