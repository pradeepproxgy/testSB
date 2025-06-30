#ifndef _OC_SMS_H
#define _OC_SMS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**  Maximum string length. */
#define FIBO_SMS_MAX_SCA_ADDR_LENGTH    3


/**  Maximum string length. */
#define FIBO_SMS_MAX_ADDR_LENGTH_V01    252

/**  Maximum length of an SMS. */
#define FIBO_SMS_MAX_MO_MSG_LENGTH_V01  1440

/**  Maximum string length. */
#define FIBO_SMS_MAX_MT_MSG_LENGTH_V01  1024

#define FIBO_SMS_MESSAGE_LENGTH_MAX 255

#define FIBO_SMS_MESSAGE_TUPLE_MAX_V01 240

#define FIBO_PKT_DEFAULT_SIZE           1024
#define FIBO_RECV_BUFF_LEN              FIBO_PKT_DEFAULT_SIZE * 4 //to void buf too small to crash

#define SMS_BUF_LEN     FIBO_RECV_BUFF_LEN


/* Structures. */
/*! \brief Structure of the SMS arrival timing information in IST format. */
typedef struct
{
    int8_t sec;     /*!< Indicate the seconds (0-60). */
    int8_t min;     /*!< Indicate the minutes (0-59). */
    int8_t hour;    /*!< Indicate the hours (0-23). */
    int8_t day;     /*!< Indicate the day (1-31). */
    int8_t month;   /*!< Indicate the month (0-11). */
    int16_t year;   /*!< Indicate the year. */
}fibo_sms_time_t;

typedef struct
{
    uint16_t nIndex;
    uint8_t sca[TEL_NUMBER_MAX_LEN];
    uint8_t sca_size;
    uint8_t tosca;
    uint8_t fo;
    uint8_t pid;
    uint8_t dcs;
    char *message;          /*!< SMS message sent from lower layer. */
    uint16_t msglen;        /*!< SMS message length in bytes. */
    char *phoneNum;         /*!< SMS Originating source. */
    uint32_t phoneNumLen;   /*!< SMS Originating source length in bytes. */
    fibo_sms_time_t time;   /*!< SMS message arrival time in IST format. */
}fibo_sms_msg_t;



typedef uint32_t sms_client_handle_type;

typedef enum
{
    E_FIBO_SMS_FORMAT_GSM_7BIT          = 0,
    E_FIBO_SMS_FORMAT_BINARY_DATA       = 1,
    E_FIBO_SMS_FORMAT_UCS2              = 2,
    E_FIBO_SMS_FORMAT_IRA               = 3
}e_fibo_sms_format_type_t;

typedef enum {
    FIBO_SMS_MSG_FORMAT_CDMA_V01 = 0x00, /**<  Message format CDMA text. */
    FIBO_SMS_MSG_FORMAT_GW_PP_V01 = 0x06, /**<  Message format GW_PP text. */
    FIBO_SMS_MSG_FORMAT_GW_BC_V01 = 0x07, /**<  Message format GW_BC text. */
}fibo_sms_msg_format_t_v01;

typedef enum {
    FIBO_SMS_MESSAGE_CLASS_0_V01 = 0, /**<  Class 0. */
    FIBO_SMS_MESSAGE_CLASS_1_V01 = 1, /**<  Class 1. */
    FIBO_SMS_MESSAGE_CLASS_2_V01 = 2, /**<  Class 2. */
    FIBO_SMS_MESSAGE_CLASS_3_V01 = 3, /**<  Class 3. */
    FIBO_SMS_MESSAGE_CLASS_NONE_V01 = 4, /**<  None. */
}e_fibo_sms_message_class_type_t;


typedef void (*fibo_sms_rxmsg_handler_func_t)(
    uint32_t               ind_flag,
    void                  *ind_msg_buf,
    uint32_t              ind_msg_len
);

