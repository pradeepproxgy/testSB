#ifndef _VOICE_PLAY_H_
#define _VOICE_PLAY_H_

#include "fibo_opencpu.h"
#include "fb_config.h"


typedef struct {
    char play_state;
    char play_text[TTS_MAX_DATA];
    char play_type; //1:tts  2:audio
}AudioPlayInfo;

/**
 * @brief fb_play_audio()   play audio
 *
 * @param[in]  need to play the audio file name.
 *
 * @return  0：success.
 *         -1: fail.
 *
 */

int fb_play_audio(char *ad_file);

/**
 * @brief push_play_list()  push play list queue
 *
 * @param[in] play audio name.
 *
 * @return  0：success.
 *          -1:queue full.
 *
 */

int push_play_list(AudioPlayInfo *play_info);

/**
 * @brief pop_play_list()  pop play list queue
 *
 * @param[out] play audio name..
 *
 * @return  0：success.
 *          -2:queue empty.
 *
 */
int pop_play_list(AudioPlayInfo *play_info);



/**
 * @brief audio_play_loop() play audio task cycle .
 *
 * @param[in] none.
 * 
 * @return  none.
 *
 */
void audio_play_loop(void);

/**
 * @brief audio_play_init() play audio init.
 *
 * @param[in] none.
 * 
 * @return  none.
 *
 */

void audio_play_init(void);

/**
 * @brief audio_play_enter()  play audio enter
 *
 * @param[in] none.
 *
 * @return  none.
 *
 */
int voice_play_enter(void);

int push_tts_play_list(char *play_text);
int push_audio_play_list(char *play_text);

#endif // !_AUDIO_PLAY_H_