#include "printf.h"

static int int_to_str(FILE *stream, long long src, int radix, bool left_filling, char fill_value, int minimal_width, bool hex_upper)
{
    int fromint_cnt = 0;
    char buf[64];
    if (src == 0)
    {
        buf[0] = 0;
        fromint_cnt = 1;
    }
    else while (src > 0)
    {
        buf[fromint_cnt++] = src % radix;
        src /= radix;
    }
    if (left_filling)
    {
        if (minimal_width - fromint_cnt > 0)
        {
            for (int i=0; i<(minimal_width-fromint_cnt); ++i)
                putc(fill_value, stream);
        }
    }
    for (int i=fromint_cnt-1; i>=0; i--)
    {
        char sym = buf[i] + '0';
        sym += sym > '9' ? hex_upper ? 0x7 : 0x27 : 0;
        putc(sym, stream);
    }
    if (!left_filling)
    {
        if (minimal_width - fromint_cnt > 0)
        {
            for (int i=0; i<(minimal_width-fromint_cnt); ++i)
                putc(fill_value, stream);
        }
    }
    return fromint_cnt < minimal_width ? minimal_width : fromint_cnt;
}

int _fvprintf(FILE *stream, const char *fmt, va_list arg)
{
    char c;
    int symbols_counter = 0;
    char fill_value;
    bool left;
    int minimal_width;
    int minimal_exp_width;
    int radix = 10;
    bool hex_upper = true;
    bool use_long = false;
    unsigned long long read_value = 0;

    while (1)
    {
        /* Get a format symbol */
        c = *fmt++;
        if (c == '\0')
        {
            break; //< end of format string
        }
        if (c == '%')
        {
            fill_value = '0';
            minimal_width = 0;
            minimal_exp_width = 0;
            left = false;
            c = *fmt++; //< get the next symbol
            if (c == '%')
            {
                putc(c, stream);
                symbols_counter += 1;
                continue;
            }
            if (c == 's')
            {
                char *string = va_arg(arg, char*);
                while (*string != '\0')
                {
                    putc(*string++, stream);
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
                fill_value = '0';
                c = *fmt++;
            }
            else if (c == '*')
            {
                fill_value = '*';
                c = *fmt++;
            }
            while (c >= '0' && c <= '9')
            {
                minimal_width *= 10;
                minimal_width += (c - '0');
                c = *fmt++;
            }
            if (c == '.')
            {
                c = *fmt++;
                while (c >= '0' && c <= '9')
                {
                    minimal_exp_width *= 10;
                    minimal_exp_width += (c - '0');
                    c = *fmt++;
                }
            }
            
            if (c == 'c') //< char
            {
                if (!left)
                {
                    if (minimal_width - (int)sizeof(char) > 0)
                    {
                        for (int i=0; i<(minimal_width-sizeof(char)); ++i)
                            putc(fill_value, stream);
                        symbols_counter += minimal_width;
                    }
                    else symbols_counter += sizeof(char);
                    putc((char)va_arg(arg, int), stream);
                }
                else //< left = true
                {
                    putc((char)va_arg(arg, int), stream);
                    if (minimal_width - (int)sizeof(char) > 0)
                    {
                        for (int i=0; i<(minimal_width-sizeof(char)); ++i)
                            putc(fill_value, stream);
                        symbols_counter += minimal_width;
                    }
                    else symbols_counter += sizeof(char);
                }
                continue;
            }
            if (c == 'p')
            {
                unsigned ptr = (unsigned)va_arg(arg, void*);
                int fromint_cnt = int_to_str(stream, ptr, 16, true, fill_value, 8, true);
                symbols_counter += fromint_cnt;
                continue;
            }

            if (c == 'e' || c == 'E')
            {
                float fval = (float)va_arg(arg, double);
                if (fval < 0)
                {
                    fval = -fval;
                    putchar('-');
                    symbols_counter += 1;
                }
                int exp = 0;
                while (fval >= 10 || fval < 1)
                {
                    if (fval >= 10)
                    {
                        fval /= 10;
                        exp += 1;
                    }
                    else
                    {
                        fval *= 10;
                        exp -= 1;
                    }
                }
                putchar((int)fval + '0');
                fval -= (float)((int)fval);
                putchar('.');
                for (int i=0; i<6; ++i)
                {
                    fval *= 10;
                    putchar((int)fval + '0');
                    fval -= (float)((int)fval);
                    symbols_counter += 1;
                }
                putchar(c); //'e' or 'E'
                symbols_counter += 3;
                if (exp < 0)
                {
                    putchar('-');
                    symbols_counter += 1;
                    exp = -exp;
                }
                int fromint_cnt = int_to_str(stream, exp, 10, true, fill_value, minimal_width, false);
                symbols_counter += fromint_cnt;
                continue;
            }
            if (c == 'f' || c == 'F')
            {
                float fval = (float)va_arg(arg, double);
                if (fval < 0)
                {
                    fval = -fval;
                    putchar('-');
                    symbols_counter += 1;
                }
                int integer_value = (int)fval;
                fval -= (float)integer_value;
                int fromint_cnt = int_to_str(stream, integer_value, 10, true, fill_value, minimal_width, false);
                putchar('.');
                symbols_counter += fromint_cnt + 1;
                if (minimal_exp_width == 0) minimal_exp_width = 7;
                for (int i=0; i<minimal_exp_width; ++i)
                {
                    fval *= 10;
                    putchar((int)fval + '0');
                    fval -= (float)((int)fval);
                    symbols_counter += 1;
                }
                continue;
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
                radix = 10;
                if (use_long)
                    read_value = (unsigned long long)va_arg(arg, long long);
                else
                    read_value = (unsigned)va_arg(arg, int);
            }
            if (c == 'd')
            {
                radix = 10;
                long long raw;
                if (use_long)
                    raw = va_arg(arg, long long);
                else
                    raw = va_arg(arg, int);
                if (raw < 0)
                {
                    putchar('-');
                    symbols_counter += 1;
                    read_value = -raw;
                }
                else read_value = raw;
            }
            if (c == 'b')
            {
                radix = 2;
                if (use_long)
                    read_value = (unsigned long long)va_arg(arg, long long);
                else
                    read_value = (unsigned)va_arg(arg, int);
            }
            if (c == 'o')
            {
                radix = 8;
                if (use_long)
                    read_value = (unsigned long long)va_arg(arg, long long);
                else
                    read_value = (unsigned)va_arg(arg, int);
            }
            if (c == 'X')
            {
                radix = 16;
                hex_upper = true;
                if (use_long)
                    read_value = (unsigned long long)va_arg(arg, long long);
                else
                    read_value = (unsigned)va_arg(arg, int);
            }
            if (c == 'x')
            {
                radix = 16;
                hex_upper = false;
                if (use_long)
                    read_value = (unsigned long long)va_arg(arg, long long);
                else
                    read_value = (unsigned)va_arg(arg, int);
            }
            int fromint_cnt = int_to_str(stream, read_value, radix, !left, fill_value, minimal_width, hex_upper);
            symbols_counter += fromint_cnt;
        }
        else
        {
            putchar(c);
            symbols_counter += 1;
        }
    }
    return symbols_counter;
}

int _vprintf(const char *fmt, va_list arg)
{
    return _fvprintf(stdout, fmt, arg);
}

int printf(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
	int res = _vprintf(fmt, arg);
	va_end(arg);
    return res;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
	int res = _fvprintf(stream, fmt, arg);
	va_end(arg);
    return res;
}