typedef enum
{
    FIBO_SMS_INIT_OK_IND = 1,
    FIBO_SMS_NEW_MSG_IND,
    FIBO_SMS_LIST_IND,
    FIBO_SMS_LIST_END_IND,
    FIBO_SMS_LIST_EX_IND,
    FIBO_SMS_MEM_FULL_IND,
    FIBO_SMS_REPORT_IND,
}fibo_sms_event_id_e;



typedef struct
{
    e_fibo_sms_format_type_t format;
    fibo_sms_msg_format_t_v01    mode;
    uint8_t message_class_valid;
    e_fibo_sms_message_class_type_t message_class;
    uint32_t length;
    int8_t mobile_number[FIBO_SMS_MAX_ADDR_LENGTH_V01+1];
    int8_t message_text[FIBO_SMS_MAX_MO_MSG_LENGTH_V01+1];
}fibo_sms_send_msg_info_t;  /* Message */



typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    int8_t timezone;
}fibo_sms_timestamp;



typedef struct
{
    int8_t service_center_addr[FIBO_SMS_MAX_ADDR_LENGTH_V01 + 1];
    uint8_t service_center_addr_type_valid;
    int8_t service_center_addr_type[FIBO_SMS_MAX_SCA_ADDR_LENGTH + 1];
}fibo_sms_service_center_cfg_t;

typedef enum
{
    E_FIBO_SMS_TAG_TYPE_MT_READ_V01 = 0,
    E_FIBO_SMS_TAG_TYPE_MT_NOT_READ_V01 = 1,
    E_FIBO_SMS_TAG_TYPE_MO_SENT_V01 = 2,
    E_FIBO_SMS_TAG_TYPE_MO_NOT_SENT_V01 = 3,
}e_fibo_sms_tag_type_t;

typedef struct
{
    uint8_t total_segments;     /**<     The number of long  short message*/
    uint8_t seg_number;         /**<     Current number.*/
    uint8_t reference_number;   /**< reference_number.*/
}fibo_sms_user_data_head_t;


typedef struct
{
    e_fibo_sms_format_type_t format;
    uint32_t length;
    int8_t mobile_number[FIBO_SMS_MAX_ADDR_LENGTH_V01+1];
    int8_t message_text[FIBO_SMS_MAX_MT_MSG_LENGTH_V01+1];
    e_fibo_sms_tag_type_t tag_type;
    uint8_t data_head_valid;
    fibo_sms_user_data_head_t data_head;
    uint8_t timestamp_valid;
    fibo_sms_timestamp timestamp;
}fibo_sms_read_message_info_t;

typedef enum
{
    E_FIBO_SMS_STORAGE_TYPE_NONE      = -1,   /**<  Message no need to store. */
    E_FIBO_SMS_STORAGE_TYPE_UIM       = 0,    /**<  Message store to UIM. */
    E_FIBO_SMS_STORAGE_TYPE_NV        = 1,    /**<  Message store to NV. */
}e_fibo_sms_storage_type_t;

typedef enum
{
    E_FIBO_SMS_MESSAGE_MODE_UNKNOWN   = -1,   /**<  Message type CDMA */
    E_FIBO_SMS_MESSAGE_MODE_CDMA      = 0,    /**<  Message type CDMA */
    E_FIBO_SMS_MESSAGE_MODE_GW        = 1,    /**<  Message type GW. */
}e_fibo_sms_mode_type_t;


typedef struct
{
    uint32_t storage_index;
    e_fibo_sms_storage_type_t storage_type;
    uint8_t message_mode_valid;
    e_fibo_sms_mode_type_t message_mode;
}fibo_sms_read_message_req_info_t;

typedef struct
{
    e_fibo_sms_storage_type_t e_storage;
    uint8_t e_mode_valid;
    e_fibo_sms_mode_type_t    e_mode;
    uint8_t storage_idx_valid;
    uint32_t                storage_idx;
} fibo_sms_storage_info_t;

