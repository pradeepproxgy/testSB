/**
 * \file aes.h
 */
#ifndef XYSSL_AES_H
#define XYSSL_AES_H

#define AES_ENCRYPT 1
#define AES_DECRYPT 0

/**
 * \brief          AES context structure
 */
typedef struct
{
    int nr;                /*!<  number of rounds  */
    unsigned long *rk;     /*!<  AES round keys    */
    unsigned long buf[68]; /*!<  unaligned data    */
} aes_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keysize  must be 128, 192 or 256
 */
void aes_setkey_enc(aes_context *ctx, unsigned char *key, int keysize);

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keysize  must be 128, 192 or 256
 */
void aes_setkey_dec(aes_context *ctx, unsigned char *key, int keysize);

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 */
void aes_crypt_ecb(aes_context *ctx,
                   int mode,
                   unsigned char input[16],
                   unsigned char output[16]);

/**
 * \brief          AES-CBC buffer encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
void aes_crypt_cbc(aes_context *ctx,
                   int mode,
                   int length,
                   unsigned char iv[16],
                   unsigned char *input,
                   unsigned char *output);

/**
 * \brief          AES-CFB buffer encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
void aes_crypt_cfb(aes_context *ctx,
                   int mode,
                   int length,
                   int *iv_off,
                   unsigned char iv[16],
                   unsigned char *input,
                   unsigned char *output);

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int aes_self_test(int verbose);

#ifdef __cplusplus
}
#endif

#endif /* aes.h */
