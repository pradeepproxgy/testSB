#ifndef A70B7324_5FF4_45C9_9AF1_57FDD07CE6FD
#define A70B7324_5FF4_45C9_9AF1_57FDD07CE6FD

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ffw_buffer.h"
#include "ffw_audio_plat.h"
#include "ffw_types.h"


typedef enum
{
    AUDIO_PLAYER_STATUS_INIT = 0,

    AUDIO_PLAYER_STATUS_PLAYING,

    AUDIO_PLAYER_STATUS_URL_PLAYING,

    AUDIO_PLAYER_STATUS_URL_PLAYING_DOWN_STOP,

    AUDIO_PLAYER_STATUS_URL_PLAYING_DOWN_FNI,

    AUDIO_PLAYER_STATUS_FINISH
} audio_player_status_t;


/**
* @brief 
* 
* @return int 
 */
int ffw_audio_init(void);

/**
* @brief 
* 
* @param format 
* @param filename 
* @param on_finish 
* @return int 
 */
int ffw_audio_play_file(ffw_audio_format_t format, const char *filename, ffw_playback_finish_t on_finish, void *arg);

/**
* @brief 
* 
* @param format 
* @param data 
* @param size 
* @param on_finish 
* @return int 
 */
int ffw_audio_play_buffer(ffw_audio_format_t format, const void *data, uint32_t size, ffw_playback_finish_t on_finish, void *arg);

/**
* @brief 
* 
* @return int 
 */
int ffw_audio_stop(void);
int ffw_thread_audio_url_stop(void);
/**
* @brief 
* 
* @return int 
 */
int ffw_audio_pause(void);

/**
* @brief 
* 
* @return int 
 */
int ffw_audio_resume(void);

/**
* @brief 
* 
* @return int 
 */
int ffw_audio_list(void);

/**
* @brief 开始播放或者停止播放tone音
* 
* @param op 0 开始播放 1 停止播放
* @param tone_id 
* @param tone_duration 时间长度，单位毫秒
* @param mix_factor 音量
* @return int 
 */
int ffw_tone_play(int op, uint8_t tone_id, uint32_t tone_duration, uint8_t mix_factor);

/**
* @brief 
* 
* @param vol_type 
* @param vol 
* @return int 
 */
int ffw_set_audio_volume(ffw_audio_vol_type_t vol_type, uint8_t vol);

/**
* @brief 设置音量的增益
* 
* @param value 0-15
* @return int 
 */
int ffw_set_microphone_gain_value(uint8_t value);

/**
* @brief 获取音量的增益
* 
* @param value 
* @return int 
 */
int ffw_get_microphone_gain_value(uint8_t *value);


/**
* @brief 设置回声抑制，噪声抑制，频响曲线等声学回声消除调节参数
* 
* @param mode 
* @param path 
* @param ctrl 
* @param param 
* @param param_len 
* @return int 
 */
int ffw_set_acoustic_echo_canceler(uint8_t mode, uint8_t path, uint8_t ctrl, const char *param, uint16_t param_len);

/**
* @brief 
* 
* @param mode 
* @param path 
* @param ctrl 
* @param param 
* @return int 
 */
int ffw_get_acoustic_echo_canceler(uint8_t mode, uint8_t path, uint8_t ctrl, char *param);


//int fibo_audio_stream_play(uint8_t format, uint8_t *buff, uint32_t *Rp, uint32_t *Wp, uint32_t buffsize, int32_t timeout);

/**
* @brief 
* 
* @param filenames 
* @param num 
* @return int 
 */
int ffw_audio_list_play(const char **filenames, int32_t num);

/**
* @brief 
* 
* @param filename 
* @param num 
* @param b 
* @return int 
 */
int ffw_audio_merge_file(const char **filename, int num, ffw_buffer_t *b);

/**
 * @brief 
 * 
 * @param handle 
 * @param cb 
 * @param arg 
 * @return int 
 */
int ffw_audio_ts_create(ffw_handle_t *handle, void (*cb)(void *data, uint32_t len, uint32_t stream_type, void *arg), void *arg);

/**
 * @brief 
 * 
 * @param handle 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_audio_ts_decode(ffw_handle_t handle, uint8_t *data, uint32_t len);

/**
 * @brief 
 * 
 * @param handle 
 * @return int 
 */
int ffw_audio_ts_delete(ffw_handle_t handle);

/**
* @brief 
* 
* @param url 
* @param on_finish 
* @param arg 
* @return int 
 */
int ffw_audio_play_url(const char *url, ffw_playback_finish_t on_finish, void *arg);

/**
* @brief 
* 
* @return int 
*/
int ffw_audio_url_status(void);

#endif /* A70B7324_5FF4_45C9_9AF1_57FDD07CE6FD */
