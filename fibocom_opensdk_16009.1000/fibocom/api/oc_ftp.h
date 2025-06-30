#ifndef __FFW_FTP_OPENCPU_API__
#define __FFW_FTP_OPENCPU_API__

/**
 * @brief 创建ftp客户端
 * 
 * @param server_addr 服务器地址
 * @param username 服务器用户名
 * @param passwd 服务器密码
 * @return INT32 
 * @retval 0 成功
 * @retval -1 失败
 */
INT32 fibo_ftp_open(char *server_addr, char *username, char *passwd);

/**
 * @brief 创建加密的ftp客户端
 * 
 * @param server_addr 服务器地址
 * @param username 服务器用户名
 * @param passwd 服务器密码
 * @return INT32 
 * @retval 0 成功
 * @retval -1 失败
 */
INT32 fibo_ftps_open(char *server_addr, char *username, char *passwd);

/**
 * @brief 上传文件到服务器
 * 
 * @param server_path 服务器文件名称
 * @param local_path 本地文件名称
 * @return INT32
 * @retval 0 成功
 * @retval -1 失败
 */
INT32 fibo_ftp_write(char *server_path, char *local_path);

/**
 * @brief 下载服务器文件
 * 
 * @param server_path 服务器文件名称
 * @param local_path 本地文件路径
 * @return INT32
 * @retval 0 成功
 * @retval -1 失败
 */
INT32 fibo_ftp_read(char *server_path, char *local_path);

/**
 * @brief 关闭ftp客户端
 * 
 * @return INT32
 * @retval 0 成功
 * @retval -1 失败
 */
INT32 fibo_ftp_close();

#endif
