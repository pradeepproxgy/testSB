#ifndef C88D9A13_23AF_470E_B29D_BE3AAB0F6C18
#define C88D9A13_23AF_470E_B29D_BE3AAB0F6C18

#include "osi_api.h"
#include "osi_log.h"
#include "fibo_opencpu.h"

#define APP_LOG_INFO(format, ...) fibo_textTrace("[app][info][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

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

FIBO_CALLBACK_T *app_init(void (*cb)());

int app_register_sig_callback(GAPP_SIGNAL_ID_T sig, fibo_signal_t cb);

#endif /* C88D9A13_23AF_470E_B29D_BE3AAB0F6C18 */
