#ifndef OC_MBEDTLS_H
#define OC_MBEDTLS_H

/*****************************************************************
*   Copyright (C), 2022, Xian Fibocom Wireless Inc
*                  All rights reserved.
*   FileName    :oc_mbedtls.h
*   Author      :qiaoyu
*   Created     :2022-08-08
*   Description :mbedtls opencpu api declaration.
*****************************************************************/
#include "mbedtls/aes.h"
#include "mbedtls/des.h"
#include "mbedtls/md5.h"
#include "mbedtls/md.h"
#include "mbedtls/sha1.h"
#include "mbedtls/pk.h"
#include "mbedtls/gcm.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"


typedef enum {
        TLS_RET_SUCCESS                     = 0,     //tls Successful return
        TLS_ERR_SSL_INIT                    = -101,  // TLS/SSL init fail
        TLS_ERR_SSL_CERT                    = -102,  // TLS/SSL certificate issue
        TLS_ERR_SSL_CONNECT                 = -103,  // TLS/SSL connect fail
        TLS_ERR_SSL_CONNECT_TIMEOUT         = -104,  // TLS/SSL connect timeout
        TLS_ERR_SSL_WRITE_TIMEOUT           = -105,  // TLS/SSL write timeout
        TLS_ERR_SSL_WRITE                   = -106,  // TLS/SSL write error
        TLS_ERR_SSL_READ_TIMEOUT            = -107,  // TLS/SSL read timeout
        TLS_ERR_SSL_READ                    = -108,  // TLS/SSL read error
        TLS_ERR_SSL_NOTHING_TO_READ         = -109,  // TLS/SSL nothing to read

        TLS_ERR_TCP_SOCKET_FAILED           = -201,  // TLS TCP socket connect fail
        TLS_ERR_TCP_UNKNOWN_HOST            = -202,  // TCP unknown host (DNS fail)
        TLS_ERR_TCP_CONNECT                 = -203,  // TCP/UDP socket connect fail
}tls_err;

typedef struct {
    const char *ca_crt;
    uint16_t    ca_crt_len;
#ifdef AUTH_MODE_CERT
    /**
     * Device with certificate
     */
    const char *cert_file;  // public certificate file
    const char *key_file;   // pravite certificate file
#else
    /**
     * Device with PSK
     */
    const char *psk;     // PSK string
    const char *psk_id;  // PSK ID
#endif

    size_t psk_length;  // PSK length

    unsigned int timeout_ms;  // SSL handshake timeout in millisecond

    /// sni 设置
    const char *sni;

} GAPP_TLS_CONNECT;

typedef GAPP_TLS_CONNECT  GAPP_TLSConnectParams;

uintptr_t fibo_tls_connect(GAPP_TLSConnectParams *pConnectParams, const char *host, int port);
void fibo_tls_disconnect(uintptr_t handle);

int fibo_tls_write(uintptr_t handle, unsigned char *msg, size_t totalLen, uint32_t timeout_ms, size_t *written_len);
int fibo_tls_read(uintptr_t handle, unsigned char *msg, size_t totalLen, uint32_t timeout_ms, size_t *read_len);

int fibo_mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx, const unsigned char *key,
                    unsigned int keybits );

int fibo_mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx, const unsigned char *key,
                    unsigned int keybits );

int fibo_mbedtls_aes_crypt_ecb( mbedtls_aes_context *ctx,
                           int mode,
                           const unsigned char input[16],
                           unsigned char output[16] );

int fibo_mbedtls_aes_crypt_cbc( mbedtls_aes_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[16],
                    const unsigned char *input,
                    unsigned char *output );

int fibo_mbedtls_aes_crypt_cfb128( mbedtls_aes_context *ctx,
                       int mode,
                       size_t length,
                       size_t *iv_off,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );


int fibo_mbedtls_aes_crypt_cfb8( mbedtls_aes_context *ctx,
                            int mode,
                            size_t length,
                            unsigned char iv[16],
                            const unsigned char *input,
                            unsigned char *output );

int fibo_mbedtls_aes_crypt_ctr( mbedtls_aes_context *ctx,
                       size_t length,
                       size_t *nc_off,
                       unsigned char nonce_counter[16],
                       unsigned char stream_block[16],
                       const unsigned char *input,
                       unsigned char *output );

int fibo_mbedtls_des_setkey_enc( mbedtls_des_context *ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE] );

