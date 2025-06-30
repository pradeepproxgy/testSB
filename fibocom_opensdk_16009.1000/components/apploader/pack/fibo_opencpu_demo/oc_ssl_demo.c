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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osi_log.h"
#include "oc_gprs.h"
#include "errno.h"

#define FFW_SSL_VER_TLS1_2 (4)
//#define ISBLOCK
#define ISNONBLOCK

extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
    __init_array_start[i]();
}

#define TEST_CA_FILE "-----BEGIN CERTIFICATE-----\r\n"                                      \
                     "MIID9TCCAt2gAwIBAgIJAOEIwHHcR9K7MA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\r\n" \
                     "VQQGEwJDTjEPMA0GA1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQK\r\n" \
                     "DAdmaWJvY29tMRAwDgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0\r\n" \
                     "LjM2MSQwIgYJKoZIhvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wIBcNMTkw\r\n" \
                     "ODMwMDY1MjUwWhgPMjExOTA4MDYwNjUyNTBaMIGPMQswCQYDVQQGEwJDTjEPMA0G\r\n" \
                     "A1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQKDAdmaWJvY29tMRAw\r\n" \
                     "DgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0LjM2MSQwIgYJKoZI\r\n" \
                     "hvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n" \
                     "A4IBDwAwggEKAoIBAQC6UMQfxHL0oW9pY1cGvq5QPdw8OU7dX2YsCbPdEiXePKce\r\n" \
                     "E6AN3IKqOuZhEd1iIypXG2AywzIu9bd5w1d4COjjSC/Tpf2AKYw+jqfxHsQAvSKt\r\n" \
                     "Rvwp1wrS5IvWy8yEG9lNpyVJHBUWlVpU/tUf02MYYU5xUBS+mJE9Tc10j7kd/uV7\r\n" \
                     "aEfM0pYhm7VmHPWDHXeXj3LKYigjttNxUgpDh2UVpq6ejzzHA5T/k2+XtKtWu7Pb\r\n" \
                     "ag6lONzz6Zxya9htVLBy7I4uTFrcRPxNgc/KF2BuwEVc4rqGUZ4vpVdwmCyKGIua\r\n" \
                     "fvit1nsvnhvYMu01HhWuK6e3IO6hOpeyR1wk75ofAgMBAAGjUDBOMB0GA1UdDgQW\r\n" \
                     "BBTT9RodyqsY/C2WS/7k8GFWidQrlTAfBgNVHSMEGDAWgBTT9RodyqsY/C2WS/7k\r\n" \
                     "8GFWidQrlTAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4IBAQCkg9dUYBRs\r\n" \
                     "uqCz71Q75B2n768icIeMfQmf969fNVK/mwaUnFxjqq+4Xw3zADdUdnTZ8FEfjAt2\r\n" \
                     "LQaxmsiGlM3KmUhXy/k1xKypIu2KecxEX/NqdG02SYcBmrIAP6ZxOxyyJZXbPRBt\r\n" \
                     "-----END CERTIFICATE-----\r\n"

