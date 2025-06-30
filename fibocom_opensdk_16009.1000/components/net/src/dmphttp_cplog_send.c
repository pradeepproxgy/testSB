/* Copyright (C) 2022 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "cJSON.h"
#include "osi_log.h"
#include "at_engine.h"
#include "lwip/netdb.h"
#include "netutils.h"
#include "sockets.h"
#include "dmphttp_api.h"

typedef struct _Options
{
    char *data;   //CPlog string
    uint16_t len; // CPlog len
    flag_callback_t flag_set_cb;

} Cplog_opt;

Cplog_opt g_cplogopt = {NULL, 0, NULL};

#define CPLOG_SHA256_KEY_SIZE (64)
#define CPLOG_SHA256_DIGEST_SIZE (32)

/**
 * \brief          HMAC_SHA-256 context structure
 */
typedef struct
{
    uint32_t total[2];        /*!< number of bytes processed  */
    uint32_t state[8];        /*!< intermediate digest state  */
    unsigned char buffer[64]; /*!< data block being processed */
    int is224;                /*!< 0 => SHA-256, else SHA-224 */
} iot_sha256_context;

typedef union {
    char sptr[8];
    uint64_t lint;
} u_retLen;

#define HTTP1_1 "HTTP/1.1"

#define CPDMP_PRODUCT_KEY "cu1lq3bbrsz37vbB"
#define CPDMP_DEVICE_KEY "KKPLBstzdgqiSRr"
#define CPDMP_DEVICE_SECRET "61BE4D8756F415A1EC5BBF5877E8E0F3"
#define CPDMP_OPERATOR "0"
#define CPDMP_DEVICEID "KKPLBstzdgqiSRr"
#define CPDMP_SIGNMETHOD "hmacsha256"

#define PRODUCTKEY_NAME "Content-Disposition: form-data; name=\"productKey\"\r\n\r\ncu1lq3bbrsz37vbB\r\n--MyBoundary\r\n"
#define DEVICEKEY_NAME "Content-Disposition: form-data; name=\"deviceKey\"\r\n\r\nKKPLBstzdgqiSRr\r\n--MyBoundary\r\n"
#define DEVICEID_NAME "Content-Disposition: form-data; name=\"deviceId\"\r\n\r\nKKPLBstzdgqiSRr\r\n--MyBoundary\r\n"
#define TIMESTAMP_NAME "Content-Disposition: form-data; name=\"timestamp\"\r\n\r\n%s\r\n--MyBoundary\r\n"
#define SIGNMETHOD_NAME "Content-Disposition: form-data; name=\"signMethod\"\r\n\r\nhmacsha256\r\n--MyBoundary\r\n"
#define OPERATOR1_NAME "Content-Disposition: form-data; name=\"operator\"\r\n\r\n0\r\n--MyBoundary\r\n"
#define SIGN_NAME "Content-Disposition: form-data; name=\"sign\"\r\n\r\n%s\r\n--MyBoundary\r\n"
#define FILE_NAME "%sContent-Disposition: form-data; name=\"file\"; filename=\"upload.txt\"\r\nContent-Type: application/octet-stream\r\n\r\n%s\r\n--%s--\r\n"

static char Password[65] = {0};
osiThread_t *logsendthread = NULL;
#define CPDMP_FILE_URL "https://dmp.cuiot.cn/dmp-file/uploadFiles"
//#define CPDMP_FILE_URL "https://153.35.119.70:443/dmp-file/uploadFiles"

char timestamp[100] = {0};
int deviceflag = 0;

#define cplog_mem (33 * 1024)

