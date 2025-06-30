#ifndef A1CC4F62_2B62_4EC1_BFB8_70188C59B263
#define A1CC4F62_2B62_4EC1_BFB8_70188C59B263

#include <stdio.h>
#include <stdint.h>

#include "ffw_plat_header.h"
#include <ffw_audio_plat.h>

typedef struct
{
    uint32_t length; /* filelen */
    uint32_t rate;
    ffw_audio_fmt_t format; /* see WAV_FMT_* */
    uint16_t channels;
    uint16_t audio_format; /*PCM */
    uint32_t pcm_length; /*PCM data length   (original data)*/
    uint32_t pcm_offset;/*The original belongs to the offset in the file*/
} ffw_wave_header_t;

/**
* @brief 
* 
* @param fd 
* @param wavheader 
* @return int 
 */
int ffw_wav_parsed_header(char *data, int size,ffw_wave_header_t *wavheader );

#if 0
/**
* @brief 
* 
* @param wavfile 
* @param wavheader 
* @return int 
 */
int ffw_wavfile_parsed_header(const char *wavfile, ffw_wave_header_t *wavheader);

/**
* @brief 
* 
* @param fd 
* @return int 
 */
int ffw_mp3_strip_idv3(ffw_fs_handle_t fd);
#endif


#endif /* A1CC4F62_2B62_4EC1_BFB8_70188C59B263 */
