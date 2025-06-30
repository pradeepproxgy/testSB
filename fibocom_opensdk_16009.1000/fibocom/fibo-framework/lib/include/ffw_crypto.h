#ifndef LIB_INCLUDE_FFW_CRYPTO
#define LIB_INCLUDE_FFW_CRYPTO
/**
 * @file ffw_crypto.h
 * @author sundaqing (sundq@fibocom.com)
 * @brief AES加密和签名的API
 * @version 0.1
 * @date 2021-08-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <stdbool.h>

#include <ffw_buffer.h>

/// md5 length
#define FFW_MD5_DIGEST_LENGTH  (16)

/// sha1 length
#define FFW_SHA1_DIGEST_LENGTH  (20)

/// sha256 length
#define FFW_SHA224_DIGEST_LENGTH (32)

/// sha256 length
#define FFW_SHA256_DIGEST_LENGTH (32)

/// sha384 length
#define FFW_SHA384_DIGEST_LENGTH (64)

/// sha512 length
#define FFW_SHA512_DIGEST_LENGTH (64)

/// 签名类型定义
typedef enum
{
    FFW_MD_NONE = 0,  /**< None. */
    FFW_MD_MD2,       /**< The MD2 message digest. */
    FFW_MD_MD4,       /**< The MD4 message digest. */
    FFW_MD_MD5,       /**< The MD5 message digest. */
    FFW_MD_SHA1,      /**< The SHA-1 message digest. */
    FFW_MD_SHA224,    /**< The SHA-224 message digest. */
    FFW_MD_SHA256,    /**< The SHA-256 message digest. */
    FFW_MD_SHA384,    /**< The SHA-384 message digest. */
    FFW_MD_SHA512,    /**< The SHA-512 message digest. */
    FFW_MD_RIPEMD160, /**< The RIPEMD-160 message digest. */
} ffw_md_type_t;

/// 算法类型定义
typedef enum
{
    FFW_CIPHER_AES_128_ECB = 2, /**< AES cipher with 128-bit ECB mode. */
    FFW_CIPHER_AES_192_ECB,     /**< AES cipher with 192-bit ECB mode. */
    FFW_CIPHER_AES_256_ECB,     /**< AES cipher with 256-bit ECB mode. */
    FFW_CIPHER_AES_128_CBC,     /**< AES cipher with 128-bit CBC mode. */
    FFW_CIPHER_AES_192_CBC,     /**< AES cipher with 192-bit CBC mode. */
    FFW_CIPHER_AES_256_CBC,     /**< AES cipher with 256-bit CBC mode. */
    FFW_CIPHER_AES_128_CFB128,  /**< AES cipher with 128-bit CFB128 mode. */
    FFW_CIPHER_AES_192_CFB128,  /**< AES cipher with 192-bit CFB128 mode. */
    FFW_CIPHER_AES_256_CFB128,  /**< AES cipher with 256-bit CFB128 mode. */
    FFW_CIPHER_AES_128_CTR,     /**< AES cipher with 128-bit CTR mode. */
    FFW_CIPHER_AES_192_CTR,     /**< AES cipher with 192-bit CTR mode. */
    FFW_CIPHER_AES_256_CTR,     /**< AES cipher with 256-bit CTR mode. */
    FFW_CIPHER_AES_128_GCM,     /**< AES cipher with 128-bit GCM mode. */
    FFW_CIPHER_AES_192_GCM,     /**< AES cipher with 192-bit GCM mode. */
    FFW_CIPHER_AES_256_GCM,     /**< AES cipher with 256-bit GCM mode. */
} ffw_cipher_type_t;

/// 编码类型定义
typedef enum
{
    /// HEX编码方式
    FFW_ENCODE_TYPE_HEX = 0,

    /// BASE64编方式
    FFW_ENCODE_TYPE_BASE64,

    /// 原始数据
    FFW_ENCODE_TYPE_RAW
} ffw_encode_type_t;

/// padding type define
typedef enum
{
    FFW_PADDING_PKCS7 = 0,     /**< PKCS7 padding (default).        */
    FFW_PADDING_ONE_AND_ZEROS, /**< ISO/IEC 7816-4 padding.         */
    FFW_PADDING_ZEROS_AND_LEN, /**< ANSI X.923 padding.             */
    FFW_PADDING_ZEROS,         /**< Zero padding (not reversible). */
    FFW_PADDING_NONE,          /**< Never pad (full blocks only).   */
} ffw_cipher_padding_t;

