/**
* @file ffw_rat.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 无线网络相关的接口实现，依赖于各个平台实现
*        PDP激活接口(ffw_rat_pdp_active)成功时，需要发送 @c FFW_EVT_PDP_ACTIVED 事件
*        去激活(ffw_rat_pdp_deactive)成功时，需要发送 @c FFW_EVT_PDP_DEACTIVED 事件
*        被动掉网时需要发送 @c FFW_EVT_PDP_RELEASE 事件，使用ffw_broker_send_msg发送
* @version 0.1
* @date 2021-05-20
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#ifndef LIB_INCLUDE_FFW_RAT
#define LIB_INCLUDE_FFW_RAT

#include <ffw_utils.h>
#include <stdint.h>

#define FFW_CID_MIN 1
#define FFW_CID_MAX 7

#define FFW_APN_MAX_LEN 99
#define FFW_PDP_ADDR_MAX_LEN 21
#define FFW_APN_USER_MAX_LEN 64
#define FFW_APN_PWD_MAX_LEN 64
// #define FFW_PDP_DNS_MAX_LEN 21 ///< 0-3: IPV4  4-19:IPV6
#define FFW_PDP_DNS_NUM 2


#define FFW_PDP_D_COMP_OFF 0 ///<  off (default if value is omitted)
#define FFW_PDP_D_COMP_ON 1  ///<  on(manufacturer preferred compression)
#define FFW_PDP_D_COMP_V42 2 ///<  V.42bis
#define FFW_PDP_D_COMP_V44 3 ///<  V.44

// PDP header compression
#define FFW_PDP_H_COMP_OFF 0     ///<  off (default if value is omitted)
#define FFW_PDP_H_COMP_ON 1      ///<  on (manufacturer preferred compression)
#define FFW_PDP_H_COMP_RFC1144 2 ///<  RFC1144 (applicable for SNDCP only)
#define FFW_PDP_H_COMP_RFC2507 3 ///<  RFC2507
#define FFW_PDP_H_COMP_RFC3095 4 ///<  RFC3095 (applicable for PDCP only)

#define PSM_SHAREIP_MIPCALL_TRUE 1
#define PSM_SHAREIP_MIPCALL_FALSE 0


typedef enum ffw_operator
{
    /// other operator
    FFW_OTHER_OPERATOR = 0,

    /// CHINA_MOBILE
    FFW_CHINA_MOBILE,

    /// CHN_CT
    FFW_CHN_CT,

    /// CHN_UNICOM
    FFW_CHN_UNICOM,

    /// CHN_GUANGDIAN
    FFW_CHN_GUANGDIAN
}ffw_operator_t;

/// type of ffw_pdp_profile_s
typedef struct ffw_pdp_profile_s ffw_pdp_profile_t;

/// type of ffw_cellinfo_s
typedef struct ffw_cellinfo_s ffw_cellinfo_t;

/// type of ffw_wifiinfo_s
typedef struct ffw_wifiinfo_s ffw_wifiinfo_t;

/// type of ffw_gsm_cellinfo_s
typedef struct ffw_gsm_cellinfo_s ffw_gsm_cellinfo_t;

/// type of ffw_lte_cellinfo_s
typedef struct ffw_lte_cellinfo_s ffw_lte_cellinfo_t;

typedef enum ffw_reattach_status_s
{
    /// 空闲态
    FFW_IDLE = 0,

    /// 正在去附着
    FFW_DETACHING,

    /// 去附着成功
    FFW_DETACHED,

    /// 正在附着
    FFW_ATTACHING,

    /// 附着成功，
    FFW_ATTACHED,
} ffw_reattach_status_t;

/// plateform type define
typedef enum ffw_plateform_type_s
{
    /// IMEI
    FFW_PLAT_TYPE_IMEI = 0,

    /// IMSI
    FFW_PLAT_TYPE_IMSI,

    /// soft version
    FFW_PLAT_TYPE_SOFT_VER,

    /// hard version
    FFW_PLAT_TYPE_HARD_VER,

    /// model type
    FFW_PLAT_TYPE_MODEL,

    /// ICCID
    FFW_PLAT_TYPE_ICCID,

    /// CHIP ID
    FFW_PLAT_TYPE_CHIP_TYPE,

    // SN
    FFW_PLAT_TYPE_SN,
} ffw_plateform_type_t;

/**
 * @brief The callback function type for cell info query
 *
 * @param result The result of cell query, @c FFW_R_FAILED for fail and @c FFW_R_SUCCESS for ok
 * @param cellinfo The cell infomation, this value valid if @c result is @c FFW_R_SUCCESS
 * @param arg The context of callback
 */
