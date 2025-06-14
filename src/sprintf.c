#include "sprintf.h"


static int int_to_str(long long src, char *dst, int radix, bool left_filling, char fill_value, int minimal_width, bool hex_upper)
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
                *dst++ = fill_value;
        }
    }
    for (int i=fromint_cnt-1; i>=0; i--)
    {
        char sym = buf[i] + '0';
        sym += sym > '9' ? hex_upper ? 0x7 : 0x27 : 0;
        *dst++ = sym;
    }
    if (!left_filling)
    {
        if (minimal_width - fromint_cnt > 0)
        {
            for (int i=0; i<(minimal_width-fromint_cnt); ++i)
                *dst++ = fill_value;
        }
    }
    return fromint_cnt < minimal_width ? minimal_width : fromint_cnt;
}

int _vsprintf(char *dst, const char *fmt, va_list arg)
{
    char c;
    int symbols_counter = 0;
    char fill_value;
    bool left;
    int minimal_width;
    int minimal_exp_width;
    int radix;
    bool hex_upper;
    bool use_long = false;
    unsigned long long read_value;

    while (1)
    {
        fill_value = '0';
        minimal_width = 0;
        minimal_exp_width = 0;
        left = false;
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
                            *dst++ = fill_value;
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
                            *dst++ = fill_value;
                        symbols_counter += minimal_width;
                    }
                    else symbols_counter += sizeof(char);
                }
                continue;
            }
            if (c == 'p')
            {
                unsigned ptr = (unsigned)va_arg(arg, void*);
                int fromint_cnt = int_to_str(ptr, dst, 16, true, fill_value, 8, true);
                dst += fromint_cnt;
                symbols_counter += fromint_cnt;
                continue;
            }

            if (c == 'e' || c == 'E')
            {
                float fval = (float)va_arg(arg, double);
                if (fval < 0)
                {
                    fval = -fval;
                    *dst++ = '-';
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
                *dst++ = (int)fval + '0';
                fval -= (float)((int)fval);
                *dst++ = '.';
                for (int i=0; i<6; ++i)
                {
                    fval *= 10;
                    *dst++ = (int)fval + '0';
                    fval -= (float)((int)fval);
                    symbols_counter += 1;
                }
                *dst++ = c; //'e' or 'E'
                symbols_counter += 3;
                if (exp < 0)
                {
                    *dst++ = '-';
                    symbols_counter += 1;
                    exp = -exp;
                }
                int fromint_cnt = int_to_str(exp, dst, 10, true, fill_value, minimal_width, false);
                symbols_counter += fromint_cnt;
                dst += fromint_cnt;
                continue;
            }
            if (c == 'f' || c == 'F')
            {
                float fval = (float)va_arg(arg, double);
                if (fval < 0)
                {
                    fval = -fval;
                    *dst++ = '-';
                    symbols_counter += 1;
                }
                int integer_value = (int)fval;
                fval -= (float)integer_value;
                int fromint_cnt = int_to_str(integer_value, dst, 10, true, fill_value, minimal_width, false);
                dst += fromint_cnt;
                *dst++ = '.';
                symbols_counter += fromint_cnt + 1;
                if (minimal_exp_width == 0) minimal_exp_width = 7;
                for (int i=0; i<minimal_exp_width; ++i)
                {
                    fval *= 10;
                    *dst++ = (int)fval + '0';
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
                    *dst++ = '-';
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
            int fromint_cnt = int_to_str(read_value, dst, radix, !left, fill_value, minimal_width, hex_upper);
            dst += fromint_cnt;
            symbols_counter += fromint_cnt;
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
	int res = _vsprintf(dst, fmt, arg);
	va_end(arg);
    return res;
}