#define TEST_CLIENT_KEY_FILE "-----BEGIN RSA PRIVATE KEY-----\r\n"                                  \
                     "MIIEowIBAAKCAQEArwZekEwabeqU3vfWXfSjs1MSUF20AqrQ3iXc5nsKWtmtKakr\r\n" \
                     "T7DHgd/jNJNzobHnfRwDwnJuvpu3CEdg5VHuYtZ40R3qMR3NodW4UtJlzyaODYCx\r\n" \
                     "aiI6q33XoPmwQo/Ids3F1x/Oyj8wm1Fzkzm0/60lCxxvHI5vrUp2II1h4yK3TpQa\r\n" \
                     "peX4HV23+XmCApyPGERiXFjvlAw+9NNV6LWZ7VbIK63HovnA/HyPuor8RW4iy0Ng\r\n" \
                     "lOc82NGT5hb20QcESvKDPJPNutcBIyrMVI4hWMegmLX7gs4lU5XgajrQvQNH0q6e\r\n" \
                     "+6I6LxUt7pqqopOUdbgs0AjmWV6cpMii/DpEawIDAQABAoIBADrTbjcbjQqCfJTQ\r\n" \
                     "QdmEXvznn9EpHVaEKP1xRmSk2B8e6GeHN0pqhTOul0PVh1jCXaacIttY8MXZulPr\r\n" \
                     "AbMxrWjE4wiOAGePt8x7857KnnNYZwg8x+R/Kq729eFh6o8EmoDrumIKi8tIH8Mk\r\n" \
                     "Ri8mhyIkBL5OST4U1Y4t57QbMNpRA3bSAIoD6/QxaZ9t2/m7IUyClf1KKFbXCaTL\r\n" \
                     "6FZh68mXoJpPNV75rXTq9TNYtVO2k7h6oW8iuu5UwnQQpXBkLloOm6VMmFRCFuC+\r\n" \
                     "fh2LBgxTEkFdfDJnJpeuEVCc558zPonLvKVmD8rkaCLHIETePm9R5JOC/Rv9ROH1\r\n" \
                     "tBgrQwkCgYEA1DoL2Qu1A2xg8jJfHPOqv17vlrxa4ENq2xG1yG0siKOpd6X1MRwc\r\n" \
                     "dc3uh5DAA/80KTn7xFArB1KHweosUGIO9fmU/gkvXfLlGQnNsgIdKFZvFyM1sTP5\r\n" \
                     "fdXcsJS5fKAcC1CsvPVvpxJyiT3LhGCRO4hfmcl6jJ9OzsnW/eplIh8CgYEA0x//\r\n" \
                     "csZ0V1OoXQeTsJ5QRod8Qh0dPwkLqDQsVZyb4bllO2TkC/dLyqVowijpHxBh5Elq\r\n" \
                     "jCa/jjNSYGo4hiKdvlriYEFTDebDC3SXxdcv86ixC0vH4zKti8NDKhunVIG/8fCi\r\n" \
                     "iqvB/Tjf7SQEGWRqjykyLRVeH4kDF03kLJW3TDUCgYEAh4wzeQMzL+aO3OIzQYiX\r\n" \
                     "6/a0y++tk0M8AoODOWoRYYw2dwb2XdF4k/1ddhSLr4HWTOaN2Uri0KBzuPTaLNUU\r\n" \
                     "fSJVeRNgv36duKo8SI91FAhwl7STXIS3uxlXBSlYdzLD9q4mReH02B6+LM3dKMWM\r\n" \
                     "vRtTBCRdM2ekrAraV/7XbT0CgYBAy+dIwKPgUWqw8qxfXpdgriBy4iChwhLz0t9w\r\n" \
                     "fxpQkugA7JwZGBMI5O9b99ZklFCXEflDfnj4GcRElxU2BdXIIHit9h6Ze6ONFoGm\r\n" \
                     "VL8A11tPDjkQ//LHnGw2tjoK86+Hf8VDLifhod0IGS+w42LZAVnHAHHc1948/sjy\r\n" \
                     "7hhNqQKBgBs5xypuEz9MMof6+6vxFAWDmTMuZr7CVrgR3AaksHHD1w5Pz0J6wp6I\r\n" \
                     "kOs3Yx8d14lzom1voiO/MxBpiFssymxLl/0XAtY6NwwMYpvxgGntjABZJsTZ+xsv\r\n" \
                     "SVqKuwoZ7c0EXnBRKTcWwIhN/YgrEZ2ljqnseDAARdXFEY+Ga070\r\n"             \
                     "-----END RSA PRIVATE KEY-----\r\n"
