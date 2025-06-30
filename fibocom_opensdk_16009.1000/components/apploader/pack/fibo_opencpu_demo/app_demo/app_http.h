#ifndef BF3EA7F9_BA74_490D_8A7D_4093989ABD97
#define BF3EA7F9_BA74_490D_8A7D_4093989ABD97

#include "app.h"

typedef struct
{
    const char *name;
    const char *value;
}app_http_header_t;

int app_http_init();

int app_http_request_get(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, void **dataout, UINT32 *datalen);

int app_http_request_get_file(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, const char *filename);

int app_http_request_post(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, const char *data, UINT32 datalen, void **dataout, UINT32 *dataoutlen);

void app_http_demo();
#endif /* BF3EA7F9_BA74_490D_8A7D_4093989ABD97 */