typedef void (*ffw_cellinfo_callback_t)(int result, ffw_cellinfo_t *cellinfo, void *arg);

/**
 * @brief The callback function type for cell info query
 *
 * @param result The result of cell query, @c FFW_R_FAILED for fail and @c FFW_R_SUCCESS for ok
 * @param wifiinfo The wifi tap infomation, this value valid if @c result is @c FFW_R_SUCCESS
 * @param cnt 热点的个数
 * @param arg The context of callback
 */
typedef void (*ffw_wifiinfo_callback_t)(int result, ffw_wifiinfo_t *wifiinfo, int cnt, void *arg);

/**
* @brief PDP TYPE 定义
* 
 */
typedef enum ffw_pdp_type_s
{
    /// IP
    FFW_PDP_TYPE_IP = 1,

    /// IP Version 6
    FFW_PDP_TYPE_IPV6 = 2,

    /// DualStack
    FFW_PDP_TYPE_IPV4V6 = 3
} ffw_pdp_type_t;

///RAT 类型定义
typedef enum ffw_rat_s
{
    ///GSM
    FFW_RAT_GSM = 0,

    ///WCDMA 3G
    FFW_RAT_WCDMA = 2,

    ///LTE
    FFW_RAT_LTE = 7,

    /// NO SERVICE
    FFW_RAT_NO_SERVICE = 0XFF
} ffw_rat_t;

/// PDP 鉴权方式
typedef enum ffw_pdp_auth_proto_s
{
    /// NONE
    FFW_PDP_AUTOPROTO_NONE = 0,

    /// PAP
    FFW_PDP_AUTOPROTO_PAP = 1,

    /// CHAP
    FFW_PDP_AUTOPROTO_CHAP = 2,

    /// PAP_CHAP
    FFW_PDP_AUTOPROTO_PAP_CHAP = 3,

    /// invalid
    FFW_PDP_AUTOPROTO_INVALID,

} ffw_pdp_auth_proto_t;

/**
* @brief PDP配置定义
* 
 */
struct ffw_pdp_profile_s
{
    /// The ID of cid
    int8_t cid;

    /// 自动创建pdp context
    bool auto_create_pdpctx;

    /// PDP type, see ::ffw_pdp_type_t
    ffw_pdp_type_t pdp_type;

    /// The value of apn
    const char *apn;

    /// 鉴权用户名
    const char *apn_user;

    ///鉴权密码
    const char *apn_passwd;

    /// 鉴权方式 see ::ffw_pdp_auth_proto_t
    ffw_pdp_auth_proto_t auth_proto;

    /// auth_proto whether need to change
    bool need_set_auth;

    /// no reteach
    bool no_reteach;
};

/**
* @brief LTE网络小区信息
* 
 */
struct ffw_lte_cellinfo_s
{
    /// 接入类型
    ffw_rat_t rat;

    /// mcc
    uint8_t mcc[4];

    /// mnc
    uint8_t mnc[4];

    /// The number of mnc
    uint8_t mnc_num;

    /// imsi[0]:length of IMSI
    uint8_t imsi[16];

    /// 1:roaming,0:non roaming
    uint8_t roamingFlag;

    /// tac
    uint16_t tac;

    /// The ID of cell
    uint32_t cellid;

