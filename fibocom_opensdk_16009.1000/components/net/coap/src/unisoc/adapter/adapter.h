/*
 * coap_debug.h -- debug utilities
 *
 * Copyright (C) 2010-2011,2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

#ifndef COAP_ADAPTER_H_
#define COAP_ADAPTER_H_

typedef enum {
  LOG_EMERG=0, /**< Emergency */
  LOG_ALERT,   /**< Alert */
  LOG_CRIT,    /**< Critical */
  LOG_ERR,     /**< Error */
  LOG_WARNING, /**< Warning */
  LOG_NOTICE,  /**< Notice */
  LOG_INFO,    /**< Information */
  LOG_DEBUG,   /**< Debug */
  COAP_LOG_CIPHERS=LOG_DEBUG+2 /**< CipherInfo */
} coap_log_t;

coap_log_t coap_get_log_level(void);

void coap_log_impl(coap_log_t level, const char *format, ...);

#ifndef coap_log
#define coap_log(level, ...) do { \
     coap_log_impl(level, __VA_ARGS__); \
} while(0)
#endif

void coap_show_pdu(coap_log_t level, const coap_pdu_t *pdu);

size_t coap_print_addr(const struct coap_address_t *address,
                       unsigned char *buffer, size_t size);

int coap_debug_send_packet(void);


#endif /* COAP_ADAPTER_H_ */