#if defined(ISBLOCK)
static void ssl_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    //srand(100);
    int test = 1;
    reg_info_t reg_info;
    fibo_pdp_profile_t pdp_profile;

    memset(&pdp_profile, 0, sizeof(fibo_pdp_profile_t));
    pdp_profile.cid = 1;
    memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));

    while (test)
    {
        fibo_reg_info_get(&reg_info, 0);
        fibo_taskSleep(1000);
        fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
        if (reg_info.nStatus == 1)
        {
            test = 0;
            fibo_pdp_active(1, &pdp_profile, 0);
            fibo_taskSleep(1000);
            fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
        }
    }

    //If you need to verify the server's certificate, set this value to 1, otherwise set it to 0
    fibo_ssl_chkmode_set(1);

    //If the value of the previous step (fibo_set_ssl_chkmode) is set to 1, you must load your trusted CA certificate here, otherwise the connection will fail
    fibo_ssl_write_file("TRUSTFILE", (UINT8*)TEST_CA_FILE, sizeof(TEST_CA_FILE));

    //If the server also needs to verify the client, the certificate and private key of the client need to be loaded here, otherwise the connection will fail
    fibo_ssl_write_file("CAFILE", (UINT8*)TEST_CA_FILE, sizeof(TEST_CA_FILE));
    fibo_ssl_write_file("CAKEY", (UINT8*)TEST_CLIENT_KEY_FILE, sizeof(TEST_CLIENT_KEY_FILE));

    fibo_taskSleep(10000);

    char buf[32] = {0};
    UINT8 p[] = "xxxxxxxxxxxtest ssl";
    INT32 sock = fibo_ssl_sock_create();
    if (sock == -1)
    {
        fibo_textTrace("create ssl sock failed");
        fibo_thread_delete();
        return;
    }
    fibo_textTrace("fibossl sys_sock_connect %d", sock);
    int ret = fibo_ssl_sock_connect(sock, "47.110.234.36", 8887);
    fibo_textTrace("fibossl sys_sock_connect %d", ret);

    ret = fibo_ssl_sock_send(sock, p, sizeof(p) - 1);
    fibo_textTrace("fibossl sys_sock_send %d", ret);

    ret = 0;
    ret = fibo_ssl_sock_recv(sock, (UINT8*)buf, sizeof(buf));
    fibo_textTrace("fibossl sys_sock_recv %d, buf=%s", ret, buf);
    if (ret > 0)
    {
        fibo_textTrace("fibossl sys_sock_send %s", buf);
    }

    fibo_thread_delete();
}
#elif defined(ISNONBLOCK)
static int ssl_recv_unblock(INT32 sock, void *buf, INT32 size, INT32 timeout)
{
    struct timeval tm = {0};
    fd_set rset;
    int ret = -1;
    int fd = fibo_ssl_sock_fd_get(sock);

    ret = fibo_ssl_sock_recv(sock, buf, size);
    if (ret > 0)
    {
         fibo_textTrace("recv data size:%d", ret);
         return ret;
    }
    else if (ret < 0)
    {
        fibo_textTrace("recv data fail");
        return ret;
    }

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tm.tv_sec = timeout / 1000;
    tm.tv_usec = (timeout % 1000) * 1000;

    ret = fibo_sock_select(fd + 1, &rset, NULL, NULL, timeout > 0 ? &tm : NULL);
    if (ret < 0)
    {
        fibo_textTrace("select failed:%s", strerror(errno));
        ret = -1;
    }
    else if (ret == 0)
    {
        fibo_textTrace("select timeout");
        ret = -1;
    }
    else
    {
        fibo_textTrace("data coming");
        ret = fibo_ssl_sock_recv(sock, buf, size);
        if (ret > 0)
        {
            fibo_textTrace("recv data size:%d", ret);
        }
        else if (ret < 0)
        {
            fibo_textTrace("recv data fail");
        }
    }
    return ret;
}

