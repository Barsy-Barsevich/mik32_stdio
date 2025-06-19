#pragma once

#include <stdio.h>
#include "usart_transaction.h"

#define PRINTF_BUFFER_SIZE          50
#define PRINTF_FLUSHING_SYMBOL      '\n'

void mik32_stdout_init(void);
void mik32_stdout_enable_blocking(void);
void mik32_stdout_disable_blocking(void);
void mik32_stdout_flush(void);
int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len);
void mik32_stdout_putc(char symbol);