static char *iotx_ca_crt =
    {

        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIFGjCCBAKgAwIBAgIQCgRw0Ja8ihLIkKbfgm7sSzANBgkqhkiG9w0BAQsFADBh\r\n"
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
        "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
        "QTAeFw0xOTA2MjAxMjI3NThaFw0yOTA2MjAxMjI3NThaMF8xCzAJBgNVBAYTAlVT\r\n"
        "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
        "b20xHjAcBgNVBAMTFUdlb1RydXN0IENOIFJTQSBDQSBHMTCCASIwDQYJKoZIhvcN\r\n"
        "AQEBBQADggEPADCCAQoCggEBALFJ+j1KeZVG4jzgQob23lQ8PJUNhY31ufZihuUx\r\n"
        "hYc6HSU4Lw0fxfA43a9DpJl74M3E6F1ZRBOfJ+dWnaiyYD0PxRIQd4wJisti4Uad\r\n"
        "vz61IYY/oQ/Elxk/X7GFDquYuxCSyBdHtTVMXCxFSvQ2C/7jWZFDfGGKKNoQSiJy\r\n"
        "wDe8iiHbUOakLMmXmOTZyWJnFdR/TH5YNTiMKCNUPHAleG4IigGxDyL/gbwrdDNi\r\n"
        "bDA4lUNhD0xNvPjQ8BNKqm5HWDvirUuHdC+4hpi0GJO34O3iiRV16YmWTuVFNboU\r\n"
        "LDZ0+PQtctJnatpuZKPGyKX6jCpPvzzPw/EhNDlpEdrYHZMCAwEAAaOCAc4wggHK\r\n"
        "MB0GA1UdDgQWBBSRn14xFa4Qn61gwffBzKpINC8MJjAfBgNVHSMEGDAWgBQD3lA1\r\n"
        "VtFMu2bwo+IbG8OXsj3RVTAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYB\r\n"
        "BQUHAwEGCCsGAQUFBwMCMA8GA1UdEwEB/wQFMAMBAf8wMQYIKwYBBQUHAQEEJTAj\r\n"
        "MCEGCCsGAQUFBzABhhVodHRwOi8vb2NzcC5kY29jc3AuY24wRAYDVR0fBD0wOzA5\r\n"
        "oDegNYYzaHR0cDovL2NybC5kaWdpY2VydC1jbi5jb20vRGlnaUNlcnRHbG9iYWxS\r\n"
        "b290Q0EuY3JsMIHOBgNVHSAEgcYwgcMwgcAGBFUdIAAwgbcwKAYIKwYBBQUHAgEW\r\n"
        "HGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwgYoGCCsGAQUFBwICMH4MfEFu\r\n"
        "eSB1c2Ugb2YgdGhpcyBDZXJ0aWZpY2F0ZSBjb25zdGl0dXRlcyBhY2NlcHRhbmNl\r\n"
        "IG9mIHRoZSBSZWx5aW5nIFBhcnR5IEFncmVlbWVudCBsb2NhdGVkIGF0IGh0dHBz\r\n"
        "Oi8vd3d3LmRpZ2ljZXJ0LmNvbS9ycGEtdWEwDQYJKoZIhvcNAQELBQADggEBABfg\r\n"
        "eXrxIrtlixBv+KMDeqKxtNJbZiLDzJBkGCd4HI63X5eS6BElJBn6mI9eYVrr7qOL\r\n"
        "Tp7WiO02Sf1Yrpaz/ePSjZ684o89UAGpxOfbgVSMvo/a07n/220jUWLxzaJhQNLu\r\n"
        "lACXwwWsxYf8twP8glkoIHnUUNTlhsyyl1ZzvVC4bDpI4hC6QkJGync1MNqYSMj8\r\n"
        "tZbhQNw3HdSmcTO0Nc/J/pK2VZc6fFbKBgspmzdHc6jMKG2t4lisXEysS3wPcg0a\r\n"
        "Nfr1Odl5+myh3MnMK08f6pTXvduLz+QZiIh8IYL+Z6QWgTZ9e2jnV8juumX1I8Ge\r\n"
        "7cZdtNnTCB8hFfwGLUA=\r\n"
        "-----END CERTIFICATE-----"};

static void utils_hmac_sha256(const uint8_t *msg, uint32_t msg_len, const uint8_t *key, uint32_t key_len, uint8_t output[32]);

static void _hex2str(uint8_t *input, uint16_t input_len, char *output)
{
    char *zEncode = "0123456789ABCDEF";
    int i = 0, j = 0;

    for (i = 0; i < input_len; i++)
    {
        output[j++] = zEncode[(input[i] >> 4) & 0xf];
        output[j++] = zEncode[(input[i]) & 0xf];
    }
}

