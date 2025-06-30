/*
 * CUDW.h
 *
 *  Created on: 2018年01月19日
 *  release 0.2.19-CUDW-SDK-20190418
 *      Author: Cloud
 */

#ifndef _CUDW_H_
#define _CUDW_H_

/* return value */
#define	RET_SUCCESS                 0
#define	RET_ERR_PARAM               -1
#define	RET_FAILED                  -2
#define RET_AUTH_FAILED_3           -3
#define RET_TIMEISEXPIRE            -4
#define RET_VERIFY_IS_NULL          -5
#define RET_NO_RESULT               -6

/* code type define */
#define SET_CODETYPE_EAN13          1
#define SET_CODETYPE_EAN8           2
#define SET_CODETYPE_UPCA           3
#define SET_CODETYPE_UPCE           4
#define SET_CODETYPE_C128           5
#define SET_CODETYPE_C39            6
#define SET_CODETYPE_ITF25          7
#define SET_CODETYPE_CBAR           8
#define SET_CODETYPE_C93            9
#define SET_CODETYPE_TRIOPTIC       10
#define SET_CODETYPE_NL128          11
#define SET_CODETYPE_AIM128         12
#define SET_CODETYPE_EAN128         13
#define SET_CODETYPE_PHARMA_ONE     14
#define SET_CODETYPE_PHARMA_TWO     15
#define SET_CODETYPE_MATRIX25       16
#define SET_CODETYPE_PLESSEY        17
#define SET_CODETYPE_C11            18
#define SET_CODETYPE_RSS_EXP        19
#define SET_CODETYPE_RSS_EXP_ST     20
#define SET_CODETYPE_RSS_14_LIM     21
#define SET_CODETYPE_RSS_14         22
#define SET_CODETYPE_RSS_14_ST      23
#define SET_CODETYPE_MSI            24
#define SET_CODETYPE_STD25          25
#define SET_CODETYPE_IDSTL25        26
#define SET_CODETYPE_TELEPEN        27
#define SET_CODETYPE_NEC25          28
#define SET_CODETYPE_DTLOGC25       29
#define SET_CODETYPE_C32		    30
#define SET_CODETYPE_DOTCODE        31
#define SET_CODETYPE_PDF417		    32
#define SET_CODETYPE_QR			    33
#define SET_CODETYPE_DATAMATRIX	    34
#define SET_CODETYPE_M_PDF		    35
#define SET_CODETYPE_M_QR		    36
#define SET_CODETYPE_MAXICODE	    37
#define SET_CODETYPE_AZTEC		    38
#define SET_CODETYPE_HX			    39
#define SET_CODETYPE_GM             40
#define SET_CODETYPE_CODABLOCK_A    41
#define SET_CODETYPE_CODABLOCK_F    42
#define SET_CODETYPE_COD16K         43
#define SET_CODETYPE_COD49          44
#define SET_CODETYPE_COMPOSITE      45

#define SET_CODETYPE_KP_POST        51
#define SET_CODETYPE_AUT_POST       52
#define SET_CODETYPE_OCR            54
#define SET_CODETYPE_LOTTERY        56
#define SET_CODETYPE_POSTAL			57


/* SetBar paramter define */
#define SET_CLASS_ENABLE            0x10001000
#define SET_CLASS_PROPERTY          0x10001001
#define SET_CLASS_LENGTH            0x10001002
#define SET_CLASS_MISC              0x10001003


/* SET_CLASS_ENABLE begin */

// 条码使能开关
#define SET_VAL_DISABLE             0           // 关闭使能
#define SET_VAL_ENABLE              1           // 开启使能
#define SET_VAL_INVENABLE           2           // 开启反色使能

/* SET_CLASS_ENABLE end */



/* SET_CLASS_PROPERTY begin */

// 通用码制属性，种类有:CodeBar\ITF25\Matrix25\STD25\IDSTL25\DTLOGC25\NEC25
//|_________________________________________________________________________________________
//| bit0     | bit1     | bit2     | bit3     | bit4     | bit5     | bit6     | bit7      |
//|__________|__________|__________|__________|__________|__________|__________|___________|
//| checksum | undef    | undef    | strip    | undef    | undef    | undef    | undef     |
//|__________|__________|__________|__________|__________|__________|__________|___________|
#define SET_VAL_NO_CHECK            0           // Disable checksum checking (default)
#define SET_VAL_CHECKSUM            1           // Enable checksum checking
#define SET_VAL_STRIP               8           // Strip the checksum from the result string(与checksum属性组合使用)

// C39专用属性
//|_________________________________________________________________________________________
//| bit0     | bit1     | bit2     | bit3     | bit4     | bit5     | bit6     | bit7      |
//|__________|__________|__________|__________|__________|__________|__________|___________|
//| checksum | undef    | undef    | strip    | fullASC  | undef    | undef    | undef     |
//|__________|__________|__________|__________|__________|__________|__________|___________|
#define SET_VAL_C39_NO_CHECK        0x00        // Disable checksum checking (default)
#define SET_VAL_C39_CHECKSUM        0x01        // Enable checksum checking
#define SET_VAL_C39_STRIP           0x08        // Strip the checksum from the result string(与checksum属性组合使用)
#define SET_VAL_C39_FULLASCII       0x10        // Full ASCII