    /// earfcn
    uint32_t earfcn;

    /// The physical ID of cell
    uint16_t physical_cellid;

    ///当 is_service_cell 为 true 时有效
    uint32_t band;

    /// 带宽
    uint32_t bandwidth;

    ///当 is_service_cell 为 true 时有效
    uint16_t rssnr;

    /// rx level
    uint16_t rx_level;

    /// rsrp
    uint16_t rsrp;

    /// rsrq
    uint16_t rsrq;

    /// sinr
    uint16_t sinr;

    /// cell prority
    uint8_t cellPrority;

    /// p max
    uint8_t p_max;

    /// rssi
    int16_t rssi;

    /// default value 0xffff
    uint16_t tsMode;

    /// default value 0xffff
    uint16_t SA;

    /// default value 0xffff
    uint16_t SSP;

    /// default value 0xffff
    uint32_t ulMcs;

    /// default value 0xffff
    uint32_t dlMcs;

    uint8_t ulBler;

    uint8_t dlBler;

    /// first 20 bits of SIB1 cell identity
    uint32_t enodeBID;

    /// 0--tdd  1--fdd
    uint16_t netMode;

    /// DL receiving power; Real value(Unit:dbm)=rxPower divide 16
    int16_t rxPower;
};

/**
* @brief GSM小区信息定义
*
 */
struct ffw_gsm_cellinfo_s
{
    /// 接入类型
    ffw_rat_t rat;

    /// mcc
    uint8_t mcc[3];

    /// mnc
    uint8_t mnc[3];

    /// The number of mnc
    uint8_t mnc_num;

    /// lac
    uint16_t lac;

    /// cell id
    uint16_t cellid;

    /// arfcn
    uint16_t arfcn;

    /// basic
    uint8_t basic;

    /// rxlev
    uint8_t rxlev;

    /// rssi
    uint8_t rssi;

    ///当 is_service_cell 为 true 时有效
    uint16_t band;

    /// txpwr
    uint16_t txpwr;

    /// dtx
    bool dtx;

    /// c1
    uint8_t c1;

    /// c2
    uint16_t c2;

    ///当 is_service_cell 为 false 时有效
    uint32_t c31;

    /// c32
    uint32_t c32;

    ///当 is_service_cell 为 true 时有效
    uint8_t access_tech;

    /// amr acs
    uint8_t amr_acs;

    /// maio
    uint8_t maio;

    /// hsn
    uint8_t hsn;

    /// rx lev sub
    uint8_t rxlevsub;

    /// rx level of full
    uint8_t rxlevfull;

    /// rx qual sub
    uint8_t rxqualsub;

    /// rx qual full
    uint8_t rxqualfull;

    /// ber
    uint8_t ber_lev;
};

/**
* @brief 无线消息信息定义
* 
 */
struct ffw_cellinfo_s
{
    /// 接入类型
    ffw_rat_t rat;

    /// The information of lte cell info
    ffw_lte_cellinfo_t lte_cellinfo;

    /// The information of gsm cell info
    ffw_gsm_cellinfo_t gsm_cellinfo;

    /// LTE 的邻区信息
    ffw_lte_cellinfo_t lte_neb_cellinfo[10];

    /// GSM 的邻区信息
    ffw_gsm_cellinfo_t gsm_neb_cellinfo[10];

    /// gsm邻区信息的个数
    uint8_t gsm_neb_num;

    /// lte邻区信息的个数
    uint8_t lte_neb_num;

    /// txpower
    int32_t txpower;
};

/**
* @brief WIFI消息信息定义
* 
 */
struct ffw_wifiinfo_s
{
    /// mac 地址
    char ap_mac[32];

    /// 信号值
    int8_t ap_rssi;

    /// 热点名字
    char ap_name[64];
};

