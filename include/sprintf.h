#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

int avsprintf(char *dst, char *fmt, va_list arg);
int sprintf(char *dst, const char *fmt, ...);

#ifdef __cplusplus
}
#endif