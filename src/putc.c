#include "putc.h"

int __attribute__((weak)) putc(int symbol, FILE *file)
{
    if (file->_write != NULL)
    {
        return file->_write(NULL, NULL, (char*)&symbol, 1);
    }
    else return -1;
}

int putchar(int symbol)
{
    return putc(symbol, stdout);
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