#ifndef __EDP_KIT_H__
#define __EDP_KIT_H__

#ifdef EDPKIT_EXPORTS
#define EDPKIT_DLL __declspec(dllexport)
#else
#define EDPKIT_DLL
#endif

#include "Common.h"
#include "cJSON.h"
#include "time.h"

/*
 * history
 * 2015-06-01 v1.0.1 wululu fix bug: malloc for string, MUST memset to 0
 * 2015-07-10 v1.1.0 wusongwei add UnpackCmdReq() and PacketCmdResp()
 * 2015-07-13 v1.1.1 wululu ���ӷ�װjson�Ľӿ�, windows�汾dll
 * 2015-07-13 v1.1.2 wululu ֧��double��string���͵Ĵ�������ͽ������
 * 2015-07-15 v1.1.3 wusongwei ���SAVEACK��Ӧ
 * 2015-07-20 v1.1.4 wusongwei ���/�޸�SAVEDATA��Ϣ�Ĵ��/�������
 * 2016-04-08 v1.1.5 wangmanjie ���savedata ����6��7�Ĵ���������
 */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define localtime_r(a, b) localtime_s(b, a)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
#define MOSQ_MSB(A) (uint8)((A & 0xFF00) >> 8)
#define MOSQ_LSB(A) (uint8)(A & 0x00FF)
#define BUFFER_SIZE 10240 //(0x01<<20)
#define PROTOCOL_NAME "EDP"
#define PROTOCOL_VERSION 1

#define MAX_FLOAT_DPS_COUNT 1000
/*----------------------------������-----------------------------------------*/
#define ERR_UNPACK_CONNRESP_REMAIN -1000
#define ERR_UNPACK_CONNRESP_FLAG -1001
#define ERR_UNPACK_CONNRESP_RTN -1002
#define ERR_UNPACK_PUSHD_REMAIN -1010
#define ERR_UNPACK_PUSHD_DEVID -1011
#define ERR_UNPACK_PUSHD_DATA -1012
#define ERR_UNPACK_SAVED_REMAIN -1020
#define ERR_UNPACK_SAVED_TANSFLAG -1021
#define ERR_UNPACK_SAVED_DEVID -1022
#define ERR_UNPACK_SAVED_DATAFLAG -1023
#define ERR_UNPACK_SAVED_JSON -1024
#define ERR_UNPACK_SAVED_PARSEJSON -1025
#define ERR_UNPACK_SAVED_BIN_DESC -1026
#define ERR_UNPACK_SAVED_PARSEDESC -1027
#define ERR_UNPACK_SAVED_BINLEN -1028
#define ERR_UNPACK_SAVED_BINDATA -1029
#define ERR_UNPACK_PING_REMAIN -1030
#define ERR_UNPACK_CMDREQ -1031
#define ERR_UNPACK_ENCRYPT_RESP -1032
#define ERR_UNPACK_SAVEDATA_ACK -1033
#define ERR_UNPACK_SAVED_STR_WITH_TIME -1034
#define ERR_UNPACK_SAVED_FLOAT_WITH_TIME -1035
#define ERR_UNPACK_UPDATE_RESP -1040
/*----------------------------��Ϣ����---------------------------------------*/
/* �������� */
#define CONNREQ 0x10
/* ������Ӧ */
#define CONNRESP 0x20
/* ת��(͸��)���� */
#define PUSHDATA 0x30
/* �������� */
#define UPDATEREQ 0x50
/* ������Ӧ */
#define UPDATERESP 0x60
/* �洢(ת��)���� */
#define SAVEDATA 0x80
/* �洢ȷ�� */
#define SAVEACK 0x90
/* �������� */
#define CMDREQ 0xA0
/* ������Ӧ */
#define CMDRESP 0xB0
/* �������� */
#define PINGREQ 0xC0
/* ������Ӧ */
#define PINGRESP 0xD0
/* �������� */
#define ENCRYPTREQ 0xE0
/* ������Ӧ */
#define ENCRYPTRESP 0xF0

#define CLOSECONN 0x40

#define UPDATE 0x50

#ifndef NULL
#define NULL (void *)0
#endif

