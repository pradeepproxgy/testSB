/*****************************************************************
*   Copyright (C), 2018, Xian Fibocom Wireless Inc
*   		       All rights reserved.                  
*   FileName    :fibocom_com_api.h
*   Author      : taric.wang
*   Created     :2018-10-17
*   Description :fibocom_com_api.h
*****************************************************************/
#ifndef _FIBOCOM_COM_API_H_
#define _FIBOCOM_COM_API_H_
#include "fibocom_com_msg.h"

// add  sys  file
#include "FreeRTOS.h"
#include "task.h"
//#include "ril_utils.h"
#include "fibo_log.h"
#include "sockets.h"
#include "stdbool.h"

#define FIBOCOM_TCPIP_SOCK_NUM 4
#define FIBOCOM_AT_COMMAND_LENGTH 1400
#define FIBOCOM_TCPIP_ADDR_LENGTH 40 //ipv4 || ipv6 every group four number hex number,total eight group
#define FIBOCOM_INVALIBLE -1
#define FIBOCOM_ZERO 0
#define FIBOCOM_TEMP_RESULT_STR_LEN 256
#define FIBOCOM_SUC 0
#define FIBOCOM_SEND_EXPIRE 12
#define FIBOCOM_CTRL_Z 0x1A
#define FIBOCOM_ERR -1
#define FIBOCOM_PROCESS 1

#define FIBOCOM_MIPCALL_OPER_NUM 3

#define FIBOCOM_TCPIP_APN_LEN 100
#define FIBOCOM_TCPIP_PHONE_LEN 100
#define FIBOCOM_TCPIP_USER_LEN 68
#define FIBOCOM_TCPIP_PASSWD_LEN 68
#define FIBOCOM_TCPIP_ADDR_LEN 48
#define FIBOCOM_TCPIP_DATA_LENGTH 1024
#define FIBOCOM_TCPIP_DOMAIN 256
#define FIBOCOM_TCPIP_FUTURE_LEN 40

#define FIBOCOM_TCPIP_RT_NUM 5
#define FIBOCOM_TCPIP_MIN_TO 500
#define FIBOCOM_TCPIP_MAX_TO 60000
#define FIBOCOM_TCPIP_CLOSE_DELY 7500
#define FIBOCOM_TCPIP_IND_REQ 0

#define FIBOCOM_TCPIP_PING_COUNT 4
#define FIBOCOM_TCPIP_PING_SIZE 32
#define FIBOCOM_TCPIP_PING_TTL 64
#define FIBOCOM_TCPIP_PING_TOS 0
#define FIBOCOM_TCPIP_PING_TIMEOUT 5000

#define FIBOCOM_CCINFO_MCC 16
#define FIBOCOM_CCINFO_MNC 16
#define FIBOCOM_CCINFO_TAC 16
#define FIBOCOM_CCINFO_CELLID 32
#define FIBOCOM_CCINFO_SCRRSI 32

#define FIBOCOM_HEARTBEAT_TIME_NUM 6

#define FIBOCOM_STORE_DNS_NUM 4
#define FIBOCOM_URL_LENGTH 256
#define FIBOCOM_IP_LEN 4
#define FIBOCOM_IPV6_LEN 16

#define FIBOCOM_MIPSETS_MAX_BUF_LEN 2048
#define FIBOCOM_MIPDSETS_MAX_BUF_LEN 2048

#define FIBOCOM_ODM_SENDTIME 0
#define FIBOCOM_ODM_SEND_SIZE 2*1024

#define TCPIP_SOCK_NUM   7
#define NTP_SOCK_NUM     1
#define MQTT_SOCK_NUM    2
#define COAP_SOCK_NUM    2 /*E_COAP \E_ECLOUD*/
#define LBS_SOCK_NUM     1
#define ONENET_SOCK_NUM 1
#define MQTTS_SOCK_NUM   2
 typedef  enum  fib_module_type
 {
     E_INVALIBLE,
     E_IO_MODULE,
     E_TCPIP,  /*TCP IP*/
     E_MQTT,   /*MQTT  */
     E_NTP,
     E_COAP,
     E_ECLOUD,
     E_HTTP,
     E_LBS,
     E_MQTTS,
     E_CT_REG,
     E_GTH,
     E_MODULE_NUM
 }fib_module_type;
