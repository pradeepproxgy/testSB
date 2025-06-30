#ifndef _EVENT_SIGNAL_H_
#define _EVENT_SIGNAL_H_

#include "fb_config.h"

typedef enum
{
    CMD_MODEL_IMEI=0,
    CMD_SIM_REMOVE=1,

    CMD_PDP_ACTIVE_NOT_IP,
    CMD_PDP_IS_ACTIVE,
    CMD_CONNECT_NETWORK_FAIL,

    CMD_WARNING_POWER_ON,
    CMD_WARNING_POWER_OFF,
    CMD_WARNING_LOW_BATTERY,
    CMD_WARNING_CONNECTING,
    CMD_WARNING_CONNECTED_NET,
    CMD_WARNING_DISCONNECT_NET,
    
   
    CMD_WARNING_VOL_UP,
    CMD_WARNING_VOL_DOWN,
    CMD_WARNING_VOL_MAX,
    CMD_WARNING_VOL_MIN,
    CMD_WARNING_AD_DQTH, 

//warning tone
    CMD_AUDPLAY_END,
    CMD_REPETITION_PLAY,
    CMD_PLAY_SIGNAL_VAL,
    CMD_PLAY_SIGNAL_BAT,
    CMD_START_CHARGING,
    CMD_FINISH_CHARGING,
    CMD_BATTERY_FULL_CHARGING,
    CMD_SET_PLAY_VOL,
    CMD_AUDIO_PLAY,
    CMD_AUDIO_VOL_MAXMIN,

    CMD_CLOSE_LOG1,
    CMD_CGREG_CHECK,
    CMD_CEREG_CHECK,
    CMD_CGREG_STATE,
    CMD_CEREG_STATE,
    CMD_CLOSE_LOG2,
    CMD_LOCREL_TIME,
    CMD_AGPS_INJECTTION,
    CMD_AGPS_DOWNLOAD_FAIL,
    CMD_APP_FOTA,
}cmd_event;

/**
 * @brief fb_event_signal_enter()  时间消息任务入口
 *
 * @param[in] none.
 *
 * @return  0：success.
 *         -1: fail.
 *
 */

int fb_event_signal_enter(void);

/**
 * @brief fb_event_signal_push()  发送时间消息
 *
 * @param[in] message.
 *
 * @return  0：success.
 *         -1: fail.
 *
 */

int fb_event_signal_push(int msg);
#endif // !_EVENT_SIGNAL_H_