int fibo_mbedtls_des_setkey_dec( mbedtls_des_context *ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE] );
int fibo_mbedtls_des3_set2key_enc( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2] );
int fibo_mbedtls_des3_set2key_dec( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2] );

int fibo_mbedtls_des3_set3key_enc( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3] );

int fibo_mbedtls_des3_set3key_dec( mbedtls_des3_context *ctx,
                      const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3] );

int fibo_mbedtls_des_crypt_ecb( mbedtls_des_context *ctx,
                    const unsigned char input[8],
                    unsigned char output[8] );

int fibo_mbedtls_des_crypt_cbc( mbedtls_des_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[8],
                    const unsigned char *input,
                    unsigned char *output );

int fibo_mbedtls_des3_crypt_ecb( mbedtls_des3_context *ctx,
                     const unsigned char input[8],
                     unsigned char output[8] );

int fibo_mbedtls_des3_crypt_cbc( mbedtls_des3_context *ctx,
                     int mode,
                     size_t length,
                     unsigned char iv[8],
                     const unsigned char *input,
                     unsigned char *output );

void fibo_mbedtls_rsa_init( mbedtls_rsa_context *ctx,
               int padding,
               int hash_id );

int fibo_mbedtls_rsa_import( mbedtls_rsa_context *ctx,
                        const mbedtls_mpi *N,
                        const mbedtls_mpi *P, const mbedtls_mpi *Q,
                        const mbedtls_mpi *D, const mbedtls_mpi *E );

int fibo_mbedtls_rsa_import_raw( mbedtls_rsa_context *ctx,
                            unsigned char const *N, size_t N_len,
                            unsigned char const *P, size_t P_len,
                            unsigned char const *Q, size_t Q_len,
                            unsigned char const *D, size_t D_len,
                            unsigned char const *E, size_t E_len );

int fibo_mbedtls_rsa_complete( mbedtls_rsa_context *ctx );

int fibo_mbedtls_rsa_export( const mbedtls_rsa_context *ctx,
                        mbedtls_mpi *N, mbedtls_mpi *P, mbedtls_mpi *Q,
                        mbedtls_mpi *D, mbedtls_mpi *E );

int fibo_mbedtls_rsa_export_raw( const mbedtls_rsa_context *ctx,
                            unsigned char *N, size_t N_len,
                            unsigned char *P, size_t P_len,
                            unsigned char *Q, size_t Q_len,
                            unsigned char *D, size_t D_len,
                            unsigned char *E, size_t E_len );

int fibo_mbedtls_rsa_export_crt( const mbedtls_rsa_context *ctx,
                            mbedtls_mpi *DP, mbedtls_mpi *DQ, mbedtls_mpi *QP );

void fibo_mbedtls_rsa_set_padding( mbedtls_rsa_context *ctx, int padding,
                              int hash_id );

size_t fibo_mbedtls_rsa_get_len( const mbedtls_rsa_context *ctx );

int fibo_mbedtls_rsa_gen_key( mbedtls_rsa_context *ctx,
                 int (*f_rng)(void *, unsigned char *, size_t),
                 void *p_rng,
                 unsigned int nbits, int exponent );

int fibo_mbedtls_rsa_check_pubkey( const mbedtls_rsa_context *ctx );

int fibo_mbedtls_rsa_check_privkey( const mbedtls_rsa_context *ctx );

int fibo_mbedtls_rsa_check_pub_priv( const mbedtls_rsa_context *pub,
                                const mbedtls_rsa_context *prv );

int fibo_mbedtls_rsa_public( mbedtls_rsa_context *ctx,
                const unsigned char *input,
                unsigned char *output );

int fibo_mbedtls_rsa_private( mbedtls_rsa_context *ctx,
                 int (*f_rng)(void *, unsigned char *, size_t),
                 void *p_rng,
                 const unsigned char *input,
                 unsigned char *output );

int fibo_mbedtls_rsa_pkcs1_encrypt( mbedtls_rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t ilen,
                       const unsigned char *input,
                       unsigned char *output );

int fibo_mbedtls_rsa_rsaes_pkcs1_v15_encrypt( mbedtls_rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode, size_t ilen,
                                 const unsigned char *input,
                                 unsigned char *output );

int fibo_mbedtls_rsa_rsaes_oaep_encrypt( mbedtls_rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t ilen,
                            const unsigned char *input,
                            unsigned char *output );

int fibo_mbedtls_rsa_pkcs1_decrypt( mbedtls_rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t *olen,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t output_max_len);

