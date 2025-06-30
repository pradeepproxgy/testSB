#ifndef __OC_GNSS_H__
#define __OC_GNSS_H__

int32_t fibo_gnss_enable(uint8_t en);

int32_t fibo_gnss_nmea_get(uint8_t *nmea_data, int size);

int32_t fibo_gnss_nmea_freq_set(int freq);

int32_t fibo_gnss_set_agnss_mode(int mode);

int32_t fibo_gnss_set_satellite(int satellite);

int32_t fibo_gnss_get_satellite(void);

int32_t fibo_gnss_nmea_config(char *nmea_names, int on);

int32_t fibo_gnss_cmd_send(void *cmd, int len);

int32_t fibo_gnss_set_agnss_server(char* host, int host_len, int port, uint8_t srv_type);

#endif

