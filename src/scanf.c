#include "scanf.h"

/* 
 * thanks to https://github.com/aligrudi/neatlibc
 */

int fgetc(FILE *file)
{
    if (file->_read != NULL)
    {
        char dummy;
        int received = file->_read(NULL, NULL, &dummy, 1);
        if (received != 1) return -1;
        return (int)dummy;
    }
	else return -1;
}

// int getchar(void)
// {
// 	return fgetc(stdin);
// }

int ungetc(int c, FILE *fp)
{
	// FIXME
	return 0;
}

static int idig(int c, int hex)
{
	static char *digs = "0123456789abcdef";
	char *r = memchr(digs, hex ? tolower(c) : c, hex ? 16 : 10);
	return r == NULL ? -1 : r - digs;
}

/* t is 1 for char, 2 for short, 4 for int, and 8 for long */
static int iint(FILE *fp, void *dst, int t, int hex, int wid)
{
	long n = 0;
	int c;
	int neg = 0;
	c = fgetc(fp);
	if (c == '-')
		neg = 1;
	if ((c == '-' || c == '+') && wid-- > 0)
		c = fgetc(fp);
	if (c == EOF || idig(c, hex) < 0 || wid <= 0) {
		ungetc(c, fp);
		return 1;
	}
	do {
		n = n * (hex ? 16 : 10) + idig(c, hex);
	} while ((c = fgetc(fp)) != EOF && idig(c, hex) >= 0 && --wid > 0);
	ungetc(c, fp);
	if (t == 8)
		*(long *) dst = neg ? -n : n;
	else if (t == 4)
		*(int *) dst = neg ? -n : n;
	else if (t == 2)
		*(short *) dst = neg ? -n : n;
	else
		*(char *) dst = neg ? -n : n;
	return 0;
}

static int istr(FILE *fp, char *dst, int wid)
{
	char *d = dst;
	int c;
	// while ((c = fgetc(fp)) != '\n'/*EOF*/ && wid-- > 0 && !isspace(c))
	// 	*d++ = c;
    c = fgetc(fp);
    while (c != '\n' && c != '\r' && c != EOF && wid-- > 0)
    {
        *d++ = c;
        c = fgetc(fp);
    }
	*d = '\0';
	return d == dst;
}

int vfscanf(FILE *fp, const char *fmt, va_list ap)
{
	int ret = 0;
	int t, c;
	int wid = 1 << 20;
	while (*fmt) {
		while (isspace((unsigned char) *fmt))
			fmt++;
		// while (isspace(c = fgetc(fp)))
		// 	;
		// ungetc(c, fp);
		while (*fmt && *fmt != '%' && !isspace((unsigned char) *fmt))
			if (*fmt++ != fgetc(fp))
				return ret;
		if (*fmt != '%')
			continue;
		fmt++;
		if (isdigit((unsigned char) *fmt)) {
			wid = 0;
			while (isdigit((unsigned char) *fmt))
				wid = wid * 10 + *fmt++ - '0';
		}
		t = sizeof(int);
		while (*fmt == 'l') {
			t = sizeof(long);
			fmt++;
		}
		while (*fmt == 'h') {
			t = t < sizeof(int) ? sizeof(char) : sizeof(short);
			fmt++;
		}
		switch (*fmt++) {
		case 'u':
		case 'd':
			if (iint(fp, va_arg(ap, long *), t, 0, wid))
				return ret;
			ret++;
			break;
		case 'x':
			if (iint(fp, va_arg(ap, long *), t, 1, wid))
				return ret;
			ret++;
			break;
		case 's':
			if (istr(fp, va_arg(ap, char *), wid))
				return ret;
			ret++;
			break;
		}
	}
	return ret;
}

int fscanf(FILE *fp, const char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfscanf(fp, fmt, ap);
	va_end(ap);
	return ret;
}

int scanf(const char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfscanf(stdin, fmt, ap);
	va_end(ap);
	return ret;
}

int vsscanf(const char *s, const char *fmt, va_list ap)
{
	// FILE f = {-1, EOF};
	// f.ibuf = s;
	// f.ilen = strlen(s);
	// return vfscanf(&f, fmt, ap);
    return 0; //< FIXME
}

int sscanf(const char *s, const char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vsscanf(s, fmt, ap);
	va_end(ap);
	return ret;
}

char *fgets(char *s, int sz, FILE *fp)
{
	int i = 0;
	int c;
	while (i + 1 < sz && (c = fgetc(fp)) != EOF) {
		s[i++] = c;
		if (c == '\n')
			break;
	}
	s[i] = '\0';
	return i ? s : NULL;
}

size_t fread(void *v, size_t sz, size_t n, FILE *fp)
{
	char *s = v;
	int i = n * sz;
	while (i-- > 0)
		if ((*s++ = fgetc(fp)) == EOF)
			return n * sz - i - 1;
	return n * sz;
}