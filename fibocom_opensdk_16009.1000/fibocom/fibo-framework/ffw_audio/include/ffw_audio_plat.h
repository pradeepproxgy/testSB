#ifndef BB2DE9E3_C9F9_4CB1_8990_FE699A97B348
#define BB2DE9E3_C9F9_4CB1_8990_FE699A97B348

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ffw_types.h"

typedef void (*ffw_playback_finish_t)(int result, void *arg);
typedef struct ffw_audio_params_s ffw_audio_params_t;

/// 采样格式
typedef enum ffw_audio_format ffw_audio_fmt_t;

typedef enum
{
    /// pcm
    FFW_AUDIO_FMT_PCM = 0,

    /// wav
    FFW_AUDIO_FMT_WAV,

    /// mp3
    FFW_AUDIO_FMT_MP3,

    /// amr
    FFW_AUDIO_FMT_AMR,

    /// awb
    FFW_AUDIO_FMT_AWB,

    /// aac
    FFW_AUDIO_FMT_AAC,

    /// ts
    FFW_AUDIO_FMT_TS,

    /// unknown
    FFW_AUDIO_FMT_UNKNOWN
} ffw_audio_format_t;

typedef enum
{
    /// voice
    FFW_AUDIO_VOL_VOICE = 1,

    /// tone
    FFW_AUDIO_VOL_TONE,

    /// voice and tone
    FFW_AUDIO_VOL_VOICE_TONE,

    /// MIDI
    FFW_AUDIO_VOL_MIDI,

    /// voice and midi
    FFW_AUDIO_VOL_VOICE_MIDI,

    /// tone and midi
    FFW_AUDIO_VOL_TONE_MIDI,

    /// voice tone and midi
    FFW_AUDIO_VOL_VOICE_TONE_MIDI,

} ffw_audio_vol_type_t;

/// 播放器格式参数
enum ffw_audio_format
{
    FFW_FMT_S8,
    FFW_FMT_U8,
    FFW_FMT_S16_LE,
    FFW_FMT_S16_BE,
    FFW_FMT_U16_LE,
    FFW_FMT_U16_BE,
    FFW_FMT_S24_LE,
    FFW_FMT_S24_BE,
    FFW_FMT_U24_LE,
    FFW_FMT_U24_BE,
    FFW_FMT_S32_LE,
    FFW_FMT_S32_BE,
    FFW_FMT_U32_LE,
    FFW_FMT_U32_BE
};

/// 播放器参数定义
struct ffw_audio_params_s
{
    /// 通道数
    uint8_t channels;

    /// 格式
    ffw_audio_fmt_t format;

    /// 采样率
    uint32_t sample_rate;
};

/**
* @brief 
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_create(ffw_handle_t *player);

/**
* @brief 设置播放器的参数
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_set_params(ffw_handle_t player, ffw_audio_params_t *params);

/**
* @brief 
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_delete(ffw_handle_t player);

/**
* @brief 是否支持特定格式
* 
* @param fmt 
* @return bool 
 */
bool ffw_audio_player_plat_support(ffw_audio_format_t fmt);

/**
* @brief 
* 
* @param player 
* @param fmt 
* @param on_finish 
* @param arg 
* @return int 
 */
int ffw_audio_player_plat_play_stream(ffw_handle_t player, ffw_audio_format_t fmt, ffw_playback_finish_t on_finish, void *arg);

/**
* @brief 
* 
* @param player 
* @param data 
* @param size 
* @return int 
 */
int ffw_audio_player_plat_play_stream_send_data(ffw_handle_t player, const void *data, uint32_t size);

/**
* @brief 
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_pause(ffw_handle_t player);

/**
* @brief 
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_resume(ffw_handle_t player);

/**
* @brief 
* 
* @param player 
* @return int 
 */
int ffw_audio_player_plat_stop(ffw_handle_t player);

/**
* @brief 
*  
* @return int 
 */
int ffw_audio_tone_start();

/**
* @brief 
*  
* @return int 
 */
int ffw_audio_tone_stop();
#endif /* BB2DE9E3_C9F9_4CB1_8990_FE699A97B348 */