// QR专用属性
#define SET_VAL_QR_NORMAL           0x00        // Disable all
#define SET_VAL_QR_SMALL            0x01        // Enable QR small
#define SET_VAL_QR_LOW_CONTRAST     0x02        // Enable QR low contrast

// DM专用属性
#define SET_VAL_DM_NORMAL           0x00        // Disable all
#define SET_VAL_DM_LOW_CONTRAST     0x02        // Enable DM low contrast

/* SET_CLASS_PROPERTY end */




/* SET_CLASS_LENGTH begin */

// 最大最小长度定义，种类有:ITF25\Matrix25\NEC25\STD25\IDSTL25\DTLOGC25
//|____________________________________
//| byte0 byte1     | byte2 byte3      |
//|_________________|__________________|
//| min length      | max length       |
//|_________________|__________________|
#define SET_VAL_LENGTH(min, max)    ((min & 0xFFFF) + ((max & 0xFFFF) << 16))

/* SET_CLASS_LENGTH end */



/* SET_CLASS_MISC begin */
#define SET_MISCTYPE_AIM_CENTER_X   1           // 设置中心点x坐标, nVal = x
#define SET_MISCTYPE_AIM_CENTER_Y   2           // 设置中心点y坐标, nVal = y
#define SET_MISCTYPE_MULTICODE      3           // 设置多码的个数, nVal = 1表示只解单码（默认），nVal > 1表示解多码（最高30）
#define SET_MISCTYPE_H_DEC          4           // 设置二维码的纵向扫描
#define SET_MISCTYPE_1D_TIMEOUT     5           // 设置一维码超时
#define SET_MISCTYPE_2D_TIMEOUT     6           // 设置二维码超时
#define SET_MISCTYPE_GS1_128        7           // 设置C128按GS1输出，nVal = 0表示不输出GS1(默认),nVal > 0表示输出GS1格式
#define SET_MISCTYPE_GS1_DM         8           // 设置DM按GS1输出，nVal = 0表示不输出GS1(默认),nVal > 0表示输出GS1格式
#define SET_MISCTYPE_CB             9           // 设置CB去掉前后缀，nVal = 0表示保留前后缀(默认),nVal > 0表示删除前后缀
#define SET_MISCTYPE_UTF8_PREX      10          // 设置UTF8编码的前缀，nVal = 0表示保留前缀(默认),nVal > 0表示删除前缀
#define SET_MISCTYPE_S25LASTBIT     11          // 设置S25码输出，nVal = 0表示保留最后一位(默认),nVal > 0表示删除最后一位
#define SET_MISCTYPE_ISSN           12          // 设置ISSN码输出格式，nVal = 0表示EAN13格式输出(默认),nVal > 0表示ISSN格式输出
#define SET_MISCTYPE_AIM_DEGREE		13			// 设置条码最大倾斜角，范围0-45
/* SET_CLASS_MISC end */

// 坐标点结构体
typedef struct tag_Position
{
    int x;
    int y;
} stPosition;

// 解码参数结构体
typedef struct tagBarResult
{
    stPosition  Center;         // 条码中心位置
    stPosition  Point[4];       // 条码四个角点位置
    int         Length;         // 单个条码信息长度
    int         CodeType;       // 单个条码类型
    char        *CodeInfo;      // 单个条码信息，输出解码库内定义好的信息缓存地址
} stBarResult;
typedef struct tagDecodeBar
{
    // input
    int   nWidth;       // 条码位图宽
    int   nHeight;      // 条码位图高
    char *pBmpBit;      // 条码位图指针
    // output
    stBarResult Result[30]; // 输出多码结果
} stDecodeBar;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * CDWiInit
 *
 * input param ：
 *               InBuf -- Verify string
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is Ok
 */
int  CDWiInit(char *InBuf);

/*
 * CDWiDecodeBar
 *
 * input param ：
 *               nWidth    -- bmp width
 *               nHeight   -- bmp height
 *               pBmpBit   -- bmp buffer
 *               nDataSize -- pData size
 * output param:
 *               nType     -- code type
 *               pData     -- barcode info
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is decode end
 *          > 0 is barcode length
 */
int  CDWiDecodeBar(int nWidth, int nHeight, char *pBmpBit, int *nType, char *pData, int nDataSize);

/*
 * CDWiDecodeMultiBar
 *
 * input param ：
 *               DecodeBar -- reference stDecodeBar struct
 * output param:
 *               nType     -- code type
 *               pData     -- barcode info
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is decode end
 *          > 0 is result number
 */
int CDWiDecodeMultiBar(stDecodeBar *DecodeBar);

/*
 * CDWiSetBar
 *
 * input param ：
 *               nClass    -- SET_CLASS_ENABLE / SET_CLASS_PROPERTY / SET_CLASS_LENGTH
 *               nType     -- code type define
 *               nVal      -- property value
 * output param:
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is set success
 */
int CDWiSetBar(int nClass, int nType, int nVal);

/*
 * CDWiGetVer
 *
 * input param ：
 *               nVerSize  -- pVer size, >= 36 Bytes
 * output param:
 *               pVer      -- output Version
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          >0  pVer length
 */
int CDWiGetVer(char* pVer, unsigned int nVerSize);


#endif /* _HDSD_H_ */