static int aiothtttpSign(const char *deviceID, const char *deviceKey, const char *productKey, const char *signMethod,
                         const char *operator, const char * timestamp, const char * deviceSecret)
{
    char macSrc[500] = {0};
    uint8_t macRes[32] = {0};
    /* setup password */

    memcpy(macSrc + strlen(macSrc), deviceID, strlen(deviceID));
    memcpy(macSrc + strlen(macSrc), deviceKey, strlen(deviceKey));
    memcpy(macSrc + strlen(macSrc), productKey, strlen(productKey));
    memcpy(macSrc + strlen(macSrc), signMethod, strlen(signMethod));
    memcpy(macSrc + strlen(macSrc), operator, strlen(operator));
    memcpy(macSrc + strlen(macSrc), timestamp, strlen(timestamp));

    OSI_LOGXI(OSI_LOGPAR_S, 0x1000a0ff, "hmacsha256 string : %s", macSrc);

    utils_hmac_sha256((uint8_t *)macSrc, strlen(macSrc), (uint8_t *)deviceSecret,
                      strlen(deviceSecret), macRes);

    _hex2str(macRes, sizeof(macRes), Password);
    OSI_LOGXI(OSI_LOGPAR_S, 0x1000a100, "hmacsha256 exchange string : %s", Password);
    return 0;
}

/******************************
 * hmac-sha256 implement below
 ******************************/

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n, b, i)                                                                                                        \
    do                                                                                                                                \
    {                                                                                                                                 \
        (n) = ((uint32_t)(b)[(i)] << 24) | ((uint32_t)(b)[(i) + 1] << 16) | ((uint32_t)(b)[(i) + 2] << 8) | ((uint32_t)(b)[(i) + 3]); \
    } while (0)
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n, b, i)                     \
    do                                             \
    {                                              \
        (b)[(i)] = (unsigned char)((n) >> 24);     \
        (b)[(i) + 1] = (unsigned char)((n) >> 16); \
        (b)[(i) + 2] = (unsigned char)((n) >> 8);  \
        (b)[(i) + 3] = (unsigned char)((n));       \
    } while (0)
#endif

static void utils_sha256_init(iot_sha256_context *ctx)
{
    memset(ctx, 0, sizeof(iot_sha256_context));
}

static void utils_sha256_starts(iot_sha256_context *ctx)
{
    int is224 = 0;
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if (is224 == 0)
    {
        /* SHA-256 */
        ctx->state[0] = 0x6A09E667;
        ctx->state[1] = 0xBB67AE85;
        ctx->state[2] = 0x3C6EF372;
        ctx->state[3] = 0xA54FF53A;
        ctx->state[4] = 0x510E527F;
        ctx->state[5] = 0x9B05688C;
        ctx->state[6] = 0x1F83D9AB;
        ctx->state[7] = 0x5BE0CD19;
    }

    ctx->is224 = is224;
}

static const uint32_t K[] = {
    0x428A2F98,
    0x71374491,
    0xB5C0FBCF,
    0xE9B5DBA5,
    0x3956C25B,
    0x59F111F1,
    0x923F82A4,
    0xAB1C5ED5,
    0xD807AA98,
    0x12835B01,
    0x243185BE,
    0x550C7DC3,
    0x72BE5D74,
    0x80DEB1FE,
    0x9BDC06A7,
    0xC19BF174,
    0xE49B69C1,
    0xEFBE4786,
    0x0FC19DC6,
    0x240CA1CC,
    0x2DE92C6F,
    0x4A7484AA,
    0x5CB0A9DC,
    0x76F988DA,
    0x983E5152,
    0xA831C66D,
    0xB00327C8,
    0xBF597FC7,
    0xC6E00BF3,
    0xD5A79147,
    0x06CA6351,
    0x14292967,
    0x27B70A85,
    0x2E1B2138,
    0x4D2C6DFC,
    0x53380D13,
    0x650A7354,
    0x766A0ABB,
    0x81C2C92E,
    0x92722C85,
    0xA2BFE8A1,
    0xA81A664B,
    0xC24B8B70,
    0xC76C51A3,
    0xD192E819,
    0xD6990624,
    0xF40E3585,
    0x106AA070,
    0x19A4C116,
    0x1E376C08,
    0x2748774C,
    0x34B0BCB5,
    0x391C0CB3,
    0x4ED8AA4A,
    0x5B9CCA4F,
    0x682E6FF3,
    0x748F82EE,
    0x78A5636F,
    0x84C87814,
    0x8CC70208,
    0x90BEFFFA,
    0xA4506CEB,
    0xBEF9A3F7,
    0xC67178F2,
};

