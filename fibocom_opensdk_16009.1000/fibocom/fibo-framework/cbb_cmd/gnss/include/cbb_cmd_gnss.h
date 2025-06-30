#ifndef __CBB_CMD_GNSS_H__
#define __CBB_CMD_GNSS_H__

#include "ffw_types.h"
#include "ffw_crypto.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    ffw_handle_t engine;
    char    url[512];
    /* save download data */
    char *  data;
    int len;
    int total_len;
    int state;
} gnss_info;

int cbb_cmd_gtgpspower(uint8_t mode);

int cbb_cmd_gtgpspower_get();

int cbb_cmd_gtgps(uint8_t* nmea, int len, int*out_len);

int cbb_cmd_get_gtgps_by_name(const char*nmea_name, uint8_t* nmea, int len, int* len_out);

// key -- value
// 0: supl Version  -- 0: SUPL1.0.
//                  -- 1: SUPL2.0 (default)
// 1: xtra Enable   -- 0: xtra disable (default)
//                  -- 1: xtra enable
// 2: satellite     -- 0: GPS+GLONASS
//                        GPRMC GPGSV GPGSA GPGGA GLGSV
//                  -- 1: GPS+BEIDOU
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA
//                  -- 2: GPS+GALILEO
//                        GPRMC GPGSV GPGSA GPGGA GARMC GAGSV GAGSA GAGGA
//                  -- 3: GPS+QZSS
//                        GPRMC GPGSV GPGSA GPGGA PQGSV PQGSA
//                  -- 4: GPS+BEIDOU+GALILEO
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GARMC GAGSV GAGSA GAGGA
//                  -- 5: GPS+BEIDOU+GLONASS
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GLGSV
//                  -- 6: GPS+BEIDOU+QZSS
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA PQGSV PQGSA
//                  -- 7: GPS+GLONASS+GALILEO
//                        GPRMC GPGSV GPGSA GPGGA GLGSV GARMC GAGSV GAGSA GAGGA
//                  -- 8: GPS+GALILEO+QZSS
//                        GPRMC GPGSV GPGSA GPGGA GARMC GAGSV GAGSA GAGGA PQGSV PQGSA
//                  -- 9: GPS+GLONASS+QZSS
//                        GPRMC GPGSV GPGSA GPGGA GLGSV PQGSV PQGSA
//                  -- 10:GPS+BEIDOU+GALILEO+GLONASS
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GARMC GAGSV GAGSA GAGGA GLGSV
//                  -- 11:GPS+BEIDOU+GLONASS+QZSS
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GLGSV PQGSV PQGSA
//                  -- 12 GPS+BEIDOU+GALILEO+QZSS
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GARMC GAGSV GAGSA GAGGA PQGSV PQGSA
//                  -- 13:GPS+GALILEO+GLONASS+QZSS
//                        GPRMC GPGSV GPGSA GPGGA GARMC GAGSV GAGSA GAGGA GLGSVPQGSV PQGSA
//                  -- 14:GPS+BEIDOU+GALILEO+GALONASS+QZSS (default)
//                        GPRMC GPGSV GPGSA GPGGA BDGSV BDGSA GARMC GAGSV GAGSA GAGGA GLGSA GLGSV PQGSV PQGSA
//                  -- 15:GPS
//                        GPRMC GPGSV GPGSA GPGGA
//                  -- 16:BEIDOU
//                        BD---
//                  -- 17:GLONASS
//                        GL---
//                  -- 18:GALILEO
//                        GA---
//                  -- 19:BEIDOU+GALILEO
//                        BD--- GA---
//                  -- 20:GLONASS+GALILEO
//                        GL--- GA---
//                  -- 21:BEIDOU(B1C)
//                        BD---
//                  -- 22:BEIDOU(B1C)+GALILEI
//                        BD--- GA---
//                  -- 23:GPS+BEIDOU(B1C)
//                        GP--- BD---
//                  -- 24:GPS+BEIDOU(B1C)+GLONASS
//                        GP--- BD--- GL---
//                  -- 25:GPS+BEIDOU(B1C)+GALILEO
//                        GP--- BD--- GA---
//                  -- 26:GPS+BEIDOU(B1C)+GLONASS+GALILEO
//                        GP--- BD--- GL--- GA---
// 3: supl Certification Enable -- 0: SUPL Certification disable (default)
//                              -- 1: SUPL Certification enable
int cbb_cmd_gtgpscfg(int key, int value);

int cbb_cmd_gtgpscfg_get(int* satellite);

int cbb_cmd_gtgpsnmeacfg(char* nmeas, bool on);

int cbb_cmd_gtgpsnmeacfg_get(char* nmeas, int len);

int cbb_cmd_gtgpsnmeafreq(int freq);

int cbb_cmd_gtgpsnmeafreq_get();

int cbb_cmd_gtagpsserv(char* host, int host_len, int port, uint8_t srv_type);
int cbb_cmd_gtagpsserv_get(char** host, int* port, uint8_t* srv_type);

int cbb_cmd_gtgpsepo(int32_t mode);

int32_t cbb_cmd_gtgpsepo_get();

int cbb_cmd_gtgpsset(int format, const char *set_command);

int cbb_cmd_gtgnssoat(void *engine, const char *filename, const char *md5sum, int32_t length);

int cbb_cmd_gtgnssver(void *engine);

void gnss_firmware_dl_http_handle(void *engine, const char *url);


#endif

