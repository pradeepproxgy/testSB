#if !defined(LOG_H)
#define LOG_H

enum LOG_LEVELS
{
    INVALID_LEVEL = -1,
    TRACE_MAXIMUM = 1,
    TRACE_MEDIUM,
    TRACE_MINIMUM,
    TRACE_PROTOCOL,
    LOG_ERROR,
    LOG_SEVERE,
    LOG_FATAL,
};

typedef struct
{
    enum LOG_LEVELS trace_level;        /**< trace level */
    int max_trace_entries;              /**< max no of entries in the trace buffer */
    enum LOG_LEVELS trace_output_level; /**< trace level to output to destination */
} trace_settings_type;

#define LOG_PROTOCOL TRACE_PROTOCOL
#define TRACE_MAX TRACE_MAXIMUM
#define TRACE_MIN TRACE_MINIMUM

typedef struct
{
    const char *name;
    const char *value;
} Log_nameValue;

int Log_initialize(Log_nameValue *);
void Log_terminate(void);

void Log(enum LOG_LEVELS, int, const char *, ...);

typedef void Log_traceCallback(enum LOG_LEVELS level, const char *message);
void Log_setTraceCallback(Log_traceCallback *callback);
void Log_setTraceLevel(enum LOG_LEVELS level);

#endif