int fibo_mbedtls_rsa_rsaes_pkcs1_v15_decrypt( mbedtls_rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode, size_t *olen,
                                 const unsigned char *input,
                                 unsigned char *output,
                                 size_t output_max_len);

int fibo_mbedtls_rsa_rsaes_oaep_decrypt( mbedtls_rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t output_max_len );

int fibo_mbedtls_rsa_pkcs1_sign( mbedtls_rsa_context *ctx,
                    int (*f_rng)(void *, unsigned char *, size_t),
                    void *p_rng,
                    int mode,
                    mbedtls_md_type_t md_alg,
                    unsigned int hashlen,
                    const unsigned char *hash,
                    unsigned char *sig );

int fibo_mbedtls_rsa_rsassa_pkcs1_v15_sign( mbedtls_rsa_context *ctx,
                               int (*f_rng)(void *, unsigned char *, size_t),
                               void *p_rng,
                               int mode,
                               mbedtls_md_type_t md_alg,
                               unsigned int hashlen,
                               const unsigned char *hash,
                               unsigned char *sig );

int fibo_mbedtls_rsa_rsassa_pss_sign( mbedtls_rsa_context *ctx,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng,
                         int mode,
                         mbedtls_md_type_t md_alg,
                         unsigned int hashlen,
                         const unsigned char *hash,
                         unsigned char *sig );

int fibo_mbedtls_rsa_pkcs1_verify( mbedtls_rsa_context *ctx,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng,
                      int mode,
                      mbedtls_md_type_t md_alg,
                      unsigned int hashlen,
                      const unsigned char *hash,
                      const unsigned char *sig );

int fibo_mbedtls_rsa_rsassa_pkcs1_v15_verify( mbedtls_rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode,
                                 mbedtls_md_type_t md_alg,
                                 unsigned int hashlen,
                                 const unsigned char *hash,
                                 const unsigned char *sig );

int fibo_mbedtls_rsa_rsassa_pss_verify( mbedtls_rsa_context *ctx,
                           int (*f_rng)(void *, unsigned char *, size_t),
                           void *p_rng,
                           int mode,
                           mbedtls_md_type_t md_alg,
                           unsigned int hashlen,
                           const unsigned char *hash,
                           const unsigned char *sig );

int fibo_mbedtls_rsa_rsassa_pss_verify_ext( mbedtls_rsa_context *ctx,
                               int (*f_rng)(void *, unsigned char *, size_t),
                               void *p_rng,
                               int mode,
                               mbedtls_md_type_t md_alg,
                               unsigned int hashlen,
                               const unsigned char *hash,
                               mbedtls_md_type_t mgf1_hash_id,
                               int expected_salt_len,
                               const unsigned char *sig );

int fibo_mbedtls_rsa_copy( mbedtls_rsa_context *dst, const mbedtls_rsa_context *src );

int fibo_mbedtls_md5_starts_ret( mbedtls_md5_context *ctx );

int fibo_mbedtls_md5_update_ret( mbedtls_md5_context *ctx,
                            const unsigned char *input,
                            size_t ilen );

int fibo_mbedtls_md5_finish_ret( mbedtls_md5_context *ctx,
                            unsigned char output[16] );

int fibo_mbedtls_md_setup( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info, int hmac );

int fibo_mbedtls_md_hmac_starts( mbedtls_md_context_t *ctx, const unsigned char *key, size_t keylen );

int fibo_mbedtls_md_hmac_update( mbedtls_md_context_t *ctx, const unsigned char *input, size_t ilen );

int fibo_mbedtls_md_hmac_finish( mbedtls_md_context_t *ctx, unsigned char *output );

int fibo_mbedtls_sha1_ret( const unsigned char *input,
                      size_t ilen,
                      unsigned char output[20] );

void fibo_mbedtls_rsa_free( mbedtls_rsa_context *ctx );

void fibo_mbedtls_md_free( mbedtls_md_context_t *ctx );

const mbedtls_md_info_t *fibo_mbedtls_md_info_from_type( mbedtls_md_type_t md_type );

void fibo_mbedtls_pk_init( mbedtls_pk_context *ctx );

int fibo_mbedtls_pk_parse_key( mbedtls_pk_context *pk,
                  const unsigned char *key, size_t keylen,
                  const unsigned char *pwd, size_t pwdlen );

void fibo_mbedtls_pk_free( mbedtls_pk_context *ctx );

