#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>


int avsprintf(char *dst, char *fmt, va_list arg);
int sprintf(char *dst, const char *fmt, ...);