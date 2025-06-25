#pragma once

#include <stdio.h>
#include "usart_transaction.h"
#include "pad_config.h"

#define PRINTF_BUFFER_SIZE          50
#define PRINTF_FLUSHING_SYMBOL      '\n'

bool mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate);
void mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate);
void mik32_stdout_enable_blocking(void);
void mik32_stdout_disable_blocking(void);
void mik32_stdout_flush(void);
int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len);
void mik32_stdout_putc(char symbol);