//typedef bool boolean;
typedef enum
{
    E_ENCODE_WITH_MIP,
    E_CODE_WITHOUT_MIP,
    E_CODE_WITH_MIP,
    E_SAVE_DATA_TO_BUF = 5
} fib_iprfmt;

typedef enum dnsIpType
{
    E_DNS_UNKNOW,
    E_DNS_IPV4 = 0x01,
    E_DNS_IPV6 = 0x10
} dnsIpType;

typedef struct dns_url_ip
{
    dnsIpType exitFlag;
    int usefrq;
    char url[FIBOCOM_URL_LENGTH];
    char addrv4[FIBOCOM_IP_LEN];
    char addrv6[FIBOCOM_IPV6_LEN];
} dns_url_ip;

typedef struct dns_info
{
    bool dns_initflag;    //whether or not change DNS server
    int dns_sock;         //udp  sock used for  dns
    bool dns_runing_flag; //if has request ip form server
    char dns_index;       //request ip from which  index of dns server
} dns_info;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef enum ip_type
{
    E_UNKNOW_STRING = 0x00,
    E_IPV4_TYPE = 0x01,
    E_IPV6_TYPE = 0x10,
    E_IPV4V6_TYPE = 0x11
} ip_type;

typedef enum mode_type
{
    E_COMMAND,
    E_DATA,
    E_HEARTBEAT
} mode_type;

typedef enum send_type
{
    E_SEND_NORMAL,
    E_SEND_HEX_TIME,
    E_SEND_HEX_LEN
} send_type;

typedef enum select_type
{
    E_NULL_OK,
    E_OPTIONAL,
    E_NECESSARY
} select_type;

typedef enum socket_pro_type
{
    E_TCP_SOCK,
    E_UDP_SOCK,
    E_SSL_SOCK,
    E_INVALIBLE_SOCK
} socket_pro_type;

typedef enum data_type
{
    type_char,
    type_short,
    type_int,
    type_long,
    type_long_with_quotation, //can with """
    type_double,
    type_string_with_quotation,
    type_string_without_quotation,
    type_string_url_with_quotation,
    type_string_ip_with_quotation,
    type_string_ip_without_quotation,
    type_string_hex_with_quotation,
    type_string_hex_without_quotation,
    data_type_num
} data_type;

typedef enum
{
    E_QUERY = 0,
    E_ANSWER = 1
} E_RC;

typedef enum
{
    E_STANDARD_QUERY = 0,
    E_OPPOSITE_QUERY = 1,
    E_SERVER_STAT_REQ = 2

} E_OPCODE;

typedef enum
{
    DNS_TYPE_A = 1, //IPV4
    DNS_TYPE_NS = 2,
    DNS_TYPE_CNAME = 5,
    DNS_TYPE_SOA = 6,
    DNS_TYPE_WKS = 11,
    DNS_TYPE_PTR = 12, // IP to domain
    DNS_TYPE_HINFO = 13,
    DNS_TYPE_MX = 15,
    DNS_TYPE_AAAA = 28, //IPV6
    DNS_TYPE_AXFR = 252,
    DNS_TYPE_ANY = 255
} dns_type;

#define DNS_CLASS_INTERNET_DATA 1

#define FIBOCOM_TRANSID 16
#define FIBOCOM_FLAG_QR 1
#define FIBOCOM_FLAG_OPCODE 4
#define FIBOCOM_FLAG_AA 1
#define FIBOCOM_FLAG_TC 1
#define FIBOCOM_FLAG_RD 1
#define FIBOCOM_FLAG_RA 1
#define FIBOCOM_FLAG_ZERO 3
#define FIBOCOM_FLAG_RCODE 4
#define FIBOCOM_QUETIONS 16
#define FIBOCOM_ANSWER_RRS 16
#define FIBOCOM_AUTHORITY_RRS 16
#define FIBOCOM_ADDITIONS_RRS 16

#define FIBOCOM_DNS_TYPE 16
#define FIBOCOM_DNS_CLASS 16

