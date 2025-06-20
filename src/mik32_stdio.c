#include "mik32_stdio.h"

void mik32_stdio_init(UART_TypeDef *host)
{
    mik32_stdout_init(host);
    mik32_stdio_init(host);
}