/* SAVEDATA��Ϣ֧�ֵĸ�ʽ���� */
typedef enum
{
    kTypeFullJson = 0x01,
    kTypeBin = 0x02,
    kTypeSimpleJsonWithoutTime = 0x03,
    kTypeSimpleJsonWithTime = 0x04,
    kTypeString = 0x05,
    kTypeStringWithTime = 0x06,
    kTypeFloatWithTime = 0x07
} SaveDataType;

/*�洢ת����������6��ʱ��ṹ��*/
EDPKIT_DLL
typedef struct stDataTime
{
    uint16 year;  /*�ꣻ��2016*/
    uint8 month;  /*�£���4*/
    uint8 day;    /*�գ���5*/
    uint8 hour;   /*ʱ����14*/
    uint8 minute; /*�֣���30*/
    uint8 second; /*�룻��28*/
} DataTime, *LPDataTime;

/*�洢ת����������7�ṹ��*/
EDPKIT_DLL
typedef struct stFloatDPS
{
    uint16 ds_id; /*������ID*/
    float f_data; /*float ����*/
} FloatDPS;

/*-------------����buffer, ����buffer, EDP���ṹ����-------------------------*/
EDPKIT_DLL
typedef struct Buffer
{
    uint8 *_data;      /* buffer���� */
    uint32 _write_pos; /* bufferд��λ�� */
    uint32 _read_pos;  /* buffer��ȡλ�� */
    uint32 _capacity;  /* buffer���� */
} Buffer, SendBuffer, RecvBuffer, EdpPacket;

/*-----------------------------����Buffer�Ľӿ�------------------------------*/
/* 
 * ������:  NewBuffer
 * ����:    ����Buffer
 * ˵��:    һ�������, NewBuffer��DeleteBufferӦ�óɶԳ���
 * ����:    ��
 * ����ֵ:  ���� (Buffer*)
 *          ����ֵ�ǿ� ����Buffer�ɹ�, �������Buffer��ָ��
 *          ����ֵΪ�� ����Bufferʧ��, �ڴ治��
 */
EDPKIT_DLL
Buffer *NewBuffer();
/* 
 * ������:  DeleteBuffer
 * ����:    ����Buffer
 * ˵��:    һ�������, NewBuffer��DeleteBufferӦ�óɶԳ���
 * ����:    buf     һ��Buffer��ָ���ָ��
 * ����ֵ:  ��
 */
EDPKIT_DLL
void DeleteBuffer(Buffer **buf);
/* 
 * ������:  CheckCapacity
 * ����:    ���Buffer�Ƿ��ܹ�д�볤��Ϊlen���ֽ���, 
 *          ���Buffer����������, �Զ��ɱ���չBuffer������(��Ӱ��Buffer����)
 * ����:    buf     ��Ҫд��Buffer��ָ��
 *          len     ����д��ĳ���
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, �ڴ治��
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 CheckCapacity(Buffer *buf, uint32 len);

/*------------------------��ȡEDP�����ݵĽӿ�-------------------------------*/
/* 
 * ������:  ReadByte
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡһ���ֽ�����
 * ����:    pkg     EDP��
 *          val     ����(һ���ֽ�)
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadByte(EdpPacket *pkg, uint8 *val);
/* 
 * ������:  ReadBytes
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡcount���ֽ�����
 * ˵��:    val��malloc������, ��Ҫ�ͻ����Լ�free 
 * ����:    pkg     EDP��
 *          val     ����(count���ֽ�)
 *          count   �ֽ���
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadBytes(EdpPacket *pkg, uint8 **val, uint32 count);
/* 
 * ������:  ReadUint16
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡuint16ֵ
 * ����:    pkg     EDP��
 *          val     uint16ֵ
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadUint16(EdpPacket *pkg, uint16 *val);
/* 
 * ������:  ReadUint32
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡuint32ֵ
 * ����:    pkg     EDP��
 *          val     uint32ֵ
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadUint32(EdpPacket *pkg, uint32 *val);

/* 
 * ������:  ReadFloat
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡfloatֵ
 * ����:    pkg     EDP��
 *                 val     floatֵ,�˴��ٶ�����ΪС��(���ֽ���ǰ�����ֽ��ں�)
 *                          ��Ϊ��˻��������뽫������float���ֽڵߵ�˳�����ʹ��
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadFloat(EdpPacket *pkg, float *val);
/* 
 * ������:  ReadStr
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡ�ַ���, ��\0��β
 * ����:    pkg     EDP��
 *          val     �ַ���
 * ˵��:    val��malloc������, ��Ҫ�ͻ����Լ�free 
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadStr(EdpPacket *pkg, char **val);

/* 
 * ������:  ReadDateTime
 * ����:    ��EDPЭ��, ��Buffer(��)�ж�ȡһ������ʱ���ʽ
 * ����:    pkg:     EDP��
 *        	 	  val  :    ����������ʱ�����ʱ���ʽ�ṹ�� ָ��
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadDataTime(EdpPacket *pkg, LPDataTime val);

/* 
 * ������:  ReadRemainlen
 * ����:    ��EDPЭ��, ��Buffer(��)��remainlen
 * ˵��:    remainlen��EDPЭ���еĸ���, ��һ��EDP����ĳ���
 * ����:    pkg     EDP��
 *          len_val remainlen
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 ReadRemainlen(EdpPacket *pkg, uint32 *len_val);

/*------------------------����д��EDP���Ľӿ�-------------------------------*/
/*
 * ˵��:    Ŀǰ��֧��һ��������д�����ڶ�ȡ, ���, ֻ�ж���_read_posΪ0�İ����ܱ�д��
 */
