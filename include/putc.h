#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int putc(int symbol, FILE*);
int putchar(int symbol);
int puts(const char *str);

#ifdef __cplusplus
}
#endif