#ifndef FFW_TLS_CYCLONETLS_CYCLONESSL_PORT
#define FFW_TLS_CYCLONETLS_CYCLONESSL_PORT

#include "compiler_port.h"

/**
 * @brief Mutex object
 **/

typedef struct
{
    void *mutex;
} OsMutex;

//Mutex management
bool_t osCreateMutex(OsMutex *mutex);
void osDeleteMutex(OsMutex *mutex);
void osAcquireMutex(OsMutex *mutex);
void osReleaseMutex(OsMutex *mutex);

//System time
systime_t osGetSystemTime(void);

//Memory management
void *osAllocMem(size_t size);
void osFreeMem(void *p);

#endif /* FFW_TLS_CYCLONETLS_CYCLONESSL_PORT */
