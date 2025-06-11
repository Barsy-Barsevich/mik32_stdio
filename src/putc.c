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

int puts(const char *str)
{
    while (*str != '\0')
    {
        putchar(*str++);
    }
    putchar('\n');
    return '\0';
}