#define SHR(x, n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x, n) (SHR(x, n) | (x << (32 - n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define S1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#define F0(x, y, z) ((x & y) | (z & (x | y)))
#define F1(x, y, z) (z ^ (x & (y ^ z)))

#define R(t)                             \
    (                                    \
        W[t] = S1(W[t - 2]) + W[t - 7] + \
               S0(W[t - 15]) + W[t - 16])

#define P(a, b, c, d, e, f, g, h, x, K)          \
    {                                            \
        temp1 = h + S3(e) + F1(e, f, g) + K + x; \
        temp2 = S2(a) + F0(a, b, c);             \
        d += temp1;                              \
        h = temp1 + temp2;                       \
    }

static void utils_sha256_process(iot_sha256_context *ctx, const unsigned char data[64])
{
    uint32_t temp1, temp2, W[64];
    uint32_t A[8];
    unsigned int i;

    for (i = 0; i < 8; i++)
    {
        A[i] = ctx->state[i];
    }

    for (i = 0; i < 64; i++)
    {
        if (i < 16)
        {
            GET_UINT32_BE(W[i], data, 4 * i);
        }
        else
        {
            R(i);
        }

        P(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i], K[i]);

        temp1 = A[7];
        A[7] = A[6];
        A[6] = A[5];
        A[5] = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp1;
    }

    for (i = 0; i < 8; i++)
    {
        ctx->state[i] += A[i];
    }
}
static void utils_sha256_update(iot_sha256_context *ctx, const unsigned char *input, uint32_t ilen)
{
    size_t fill;
    uint32_t left;

    if (ilen == 0)
    {
        return;
    }

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (uint32_t)ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (uint32_t)ilen)
    {
        ctx->total[1]++;
    }

    if (left && ilen >= fill)
    {
        memcpy((void *)(ctx->buffer + left), input, fill);
        utils_sha256_process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        utils_sha256_process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left), input, ilen);
    }
}

static const unsigned char sha256_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void utils_sha256_finish(iot_sha256_context *ctx, uint8_t output[32])
{
    uint32_t last, padn;
    uint32_t high, low;
    unsigned char msglen[8];

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low = (ctx->total[0] << 3);

    PUT_UINT32_BE(high, msglen, 0);
    PUT_UINT32_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    utils_sha256_update(ctx, sha256_padding, padn);
    utils_sha256_update(ctx, msglen, 8);

    PUT_UINT32_BE(ctx->state[0], output, 0);
    PUT_UINT32_BE(ctx->state[1], output, 4);
    PUT_UINT32_BE(ctx->state[2], output, 8);
    PUT_UINT32_BE(ctx->state[3], output, 12);
    PUT_UINT32_BE(ctx->state[4], output, 16);
    PUT_UINT32_BE(ctx->state[5], output, 20);
    PUT_UINT32_BE(ctx->state[6], output, 24);

    if (ctx->is224 == 0)
    {
        PUT_UINT32_BE(ctx->state[7], output, 28);
    }
}

static void utils_hmac_sha256(const uint8_t *msg, uint32_t msg_len, const uint8_t *key, uint32_t key_len, uint8_t output[32])
{
    iot_sha256_context context;
    uint8_t k_ipad[CPLOG_SHA256_KEY_SIZE]; /* inner padding - key XORd with ipad  */
    uint8_t k_opad[CPLOG_SHA256_KEY_SIZE]; /* outer padding - key XORd with opad */
    int32_t i;

    if ((NULL == msg) || (NULL == key) || (NULL == output))
    {
        return;
    }

    if (key_len > CPLOG_SHA256_KEY_SIZE)
    {
        return;
    }

    /* start out by storing key in pads */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    /* XOR key with ipad and opad values */
    for (i = 0; i < CPLOG_SHA256_KEY_SIZE; i++)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    /* perform inner SHA */
    utils_sha256_init(&context);                                  /* init context for 1st pass */
    utils_sha256_starts(&context);                                /* setup context for 1st pass */
    utils_sha256_update(&context, k_ipad, CPLOG_SHA256_KEY_SIZE); /* start with inner pad */
    utils_sha256_update(&context, msg, msg_len);                  /* then text of datagram */
    utils_sha256_finish(&context, output);                        /* finish up 1st pass */

    /* perform outer SHA */
    utils_sha256_init(&context);                                     /* init context for 2nd pass */
    utils_sha256_starts(&context);                                   /* setup context for 2nd pass */
    utils_sha256_update(&context, k_opad, CPLOG_SHA256_KEY_SIZE);    /* start with outer pad */
    utils_sha256_update(&context, output, CPLOG_SHA256_DIGEST_SIZE); /* then results of 1st hash */
    utils_sha256_finish(&context, output);                           /* finish up 2nd pass */
}