#define FIBOCOM_ANSWER_POSITON 8
#define FIBOCOM_ANSWER_TIMELIVE 32
#define FIBOCOM_ANSWER_DATALEN 16

//DNS  Message Struct
typedef struct
{
    /*Transaction ID  2byte*/
    uint16 transId; //2byte

    /*Flags 2byte  */
    uint8 flag_Qr;     //1bit
    uint8 flag_opCode; //4bit
    uint8 flag_AA;     //1bit
    uint8 flag_TC;     //1bit
    uint8 flag_RD;     //1bit
    uint8 flag_RA;     //1bit
    uint8 flag_zero;   //3bit
    uint8 flag_rCode;  //4bit

    /*8byte  */
    uint16 quetions;
    uint16 answer_RRs;
    uint16 authority_RRs;
    uint16 additions_RRs;

    char query_name[256]; //host|IP ---len[www]len[baidu]len[com][0]
    uint16 query_type;
    uint16 query_class;

    uint16 answer_positon; //len[2byte]-->domain,
    uint16 answer_type;
    uint16 answer_class;
    uint32 answer_timelive;
    uint16 answer_datalen;
    uint8 answer_data[16];

    //char     Authoritative[256]; ��ʱ����Ҫ����֧��
    //char     Additional[256];

} fibip_dns_packet;
//NTP message struct
typedef struct
{

    uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
                        // li.   Two bits.   Leap indicator.
                        // vn.   Three bits. Version number of the protocol.
                        // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;   // Eight bits. Stratum level of the local clock.
    uint8_t poll;      // Eight bits. Maximum interval between successive messages.
    uint8_t precision; // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s; // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.

} fibip_ntp_packet; // Total:  48 bytes.

typedef struct cmd_postion
{
    int icount;   //参数个数统计
    int maxcount; //最大参数个数
    bool jumpflag;
    bool nextflag;       //是否还有参数
    char *start_pos;     // 起始位置
    unsigned int length; // 当前分隔符截取的长度
    long intergVal;      // 整形值
    char *strVal;        // 字符串值 不带引号
    //char  strVal[FIBOCOM_AT_COMMAND_LENGTH];    // 字符串值 不带引号

} cmd_postion;

typedef struct fibocom_mipcall
{
    char operation;                        //0-1
    char callcid;                          //1-3
    char apn[FIBOCOM_TCPIP_APN_LEN];       //99
    char phone[FIBOCOM_TCPIP_PHONE_LEN];   //99
    char user[FIBOCOM_TCPIP_USER_LEN];     //64
    char passwd[FIBOCOM_TCPIP_PASSWD_LEN]; //64
    char auth;                             //0-2
} fibocom_mipcall;

typedef struct fibocom_mipopen
{
    int sockindex;                      //0-4
    int srcport;                        //1-65535
    char desip[FIBOCOM_TCPIP_ADDR_LEN]; //99
    int desport;                        //1-65535
    char protocol;                      //0-2
    char pseudo;                        //0-1
    mode_type mode;
} fibocom_mipopen;

typedef struct fibocom_mipodm
{
    int sockindex;                      //0-4
    short srcport;                      //1-65535
    char desip[FIBOCOM_TCPIP_ADDR_LEN]; //99
    int desport;                        //1-65535
    char protocol;                      //0-2
    char mode;                          //0-1
} fibocom_mipodm;

typedef struct fibocom_mipsend
{
    int sockindex;                             //0-4
    char send_data[FIBOCOM_TCPIP_DATA_LENGTH]; //99  2*FIBOCOM_TCPIP_DATA_LENGTH+4
    int len;
    send_type sendmode;
} fibocom_mipsend;

typedef struct fibocom_mippush
{
    int sockindex;                      //0-4
    char desip[FIBOCOM_TCPIP_ADDR_LEN]; //99
    int desport;                        //1-65535
    int number;
} fibocom_mippush;

typedef struct fibocom_sockinfo
{
    int sockindex; //0-6
} fibocom_sockinfo;

typedef struct fibocom_mipclose
{
    int sockindex; //0-4
    char mode;     //0-2
    int number;
} fibocom_mipclose;