typedef struct ffw_nw_status_info_s
{
    uint8_t nStatus;
    uint8_t PsType;          // 1: GPRS 2: EDGE 3: UMTS 4: NB-IoT 3: WB-EUTRAN(eg.CATM)
    uint8_t nAreaCode[5];
    uint8_t nCurrRat;
    uint8_t nCellId[4]; //cell id in umts is bitstring(28), so 4 bytes is necessary.
    uint8_t cause_type;
    uint8_t reject_cause;
    uint8_t activeTime;
    uint8_t periodicTau;
    uint16_t bsic;
    uint16_t gsmFreq;
} ffw_nw_status_info_t;

typedef struct
{
    int cid;

    uint8_t nDComp;
    uint8_t nHComp;

    // uint8_t nPcscfType[MAX_PCSCF_ADDR_NUMBER];
    // uint8_t nPcscfNumber;
    // uint8_t PcscfAddr[16 * MAX_PCSCF_ADDR_NUMBER];
    uint8_t nIsEmergency;

    /// dns type
    ffw_pdp_type_t dnstype;

    /// dns address
    char dnsaddr[2][FFW_INET_ADDRSTRLEN];

    /// dns address v6
    char dnsaddr6[2][FFW_INET6_ADDRSTRLEN];

    /// pdpaddr address. ([0] is ipv4, [1] is ipv6)
    char pdpaddr[2][FFW_INET6_ADDRSTRLEN];

    /// username
    char apn_username[FFW_APN_USER_MAX_LEN + 1];

    /// password
    char apn_password[FFW_APN_PWD_MAX_LEN + 1];

    /// apn
    char apn[FFW_APN_MAX_LEN + 1];

    // uint8_t nNSLPI;

    /// pdp type
    ffw_pdp_type_t pdptype;

    /// auth proto
    ffw_pdp_auth_proto_t auth_proto;
} ffw_pdp_context_t;

typedef struct
{
    uint8_t simid;
    int result;
    uint16_t uti;
    bool pending;
} sim_channel_s_t;


typedef struct
{
    char *outDate;
    int ioLen;

}cgla_date_t;


typedef enum CauseTypeEnum_Tag
{
    NW_EMM_CAUSE = 0,
    NW_ESM_CAUSE,

    CAUSE_END
}CauseTypeEnum;

typedef struct reject_code_s
{
    int8_t type;//CauseTypeEnum
    int8_t cause;//EMM cause 3GPP TS 24.301
    char   time[32];
} reject_code_t;




/**
* @brief 激活PDP
* 
* @param simid sim卡
* @param cid cid
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_rat_pdp_active(int8_t simid, int8_t cid);

/**
* @brief 去激活PDP
* 
* @param simid sim卡
* @param cid cid
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_rat_pdp_deactive(int8_t simid, int8_t cid);


/**
* @brief 判断当前cid是否激活，为了防止没有调用ffw激活接口的情况
* 
* @param simid sim卡
* @param cid cid
* @return int 
* @retval FFW_R_SUCCESS: true 激活
* @retval FFW_R_FAILED: 未激活
 */
bool ffw_rat_pdp_actived(int8_t simid, int8_t cid);

/**
* @brief 获取RAT类型
* 
* @param simid sim卡号
* @return ffw_rat_t 
* @retval FFW_RAT_GSM GSM
* @retval FFW_RAT_LTE LTE
 */
ffw_rat_t ffw_get_rat(int simid);

/**
* @brief 异步获取小区信息
* 
* @param simid sim卡号
* @param timeout 超时时间，单位毫秒
* @param cb 回调函数
* @param arg 回调上下文
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败

 */
int ffw_get_cellinfo(int simid, int32_t timeout, ffw_cellinfo_callback_t cb, void *arg);

/**
* @brief 异步获取小区信息V2
*        增加可获取的参数
* @param simid sim卡号
* @param timeout 超时时间，单位毫秒
* @param cb 回调函数
* @param arg 回调上下文
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败

 */
int ffw_get_cellinfo_V2(int simid, int32_t timeout, ffw_cellinfo_callback_t cb, void *arg);