int http_send_body(char *string, char *output)
{
    int64_t time_ms = 0;
    char _boundary[200] = {0};
    char _str1[1024] = {0};
    char _bodyData[2000] = {0};
    char timestamp[100] = {0};
    int len = 0;

    time_ms = osiEpochTime();
    sprintf(timestamp, "%lld", time_ms);
    aiothtttpSign(CPDMP_DEVICEID, CPDMP_DEVICE_KEY, CPDMP_PRODUCT_KEY, CPDMP_SIGNMETHOD,
                  CPDMP_OPERATOR, (const char *)timestamp, CPDMP_DEVICE_SECRET);
    //OSI_LOGXI(OSI_LOGPAR_S, 0x1000a101, "guangzu sign bodydata string : %s\n", string);
    /*The separator can be set arbitrarily here is set to " MyBoundary"
    Try to be as complicated as possible to avoid duplication with the text                                                            */
    strcpy(_boundary, "MyBoundary");
    sprintf(_str1, "--%s\r\n", _boundary);
    memcpy(_bodyData + strlen(_bodyData), _str1, strlen(_str1));
    memset(_str1, 0, sizeof(_str1));
    memcpy(_bodyData + strlen(_bodyData), PRODUCTKEY_NAME, strlen(PRODUCTKEY_NAME));
    memcpy(_bodyData + strlen(_bodyData), DEVICEKEY_NAME, strlen(DEVICEKEY_NAME));
    memcpy(_bodyData + strlen(_bodyData), DEVICEID_NAME, strlen(DEVICEID_NAME));
    sprintf(_str1, TIMESTAMP_NAME, timestamp);
    memcpy(_bodyData + strlen(_bodyData), _str1, strlen(_str1));
    memcpy(_bodyData + strlen(_bodyData), SIGNMETHOD_NAME, strlen(SIGNMETHOD_NAME));
    memset(_str1, 0, sizeof(_str1));
    memcpy(_bodyData + strlen(_bodyData), OPERATOR1_NAME, strlen(OPERATOR1_NAME));
    sprintf(_str1, SIGN_NAME, Password);
    memcpy(_bodyData + strlen(_bodyData), _str1, strlen(_str1));
    memset(_str1, 0, sizeof(_str1));
    // Write Content-Disposition: form-data; name="parameter name"; filename="File name"£¬
    //Then carriage return and line feed,write Content-Type: application/octet-stream£¬Then two carriage returns
    len = strlen(FILE_NAME) + strlen(_bodyData) + strlen(string) + strlen(_boundary) - 6;
    OSI_LOGI(0x1000bca9, " enter dmphttps Content-Length %d", len);
    // first line
    sprintf(output, "POST /dmp-file/uploadFiles %s\r\n", HTTP1_1);
    //header line
    sprintf(output + strlen(output), "Host: dmp.cuiot.cn:443\r\n");
    sprintf(output + strlen(output), "Content-type: multipart/form-data; boundary=MyBoundary\r\n");
    sprintf(output + strlen(output), "Content-Length: %d\r\n\r\n", len);

    sprintf(output + strlen(output), FILE_NAME, _bodyData, string, _boundary);
    // write file

    // write trailing deliniter format --${boundary}--,Then carriage return and line feed.
    //sprintf(_str1, "--%s--\r\n", _boundary);
    // memcpy(_bodyData + strlen(_bodyData), _str1, strlen(_str1));
    //OSI_LOGXI(OSI_LOGPAR_S, 0, "dmphttp sign bodydata string : %s", cp_nHttp_reg->user_data);
    return 0;
}

//#ifdef CONFIG_CTHTTP_TLS_SUPPORT

