#pragma once

#include <stdint.h>
#include "mik32_stdout.h"

int putc(int symbol, FILE*);
int putchar(int symbol);
int puts(const char *str);