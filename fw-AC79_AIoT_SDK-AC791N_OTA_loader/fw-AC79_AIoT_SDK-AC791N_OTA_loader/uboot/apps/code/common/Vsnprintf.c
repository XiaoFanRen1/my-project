/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

/*
 * Fri Jul 13 2001 Crutcher Dunnavant <crutcher+kernel@datastacks.com>
 * - changed to provide snprintf and vsnprintf functions
 * So Feb  1 16:51:32 CET 2004 Juergen Quade <quade@hsnr.de>
 * - scnprintf and vscnprintf
 */

/**
 * snprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The return value is the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */


#include <stdarg.h>
#include "string.h"
#include "printf.h"
#include "typedef.h"
//#include "ctype.h"
/*#define unlikely*/
#define unlikely(x)	__builtin_expect(!!(x), 0)

#ifndef false
#define false 0
#endif

#ifndef true
#define true !false
#endif

#ifndef NULL
#define NULL (void *)0
#endif

typedef unsigned int resource_size_t;
typedef unsigned char bool, u8, __u8;
typedef unsigned short u16, __be16;
typedef unsigned int __be32;

struct resource {
    const char *name;
    unsigned long start, end;
    unsigned long flags;
    struct resource *parent, *sibling, *child;
};

/* Works only for digits and letters, but small and fast */
#define TOLOWER(x) ((x) | 0x20)

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long long) n) % (unsigned) base; \
n = ((unsigned long long) n) / (unsigned) base; \
__res; })


#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* use lowercase in hex (must be 32 == 0x20) */
#define SPECIAL	64		/* prefix hex with "0x", octal with "0" */



enum format_type {
    FORMAT_TYPE_NONE, /* Just a string part */
    FORMAT_TYPE_WIDTH,
    FORMAT_TYPE_PRECISION,
    FORMAT_TYPE_CHAR,
    FORMAT_TYPE_STR,
    FORMAT_TYPE_PTR,
    FORMAT_TYPE_PERCENT_CHAR,
    FORMAT_TYPE_INVALID,
    FORMAT_TYPE_LONG_LONG,
    FORMAT_TYPE_ULONG,
    FORMAT_TYPE_LONG,
    FORMAT_TYPE_UBYTE,
    FORMAT_TYPE_BYTE,
    FORMAT_TYPE_USHORT,
    FORMAT_TYPE_SHORT,
    FORMAT_TYPE_UINT,
    FORMAT_TYPE_INT,
    FORMAT_TYPE_NRCHARS,
    FORMAT_TYPE_SIZE_T,
    FORMAT_TYPE_PTRDIFF,
    FORMAT_TYPE_ULONG_LONG,
};

struct printf_spec {
    unsigned char 	type;		/* format_type enum */
    unsigned char 	flags;		/* flags to number() */
    unsigned char 	base;		/* number base, 8, 10 or 16 only */
    unsigned char 	qualifier;	/* number qualifier, one of 'hHlLtzZ' */
    short	field_width;	/* width of output field */
    short	precision;	/* # of digits/chars */
};

#define isalnum(c) (((c)>='0' && (c)<='9') || ((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))



#define dereference_function_descriptor(ptr) ptr

static char *ip6_addr_string(char *buf, char *end, const u8 *addr,
                             struct printf_spec spec, const char *fmt)
{
    puts("we don't support ip6_addr_string()\r\n");
    return NULL;
}
static char *ip4_addr_string(char *buf, char *end, const u8 *addr,
                             struct printf_spec spec, const char *fmt)
{
    puts("we don't support ip4_addr_string()\r\n");
    return NULL;
}
static char *uuid_string(char *buf, char *end, const u8 *addr,
                         struct printf_spec spec, const char *fmt)
{
    puts("we don't support uuid_string()\r\n");
    return NULL;
}
static char *mac_address_string(char *buf, char *end, u8 *addr,
                                struct printf_spec spec, const char *fmt)
{
    puts("we don't support mac_address_string()\r\n");
    return NULL;
}
static char *symbol_string(char *buf, char *end, void *ptr,
                           struct printf_spec spec, char ext)
{
    puts("we don't support symbol_string()\r\n");
    return NULL;
}
static char *resource_string(char *buf, char *end, struct resource *res,
                             struct printf_spec spec, const char *fmt)
{
    puts("we don't support resource_string()\r\n");
    return NULL;
}