/* 
 * ������:  WriteByte
 * ����:    ��EDPЭ��, ��һ���ֽ�����д��Buffer(��)��
 * ����:    pkg     EDP��
 *          byte    ����(һ���ֽ�)
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteByte(Buffer *buf, uint8 byte);
/* 
 * ������:  WriteBytes
 * ����:    ��EDPЭ��, ��count���ֽ�����д��Buffer(��)��
 * ����:    pkg     EDP��
 *          bytes   ����
 *          count   �ֽ���
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteBytes(Buffer *buf, const void *bytes, uint32 count);
/* 
 * ������:  WriteUint16
 * ����:    ��EDPЭ��, ��uint16д��Buffer(��)��
 * ����:    pkg     EDP��
 *          val     uint16����
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteUint16(Buffer *buf, uint16 val);
/* 
 * ������:  WriteUint32
 * ����:    ��EDPЭ��, ��uint32д��Buffer(��)��
 * ����:    pkg     EDP��
 *          val     uint32����
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteUint32(Buffer *buf, uint32 val);

/* 
 * ������:  WriteFloat
 * ����:    ��EDPЭ��, ��һ��floatֵд��Buffer��
 * ����:    pkg     EDP��
 *                 val     floatֵ,�˴��ٶ�����ΪС��(���ֽ���ǰ�����ֽ��ں�)
 *                          ��Ϊ��˻���������float���ֽڵߵ�˳����ٴ洢
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteFloat(EdpPacket *pkg, float val);

/* 
 * ������:  WriteStr
 * ����:    ��EDPЭ��, ���ַ���д��Buffer(��)��
 * ����:    pkg     EDP��
 *          val     �ַ���
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteStr(Buffer *buf, const char *str);
/* 
 * ������:  WriteRemainlen 
 * ����:    ��EDPЭ��, ��remainlenд��Buffer(��)��
 * ˵��:    remainlen��EDPЭ���еĸ���, ��һ��EDP����ĳ���
 * ����:    pkg     EDP��
 *          len_val remainlen
 * ����ֵ:  ���� (int32)
 *          <0      ʧ��, pkg��������
 *          =0      �ɹ�
 */
EDPKIT_DLL
int32 WriteRemainlen(Buffer *buf, uint32 len_val);
/* 
 * ������:  IsPkgComplete 
 * ����:    �жϽ��յ���Buffer, �Ƿ�Ϊһ��������EDP��
 * ����:    buf     ���յ���Buffer(��������)
 * ����ֵ:  ���� (int32)
 *          =0      ���ݻ�δ����, ��Ҫ��������
 *          >0      �ɹ�
 *          <0      ���ݴ���, ������EDPЭ��
 */
EDPKIT_DLL
int32 IsPkgComplete(RecvBuffer *buf);

/*-----------------------------�ͻ��˲����Ľӿ�------------------------------*/
/* 
 * ������:  GetEdpPacket 
 * ����:    �����յ��Ķ�������, �ֽ��һ��һ����EDP��
 * ˵��:    ���ص�EDP��ʹ�ú�, ��Ҫɾ��
 * ��غ���:EdpPacketType, Unpack***�ຯ��
 * ����:    buf         ���ջ���
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        ��������EDPЭ���
 */