///Non blocking mode
static void ssl_thread_entry_ssl_unblock(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    //srand(100);
    int test = 1;
    reg_info_t reg_info;
    fibo_pdp_profile_t pdp_profile;

    memset(&pdp_profile, 0, sizeof(fibo_pdp_profile_t));
    pdp_profile.cid = 1;
    memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));

    while (test)
    {
        fibo_reg_info_get(&reg_info, 0);
        fibo_taskSleep(1000);
        fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
        if (reg_info.nStatus == 1)
        {
            test = 0;
            fibo_pdp_active(1, &pdp_profile, 0);
            fibo_taskSleep(1000);
            fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
        }
    }

    //If you need to verify the server's certificate, set this value to 1, otherwise set it to 0
    fibo_ssl_chkmode_set(1);

    //If the value of the previous step (fibo_set_ssl_chkmode) is set to 1, you must load your trusted CA certificate here, otherwise the connection will fail
    fibo_textTrace("fibossl sizeof(TEST_CA_FILE)=%d, sizeof(TEST_CA_FILE)=%d, sizeof(TEST_CLIENT_KEY_FILE)=%d", sizeof(TEST_CA_FILE), sizeof(TEST_CA_FILE), sizeof(TEST_CLIENT_KEY_FILE));
    fibo_ssl_write_file("TRUSTFILE", (UINT8*)TEST_CA_FILE, sizeof(TEST_CA_FILE));

    //If the server also needs to verify the client, the certificate and private key of the client need to be loaded here, otherwise the connection will fail
    fibo_ssl_write_file("CAFILE", (UINT8*)TEST_CA_FILE, sizeof(TEST_CA_FILE));
    fibo_ssl_write_file("CAKEY", (UINT8*)TEST_CLIENT_KEY_FILE, sizeof(TEST_CLIENT_KEY_FILE));
    int ret = fibo_ssl_set_ver(FFW_SSL_VER_TLS1_2);
    fibo_textTrace("fibossl ssl_set_ver %d", ret);

    fibo_taskSleep(10000);

    UINT8 buf[32] = {0};
    UINT8 p[] = "xxxxxxxxxxxtest ssl";
    INT32 sock = fibo_ssl_sock_create();
    if (sock == -1)
    {
        fibo_textTrace("create ssl sock failed");
        fibo_thread_delete();
        return;
    }

    fibo_textTrace("fibossl sys_sock_connect %d", sock);
    ret = fibo_ssl_sock_connect(sock, "47.110.234.36", 8887);
    fibo_textTrace("fibossl sys_sock_connect %d", ret);

    ///Obtain the socket fd corresponding to the SSL socket
    int fd = fibo_ssl_sock_fd_get(sock);
    fibo_textTrace("fibo_ssl_sock_fd_get %d", fd);

    ///Set the socket fd to non blocking
    ret = fibo_sock_fcntl(fd, F_SETFL, fibo_sock_fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    fibo_textTrace("fibo_sock_fcntl %d", ret);

    ret = fibo_ssl_sock_send(sock, p, sizeof(p));
    fibo_textTrace("fibossl sys_sock_send %d", ret);

    int total = 0;
    while (total < sizeof(p))
    {
        fibo_textTrace("fibossl total=%d, sizeof(p)=%d", total, sizeof(p));
        memset(buf, 0, sizeof(buf));
        total += ssl_recv_unblock(sock, buf, sizeof(buf), 10000);
        fibo_textTrace("fibossl sys_sock_recv=%s,size=%d,total=%d", (char *)buf, strlen((char*)buf), total);
    }
    ret = fibo_ssl_errcode_get();
    fibo_textTrace("fibossl ssl_errcode_get %d", ret);

    ret = fibo_ssl_sock_close(sock);
    fibo_textTrace("fibossl ssl_sock_close %d", ret);

    fibo_thread_delete();
}
#endif

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

#if defined(ISBLOCK)
    fibo_thread_create(ssl_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
#elif defined(ISNONBLOCK)
    fibo_thread_create(ssl_thread_entry_ssl_unblock, "sslthread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
#endif
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
