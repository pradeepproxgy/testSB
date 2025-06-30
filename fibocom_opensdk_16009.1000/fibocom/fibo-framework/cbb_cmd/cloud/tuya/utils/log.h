/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "ffw_utils.h"
#include "ffw_log.h"

#if 0
#define log_trace(format, ...) FFW_LOG_TRACE(format, ##__VA_ARGS__)
#define log_debug(format, ...) FFW_LOG_DEBUG(format, ##__VA_ARGS__)
#define log_info(format, ...)  FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_warn(format, ...)  FFW_LOG_WARN(format, ##__VA_ARGS__)
#define log_error(format, ...) FFW_LOG_ERROR(format, ##__VA_ARGS__)
#define log_fatal(format, ...) FFW_LOG_FATAL(format, ##__VA_ARGS__)
#else
#define log_trace(format, ...) FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_debug(format, ...) FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_info(format, ...)  FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_warn(format, ...)  FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_error(format, ...) FFW_LOG_INFO(format, ##__VA_ARGS__)
#define log_fatal(format, ...) FFW_LOG_INFO(format, ##__VA_ARGS__)

#endif

#endif