typedef struct fibocom_sets
{
    int sockindex; //0-4
    int size;      //1-512
    int timeout;   //1-1000
} fibocom_sets;

typedef struct fibocom_dsets
{
    int sockindex; //0-6
    int size;      //1-512
    int timeout;   //1-1000
} fibocom_dsets;

typedef struct fibocom_conf
{
    int sockindex;     //0-4
    short retr_num;    //1-12
    short min_to;      //100-1000
    int max_to;        //1-60000
    short close_delay; //100-7500
    short is_nack_ind; //0-2

} fibocom_conf;

typedef struct fibocom_keepconf
{
    short mode;      //0-1     , 0
    short keepidle;  //1-36000 ,7200
    short keepintvl; //1-750   ,65
    short keepcnt;   //1-50    ,9
} fibocom_keepconf;

typedef struct fibocom_ping
{
    int mode;                              //0-1
    char desthost[FIBOCOM_TCPIP_ADDR_LEN]; //max len 255
    short count;                           //1-255     4
    short size;                            //0-1372   32
    short ttl;                             //1-255    64
    short tos;                             //0-255     0
    int timeout;                           //500-600000 4000

} fibocom_ping;

typedef struct fibocom_msdns
{
    int sockindex;                                 //1-4
    char first_dnsserver[FIBOCOM_TCPIP_ADDR_LEN];  //max len 64
    char second_dnsserver[FIBOCOM_TCPIP_ADDR_LEN]; //max len 64
    int number;
} fibocom_msdns;

typedef struct fibocom_mipdns
{
    int src_id;
    char host[FIBOCOM_TCPIP_DOMAIN]; //max len 64
    int mode;
} fibocom_mipdns;

typedef struct fibocom_mipntp
{
    char desip[FIBOCOM_TCPIP_ADDR_LEN]; //99
    short desport;                      //1-65535
    int mode;
} fibocom_mipntp;


typedef struct fibocom_mipread
{
    int sockindex; //1-4
    int readlen;   //max len 64
} fibocom_mipread;

typedef struct heartbeat_en
{
    char mode;
    char protocol;
} heartbeat_en;

typedef struct heartbeat_serverinfo
{
    char server[64];
    unsigned short port;
    unsigned short time;
    int length;
    char data[256];
    int socketindex;
} heartbeat_serverinfo;

typedef struct heartbeat_time
{
    unsigned short time[FIBOCOM_HEARTBEAT_TIME_NUM]; //time1~time6
} heartbeat_time;

typedef struct heartbeat_config
{
    heartbeat_en operate;
    heartbeat_serverinfo serverinfo;
    heartbeat_time send_time;
} heartbeat_config;

typedef struct fibocom_mipdset
{
    int atcmd_sockid;
    int sockid;
    unsigned char remote[INET6_ADDRSTRLEN];
    int recv_size_setting;
    int recv_time_span;
    int recv_data_length;
} fibocom_mipdset;

extern cmd_postion *init_param(cmd_postion *param_str, char *str, int maxnum);
extern cmd_postion *jump_next_param(cmd_postion *param_str);
extern bool release_param(cmd_postion *param_str);
extern bool fibocom_ip_is_avalible(char *str);
extern bool fibocom_check_hex_num(char tp);
extern bool fibocom_analyze_param(data_type type, cmd_postion *param, long minv, long maxv, select_type op);
extern char *fibocom_data_to_hex(char *out, char *in, int in_len);
extern ip_type fibocom_check_ip(char *str);
extern char url_to_ip(char *url, char *ip, sa_family_t *fa);

extern void *fibocom_malloc(int size);
extern void fibocom_free(void *ptr);
extern bool FiboMipcallEnable();
extern char *FiboData2Hex(char *dataBuf, int len);

#define FIBOCOM_LOG(x, args...) dp(x, ##args);
#define FIBOCOM_MALLOC(size) fibocom_malloc(size)
#define FIBOCOM_FREE(ptr) fibocom_free(ptr)

#if 0
extern  char*        str_to_upper(char *str);
#endif

#endif //_FIBOCOM_COM_API_H_
