#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "usart_transaction.h"

// #define MIK32STDIN_USE_MALLOC

#define MIK32STDIN_BUFSIZE_DEFAULT      5

void mik32_stdin_init(UART_TypeDef *host);
void mik32_stdin_enable_blocking(void);
void mik32_stdin_disable_blocking(void);
uint32_t mik32_stdin_get_buffer_size(void);
void mik32_stdin_set_buffer_size(uint32_t size);
int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len);