int fibo_mbedtls_pk_encrypt( mbedtls_pk_context *ctx,
                const unsigned char *input, size_t ilen,
                unsigned char *output, size_t *olen, size_t osize,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

int fibo_mbedtls_pk_decrypt( mbedtls_pk_context *ctx,
                const unsigned char *input, size_t ilen,
                unsigned char *output, size_t *olen, size_t osize,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

void fibo_mbedtls_ctr_drbg_free( mbedtls_ctr_drbg_context *ctx );

int fibo_mbedtls_ctr_drbg_seed( mbedtls_ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );

int fibo_mbedtls_ctr_drbg_random( void *p_rng, unsigned char *output, size_t output_len );

void fibo_mbedtls_ctr_drbg_init( mbedtls_ctr_drbg_context *ctx );

int fibo_mbedtls_pk_sign( mbedtls_pk_context *ctx, mbedtls_md_type_t md_alg,
             const unsigned char *hash, size_t hash_len,
             unsigned char *sig, size_t *sig_len,
             int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

int fibo_mbedtls_pk_parse_public_key( mbedtls_pk_context *ctx,
                         const unsigned char *key, size_t keylen );

void fibo_mbedtls_entropy_init( mbedtls_entropy_context *ctx );

int fibo_mbedtls_entropy_add_source( mbedtls_entropy_context *ctx,
                        mbedtls_entropy_f_source_ptr f_source, void *p_source,
                        size_t threshold, int strong );

int fibo_mbedtls_entropy_func( void *data, unsigned char *output, size_t len );

void fibo_mbedtls_net_init( mbedtls_net_context *ctx );

void fibo_mbedtls_ssl_conf_rng( mbedtls_ssl_config *conf,
                  int (*f_rng)(void *, unsigned char *, size_t),
                  void *p_rng );

void fibo_mbedtls_ssl_conf_cert_profile( mbedtls_ssl_config *conf,
                                    const mbedtls_x509_crt_profile *profile );

int fibo_mbedtls_ssl_setup( mbedtls_ssl_context *ssl,
                       const mbedtls_ssl_config *conf );

void fibo_mbedtls_ssl_free( mbedtls_ssl_context *ssl );

void fibo_mbedtls_ssl_set_bio( mbedtls_ssl_context *ssl,
        void *p_bio,
        mbedtls_ssl_send_t *f_send,
        mbedtls_ssl_recv_t *f_recv,
        mbedtls_ssl_recv_timeout_t *f_recv_timeout );

int fibo_mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len );

int fibo_mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len );

void fibo_mbedtls_entropy_free( mbedtls_entropy_context *ctx );


int fibo_mbedtls_x509_crt_info(char *buf, size_t size, const char *prefix, const mbedtls_x509_crt *crt);


int fibo_mbedtls_platform_set_calloc_free( void * (*calloc_func)( size_t, size_t ),
                              void (*free_func)( void * ) );

int fibo_mbedtls_platform_set_time( mbedtls_time_t (*time_func)( mbedtls_time_t* time ) );


void fibo_mbedtls_ssl_config_init( mbedtls_ssl_config *conf );

void fibo_mbedtls_ssl_init( mbedtls_ssl_context *ssl );


void fibo_mbedtls_x509_crt_init(mbedtls_x509_crt *crt);


void fibo_mbedtls_pk_init(mbedtls_pk_context *ctx);

void fibo_mbedtls_entropy_init( mbedtls_entropy_context *ctx );

void fibo_mbedtls_ctr_drbg_init( mbedtls_ctr_drbg_context *ctx );

int fibo_mbedtls_ssl_config_defaults( mbedtls_ssl_config *conf,
                                 int endpoint, int transport, int preset );

void fibo_mbedtls_ssl_conf_cert_profile( mbedtls_ssl_config *conf,
                                    const mbedtls_x509_crt_profile *profile ); 

void fibo_mbedtls_ssl_conf_dbg( mbedtls_ssl_config *conf,
                  void (*f_dbg)(void *, int, const char *, int, const char *),
                  void  *p_dbg );    

int fibo_mbedtls_ctr_drbg_seed( mbedtls_ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );

void fibo_mbedtls_ssl_conf_rng( mbedtls_ssl_config *conf,
                  int (*f_rng)(void *, unsigned char *, size_t),
                  void *p_rng );  

void fibo_mbedtls_ssl_conf_verify( mbedtls_ssl_config *conf,
                     int (*f_vrfy)(void *, mbedtls_x509_crt *, int, uint32_t *),
                     void *p_vrfy );                                                                             


int fibo_mbedtls_x509_crt_parse( mbedtls_x509_crt *chain, const unsigned char *buf, size_t buflen );


int fibo_mbedtls_pk_parse_key(mbedtls_pk_context *ctx,
                                     const unsigned char *key, size_t keylen,
                                     const unsigned char *pwd, size_t pwdlen);


int fibo_mbedtls_ssl_conf_psk( mbedtls_ssl_config *conf,
                const unsigned char *psk, size_t psk_len,
                const unsigned char *psk_identity, size_t psk_identity_len );    

void fibo_mbedtls_ssl_conf_ciphersuites( mbedtls_ssl_config *conf,
                                   const int *ciphersuites );  

void fibo_mbedtls_ssl_conf_authmode( mbedtls_ssl_config *conf, int authmode );


int fibo_mbedtls_ssl_set_hostname( mbedtls_ssl_context *ssl, const char *hostname );

void fibo_mbedtls_ssl_conf_max_version( mbedtls_ssl_config *conf, int major, int minor );

void fibo_mbedtls_ssl_conf_min_version( mbedtls_ssl_config *conf, int major, int minor );

void fibo_mbedtls_ssl_conf_transport( mbedtls_ssl_config *conf, int transport );


int fibo_mbedtls_ssl_read( mbedtls_ssl_context *ssl, unsigned char *buf, size_t len );

int fibo_mbedtls_ssl_write( mbedtls_ssl_context *ssl, const unsigned char *buf, size_t len );

void fibo_mbedtls_ssl_set_timer_cb( mbedtls_ssl_context *ssl,
                               void *p_timer,
                               mbedtls_ssl_set_timer_t *f_set_timer,
                               mbedtls_ssl_get_timer_t *f_get_timer );

void fibo_mbedtls_ssl_set_bio( mbedtls_ssl_context *ssl,
                          void *p_bio,
                          mbedtls_ssl_send_t *f_send,
                          mbedtls_ssl_recv_t *f_recv,
                          mbedtls_ssl_recv_timeout_t *f_recv_timeout );


int fibo_mbedtls_ssl_setup( mbedtls_ssl_context *ssl,
                       const mbedtls_ssl_config *conf );

int fibo_mbedtls_ssl_handshake( mbedtls_ssl_context *ssl );

void fibo_mbedtls_ssl_conf_ca_chain( mbedtls_ssl_config *conf,
                               mbedtls_x509_crt *ca_chain,
                               mbedtls_x509_crl *ca_crl );


int fibo_mbedtls_ssl_conf_own_cert( mbedtls_ssl_config *conf,
                              mbedtls_x509_crt *own_cert,
                              mbedtls_pk_context *pk_key );

uint32_t fibo_mbedtls_ssl_get_verify_result( const mbedtls_ssl_context *ssl );

int fibo_mbedtls_ssl_get_session( const mbedtls_ssl_context *ssl, mbedtls_ssl_session *session );


int fibo_mbedtls_ssl_set_session( mbedtls_ssl_context *ssl, const mbedtls_ssl_session *session );

int fibo_mbedtls_ssl_close_notify( mbedtls_ssl_context *ssl );

void fibo_mbedtls_ssl_config_free( mbedtls_ssl_config *conf );


void fibo_mbedtls_ssl_free( mbedtls_ssl_context *ssl );

void fibo_mbedtls_x509_crt_free( mbedtls_x509_crt *crt );

void fibo_mbedtls_pk_free( mbedtls_pk_context *ctx );

void fibo_mbedtls_entropy_free( mbedtls_entropy_context *ctx );

void fibo_mbedtls_ctr_drbg_free( mbedtls_ctr_drbg_context *ctx );

const int *fibo_mbedtls_ssl_list_ciphersuites( void );

const char *fibo_mbedtls_ssl_get_ciphersuite_name( const int ciphersuite_id );

void fibo_mbedtls_debug_set_threshold( int threshold );

int fibo_mbedtls_gcm_setkey( mbedtls_gcm_context *ctx, const unsigned char *key, unsigned int keybits );

int fibo_mbedtls_gcm_crypt_and_tag( mbedtls_gcm_context *ctx,
                       int mode,
                       size_t length,
                       const unsigned char *iv,
                       size_t iv_len,
                       const unsigned char *add,
                       size_t add_len,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t tag_len,
                       unsigned char *tag );



#endif
