/* Copyright (C) 2024 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
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


#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define FILE_DIR_PATH  "/sftptest"

static INT32 basic_file_handle1 = -1;
char sftppublicremoteaddr[]="117.22.252.78";
char sftpdomainremoteaddr[]="xa.fibocom.com";
INT32 sftpuRemotePort = 30103;
const char sftpusername[] = "ftpuser";
const char sftppassword[] = "test";
static UINT32 g_pdp_active_sem = 0;

#define log(format, ...)  fibo_textTrace("[%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        //fibo_PDPActive  ip address resopnse event
        case GAPP_SIG_PDP_ACTIVE_ADDRESS:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            char *addr = (char *)va_arg(arg, char *);
            if(addr != NULL)
            {
                log("sig_res_callback  cid = %d, addr = %s ", cid, addr);
            }
            else
            {
                log("[%s-%d]", __FUNCTION__, __LINE__);
            }
            va_end(arg);
        }
        break;
        case GAPP_SIG_PDP_ACTIVE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            log("sig_res_callback  cid = %d", cid);
            va_end(arg);
            if(g_pdp_active_sem)
            {
                fibo_sem_signal(g_pdp_active_sem);
                log("[%s-%d]", __FUNCTION__, __LINE__);
            }
        }
        break;

        default:
            break;
    }
}


static void at_res_callback(UINT8 *buf, UINT16 len)
{
    log("FIBO <--%s", buf);
}


void getrandom(char *test,int len){
	int i,j;
	char randch[36]="1234567890qwertyuiopasdfghjklzxcvbnm";
	srand((unsigned)time(NULL));//time(0)
	
	for(i=0;i<len;i++){
		j=1+(int)(34.0*rand()/(RAND_MAX+1.0));
		*(test+i)=randch[j-1];
	}
	*(test+32)='\0';
	return ;
}

UINT8 pdp_active(void)
{
	int result = -1;
    UINT8 ip[50];
    memset(&ip, 0, sizeof(ip));
    reg_info_t reg_info;
    fibo_pdp_profile_t pdp_profile;
    UINT8 cid_status;
    log("[%s-%d]", __FUNCTION__, __LINE__);

    while(1)
    {
        fibo_reg_info_get(&reg_info, 0);
        if(reg_info.nStatus==1)
        {
            log("[%s-%d] register network", __FUNCTION__, __LINE__);
            break;
        }
        fibo_taskSleep(1000);
    }
    memset(&pdp_profile, 0, sizeof(fibo_pdp_profile_t));
    pdp_profile.cid = 1;
    result = fibo_pdp_active(1, &pdp_profile, 0);
	if(result != 0)
	{
		log("[%s-%d] fibo_pdp_active failed");
		return 0;
	}
    g_pdp_active_sem = fibo_sem_new(0);
    fibo_sem_wait(g_pdp_active_sem);
    fibo_sem_free(g_pdp_active_sem);

    if (0 == fibo_pdp_status_get(pdp_profile.cid, ip, &cid_status, 0))
    {
        log("[%s-%d] ip = %s,cid_status=%d", __FUNCTION__, __LINE__, ip,cid_status);
    }
    return cid_status;
}

INT32 sftp_test()
{
	/*
		Author:zhenghuanhuan
		Date:2020.9.17
	*/
	log("sftp_test");
	int result = -1;
	UINT8 buf[1025]={0};
	char sftppath[256] = "/L610test/L610sftp_0426.txt";
	char localpath[256] = "/sftptest/L610down.txt";
	char filename[256] = "/sftptest/test.txt";
	INT32 downfilesize = 0;

	//1. create local file 
	result = fibo_file_delete(filename);
	log("AutoTest fibo_file_delete(filename); result = %d",result);

	result = fibo_file_delete(localpath);
	log("AutoTest fibo_file_delete(localpath); result = %d",result);

	result = fibo_file_rmdir(FILE_DIR_PATH);
	log("AutoTest fibo_file_rmdir(FILE_DIR_PATH); result = %d",result);

	result = fibo_file_mkdir(FILE_DIR_PATH);
	log("fibo_file_mkdir(FILE_DIR_PATH) result = %d",result);

	basic_file_handle1 = fibo_file_open(filename, FS_O_RDWR | FS_O_CREAT);
	if(basic_file_handle1 <= 0)
	{
	    log("Open filename fail");
		return -1;
	}

	getrandom((char*)buf,1024);

	fibo_file_seek(basic_file_handle1,0,FS_SEEK_SET);
	result = fibo_file_write(basic_file_handle1, (UINT8*)buf, 1024);
	log("fibo_file_write 1024 result = %d",result);
	if(result != 1024)
	{
    	log("fibo_file_write fail");
		return -1;
	}
	fibo_taskSleep(2000);

	result = fibo_file_close(basic_file_handle1);
	log("fibo_file_close result = %d",result);
	basic_file_handle1 = -1;


	//2.login sftp server with username、password
	result = fibo_sftp_open(sftppublicremoteaddr,sftpusername,sftppassword,sftpuRemotePort);          
	log("fibo_sftp_open result = %d",result );
	if(result != 0)
	{
		log("fibo_sftp_open failed");
		return -1;
	}
	fibo_taskSleep(60000);

	
	//3.write local file to sftp server
	result = fibo_sftp_write(filename,sftppath);
	log("fibo_sftp_write result = %d",result);
	if(result != 0)
	{
	    log("AutoTest fibo_sftp_write fail");
	}

	//4.reopen connection because connection disconnected after write finish. 
	result = fibo_sftp_open(sftppublicremoteaddr,sftpusername,sftppassword,sftpuRemotePort);
	log("fibo_sftp_open result = %d",result );
	if(result != 0)
	{
		log("fibo_sftp_open failed");
		return -1;
	}
	fibo_taskSleep(60000);

	//5.load file on sftp server to local.
	result = fibo_sftp_read(sftppath,localpath);
	log("AutoTest fibo_sftp_read result = %d",result);
	if(result != 0)
	{
	    log("AutoTest fibo_sftp_read fail");
		return -1;
	}

	downfilesize = fibo_file_get_size(localpath);
	log("AutoTest fibo_file_getSize downfile= %d",downfilesize);
	return 0;	
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

static void prvThreadEntry(void *param)
{
    INT32 ret = -1;
    OSI_LOGI(0, "application thread enter, param 0x%x", param);
   
    ret = pdp_active();
    if(ret != 1)
    {
		log("AutoTest pdp_active failed");
		return;
    }
    ret = sftp_test();

	fibo_thread_delete();
	return;
}

void * appimg_enter(void *param)
{
    OSI_LOGI(0, "application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(prvThreadEntry, "mythread", 1024*16, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