/**
* @brief 同步获取小区信息
*
* @param simid sim卡号
* @param timeout 超时时间，单位毫秒
* @param cellinfo 小区信息
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_get_cellinfo_sync(int simid, int timeout, ffw_cellinfo_t *cellinfo);

/**
* @brief 异步获取WIFI SCAN信息
* 
* @param timeout 超时时间，单位毫秒
* @param cb 回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败

 */
int ffw_get_wifiinfo(int32_t timeout, ffw_wifiinfo_callback_t cb, void *arg);

/**
* @brief 异步获取WIFI SCAN信息
* 
* @param timeout 超时时间，单位毫秒
* @param[out] wifiinfo wifi 信息
* @param size wifiinfo的大小
* @param[out] cnt 扫描到的wifi热点个数
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败

 */
int ffw_get_wifiinfo_sync(int32_t timeout, ffw_wifiinfo_t *wifiinfo, int size, int *cnt);

/**
* @brief 已字符串的格式输出小区信息，@c b 参数必须是 @c ::ffw_buffer_t 类型
* 
* @param cellinfo 
* @param b 
* @return int 
 */
int ffw_cellinfo_dump(ffw_cellinfo_t *cellinfo, void *b);

/**
* @brief 获取gprs状态是否在连接状态
* 
* @param sim_id SIM卡ID
* @return true 已注册
* @return false 未注册
 */
bool ffw_gprs_reg_status(uint8_t sim_id);

/**
* @brief 获取IMEI号码
* 
* @param id SIM卡ID
* @param[out] imei 存储IMEI的缓冲区
* @param len @c imei 缓冲区的长度
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_get_imei(int id, char *imei, int len);

/**
* @brief 获取平台信息
* 
* @param id SIM卡ID
* @param ptype 类型
* @param[out] data 存储信息数据的缓冲区
* @param len @c data 缓冲区的长度
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_get_plateform_info(int id, ffw_plateform_type_t ptype, char *data, int len);

/**
* @brief 当前是否在打电话状态
* 
* @return int 
* @retval FFW_R_SUCCESS: 在打电话
* @retval FFW_R_FAILED: 未打电话
 */
int ffw_is_in_call();

/**
* @brief 
* 
* @return int 
 */
int ffw_get_default_pdninfo(int simid, ffw_pdp_context_t *pdpinfo);

/**
* @brief 修改nv里的pdp配置并保存
* 
* @param simid 
* @param cid cid 为0表示默认承载的context
* @param pdpinfo 
* @return int 
 */
int ffw_write_nv_pdninfo(int simid, int cid, ffw_pdp_context_t *pdpinfo);


/**
* @brief 读取nv里的pdp配置并保存
* 
* @param simid 
* @param pdpinfo 
* @return int 
 */
int ffw_read_nv_pdninfo(int simid, int cid, ffw_pdp_context_t *pdpinfo);

/**
* @brief 如果修改了默认context，如果需要在驻网时生效，需要将默认配置发送给协议栈，如果没有类似的接口，可以留空
* 
* @param simid 
* @param pdpinfo 
* @return int 
 */
int ffw_send_def_pdpctx_stack(int simid, ffw_pdp_context_t *pdpinfo);

/**
* @brief
*
* @param cid.
* @param addr4, len is FFW_INET_ADDRSTRLEN.
* @param addr6, len is FFW_INET6_ADDRSTRLEN.
* @return int.
 */
int ffw_get_wan_ip(int cid, char addr4[FFW_INET_ADDRSTRLEN], char addr6[FFW_INET6_ADDRSTRLEN]);

/**
* @brief 
* 
* @param simid 
* @param cid 
* @param pdpinfo 
* @return int 
 */
int ffw_get_pdpcontext(int simid, int cid, ffw_pdp_context_t *pdpinfo);

/**
* @brief 设置或者增加一个context
* 
* @param simid 
* @param cid 
* @param pdpinfo 
* @return int 
 */
