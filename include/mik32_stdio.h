#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mik32_stdin.h"
#include "mik32_stdout.h"

void mik32_stdio_init(UART_TypeDef *host, uint32_t baudrate);

#ifdef __cplusplus
}
#endif