#include "getc.h"

int getc(FILE *file)
{
    if (file->_read == NULL)
    {
        return EOF;
    }
    else
    {
        char res;
        file->_read(NULL, NULL, &res, 1);
        return res;
    }
}

int getchar(void)
{
    return getc(stdin);
}