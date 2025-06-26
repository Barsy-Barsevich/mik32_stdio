#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "usart_transaction.h"
#include "pad_config.h"
#include "mik32_stdio_status.h"

#define MIK32STDIN_BUFSIZE_DEFAULT      5

mik32_stdio_status_t mik32_stdin_uart_init(UART_TypeDef *host, uint32_t baudrate);
mik32_stdio_status_t mik32_stdin_init(UART_TypeDef *host, uint32_t baudrate);
void mik32_stdin_enable_blocking(void);
mik32_stdio_status_t mik32_stdin_disable_blocking(void);
uint32_t mik32_stdin_get_buffer_size(void);
mik32_stdio_status_t mik32_stdin_set_buffer_size(uint32_t size);
int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len);
char mik32_stdin_getc(void);

#ifdef __cplusplus
}
#endif