#define isdigit(c) ((c) >= '0' && (c) <= '9')
static int skip_atoi(const char **s)
{
    int i = 0;

    while (isdigit(**s)) {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}


/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static char *put_dec_trunc(char *buf, unsigned q)
{
    unsigned d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0'; /* least significant digit */
    d1 = q + 9 * d3 + 5 * d2 + d1;
    if (d1 != 0) {
        q = (d1 * 0xcd) >> 11;
        d1 = d1 - 10 * q;
        *buf++ = d1 + '0'; /* next digit */

        d2 = q + 2 * d2;
        if ((d2 != 0) || (d3 != 0)) {
            q = (d2 * 0xd) >> 7;
            d2 = d2 - 10 * q;
            *buf++ = d2 + '0'; /* next digit */

            d3 = q + 4 * d3;
            if (d3 != 0) {
                q = (d3 * 0xcd) >> 11;
                d3 = d3 - 10 * q;
                *buf++ = d3 + '0';  /* next digit */
                if (q != 0) {
                    *buf++ = q + '0';    /* most sign. digit */
                }
            }
        }
    }

    return buf;
}
/* Same with if's removed. Always emits five digits */
static char *put_dec_full(char *buf, unsigned long long q)
{
    /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
    /* but anyway, gcc produces better code with full-sized ints */
    unsigned long long d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    /*
     * Possible ways to approx. divide by 10
     * gcc -O2 replaces multiply with shifts and adds
     * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
     * (x * 0x67) >> 10:  1100111
     * (x * 0x34) >> 9:    110100 - same
     * (x * 0x1a) >> 8:     11010 - same
     * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
     */
    d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0';
    d1 = q + 9 * d3 + 5 * d2 + d1;
    q = (d1 * 0xcd) >> 11;
    d1 = d1 - 10 * q;
    *buf++ = d1 + '0';

    d2 = q + 2 * d2;
    q = (d2 * 0xd) >> 7;
    d2 = d2 - 10 * q;
    *buf++ = d2 + '0';

    d3 = q + 4 * d3;
    q = (d3 * 0xcd) >> 11; /* - shorter code */
    /* q = (d3 * 0x67) >> 10; - would also work */
    d3 = d3 - 10 * q;
    *buf++ = d3 + '0';
    *buf++ = q + '0';

    return buf;
}

/* No inlining helps gcc to use registers better */
static char *put_dec(char *buf, unsigned long long num)
{
    while (1) {
        unsigned long long rem;
        if (num < 100000) {
            return put_dec_trunc(buf, num);
        }
        rem = do_div(num, 100000);
        buf = put_dec_full(buf, rem);
    }
}

static char *number(char *buf, char *end, unsigned long long num,
                    struct printf_spec spec)
{
    /* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
    static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

    char tmp[66];
    char sign;
    char locase;
    int need_pfx = ((spec.flags & SPECIAL) && spec.base != 10);
    int i;

    /* locase = 0 or 0x20. ORing digits or letters with 'locase'
     * produces same digits or (maybe lowercased) letters */
    locase = (spec.flags & SMALL);
    if (spec.flags & LEFT) {
        spec.flags &= ~ZEROPAD;
    }
    sign = 0;
    if (spec.flags & SIGN) {
        if ((signed long long)num < 0) {
            sign = '-';
            num = -(signed long long)num;
            spec.field_width--;
        } else if (spec.flags & PLUS) {
            sign = '+';
            spec.field_width--;
        } else if (spec.flags & SPACE) {
            sign = ' ';
            spec.field_width--;
        }
    }
    if (need_pfx) {
        spec.field_width--;
        if (spec.base == 16) {
            spec.field_width--;
        }
    }

    /* generate full string in tmp[], in reverse order */
    i = 0;
    if (num == 0) {
        tmp[i++] = '0';
    }
    /* Generic code, for any base:
    else do {
    	tmp[i++] = (digits[do_div(num,base)] | locase);
    } while (num != 0);
    */
    else if (spec.base != 10) { /* 8 or 16 */
        int mask = spec.base - 1;
        int shift = 3;

        if (spec.base == 16) {
            shift = 4;
        }
        do {
            tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
            num >>= shift;
        } while (num);
    } else { /* base 10 */
        i = put_dec(tmp, num) - tmp;
    }

    /* printing 100 using %2d gives "100", not "00" */
    if (i > spec.precision) {
        spec.precision = i;
    }
    /* leading space padding */
    spec.field_width -= spec.precision;
    if (!(spec.flags & (ZEROPAD + LEFT))) {
        while (--spec.field_width >= 0) {
            if (buf < end) {
                *buf = ' ';
            }
            ++buf;
        }
    }
    /* sign */
    if (sign) {
        if (buf < end) {
            *buf = sign;
        }
        ++buf;
    }
    /* "0x" / "0" prefix */
    if (need_pfx) {
        if (buf < end) {
            *buf = '0';
        }
        ++buf;
        if (spec.base == 16) {
            if (buf < end) {
                *buf = ('X' | locase);
            }
            ++buf;
        }
    }
    /* zero or space padding */
    if (!(spec.flags & LEFT)) {
        char c = (spec.flags & ZEROPAD) ? '0' : ' ';
        while (--spec.field_width >= 0) {
            if (buf < end) {
                *buf = c;
            }
            ++buf;
        }
    }
    /* hmm even more zero padding? */
    while (i <= --spec.precision) {
        if (buf < end) {
            *buf = '0';
        }
        ++buf;
    }
    /* actual digits of result */
    while (--i >= 0) {
        if (buf < end) {
            *buf = tmp[i];
        }
        ++buf;
    }
    /* trailing space padding */
    while (--spec.field_width >= 0) {
        if (buf < end) {
            *buf = ' ';
        }
        ++buf;
    }

    return buf;
}

/*
 * Helper function to decode printf style format.
 * Each call decode a token from the format and return the
 * number of characters read (or likely the delta where it wants
 * to go on the next call).
 * The decoded token is returned through the parameters
 *
 * 'h', 'l', or 'L' for integer fields
 * 'z' support added 23/7/1999 S.H.
 * 'z' changed to 'Z' --davidm 1/25/99
 * 't' added for long
 *
 * @fmt: the format string
 * @type of the token returned
 * @flags: various flags such as +, -, # tokens..
 * @field_width: overwritten width
 * @base: base of the number (octal, hex, ...)
 * @precision: precision of a number
 * @qualifier: qualifier of a number (long, unsigned int, ...)
 */
static int format_decode(const char *fmt, struct printf_spec *spec)
{
    const char *start = fmt;

    /* we finished early by reading the field width */
    if (spec->type == FORMAT_TYPE_WIDTH) {
        if (spec->field_width < 0) {
            spec->field_width = -spec->field_width;
            spec->flags |= LEFT;
        }
        spec->type = FORMAT_TYPE_NONE;
        goto precision;
    }

    /* we finished early by reading the precision */
    if (spec->type == FORMAT_TYPE_PRECISION) {
        if (spec->precision < 0) {
            spec->precision = 0;
        }

        spec->type = FORMAT_TYPE_NONE;
        goto qualifier;
    }

    /* By default */
    spec->type = FORMAT_TYPE_NONE;

    for (; *fmt ; ++fmt) {
        if (*fmt == '%') {
            break;
        }
    }

    /* Return the current non-format string */
    if (fmt != start || !*fmt) {
        return fmt - start;
    }

    /* Process flags */
    spec->flags = 0;

    while (1) { /* this also skips first '%' */
        bool found = true;

        ++fmt;

        switch (*fmt) {
        case '-':
            spec->flags |= LEFT;
            break;
        case '+':
            spec->flags |= PLUS;
            break;
        case ' ':
            spec->flags |= SPACE;
            break;
        case '#':
            spec->flags |= SPECIAL;
            break;
        case '0':
            spec->flags |= ZEROPAD;
            break;
        default:
            found = false;
        }

        if (!found) {
            break;
        }
    }

    /* get field width */
    spec->field_width = -1;

    if (isdigit(*fmt)) {
        spec->field_width = skip_atoi(&fmt);
    } else if (*fmt == '*') {
        /* it's the next argument */
        spec->type = FORMAT_TYPE_WIDTH;
        return ++fmt - start;
    }

precision:
    /* get the precision */
    spec->precision = -1; //BUG FIX, 导致string函数访问内存越界异常
    if (*fmt == '.') {
        ++fmt;
        if (isdigit(*fmt)) {
            spec->precision = skip_atoi(&fmt);
            if (spec->precision < 0) {
                spec->precision = 0;
            }
        } else if (*fmt == '*') {
            /* it's the next argument */
            spec->type = FORMAT_TYPE_PRECISION;
            return ++fmt - start;
        }
    }

qualifier:
    /* get the conversion qualifier */
    spec->qualifier = -1;
    if (*fmt == 'h' || TOLOWER(*fmt) == 'l' ||
        TOLOWER(*fmt) == 'z' || *fmt == 't') {
        spec->qualifier = *fmt++;
        if (unlikely(spec->qualifier == *fmt)) {
            if (spec->qualifier == 'l') {
                spec->qualifier = 'L';
                ++fmt;
            } else if (spec->qualifier == 'h') {
                spec->qualifier = 'H';
                ++fmt;
            }
        }
    }

    /* default base */
    spec->base = 10;
    switch (*fmt) {
    case 'c':
        spec->type = FORMAT_TYPE_CHAR;
        return ++fmt - start;

    case 's':
        spec->type = FORMAT_TYPE_STR;
        return ++fmt - start;

    case 'p':
        spec->type = FORMAT_TYPE_PTR;
        return fmt - start;
    /* skip alnum */

    case 'n':
        spec->type = FORMAT_TYPE_NRCHARS;
        return ++fmt - start;

    case '%':
        spec->type = FORMAT_TYPE_PERCENT_CHAR;
        return ++fmt - start;

    /* integer number formats - set up the flags and "break" */
    case 'o':
        spec->base = 8;
        break;

    case 'x':
        spec->flags |= SMALL;

    case 'X':
        spec->base = 16;
        break;

    case 'd':
    case 'i':
        spec->flags |= SIGN;
    case 'u':
        break;

    default:
        spec->type = FORMAT_TYPE_INVALID;
        return fmt - start;
    }

    if (spec->qualifier == 'L') {
        if (spec->flags & SIGN) {
            spec->type = FORMAT_TYPE_LONG_LONG;
        } else {
            spec->type = FORMAT_TYPE_ULONG_LONG;
        }
    } else if (spec->qualifier == 'l') {
        if (spec->flags & SIGN) {
            spec->type = FORMAT_TYPE_LONG;
        } else {
            spec->type = FORMAT_TYPE_ULONG;
        }
    } else if (TOLOWER(spec->qualifier) == 'z') {
        spec->type = FORMAT_TYPE_SIZE_T;
    } else if (spec->qualifier == 't') {
        spec->type = FORMAT_TYPE_PTRDIFF;
    } else if (spec->qualifier == 'H') {
        if (spec->flags & SIGN) {
            spec->type = FORMAT_TYPE_BYTE;
        } else {
            spec->type = FORMAT_TYPE_UBYTE;
        }
    } else if (spec->qualifier == 'h') {
        if (spec->flags & SIGN) {
            spec->type = FORMAT_TYPE_SHORT;
        } else {
            spec->type = FORMAT_TYPE_USHORT;
        }
    } else {
        if (spec->flags & SIGN) {
            spec->type = FORMAT_TYPE_INT;
        } else {
            spec->type = FORMAT_TYPE_UINT;
        }
    }

    return ++fmt - start;
}


static char *string(char *buf, char *end, const char *s, struct printf_spec spec)
{
    int len, i;

//	if ((unsigned long)s < PAGE_SIZE)
//		s = "(null)";

    len = strnlen(s, spec.precision);

    if (!(spec.flags & LEFT)) {
        while (len < spec.field_width--) {
            if (buf < end) {
                *buf = ' ';
            }
            ++buf;
        }
    }
    for (i = 0; i < len; ++i) {
        if (buf < end) {
            *buf = *s;
        }
        ++buf;
        ++s;
    }
    while (len < spec.field_width--) {
        if (buf < end) {
            *buf = ' ';
        }
        ++buf;
    }

    return buf;
}

/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'F' For symbolic function descriptor pointers with offset
 * - 'f' For simple symbolic function names without offset
 * - 'S' For symbolic direct pointers with offset
 * - 's' For symbolic direct pointers without offset
 * - 'R' For decoded struct resource, e.g., [mem 0x0-0x1f 64bit pref]
 * - 'r' For raw struct resource, e.g., [mem 0x0-0x1f flags 0x201]
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'm' For a 6-byte MAC address, it prints the hex address without colons
 * - 'MF' For a 6-byte MAC FDDI address, it prints the address
 *       with a dash-separated hex notation
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way
 *       IPv4 uses dot-separated decimal without leading 0's (1.2.3.4)
 *       IPv6 uses colon separated network-order 16 bit hex with leading 0's
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses
 *       IPv6 omits the colons (01020304...0f)
 *       IPv4 uses dot-separated decimal with leading 0's (010.123.045.006)
 * - '[Ii]4[hnbl]' IPv4 addresses in host, network, big or little endian order
 * - 'I6c' for IPv6 addresses printed as specified by
 *       http://tools.ietf.org/html/draft-ietf-6man-text-addr-representation-00
 * - 'U' For a 16 byte UUID/GUID, it prints the UUID/GUID in the form
 *       "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
 *       Options for %pU are:
 *         b big endian lower case hex (default)
 *         B big endian UPPER case hex
 *         l little endian lower case hex
 *         L little endian UPPER case hex
 *           big endian output byte order is:
 *             [0][1][2][3]-[4][5]-[6][7]-[8][9]-[10][11][12][13][14][15]
 *           little endian output byte order is:
 *             [3][2][1][0]-[5][4]-[7][6]-[8][9]-[10][11][12][13][14][15]
 *
 * Note: The difference between 'S' and 'F' is that on ia64 and ppc64
 * function pointers are really function descriptors, which contain a
 * pointer to the real address.
 */
static char *pointer(const char *fmt, char *buf, char *end, void *ptr,
                     struct printf_spec spec)
{
    if (!ptr) {
        return string(buf, end, "(null)", spec);
    }

    switch (*fmt) {
    case 'F':
    case 'f':
        ptr = dereference_function_descriptor(ptr);
    /* Fallthrough */
    case 'S':
    case 's':
        return symbol_string(buf, end, ptr, spec, *fmt);
    case 'R':
    case 'r':
        return resource_string(buf, end, ptr, spec, fmt);
    case 'M':			/* Colon separated: 00:01:02:03:04:05 */
    case 'm':			/* Contiguous: 000102030405 */
        /* [mM]F (FDDI, bit reversed) */
        return mac_address_string(buf, end, ptr, spec, fmt);
    case 'I':			/* Formatted IP supported
					 * 4:	1.2.3.4
					 * 6:	0001:0203:...:0708
					 * 6c:	1::708 or 1::1.2.3.4
					 */
    case 'i':			/* Contiguous:
					 * 4:	001.002.003.004
					 * 6:   000102...0f
					 */
        switch (fmt[1]) {
        case '6':
            return ip6_addr_string(buf, end, ptr, spec, fmt);
        case '4':
            return ip4_addr_string(buf, end, ptr, spec, fmt);
        }
        break;
    case 'U':
        return uuid_string(buf, end, ptr, spec, fmt);
    }
    spec.flags |= SMALL;
    if (spec.field_width == -1) {
        spec.field_width = 2 * sizeof(void *);
        spec.flags |= ZEROPAD;
    }
    spec.base = 16;

    return number(buf, end, (unsigned long) ptr, spec);
}




/**
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * This function follows C99 vsnprintf, but has some extensions:
 * %pS output the name of a text symbol with offset
 * %ps output the name of a text symbol without offset
 * %pF output the name of a function pointer with its offset
 * %pf output the name of a function pointer without its offset
 * %pR output the address range in a struct resource with decoded flags
 * %pr output the address range in a struct resource with raw flags
 * %pM output a 6-byte MAC address with colons
 * %pm output a 6-byte MAC address without colons
 * %pI4 print an IPv4 address without leading zeros
 * %pi4 print an IPv4 address with leading zeros
 * %pI6 print an IPv6 address with colons
 * %pi6 print an IPv6 address without colons
 * %pI6c print an IPv6 address as specified by
 *   http://tools.ietf.org/html/draft-ietf-6man-text-addr-representation-00
 * %pU[bBlL] print a UUID/GUID in big or little endian using lower or upper
 *   case.
 * %n is ignored
 *
 * The return value is the number of characters which would
 * be generated for the given input, excluding the trailing
 * '\0', as per ISO C99. If you want to have the exact
 * number of characters written into @buf as return value
 * (not including the trailing '\0'), use vscnprintf(). If the
 * return is greater than or equal to @size, the resulting
 * string is truncated.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want snprintf() instead.
 */
int vsnprintf(char *buf, unsigned long size, const char *fmt, __builtin_va_list args)
{
    unsigned long long num;
    char *str, *end;
    struct printf_spec spec = {0};

    /* Reject out-of-range values early.  Large positive sizes are
       used for unknown buffer sizes. */
//	if (WARN_ON_ONCE((int) size < 0))
//		return 0;

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf) {
        end = ((void *) - 1);
        size = end - buf;
    }

    while (*fmt) {
        const char *old_fmt = fmt;
        int read = format_decode(fmt, &spec);

        fmt += read;

        switch (spec.type) {
        case FORMAT_TYPE_NONE: {
            int copy = read;
            if (str < end) {
                if (copy > end - str) {
                    copy = end - str;
                }
                memcpy(str, old_fmt, copy);
            }
            str += read;
            break;
        }

        case FORMAT_TYPE_WIDTH:
            spec.field_width = va_arg(args, int);
            break;

        case FORMAT_TYPE_PRECISION:
            spec.precision = va_arg(args, int);
            break;

        case FORMAT_TYPE_CHAR: {
            char c;

            if (!(spec.flags & LEFT)) {
                while (--spec.field_width > 0) {
                    if (str < end) {
                        *str = ' ';
                    }
                    ++str;

                }
            }
            c = (unsigned char) va_arg(args, int);
            if (str < end) {
                *str = c;
            }
            ++str;
            while (--spec.field_width > 0) {
                if (str < end) {
                    *str = ' ';
                }
                ++str;
            }
            break;
        }

        case FORMAT_TYPE_STR:
            str = string(str, end, va_arg(args, char *), spec);
            break;

        case FORMAT_TYPE_PTR:
            str = pointer(fmt + 1, str, end, va_arg(args, void *),
                          spec);
            while (isalnum(*fmt)) {
                fmt++;
            }
            break;

        case FORMAT_TYPE_PERCENT_CHAR:
            if (str < end) {
                *str = '%';
            }
            ++str;
            break;

        case FORMAT_TYPE_INVALID:
            if (str < end) {
                *str = '%';
            }
            ++str;
            break;

        case FORMAT_TYPE_NRCHARS: {
            u8 qualifier = spec.qualifier;

            if (qualifier == 'l') {
                long *ip = va_arg(args, long *);
                *ip = (str - buf);
            } else if (TOLOWER(qualifier) == 'z') {
                unsigned int *ip = va_arg(args, unsigned int *);
                *ip = (str - buf);
            } else {
                int *ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            break;
        }

        default:
            switch (spec.type) {
            case FORMAT_TYPE_LONG_LONG:
                num = va_arg(args, long long);
                break;
            case FORMAT_TYPE_ULONG_LONG:
                num = va_arg(args, unsigned long long);
                break;
            case FORMAT_TYPE_ULONG:
                num = va_arg(args, unsigned long);
                break;
            case FORMAT_TYPE_LONG:
                num = va_arg(args, long);
                break;
            case FORMAT_TYPE_SIZE_T:
                num = va_arg(args, unsigned int);
                break;
            case FORMAT_TYPE_PTRDIFF:
                num = va_arg(args, long);
                break;
            case FORMAT_TYPE_UBYTE:
                num = (unsigned char) va_arg(args, int);
                break;
            case FORMAT_TYPE_BYTE:
                num = (signed char) va_arg(args, int);
                break;
            case FORMAT_TYPE_USHORT:
                num = (unsigned short) va_arg(args, int);
                break;
            case FORMAT_TYPE_SHORT:
                num = (short) va_arg(args, int);
                break;
            case FORMAT_TYPE_INT:
                num = (int) va_arg(args, int);
                break;
            default:
                num = va_arg(args, unsigned int);
            }

            str = number(str, end, num, spec);
        }
    }

    if (size > 0) {
        if (str < end) {
            *str = '\0';
        } else {
            end[-1] = '\0';
        }
    }

    /* the trailing null byte doesn't count towards the total */
    return str - buf;

}


/*将可变个参数(...)按照format格式化成字符串，然后将其复制到str中
**(1) 如果格式化后的字符串长度 < size，则将此字符串全部复制到str中，并给其后添加一个字符串结束符('\0')；
**(2) 如果格式化后的字符串长度 >= size，则只将其中的(size-1)个字符复制到str中，并给其后添加一个字符串结束符('\0')，
**返回值为格式化后的字符串的长度。
*/
int snprintf(char *buf, unsigned long size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return i;
}