int ffw_set_pdpcontext(int simid, int cid, ffw_pdp_context_t *pdpinfo);

/**
* @brief 删除一个context
* 
* @param simid 
* @param cid 
* @return int 
 */
int ffw_remove_pdpcontext(int simid, int cid);

/**
* @brief 
* 
* @param simid 
* @param attach 
* @return int 
 */
int ffw_rat_attdeatt(int simid, bool attach);

/**
 * @brief 获取网络信息
 *
 * @param nw_status 网络信息结构体
 * @param simid
 * @return int
 */
int ffw_get_nw_info(ffw_nw_status_info_t *nw_status,uint8_t simid);

/**
 * @brief get default netif cid
 *
 * @param
 * @return int8_t
 */
int8_t ffw_get_default_netif_cid();

/**
 * @brief get rndis or ppp cid
 *
 * @param
 * @return int
 */
int ffw_rat_get_rndis_ppp_cid();
char *ffw_get_ICCID(CFW_SIM_ID nSimID);


/**
 * @brief get APN
 *
 * @param APN
 * @param simid
 * @return int
 */

int ffw_get_APN(char *apn,uint8_t simid);

/**
* @brief 异步全频段搜网
* 
* @param simid sim卡号
* @param timeout 超时时间，单位毫秒
* @param cb 回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败

 */
int ffw_cell_scan(int simid, int32_t timeout, ffw_cellinfo_callback_t cb, void *arg);

int ffw_check_ims_apn(uint8_t simid, ffw_pdp_context_t *default_pdninfo, ffw_pdp_context_t *pdpcontext);

int ffw_get_operate_apn(uint8_t simid, ffw_pdp_context_t *pdpcontext);

/**
 * @brief 设置本地RRC连接立即释放
 *        该接口不通知基站，立即掉落到IDLE态，可能会丢失下行数据
 *
 * @param simid
 * @return int
 */
int ffw_set_locrel_V2(uint8_t simid);

/**
 * @brief 设置本地RRC连接释放的时间
 *
 * @param value 单位为0.5s
 * @param simid
 * @return int
 */
int ffw_set_locrel(uint8_t simid, uint32_t value);

/**
 * @brief 获取本地RRC连接释放的时间
 *
 * @param simid
 * @return 当前设置的RRC连接释放的时间
 */
uint32_t ffw_get_loccrel(uint8_t simid);

/**
 * @brief 设置RRM测量(掉电不保存)
 *
 * @param iScene
 //0 – disable the static scene function
 //1 – enable the static scene function
 * @param simid
 * @return uint32_t
 */
uint32_t ffw_set_StaticScene(uint8_t iScene, uint8_t simid);

/**
 * @brief 获取RRM测量
 *
 * @param iScene
 //0 – disable the static scene function
 //1 – enable the static scene function
 * @param simid
 * @return uint32_t
 */
uint32_t ffw_get_StaticScene(uint8_t *iScene, uint8_t simid);

/**
 * @brief 设置网络拒绝原因
 *
 * @param type --参考结构体CauseTypeEnum
 * @param cause --EMM cause 3GPP TS 24.301
 * @param time_str --拒绝时间
 */
void ffw_set_rejcause(int8_t type,int8_t cause,char *time_str, uint8_t simid);
/**
 * @brief 获取网络拒绝原因
 *
 * @param type --参考结构体CauseTypeEnum
 * @param cause --EMM cause 3GPP TS 24.301
 * @param time_str --拒绝时间
 * @return uint32_t
 */
uint32_t ffw_get_rejcause(int8_t *type,int8_t *cause,char *time_str, uint8_t simid);

/**
 * @brief 获取当前注册的运营商
 *
 * @return uint8_t
 0：other or unknow
 1：CHINA MOBILE
 2：CHN-CT
 3：CHN-UNICOM
 4：CHN-GUANGDIAN
 */
uint8_t ffw_get_curr_operator(void);


#endif /* LIB_INCLUDE_FFW_RAT */
