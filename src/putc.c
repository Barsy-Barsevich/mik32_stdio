#include "putc.h"

int __attribute__((weak)) putc(int symbol, FILE*)
{
    mik32_stdout_putc(symbol);
    return 0;
}

int putchar(int symbol)
{
    mik32_stdout_putc(symbol);
    return 0;
}