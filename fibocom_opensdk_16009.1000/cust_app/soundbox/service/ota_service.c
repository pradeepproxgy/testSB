#include "ota_service.h"
#include "http_service.h"
#include "stdint.h"

#include "fb_uart.h"
#include "voice_play.h"


int app_ota_flag=0;

#define   MAX_SIZE_FILE_PATH  255


int ext_update_app_firmware(const char *file_name, uint32_t downloaded_size)
{

    int8_t *app_ota_buf=NULL;
    int ret=-1;
    char  bin_file_path[MAX_SIZE_FILE_PATH] = {0};

    // app check
    snprintf(bin_file_path, MAX_SIZE_FILE_PATH, "/tmp/%s", file_name);

    if(fibo_file_exist(bin_file_path)!=1) {
        fb_printf("ota firmware file %s not existed", bin_file_path);
        return -1;
    }

    int32_t filesize=vfs_file_size(bin_file_path);
    if (filesize<=0){
        //fibo_setSleepMode(1);
        return -1;
    }

    app_ota_buf=fibo_malloc(filesize+1);
    if(app_ota_buf==NULL)
    {
        fb_printf("ota firmware fibo_malloc  app_ota_buf fail:%d");
        return -1;
    }

      int fd = vfs_open(bin_file_path, O_RDONLY);
    if (fd <=0){
        fb_printf("ota firmware open file fail !!!");
        return -1;
    }

    unsigned int bytes = vfs_read(fd, app_ota_buf, filesize);

    if (bytes!=filesize){
        fb_printf("ota firmware read file error");
        vfs_close(fd);
        //fibo_setSleepMode(1);
        if (app_ota_buf!=NULL){
            fibo_free(app_ota_buf);
            app_ota_buf=NULL;
        }
        return-1;
    }
    fb_printf("ota firmware file read bytes=%d",bytes);
    vfs_close(fd);
    ret=fibo_app_check((INT8*)app_ota_buf,(UINT32)bytes);
    if(ret==0)
    {
        ret=fibo_ota_handle_no_reboot((INT8*)app_ota_buf,(UINT32)bytes);
        if(ret<0)
        {
            fb_printf("ota firmware ota failed");
            ret = -1;
        }
        else
        {
            ret = 0;
        } 
    }
    else
    {
        ret = -1;
    }

    if (app_ota_buf!=NULL)
    {
        fibo_free(app_ota_buf);
        app_ota_buf = NULL;
    }

    return ret;
}



int ota_service_enter(void)
{
    int ret=0;
    uint32_t downloaded_size=0;

    fibo_taskSleep(1000);
    while(1)
    {
        //1、调用http下载ota文件    循环
        //2、储存在falsh中
        //ret=http_download_file(OTA_HTTP_URL,LOCAL_OTA_FILE_PATH,&downloaded_size);
        if(ret==0)
        {
            //下载完成  读取文件执行ota流程
            ret=ext_update_app_firmware(APP_OTA_FILE_NAME,downloaded_size);
            if(ret==0)
            {
                //升级成功
                //升级成功后重启
                push_audio_play_list("OTA_UPGRADE_SUCCESS_RESTING.mp3");
                fibo_taskSleep(1000*5); //等待提示音播放完成
                fibo_softReset();
            }
            else
            {
                push_audio_play_list("OTA_UPGRADE_FAILED.mp3");
                fibo_taskSleep(1000*3);
               //升级失败
            }
        }
        return ret;
    }
}