typedef struct
{
    fibo_sms_msg_format_t_v01 format;
    uint32_t raw_message_len;
    uint8_t raw_message [FIBO_SMS_MESSAGE_LENGTH_MAX];
} fibo_sms_send_raw_message_data_t;

typedef enum
{
    E_FIBO_SMS_TL_CAUSE_CODE_ADDR_VACANT                    = 0x00,
    E_FIBO_SMS_TL_CAUSE_CODE_ADDR_TRANSLATION_FAILURE     = 0x01,
    E_FIBO_SMS_TL_CAUSE_CODE_NETWORK_RESOURCE_SHORTAGE   = 0x02,
    E_FIBO_SMS_TL_CAUSE_CODE_NETWORK_FAILURE                  = 0x03,
    E_FIBO_SMS_TL_CAUSE_CODE_INVALID_TELESERVICE_ID            = 0x04,
    E_FIBO_SMS_TL_CAUSE_CODE_NETWORK_OTHER                   = 0x05,
    E_FIBO_SMS_TL_CAUSE_CODE_NO_PAGE_RESPONSE                = 0x20,
    E_FIBO_SMS_TL_CAUSE_CODE_DEST_BUSY                          = 0x21,
    E_FIBO_SMS_TL_CAUSE_CODE_NO_ACK                              = 0x22,
    E_FIBO_SMS_TL_CAUSE_CODE_DEST_RESOURCE_SHORTAGE        = 0x23,
    E_FIBO_SMS_TL_CAUSE_CODE_SMS_DELIVERY_POSTPONED          = 0x24,
    E_FIBO_SMS_TL_CAUSE_CODE_DEST_OUT_OF_SERV                  = 0x25,
    E_FIBO_SMS_TL_CAUSE_CODE_DEST_NOT_AT_ADDR                  = 0x26,
    E_FIBO_SMS_TL_CAUSE_CODE_DEST_OTHER                         = 0x27,
    E_FIBO_SMS_TL_CAUSE_CODE_RADIO_IF_RESOURCE_SHORTAGE     = 0x40,
    E_FIBO_SMS_TL_CAUSE_CODE_RADIO_IF_INCOMPATABILITY           = 0x41,
    E_FIBO_SMS_TL_CAUSE_CODE_RADIO_IF_OTHER                     = 0x42,
    E_FIBO_SMS_TL_CAUSE_CODE_ENCODING                           = 0x60,
    E_FIBO_SMS_TL_CAUSE_CODE_SMS_ORIG_DENIED                   = 0x61,
    E_FIBO_SMS_TL_CAUSE_CODE_SMS_TERM_DENIED                   = 0x62,
    E_FIBO_SMS_TL_CAUSE_CODE_SUPP_SERV_NOT_SUPP               = 0x63,
    E_FIBO_SMS_TL_CAUSE_CODE_SMS_NOT_SUPP                      = 0x64,
    E_FIBO_SMS_TL_CAUSE_CODE_MISSING_EXPECTED_PARAM          = 0x65,
    E_FIBO_SMS_TL_CAUSE_CODE_MISSING_MAND_PARAM               = 0x66,
    E_FIBO_SMS_TL_CAUSE_CODE_UNRECOGNIZED_PARAM_VAL          = 0x67,
    E_FIBO_SMS_TL_CAUSE_CODE_UNEXPECTED_PARAM_VAL            = 0x68,
    E_FIBO_SMS_TL_CAUSE_CODE_USER_DATA_SIZE_ERR                = 0x69,
    E_FIBO_SMS_TL_CAUSE_CODE_GENERAL_OTHER                     = 0x6A,
} e_fibo_sms_tl_cause_code_type;

typedef struct
{
    uint16_t                              message_id;
    uint8_t                               cause_code_valid;
    e_fibo_sms_tl_cause_code_type           cause_code;
} fibo_sms_raw_send_resp_t;

