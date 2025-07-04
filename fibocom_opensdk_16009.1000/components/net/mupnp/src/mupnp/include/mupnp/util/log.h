/******************************************************************
 *
 * mUPnP for C
 *
 * Copyright (C) Satoshi Konno 2005
 * Copyright (C) 2006 Nokia Corporation. All rights reserved.
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _MUPNP_UTIL_LOG_H_
#define _MUPNP_UTIL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LOG_STRING 512

/* Message masks and descriptions */
#define SEV_UNKNOWN_S "Unknown"

#define SEV_ERROR 1
#define SEV_ERROR_S "Error"

#define SEV_WARNING 2
#define SEV_WARNING_S "Warning"

#define SEV_INFO 4
#define SEV_INFO_S "Info"

#define SEV_DEBUG_L1 8
#define SEV_DEBUG_L1_S "Debug (general)"

#define SEV_DEBUG_L2 16
#define SEV_DEBUG_L2_S "Debug (stack)"

#define SEV_DEBUG_L3 32
#define SEV_DEBUG_L3_S "Debug (application)"

/* Low level debug messages like when printing out (important) function 
 * entry/leaving points */
#define SEV_DEBUG_L4 64
#define SEV_DEBUG_L4_S "Debug (low level L4)"

/* Low level debug messages to be used e.g. when printing out less important
 * function entry/leaving points */
#define SEV_DEBUG_L5 128
#define SEV_DEBUG_L5_S "Debug (low level L5)"

/* Mask for all message types */
#define SEV_ALL (SEV_ERROR | SEV_WARNING | SEV_INFO | SEV_DEBUG_L1 | SEV_DEBUG_L2 | SEV_DEBUG_L3 | SEV_DEBUG_L4 | SEV_DEBUG_L5)

/* Filename, line_n and unique build number can be used to give unique id tag
 * to all messages.
 */
 #define HTTP_NO_TRACE
#if defined(HTTP_NO_TRACE)
#define mupnp_log_print(format, ...)
#else
void mupnp_log_print(int severity, const char* file, int line_n, const char* function, const char* format, ...);
#endif
void mupnp_log_set_separator(char* s);

/* Target defines a file or stdout/stderr, in future it may also
 * contain special targets like network addresses etc.
 */
int mupnp_log_add_target(char* target, int severity_mask);
int mupnp_log_clear_targets();

/* Standard error handling */
#if defined(__USE_ISOC99)
#define mupnp_log_error(format, ...) mupnp_log_print(SEV_ERROR, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_error(const char* format, ...);
#else
#define mupnp_log_error(format...) mupnp_log_print(SEV_ERROR, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif

#if defined(__USE_ISOC99)
#define mupnp_log_warning(format, ...) mupnp_log_print(SEV_WARNING, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_warning(const char* format, ...);
#else
#define mupnp_log_warning(format...) mupnp_log_print(SEV_WARNING, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif

#if defined(__USE_ISOC99)
#define mupnp_log_info(format, ...) mupnp_log_print(SEV_INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_info(const char* format, ...);
#else
#define mupnp_log_info(format...) mupnp_log_print(SEV_INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif

/* General and low level debug */
#ifdef CLOG_DEBUG
#if defined(__USE_ISOC99)
#define mupnp_log_debug(format, ...) mupnp_log_print(SEV_DEBUG_L1, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#define mupnp_log_debug_l4(format, ...) mupnp_log_print(SEV_DEBUG_L4, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#define mupnp_log_debug_l5(format, ...) mupnp_log_print(SEV_DEBUG_L5, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32)
void mupnp_log_debug(const char* format, ...);
void mupnp_log_debug_l4(const char* format, ...);
void mupnp_log_debug_l5(const char* format, ...);
#else
#define mupnp_log_debug(format...) mupnp_log_print(SEV_DEBUG_L1, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#define mupnp_log_debug_l4(format...) mupnp_log_print(SEV_DEBUG_L4, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#define mupnp_log_debug_l5(format...) mupnp_log_print(SEV_DEBUG_L5, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif
#else
#if defined(__USE_ISOC99)
#define mupnp_log_debug(format, ...)
#define mupnp_log_debug_l4(format, ...)
#define mupnp_log_debug_l5(format, ...)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_debug(const char* format, ...);
void mupnp_log_debug_l4(const char* format, ...);
void mupnp_log_debug_l5(const char* format, ...);
#else
#define mupnp_log_debug(format...)
#define mupnp_log_debug_l4(format...)
#define mupnp_log_debug_l5(format...)
#endif
#endif

/* Debug messages coming from stack */
#ifdef CLOG_DEBUG_STACK
#if defined(__USE_ISOC99)
#define mupnp_log_debug_s(format, ...) mupnp_log_print(SEV_DEBUG_L2, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32)
void mupnp_log_debug_s(const char* format, ...);
#else
#define mupnp_log_debug_s(format...) mupnp_log_print(SEV_DEBUG_L2, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif
#else
#if defined(__USE_ISOC99)
#define mupnp_log_debug_s(format, ...)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_debug_s(const char* format, ...);
#else
#define mupnp_log_debug_s(format...)
#endif
#endif

/* Debug messages coming from application */
#ifdef CLOG_DEBUG_APPLICATION
#if defined(__USE_ISOC99)
#define mupnp_log_debug_a(format...) mupnp_log_print(SEV_DEBUG_L3, __FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#elif defined(WIN32)
void mupnp_log_debug_a(const char* format, ...);
#else
#define mupnp_log_debug_a(format...) mupnp_log_print(SEV_DEBUG_L3, __FILE__, __LINE__, __PRETTY_FUNCTION__, format)
#endif
#else
#if defined(__USE_ISOC99)
#define mupnp_log_debug_a(format, ...)
#elif defined(WIN32) || defined(WINCE)
void mupnp_log_debug_a(const char* format, ...);
#else
#define mupnp_log_debug_a(format...)
#endif
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CLOG_H */
