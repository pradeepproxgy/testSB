#ifndef __APP_LWM2M_H__
#define __APP_LWM2M_H__

#include "fibo_opencpu.h"

#define CID_DEFAULT 1

#define APP_LOG_INFO(format, ...) fibo_textTrace("[lwm2m_app][info][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define APP_WAIT_EXPECT_STATUS(lock, sem, conditon, current_status)  \
    while (conditon)                                                 \
    {                                                                \
        fibo_mutex_unlock(lock);                                     \
        fibo_sem_try_wait(sem, 1000);                                \
        fibo_mutex_lock(lock);                                       \
        APP_LOG_INFO("%s is %d", #current_status, (current_status)); \
    }

#define APP_WAIT_EXPECT_STATUS_TIMEOUT(lock, sem, conditon, current_status, timeout) \
    while (conditon)                                                                 \
    {                                                                                \
        fibo_mutex_unlock(lock);                                                     \
        fibo_sem_try_wait(sem, timeout);                                             \
        fibo_mutex_lock(lock);                                                       \
        APP_LOG_INFO("%s is %d", #current_status, (current_status));                 \
    }

/**
 * @brief Registers a callback for a specific signal.
 *
 * This function allows the registration of a callback function that will be invoked when a specified signal is received.
 * It is used to handle asynchronous events or signals within the application.
 *
 * @param[in] sig The signal ID for which the callback is being registered.
 * @param[in] cb The callback function that should be called when the signal is received.
 * @return Returns 0 on successful registration of the callback, or a negative error code if the registration fails.
 */
int app_register_sig_callback(GAPP_SIGNAL_ID_T sig, fibo_signal_t cb);

#endif