typedef struct
{
    e_fibo_sms_storage_type_t storage_type;//无效参数，兼容前面版本
    uint8_t tag_type_valid;//无效参数，兼容前面版本
    e_fibo_sms_tag_type_t tag_type;//无效参数，兼容前面版本
    uint8_t message_mode_valid;//无效参数，兼容前面版本
    e_fibo_sms_mode_type_t message_mode;//无效参数，兼容前面版本
} fibo_sms_msg_list_req_info_t;

typedef struct
{
    uint32_t message_index;
    e_fibo_sms_tag_type_t tag_type;
}fibo_sms_msg_list_data_info_t;
typedef struct
{
    uint32_t message_list_length;
    fibo_sms_msg_list_data_info_t message_list [FIBO_SMS_MESSAGE_TUPLE_MAX_V01];
} fibo_sms_msg_list_info_t;

typedef struct
{
    e_fibo_sms_storage_type_t e_storage;//无效参数，兼容前面版本
    uint8_t e_mode_valid;//无效参数，兼容前面版本
    e_fibo_sms_mode_type_t    e_mode;//无效参数，兼容前面版本
} fibo_sms_get_store_max_size_info_t;

typedef struct
{
    uint32_t store_max_size; //最大存储空间
    uint8_t free_slots_valid;//无效参数，兼容前面版本
    uint32_t free_slots;//无效参数，兼容前面版本
} fibo_sms_store_max_size_info_t;

/*********************************************************************************
  *Function:  fibo_sms_send_message
  *Description: send sms data to other
  *Input:h_sms  : handle of client
         sms_info: sms data struct about phone number andr content
  *Output:
  *Return:  0 -> success other ->fail
  *Others:
**********************************************************************************/
int fibo_sms_send_message(sms_client_handle_type h_sms, fibo_sms_send_msg_info_t *sms_info);


/*********************************************************************************
  *Function:  fibo_sms_addrxmsghandler
  *Description: send sms data to other
  *Input:h_sms   : handle of client
         handlerPtr: sms data ind data callback
  *Output:
  *Return:  0 -> success other ->fail
  *Others:
**********************************************************************************/
int fibo_sms_addrxmsghandler(sms_client_handle_type h_sms, fibo_sms_rxmsg_handler_func_t handlerPtr);


/*********************************************************************************
  *Function:  fibo_sms_getsmscenteraddress
  *Description: get sms center address
  *Input:h_sms              : handle of client
         service_center_cfg : get center address   service_center_addr
  *Output:
  *Return:  0 -> success other ->fail
  *Others:
**********************************************************************************/
int fibo_sms_getsmscenteraddress( sms_client_handle_type         h_sms, fibo_sms_service_center_cfg_t *service_center_cfg);


/*********************************************************************************
  *Function:  fibo_sms_setsmscenteraddress
  *Description: set sms center address
  *Input:h_sms              : handle of client
         service_center_cfg : set center address   service_center_addr
  *Output:
  *Return:  0 -> success other ->fail
  *Others:
**********************************************************************************/
int fibo_sms_setsmscenteraddress( sms_client_handle_type         h_sms, fibo_sms_service_center_cfg_t *service_center_cfg);


/*********************************************************************************
*Function:  fibo_sms_read_message
*Description: get max store sms limit in system
*Input:h_sms              : handle of client
      read_info                 : resp sms data
      read_req_info             : read sms index

*Output:
*Return:  0 -> success other ->fail
*Others:
**********************************************************************************/
int fibo_sms_read_message(sms_client_handle_type h_sms, fibo_sms_read_message_info_t *read_info, fibo_sms_read_message_req_info_t *read_req_info);


/*********************************************************************************
  *Function:  fibo_sms_deletefromstorage
  *Description: send sms data to other
  *Input:h_sms          : handle of client
         pt_sms_storage : delete sms index
  *Output:
  *Return:  0 -> success other ->fail
  *Others:
**********************************************************************************/
int fibo_sms_deletefromstorage(sms_client_handle_type h_sms, fibo_sms_storage_info_t *pt_sms_storage);


#endif
