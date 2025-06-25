#include "mik32_stdio.h"

void mik32_stdio_init(UART_TypeDef *host, uint32_t baudrate)
{
    mik32_stdout_init(host, baudrate);
    mik32_stdin_init(host, baudrate);
}