/**
* @brief 生成一段数据的签名，支持MD5，SHA1, SHA224，SHA256， SHA384，SHA512
* 
* @param md_type 签名类型，参考 ::ffw_md_type_t
* @param hmac 是否需要hmac加密
* @param key hmac的秘钥，只有 @c hmac 为true的时候才有用
* @param key_len 秘钥的长度
* @param payload 签名的数据
* @param payload_len 签名的数据的长度
* @param result 签名结果
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sign(ffw_md_type_t md_type, bool hmac, const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result);

/**
 * @brief 生成文件的签名，支持MD5，SHA1, SHA224，SHA256， SHA384，SHA512，最终会编码成HEX或者base64
 * 
 * @param md_type 签名类型，参考 ::ffw_md_type_t
 * @param hmac 是否需要hmac加密
 * @param key hmac的秘钥，只有 @c hmac 为true的时候才有用
 * @param key_len 秘钥的长度
 * @param filename 文件路径
 * @param fmt 编码格式，参考 ::ffw_encode_type_t
 * @param out 签名结果，使用完毕需要调用 @c ffw_buffer_destroy 释放
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sign_file_fmt(ffw_md_type_t md_type, bool hmac, const uint8_t *key, int key_len, const char *filename, ffw_encode_type_t fmt, ffw_buffer_t *out);

/**
 * @brief 生成一段数据的签名，支持MD5，SHA1, SHA224，SHA256， SHA384，SHA512，最终会编码成HEX或者base64
 * 
 * @param md_type 签名类型，参考 ::ffw_md_type_t
 * @param hmac 是否需要hmac加密
 * @param key hmac的秘钥，只有 @c hmac 为true的时候才有用
 * @param key_len 秘钥的长度
 * @param payload 签名的数据
 * @param payload_len 签名的数据的长度
 * @param fmt 编码格式，参考 ::ffw_encode_type_t
 * @param out 签名结果，使用完毕需要调用 @c ffw_buffer_destroy 释放
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sign_fmt(ffw_md_type_t md_type, bool hmac, const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, ffw_encode_type_t fmt, ffw_buffer_t *out);

/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_sha1_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);


/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_sha224_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);

/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_sha256_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);

/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_sha384_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);

/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_sha512_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);

/**
* @brief 
* 
* @param key 
* @param key_len 
* @param payload 
* @param payload_len 
* @param result 
* @param len 
* @return int 
 */
int ffw_hmac_md5_sign(const uint8_t *key, int key_len, const uint8_t *payload, int payload_len, uint8_t *result, int len);

/**
 * @brief 
 * 
 * @param cipher_id 
 * @return ffw_handle_t 
 */
ffw_handle_t ffw_cipher_create_context(int cipher_id);

/**
 * @brief 
 * 
 * @param ctx 
 * @param key 
 * @param keylen 
 * @param encrypt 
 * @return int 
 */
int ffw_cipher_set_key(ffw_handle_t ctx, const uint8_t *key, int keylen, bool encrypt);

/**
 * @brief 
 * 
 * @param ctx 
 * @param iv 
 * @param iv_len 
 * @return int 
 */
int ffw_cipher_set_iv(ffw_handle_t ctx, const uint8_t *iv, int iv_len);

/**
 * @brief 
 * 
 * @param ctx 
 * @param padding 
 * @return int 
 */
int ffw_cipher_set_padding(ffw_handle_t ctx, ffw_cipher_padding_t padding);

/**
 * @brief 
 * 
 * @param ctx 
 * @param input 
 * @param ilen 
 * @param output 
 * @param olen 
 * @return int 
 */
int ffw_cipher_encypt_decrypt(ffw_handle_t ctx, const uint8_t *input, int ilen, uint8_t *output, int *olen);

/**
 * @brief 
 * 
 * @param ctx 
 * @param input 
 * @param ilen 
 * @param fmt_in 
 * @param out 
 * @param fmt_out 
 * @return int 
 */
int ffw_cipher_encypt_decrypt1(ffw_handle_t ctx, uint8_t *input, int ilen, ffw_encode_type_t fmt_in, ffw_buffer_t *out, ffw_encode_type_t fmt_out);

/**
 * @brief 
 * 
 * @param ctx 
 * @return int 
 */
int ffw_cipher_free(ffw_handle_t ctx);

#endif /* LIB_INCLUDE_FFW_CRYPTO */