EDPKIT_DLL
EdpPacket *GetEdpPacket(RecvBuffer *buf);

/* 
 * ������:  EdpPacketType 
 * ����:    ��ȡһ��EDP������Ϣ����, �ͻ����������Ϣ��������ͬ�Ĵ���
 * ��غ���:Unpack***�ຯ��
 * ����:    pkg         EDPЭ���
 * ����ֵ:  ���� (uint8) 
 *          ֵ          ��Ϣ����(��ϸ�μ���h����Ϣ���Ͷ���)
 */
/* ����:
 * ...
 * int8 mtype = EdpPacketType(pkg);
 * switch(mtype)
 * {
 *  case CONNRESP:
 *      UnpackConnectResp(pkg);
 *      break;
 *  case PUSHDATA:
 *      UnpackPushdata(pkg, src_devid, data, data_len);
 *      break;
 *  case SAVEDATA:
 *      UnpackSavedata(pkg, src_devid, flag, data);
 *      break;
 *  case PINGRESP:
 *      UnpackPingResp(pkg); 
 *      break;
 *  ...
 * }
 */
EDPKIT_DLL
uint8 EdpPacketType(EdpPacket *pkg);

/* 
 * ������:  PacketConnect1 
 * ����:    ��� ���豸���豸�Ƶ�EDPЭ���, �����豸�Ƶ�����(��¼��֤��ʽ1)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫ�ͻ�����ɾ���ð�
 *          �豸�ƻ�ظ�������Ӧ���豸
 * ��غ���:UnpackConnectResp
 * ����:    devid       �豸ID, �����豸ʱƽ̨���ص�ID
 *          auth_key    ��Ȩ��Ϣ(api-key), ��ƽ̨����Ŀ��Բ������豸��api-key�ַ���
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketConnect1(const char *devid, const char *auth_key);

/* 
 * ������:  PacketConnect2 
 * ����:    ��� ���豸���豸�Ƶ�EDPЭ���, �����豸�Ƶ�����(��¼��֤��ʽ2)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫ�ͻ�����ɾ���ð�
 *          �豸�ƻ�ظ�������Ӧ���豸
 * ��غ���:UnpackConnectResp
 * ����:    userid      �û�ID, ��ƽ̨ע���˺�ʱƽ̨���ص��û�ID
 *          auth_info   ��Ȩ��Ϣ, ��ƽ̨�����豸ʱ��д�豸��auth_info����
 *                      (json�����ַ���), ��������Ҫ�߱�Ψһ��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketConnect2(const char *userid, const char *auth_info);

/* 
 * ������:  UnpackConnectResp
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, ������Ӧ
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���������Ӧ��, 
 *          ��������ӦEDP����Ϊ����, �ɸú������н���
 * ��غ���:PacketConnect1, PacketConnect2, GetEdpPacket, EdpPacketType
 * ����:    pkg         EDP��, ������������Ӧ��
 * ����ֵ:  ���� (int32) 
 *          =0          ���ӳɹ�
 *          >0          ����ʧ��, ����ʧ��ԭ���<OneNet���뷽����ӿ�.docx>
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackConnectResp(EdpPacket *pkg);

/* 
 * ������:  PacketPushdata
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �豸���豸֮��ת������
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackPushdata
 * ����:    dst_devid   Ŀ���豸ID
 *          data        ����
 *          data_len    ���ݳ���
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketPushdata(const char *dst_devid,
                          const char *data, uint32 data_len);

/* 
 * ������:  UnpackPushdata
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �豸���豸֮��ת������
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���pushdata��, 
 *          ��������ӦEDP����Ϊ����, �ɸú������н��� 
 *          ���ص�Դ�豸ID(src_devid)������(data)����Ҫ�ͻ����ͷ�
 * ��غ���:PacketPushdata, GetEdpPacket, EdpPacketType
 * ����:    pkg         EDP��, ������pushdata��
 *          src_devid   Դ�豸ID
 *          data        ����
 *          data_len    ���ݳ���
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackPushdata(EdpPacket *pkg, char **src_devid,
                     char **data, uint32 *data_len);

/* 
 * ������:  PacketSavedataJson
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(json��ʽ����)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSavedata, UnpackSavedataJson
 * ����:    dst_devid   Ŀ���豸ID
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          json_obj    json����
 *          type        json������
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EdpPacket *PacketSavedataJson(const char *dst_devid, cJSON *json_obj, int type, uint16 msg_id);

/* 
 * ������:  PacketSavedataInt
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(json��ʽ����)
 * ˵��:    �ú������������ݵ�Ϊint���͵�������
 *          ���Ѳ�����װ��EDPЭ��涨��cJSON����,
 *          type���;���ʹ������JSON��ʽ�������ʽ˵�����ĵ����豸�ն˽���Э��2-EDP.docx��
 * ��غ���:UnPacketSavedataInt
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          ds_id       ������ID
 *          value       int�����ݵ�
 *          at          �������Ϊ0�������ϵͳ��ǰʱ�䣬������ø���ʱ�䡣
 *                      ���typeѡ��ΪkTypeSimpleJsonWithoutTime�������������͵�JSON��ʽ����ʱ�䣬
 *                      ��������ͳһ����ϵͳʱ�䣬��ֵ��������
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataInt(SaveDataType type, const char *dst_devid,
                             const char *ds_id, int value,
                             time_t at, uint16 msg_id);

/* 
 * ������:  PacketSavedataDouble
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(json��ʽ����)
 * ˵��:    �ú������������ݵ�Ϊdouble���͵�������
 *          ���Ѳ�����װ��EDPЭ��涨��cJSON����,
 *          type���;���ʹ������JSON��ʽ�������ʽ˵�����ĵ����豸�ն˽���Э��2-EDP.docx��
 * ��غ���:UnPacketSavedataDouble
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          ds_id       ������ID
 *          value       double�����ݵ�
 *          at          �������Ϊ0�������ϵͳ��ǰʱ�䣬������ø���ʱ�䡣
 *                      ���typeѡ��ΪkTypeSimpleJsonWithoutTime�������������͵�JSON��ʽ����ʱ�䣬
 *                      ��������ͳһ����ϵͳʱ�䣬��ֵ��������
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataDouble(SaveDataType type, const char *dst_devid,
                                const char *ds_id, double value,
                                time_t at, uint16 msg_id);

/* 
 * ������:  PacketSavedataString
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(json��ʽ����)
 * ˵��:    �ú������������ݵ�Ϊchar*���͵�������
 *          ���Ѳ�����װ��EDPЭ��涨��cJSON����,
 *          type���;���ʹ������JSON��ʽ�������ʽ˵�����ĵ����豸�ն˽���Э��2-EDP.docx��
 * ��غ���:UnPacketSavedataString
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          ds_id       ������ID
 *          value       char*�����ݵ�
 *          at          �������Ϊ0�������ϵͳ��ǰʱ�䣬������ø���ʱ�䡣
 *                      ���typeѡ��ΪkTypeSimpleJsonWithoutTime�������������͵�JSON��ʽ����ʱ�䣬
 *                      ��������ͳһ����ϵͳʱ�䣬��ֵ��������
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*)
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataString(SaveDataType type, const char *dst_devid,
                                const char *ds_id, const char *value,
                                time_t at, uint16 msg_id);

/* 
 * ������:  UnpackSavedataInt
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������,�����е�������ID��ֵ����������
 *
 * ��غ���:PacketSavedataInt
 *          
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP��, ������savedata��
 *          ds_id       ��ȡ������ID��ʹ���������ͷ�
 *          value       ��������Ӧ��ֵ
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, -1 type���Ͳ��Ϸ�������ֵ����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataInt(SaveDataType type, EdpPacket *pkg,
                        char **ds_id, int *value);

/* 
 * ������:  UnpackSavedataDouble
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������,�����е�������ID��ֵ����������
 *
 * ��غ���:PacketSavedataDouble
 *          
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP��, ������savedata��
 *          ds_id       ��ȡ������ID��ʹ���������ͷ�
 *          value       ��������Ӧ��ֵ
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, -1 type���Ͳ��Ϸ�������ֵ����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataDouble(SaveDataType type, EdpPacket *pkg,
                           char **ds_id, double *value);

/* 
 * ������:  UnpackSavedataString
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������,�����е�������ID��ֵ����������
 *
 * ��غ���:PacketSavedataString
 *          
 * ����:    type        ���õ�JSON�������ͣ���ѡ����Ϊ��kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP��, ������savedata��
 *          ds_id       ��ȡ������ID��ʹ�������Ҫ�ͷ�
 *          value       ��������Ӧ��ֵ��ʹ�������Ҫ�ͷ�
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, -1 type���Ͳ��Ϸ�������ֵ����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataString(SaveDataType type, EdpPacket *pkg,
                           char **ds_id, char **value);

/* 
 * ������:  UnpackSavedataAck
 * ����:    ������豸�Ƶ��豸��EDPЭ���, ������ת������Ϣ����Ӧ
 * ˵��:    ��������ת������Ϣ������Ϣȷ�ϱ�־ʱ��ƽ̨����Ӧ�˰���
 *          �����洢��Ϣ��ȷ�ϡ�
 * ��غ���:
 * ����:    pkg         EDP��, ������������Ӧ��
 *          msg_id      ��Ӧ����Ϣ��־
 *          result      �洢����ִ�н��
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataAck(EdpPacket *pkg, uint16 *msg_id, unsigned char *result);

/* 
 * ������:  PacketSavedataSimpleString
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(�Էֺŷָ��ļ��ַ�����ʽ)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSavedataSimpleString
 * ����:    dst_devid   Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          input       �Էֺŷָ��ļ��ַ�����ʽ��
 *                      ������豸�ն˽���Э��2-EDP.docx��
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataSimpleString(const char *dst_devid, const char *input, uint16 msg_id);

/* 
 * ������:  UnpackSavedataSimpleString
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���savedata��,
 *          ��������ӦEDP����Ϊ����, �ɸú������н���,
 *          ��ȡԴ�˷��������Էֺ���Ϊ�ָ������ַ�����
 * ��غ���: PacketSavedataSimpleString
 *          
 * ����:    pkg         EDP��, ������savedata��
 *          output      �洢���������ַ���
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataSimpleString(EdpPacket *pkg, char **output);

/* 
 * ������:  PacketSavedataSimpleStringWithTime
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(��Ĭ��ʱ����Էֺŷָ��ļ��ַ�����ʽ)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSavedataSimpleStringWithTime
 * ����:    
 *          dst_devid   Ŀ���豸ID��
 *                          ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                          NULL       �� ��Ŀ���豸����ת����
 *                          ������     �� ת����ָ���豸��
 *          input         �Էֺŷָ��ļ��ַ�����ʽ��( ������豸�ն˽���Э��2-EDP.docx��)
 *		at		  ����������ʱ�����ʱ��ṹ�壻NULL ��ʹ�õ�ǰʱ��
 *          msg_id      ��Ϣ��־
 *                          0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                          ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataSimpleStringWithTime(const char *dst_devid, const char *input, const LPDataTime at, uint16 msg_id);

/* 
 * ������:  UnpackSavedataSimpleStringWithTime
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���savedata��,
 *          ��������ӦEDP����Ϊ����, �ɸú������н���,
 *          ��ȡĬ��ʱ��󣬻�ȡԴ�˷��������Էֺ���Ϊ�ָ������ַ�����
 * ��غ���: PacketSavedataSimpleStringWithTime
 *          
 * ����:    
 *             pkg         EDP��, ������savedata��
 *          	  output     �洢���������ַ���
 *	         at		   �洢���ݰ��н���������ʱ��ṹ��
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ������ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataSimpleStringWithTime(EdpPacket *pkg, char **output, LPDataTime at);

/* 
 * ������:  PackSaveDataFloatWithTime
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(��ʱ���float������,���1000���� �����Զ��ض�)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSaveDataFloatWithTime
 * ����:    
 *          dst_devid   Ŀ���豸ID��
 *                          ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                          NULL       �� ��Ŀ���豸����ת����
 *                          ������     �� ת����ָ���豸��
 *          input         float����������
 *                          ���ֽ���ǰ�����ֽ��ں�(��С��)��
 *                          �ڴ�˻����ϣ��轫float���ĸ��ֽ�˳��ߵ����ٴ洢
 *                          (������豸�ն˽���Э��2-EDP.docx��)
 *          input_count float������������ÿ�����1000���������Զ��ض�
 *		at		  ����������ʱ�����ʱ��ṹ�壻NULL ��ʹ�õ�ǰʱ��
 *          msg_id      ��Ϣ��־
 *                          0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                          ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PackSavedataFloatWithTime(const char *dst_devid, const FloatDPS *input, int input_count, const LPDataTime at, uint16 msg_id);

/* 
 * ������:  UnpackSaveDataFloatWithTime
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���savedata��,
 *          ��������ӦEDP����Ϊ����, �ɸú������н���,
 *          ��ȡԴ�˷��������Էֺ���Ϊ�ָ������ַ�����
 * ��غ���: PackSaveDataFloatWithTime
 *          
 * ����:    pkg         EDP��, ������savedata��
 *          	      output     �洢��������float���������ڲ�����(malloc)���ⲿʹ���������ͷŵ�(free)
  *                             ���ֽ���ǰ�����ֽ��ں�(��С��)��
 *                              �ڴ�˻����ϣ��轫float���ĸ��ֽ�˳��ߵ�����ʹ��
 *                              (������豸�ն˽���Э��2-EDP.docx��)
 *                 output_count float������������������1000
 *		      at		�洢���ݰ��н���������ʱ��ṹ��
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ������ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataFloatWithTime(EdpPacket *pkg, FloatDPS **output, int *out_cout, LPDataTime at);

/* 
 * ������:  PacketSavedataBin
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(bin��ʽ����)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSavedata, UnpackSavedataBin
 * ����: dst_devid      Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          desc_obj    �������� json��ʽ
 *          bin_data    ����������
 *          bin_len     ���������ݳ���
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EdpPacket *PacketSavedataBin(const char *dst_devid, cJSON *desc_obj,
                             const uint8 *bin_data, uint32 bin_len, uint16 msg_id);

EdpPacket *PacketSavedataBinHead(const char *dst_devid, cJSON *desc_obj,
                                 uint32 bin_len, uint16 msg_id);

/* 
 * ������:  PacketSavedataBinStr
 * ����:    ��� �豸���豸�Ƶ�EDPЭ���, �洢����(bin��ʽ����)
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫɾ�������
 * ��غ���:UnpackSavedata, UnpackSavedataBin
 * ����:    dst_devid   Ŀ���豸ID��
 *                      ���ַ���"" : Ŀ���豸ΪĬ��·���豸��
 *                      NULL       �� ��Ŀ���豸����ת����
 *                      ������     �� ת����ָ���豸��
 *          desc_obj    �������� �ַ�����ʽ
 *          bin_data    ����������
 *          bin_len     ���������ݳ���
 *          msg_id      ��Ϣ��־
 *                      0   ����Ϣ��־λ������0�����в�Я����Ϣ��־��
 *                      ��������Ϣ��־λ������1������Я����Ϣ��־��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketSavedataBinStr(const char *dst_devid, const char *desc_str,
                                const uint8 *bin_data, uint32 bin_len, uint16 msg_id);

/* 
 * ������:  UnpackSavedata
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���savedata��,
 *          ��������ӦEDP����Ϊ����, �ɸú������н��� 
 *          Ȼ���ٸ���json��bin�ı�ʶ(jb_flag), ������Ӧ�Ľ�������
 *          ���ص�Դ�豸ID(src_devid)��Ҫ�ͻ����ͷ�
 * ��غ���:PacketSavedataJson, PacketSavedataBin, GetEdpPacket, 
 *          UnpackSavedataJson, UnpackSavedataBin
 * ����:    pkg         EDP��, ������savedata��
 *          src_devid   Դ�豸ID
 *          jb_flag     json or bin����, 1: json, 2: ������
 * ����ֵ:  ���� (int32)
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedata(EdpPacket *pkg, char **src_devid, uint8 *jb_flag);

/* 
 * ������:  UnpackSavedataJson
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����(json��ʽ����)
 * ˵��:    ���ص�json����(json_obj)��Ҫ�ͻ����ͷ�
 * ��غ���:PacketSavedataJson, GetEdpPacket, EdpPacketType, UnpackSavedata
 * ����:    pkg         EDP��, ������savedata����json���ݰ�
 *          json_obj    json���� 
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
int32 UnpackSavedataJson(EdpPacket *pkg, cJSON **json_obj);

/* 
 * ������:  UnpackSavedataBin
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����(bin��ʽ����)
 * ˵��:    ���ص���������(desc_obj)��bin����(bin_data)��Ҫ�ͻ����ͷ�
 * ��غ���:PacketSavedataBin, GetEdpPacket, EdpPacketType, UnpackSavedata
 * ����:    pkg         EDP��, ������savedata����bin���ݰ�
 *          desc_obj    �������� json��ʽ
 *          bin_data    ����������
 *          bin_len     ���������ݳ���
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
int32 UnpackSavedataBin(EdpPacket *pkg, cJSON **desc_obj,
                        uint8 **bin_data, uint32 *bin_len);
/* 
 * ������:  UnpackSavedataBinStr
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, �洢����(bin��ʽ����)
 * ˵��:    ���ص���������(desc_obj)��bin����(bin_data)��Ҫ�ͻ����ͷ�
 * ��غ���:PacketSavedataBin, GetEdpPacket, EdpPacketType, UnpackSavedata
 * ����:    pkg         EDP��, ������savedata����bin���ݰ�
 *          desc_obj    �������� string��ʽ
 *          bin_data    ����������
 *          bin_len     ���������ݳ���
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackSavedataBinStr(EdpPacket *pkg, char **desc_str,
                           uint8 **bin_data, uint32 *bin_len);
/* 
 * ������:  PacketCmdResp
 * ����:    ����������������Ӧ
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫ�ͻ�����ɾ���ð�
 *          
 * ��غ���:UnpackCmdReq
 * ����:    cmdid       ����id
 *          cmdid_len   ����id����
 *          resp        ��Ӧ����Ϣ
 *          resp_len    ��Ӧ��Ϣ����
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketCmdResp(const char *cmdid, uint16 cmdid_len,
                         const char *resp, uint32 resp_len);

/* 
 * ������:  UnpackCmdReq
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, ����������Ϣ
 * ˵��:    �����豸�Ʒ���������, ��������������Ϣ��
 *          ��ȡ��cmdid�Լ�req��Ҫ��ʹ�ú��ͷš�
 * ��غ���:PacketCmdResp
 * ����:    pkg         EDP��
 *          cmdid       ��ȡ����id
 *          cmdid_len   cmdid�ĳ���
 *          req         �û��������ʼλ��
 *          req_len     �û�����ĳ���
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackCmdReq(EdpPacket *pkg, char **cmdid, uint16 *cmdid_len,
                   char **req, uint32 *req_len);

EDPKIT_DLL
EdpPacket *PackUpdateReq(const char *softinfo, int len);
/* 
 * ������:  PacketPing
 * ����:    ��� ���豸���豸�Ƶ�EDPЭ���, ����
 * ˵��:    ���ص�EDP�����͸��豸�ƺ�, ��Ҫ�ͻ�����ɾ���ð�
 *          �豸�ƻ�ظ�������Ӧ���豸
 * ��غ���:UnpackPingResp
 * ����:    ��
 * ����ֵ:  ���� (EdpPacket*) 
 *          �ǿ�        EDPЭ���
 *          Ϊ��        EDPЭ�������ʧ�� 
 */