static int dmphttps_socket_setid(dmphttps_session_t *session, int socket)
{
    OSI_LOGI(0x1000bcaa, " enter dmphttps_socket_setid");
    session->socketid = socket;
    return 0;
}
static void dmphttps_debug(void *ctx, int level,
                           const char *file, int line,
                           const char *str)
{
    ((void)level);
    OSI_LOGXI(OSI_LOGPAR_S, 0x100075cc, "https mbedtls_ssl: %s", str);
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
extern struct netif *netif_unidmp;
static int mbedtls_net_connect_for_https(mbedtls_net_context *ctx, const char *host,
                                         const char *port, int proto)
{
    int ret = 0;
    struct addrinfo hints, *addr_list, *cur;

    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = proto = SOCK_STREAM;
    hints.ai_protocol = proto = IPPROTO_TCP;
    if (NULL == netif_unidmp)
    {
        OSI_LOGI(0x1000bcab, " DMP netif_default is NULL RETURN\n");
        return (-1);
    }

    else
    {
        if (netif_unidmp->pdnType != CFW_GPRS_PDP_TYPE_IP)
        {
            OSI_LOGI(0x1000bcac, " DMP netif_unidmp_default pdntype is %d\n", netif_default->pdnType);
            return (-1);
        }
    }

    if (lwip_getaddrinfo_ex(host, port, &hints, &addr_list, netif_unidmp->sim_cid) != 0)
        return (-1);

    /* Try the sockaddrs until a connection succeeds */
    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        ctx->fd = lwip_socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (ctx->fd < 0)
        {
            ret = -1;
            continue;
        }
        struct sockaddr_in dest;
        struct sockaddr_in *destaddr = (struct sockaddr_in *)&dest;
        destaddr->sin_len = sizeof(struct sockaddr_in);
        destaddr->sin_family = AF_INET;
        inet_addr_from_ip4addr(&destaddr->sin_addr, ip_2_ip4(&netif_unidmp->ip_addr));
        int len = sizeof(dest);
        lwip_bind(ctx->fd, (struct sockaddr *)destaddr, len);

        if (lwip_connect(ctx->fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            ret = 0;
            break;
        }
        OSI_LOGI(0x1000bcad, " DMP lwip_connect right ret =%d\n", ret);
        lwip_close(ctx->fd);
        ret = -1;
    }

    freeaddrinfo(addr_list);

    return (ret);
}

static int DMPHttps_mbedtlssocket_Connect(dmphttps_session_t *session, const char *addr, int port)
{
    int ret = 0;
    uint32_t flags = 0;
    const char *pers = addr;

    OSI_LOGI(0x1000bcae, "DMPHttps_mbedtlssocket_Connect begin");

    mbedtls_x509_crt_init(&session->clicert);

    mbedtls_pk_init(&session->pkey);

    mbedtls_net_init(&session->server_fd);

    session->ssl = malloc(sizeof(mbedtls_ssl_context));
    mbedtls_ssl_init(session->ssl);

    mbedtls_ssl_config_init(&session->conf);

    mbedtls_x509_crt_init(&session->cacert);

    mbedtls_ctr_drbg_init(&session->ctr_drbg);

    mbedtls_entropy_init(&session->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&session->ctr_drbg, mbedtls_entropy_func, &session->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0)
    {
        OSI_LOGI(0x100075ce, "mbedtls_ctr_drbg_seed failed...,ret=%d\n", ret);
        return ret;
    }
    if (iotx_ca_crt != NULL)
    {
        ret = mbedtls_x509_crt_parse(&session->cacert, (const unsigned char *)iotx_ca_crt,
                                     (iotx_ca_crt == NULL) ? 0 : (strlen((const char *)iotx_ca_crt) + 1));
        if (ret < 0)
        {
            OSI_LOGI(0x100075d2, "mbedtls_x509_crt_parse ca_pem failed...,ret=0x%x\n", -ret);
            //return ret;
        }
    }

    ret = mbedtls_ssl_conf_own_cert(&session->conf, &session->clicert, &session->pkey);
    if (ret < 0)
    {
        OSI_LOGI(0x100075d8, "mbedtls_ssl_conf_own_cert failed...,ret=0x%x\n", -ret);
        return ret;
    }
    char portStr[32];
    sprintf(portStr, "%d", port);
    if ((ret = mbedtls_net_connect_for_https(&session->server_fd, addr,
                                             portStr, MBEDTLS_NET_PROTO_TCP)) != 0) //htons(port)
    {
        OSI_LOGI(0x100075d9, "mbedtls_net_connect failed...,ret=0x%x\n", -ret);
        return ret;
    }
    dmphttps_socket_setid(session, (session->server_fd).fd);
    OSI_LOGI(0x100075da, "begin to mbedtls_ssl_config_defaults\n");
    if ((ret = mbedtls_ssl_config_defaults(&session->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        OSI_LOGI(0x100075db, "mbedtls_ssl_config_defaults failed ret = %d\n", ret);
        return ret;
    }
    /* OPTIONAL is not optimal for security,
                * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode(&session->conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&session->conf, &session->cacert, NULL);
    mbedtls_ssl_conf_rng(&session->conf, mbedtls_ctr_drbg_random, &session->ctr_drbg);

    //mbedtls_debug_set_threshold(5);
    //mbedtls_platform_set_printf(sys_arch_printf);
    mbedtls_ssl_conf_dbg(&session->conf, dmphttps_debug, NULL);

    if ((ret = mbedtls_ssl_setup(session->ssl, &session->conf)) != 0)
    {
        OSI_LOGI(0x100075dc, "mbedtls_ssl_setup failed ret = %d\n", ret);
        return ret;
    }

    mbedtls_ssl_conf_read_timeout(&session->conf, 30000);

    mbedtls_ssl_set_bio(session->ssl, &session->server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
    /*
        * 4. Handshake
        */
    OSI_LOGI(0x100075de, " . Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(session->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            OSI_LOGI(0x1000a1a6, " failed ! mbedtls_ssl_handshake returned %d\n\n", ret);
            return ret;
        }
    }

    OSI_LOGI(0x1000a1a7, "Handshake  ok\n");

    /*
        * 5. Verify the server certificate
        */
    OSI_LOGI(0x1000a1a8, " . Verifying peer X.509 certificate...");

    /* In real life, we probably want to bail out when ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(session->ssl)) != 0)
    {
        char vrfy_buf[512];

        OSI_LOGI(0x100075e1, " mbedtls_ssl_get_verify_result failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), " ! ", flags);

        OSI_LOGXI(OSI_LOGPAR_S, 0x07090005, "%s\n", vrfy_buf);
    }
    else
        OSI_LOGI(0x100075e2, "mbedtls_ssl_get_verify_result ok\n");

    return ret;
}

static int DMPHttps_mbedtlssocket_Close(dmphttps_session_t *session)
{
    if (NULL == session)
        return -1;
    mbedtls_net_free(&session->server_fd);
    mbedtls_x509_crt_free(&session->cacert);
    mbedtls_ssl_free(session->ssl);
    free(session->ssl);
    mbedtls_ssl_config_free(&session->conf);
    mbedtls_ctr_drbg_free(&session->ctr_drbg);
    mbedtls_entropy_free(&session->entropy);
    mbedtls_x509_crt_free(&session->clicert);
    mbedtls_pk_free(&session->pkey);
    return 0;
}

static int DMPHttps_mbedtlssocket_Send(dmphttps_session_t *session, char *httpshead, uint16_t len)
{
    int32_t WaitSend = len;
    int32_t BeenSent = 0;
    OSI_LOGI(0x1000bcaf, "enter DMPHttps_mbedtlssocket_send");
    if (session == NULL)
        return -1;
    do
    {
        BeenSent = mbedtls_ssl_write(session->ssl, (unsigned char *)(httpshead + len - WaitSend), WaitSend);
        if (BeenSent > 0)
        {
            WaitSend -= BeenSent;
        }
        else if (BeenSent == 0)
        {
            return 0;
        }
        else
        {
            OSI_LOGI(0x1000bcb0, "DMPHttps_mbedtlssocket_Send(ssl): send failed \n");
            return -1;
        }
    } while (WaitSend > 0);

    return 0;
}

static int DMPHttps_mbedtlssocket_Recv(dmphttps_session_t *session, char *httpsresp)
{
    OSI_LOGI(0x1000bcb1, "enter DMPHttps_mbedtlssocket_recv");
    int ret = 0;
    if (NULL == session)
        return -1;
    mbedtls_ssl_set_bio(session->ssl, &session->server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
    ret = mbedtls_ssl_read(session->ssl, (unsigned char *)httpsresp, 1024);
    if (ret < 0)
    {
        OSI_LOGI(0x1000bcb2, "DMPHttps_mbedtlssocket_recv failed :-0x%x\n", -ret);
        return ret;
    }
    return ret;
}
//#endif
int dmp_reg_http_resp(char *body, char *output)
{
    char addr[30] = "dmp.cuiot.cn";
    int ret = -1;

    //#ifdef CONFIG_CTHTTP_TLS_SUPPORT
    dmphttps_session_t *CtHttpsCont = NULL;
    CtHttpsCont = (dmphttps_session_t *)malloc(sizeof(dmphttps_session_t));
    memset(CtHttpsCont, 0, sizeof(dmphttps_session_t));
    ret = DMPHttps_mbedtlssocket_Connect(CtHttpsCont, addr, 443);
    if (ret != 0)
    {
        OSI_LOGI(0x1000bcb3, "DMP Https_mbedtlssocket_Connect error\n");
        free(CtHttpsCont);
        return ret;
    }

    ret = DMPHttps_mbedtlssocket_Send(CtHttpsCont, body, strlen(body));
    if (ret != 0)
    {
        OSI_LOGI(0x1000bcb4, "DMP Https_mbedtlssocket_Send error\n");
        goto SCLOSE;
    }
    ret = DMPHttps_mbedtlssocket_Recv(CtHttpsCont, output);
    if (ret < 0)
    {
        OSI_LOGI(0x1000bcb5, "DMP Https_mbedtlssocket_Recv error\n");
    }
    if (strncmp(output, "HTTP/1.1 200", strlen("HTTP/1.1 200")) == 0)
    {
        OSI_LOGI(0x1000bcb6, "DMPHttps_mbedtlssocket_recv success");
    }
    OSI_LOGXI(OSI_LOGPAR_S, 0x1000bcb7, "DMPhttps response:\n%s", output);
SCLOSE:
    ret = DMPHttps_mbedtlssocket_Close(CtHttpsCont);
    if (ret != 0)
    {
        OSI_LOGI(0x1000bcb8, "DMP Https_mbedtlssocket_Close\n");
    }
    free(CtHttpsCont);
    //#else

    //q#endif
    return ret;
}

void http_cplog_send(void *param)
{
    bool iResult = false;
    char resultdata[2000] = {0};
    uint8_t retry_time = 0;
    char *httpbody = NULL;
    httpbody = (char *)malloc(cplog_mem);
    if (NULL == httpbody)
    {
        OSI_LOGE(0x1000bcb9, " http_body_malloc fail\n");
        if (g_cplogopt.flag_set_cb)
            g_cplogopt.flag_set_cb(false);
        return;
    }
    memset(httpbody, 0, cplog_mem);
    http_send_body(g_cplogopt.data, httpbody);
    free(g_cplogopt.data);
    g_cplogopt.data = NULL;

RETRY:
    iResult = dmp_reg_http_resp(httpbody, resultdata);

    if ((iResult == 0) && (strstr(resultdata, "{\"code\":\"000000\"") != NULL))
    {
        if (g_cplogopt.flag_set_cb)
            g_cplogopt.flag_set_cb(true);
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a103, " http_cplog_send successful : %s\n", resultdata);
    }
    else
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a104, " http_cplog_send fail : %s\n", resultdata);
        ++retry_time;
        if (retry_time < 3)
        {
            osiThreadSleep(2000);
            goto RETRY;
        }
        else
        {
            if (g_cplogopt.flag_set_cb)
                g_cplogopt.flag_set_cb(false);
        }
    }
    free(httpbody);
    osiThreadSleep(100);
    logsendthread = NULL;
    osiThreadExit();
}
void http_lib_send_cplog(char *cplog, uint16_t len, flag_callback_t flag_set_cb)
{
    OSI_LOGI(0x1000a99c, "http send cplog to UNICOM Cloud Platform\n");
    if (logsendthread != NULL)
    {
        OSI_LOGE(0x1000bcba, "http send thread is no NULL\n");
        return;
    }
    g_cplogopt.data = (char *)malloc(len + 1);
    if (NULL == g_cplogopt.data)
    {
        OSI_LOGE(0x1000a99f, "http_lib_send_cplog g_cplogopt.data malloc error\n");
        return;
    }
    memset(g_cplogopt.data, 0, (len + 1));
    memcpy(g_cplogopt.data, cplog, len);
    g_cplogopt.len = len;
    g_cplogopt.flag_set_cb = flag_set_cb;
    logsendthread = osiThreadCreate("cplog_send", http_cplog_send, NULL, OSI_PRIORITY_NORMAL, 2048 * 4, 32);
    return;
}
