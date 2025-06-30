#include "app.h"
#include "app_lbs.h"

int app_lbs_getgisinfo(app_lbs_type_t lbstype, fibo_lbs_res_t **lbsinfo, int timeout)
{
    int ret = 0;

    char *key = NULL; /// 从高德申请的KEY,如果为NULL, 则用系统默认的（系统默认的仅用于测试使用，商用有风险）
    uint8_t simid = 0;

    *lbsinfo = fibo_malloc(sizeof(fibo_lbs_res_t) + LBS_LOC_MAX_LEN + 1 + LBS_DESC_MAX_LEN + 1 + LBS_RAWDATA_MAX_LEN + 1);
    if (*lbsinfo != NULL)
    {
        (*lbsinfo)->loc = ((char *)(*lbsinfo)) + sizeof(fibo_lbs_res_t);
        (*lbsinfo)->desc = (*lbsinfo)->loc + LBS_LOC_MAX_LEN + 1;
        (*lbsinfo)->raw = (*lbsinfo)->desc + LBS_DESC_MAX_LEN + 1;

        ret = fibo_lbs_get_gisinfo_v2(key, lbstype, simid, (*lbsinfo), timeout);
        APP_LOG_INFO("ret=%d", ret);
    }
    else
    {
        APP_LOG_INFO("get memory fail");
        ret = -1;
    }

    return ret;
}

int app_lbs_freegisinfo(fibo_lbs_res_t *lbsinfo)
{
    fibo_free(lbsinfo);
    return 0;
}

void app_lbs_demo()
{
    fibo_lbs_res_t *lbsinfo = NULL;
    int ret = app_lbs_getgisinfo(APP_LBS_TYPE_GPRS, &lbsinfo, 10000);
    if (ret == 0)
    {
        APP_LOG_INFO("%s,%s", lbsinfo->loc, lbsinfo->desc);
        APP_LOG_INFO("%s", lbsinfo->raw);
        app_lbs_freegisinfo(lbsinfo);
    }
}