#include "sprintf.h"
#include <stdbool.h>


int svprintf(char *dst, char *fmt, va_list arg)
{
    char c;
    int symbols_counter = 0;
    char fuller = '0';
    bool left = false;
    int minimal_width;
    int divider;
    bool hex_upper;
    bool use_long = false;
    unsigned long long val;

    while (1)
    {
        char buf[64];
        int fromint_cnt;
        fuller = '0';
        int minimal_width = 0;
        /* Get a format symbol */
        c = *fmt++;
        if (c == '\0')
        {
            break; // end of format string
        }
        if (c == '%')
        {
            c = *fmt++; //< get the next symbol
            if (c == '%')
            {
                *dst++ = c;
                symbols_counter += 1;
                continue;
            }
            if (c == 's')
            {
                char *string = va_arg(arg, char*);
                while (*string != '\0')
                {
                    *dst++ = *string++;
                    symbols_counter += 1;
                }
                continue;
            }
            if (c == '-')
            {
                left = true;
                c = *fmt++;
            }
            else left = false;
            if (c == '0')
            {
                fuller = '0';
                c = *fmt++;
            }
            else if (c == '*')
            {
                fuller = '*';
                c = *fmt++;
            }
            while (c >= '0' && c <= '9')
            {
                minimal_width *= 10;
                minimal_width += (c - '0');
                c = *fmt++;
            }
            
            if (c == 'c') //< char
            {
                if (!left)
                {
                    if (minimal_width - (int)sizeof(char) > 0)
                    {
                        for (int i=0; i<(minimal_width-sizeof(char)); ++i)
                            *dst++ = fuller;
                        symbols_counter += minimal_width;
                    }
                    else symbols_counter += sizeof(char);
                    *dst++ = (char)va_arg(arg, int);//va_arg(arg, char);
                }
                else //< left = true
                {
                    *dst++ = (char)va_arg(arg, int);//va_arg(arg, char);
                    if (minimal_width - (int)sizeof(char) > 0)
                    {
                        for (int i=0; i<(minimal_width-sizeof(char)); ++i)
                            *dst++ = fuller;
                        symbols_counter += minimal_width;
                    }
                    else symbols_counter += sizeof(char);
                }
            }
            if (c == 'l')
            {
                c = *fmt++;
                if (c == 'l')
                {
                    use_long = true;
                    c = *fmt++;
                }
                else use_long = false;
            }
            else use_long = false;
            if (c == 'u')
            {
                divider = 10;
                if (use_long)
                    val = (unsigned long long)va_arg(arg, long long);
                else
                    val = (unsigned)va_arg(arg, int);
            }
            if (c == 'd')
            {
                divider = 10;
                long long raw;
                if (use_long)
                    raw = va_arg(arg, long long);
                else
                    raw = va_arg(arg, int);
                if (raw < 0)
                {
                    *dst++ = '-';
                    symbols_counter += 1;
                }
                val = llabs(raw);
            }
            if (c == 'b')
            {
                divider = 2;
                if (use_long)
                    val = (unsigned long long)va_arg(arg, long long);
                else
                    val = (unsigned)va_arg(arg, int);
            }
            if (c == 'o')
            {
                divider = 8;
                if (use_long)
                    val = (unsigned long long)va_arg(arg, long long);
                else
                    val = (unsigned)va_arg(arg, int);
            }
            if (c == 'X')
            {
                divider = 16;
                hex_upper = true;
                if (use_long)
                    val = (unsigned long long)va_arg(arg, long long);
                else
                    val = (unsigned)va_arg(arg, int);
            }
            if (c == 'x')
            {
                divider = 16;
                hex_upper = false;
                if (use_long)
                    val = (unsigned long long)va_arg(arg, long long);
                else
                    val = (unsigned)va_arg(arg, int);
            }
            fromint_cnt = 0;
            while (val > 0)
            {
                buf[fromint_cnt++] = val % divider;
                val /= divider;
            }
            if (!left)
            {
                if (minimal_width - fromint_cnt > 0)
                {
                    for (int i=0; i<(minimal_width-fromint_cnt); ++i)
                        *dst++ = fuller;
                    symbols_counter += minimal_width-fromint_cnt;
                }
            }
            for (int i=fromint_cnt-1; i>=0; i--)
            {
                char sym = buf[i] + '0';
                sym += sym > '9' ? hex_upper ? 0x7 : 0x27 : 0;
                *dst++ = sym;
            }
            symbols_counter += fromint_cnt;
            if (left)
            {
                if (minimal_width - fromint_cnt > 0)
                {
                    for (int i=0; i<(minimal_width-fromint_cnt); ++i)
                        *dst++ = fuller;
                    symbols_counter += minimal_width-fromint_cnt;
                }
            }
        }
        else
        {
            *dst++ = c;
            symbols_counter += 1;
        }
    }

    return symbols_counter;
}

int sprintf(char *dst, const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
	int res = svprintf(dst, fmt, arg);
	va_end(arg);
    return res;
}