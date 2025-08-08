#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "usart_transaction.h"
#include "pad_config.h"
#include "mik32_stdio_status.h"

#define MIK32STDOUT_BUFSIZE_DEFAULT             50
#define MIK32STDOUT_FLUSHING_SYMBOL_DEFAULT     '\n'

mik32_stdio_status_t mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate);
mik32_stdio_status_t mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate);
void mik32_stdout_enable_blocking(void);
void mik32_stdout_disable_blocking(void);
uint32_t mik32_stdout_get_buffer_size(void);
mik32_stdio_status_t mik32_stdout_set_buffer_size(uint32_t buffer_size);
char mik32_stdout_get_flushing_symbol(void);
void mik32_stdout_set_flushing_symbol(char sym);
void mik32_stdout_flush(void);
int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len);
void mik32_stdout_putc(char symbol);

#ifdef __cplusplus
}
#endif