EDPKIT_DLL
EdpPacket *PacketPing(void);

/* 
 * ������:  UnpackPingResp
 * ����:    ��� ���豸�Ƶ��豸��EDPЭ���, ������Ӧ
 * ˵��:    �����豸�Ʒ���������, ͨ������GetEdpPacket��EdpPacketType�жϳ���������Ӧ��, 
 *          ��������ӦEDP����Ϊ����, �ɸú������н���
 * ��غ���:PacketPing, GetEdpPacket, EdpPacketType
 * ����:    pkg         EDP��, ������������Ӧ��
 * ����ֵ:  ���� (int32) 
 *          =0          �����ɹ�
 *          >0          ����ʧ��, ����ʧ��ԭ���<OneNet���뷽����ӿ�.docx>
 *          <0          ����ʧ��, ����ʧ��ԭ�����h�ļ��Ĵ�����
 */
EDPKIT_DLL
int32 UnpackPingResp(EdpPacket *pkg);

typedef struct UpdateInfoList
{
    char *name;
    char *version;
    char *url;
    char *md5; /* 32bytes */
    struct UpdateInfoList *next;
} UpdateInfoList;

EDPKIT_DLL
void FreeUpdateInfolist(struct UpdateInfoList *head);

EDPKIT_DLL
EdpPacket *PackUpdateReq(const char *softinfo, int len);

EDPKIT_DLL
EdpPacket *PacketUpdateReq(UpdateInfoList *head);

EDPKIT_DLL
int UnpackUpdateResp(EdpPacket *pkg, UpdateInfoList **head);

#ifdef __cplusplus
}
#endif

#endif /* __EDP_KIT_H__ */
