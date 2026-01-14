#include <stdarg.h>
#include <string.h>
//#include "ctype.h"
#include "printf.h"
#include "typedef.h"


#define unlikely(x)	__builtin_expect(!!(x), 0)



#define USHORT_MAX	((unsigned int)(~0U))
#define SHORT_MAX	((signed int)(USHORT_MAX>>1))
#define INT_MAX		((int)(~0U>>1))

#ifndef isspace
#define isspace(c)	(c == ' ' || c == '\t' || c == 10 || c == 13 || c == 0)
#endif
#ifndef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#endif

#ifndef isxdigit

#define in_range(c, lo, up)  ((u8)c >= lo && (u8)c <= up)
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#endif
/* Works only for digits and letters, but small and fast */
#define TOLOWER(x) ((x) | 0x20)

extern double strtod(const char *string, char **endPtr);

static int skip_atoi(const char **s)
{
    int i = 0;

    while (isdigit(**s)) {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}

/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
static char *skip_spaces(const char *str)
{
    while (isspace(*str)) {
        ++str;
    }
    return (char *)str;
}

static unsigned int simple_guess_base(const char *cp)
{
    if (cp[0] == '0') {
        if (TOLOWER(cp[1]) == 'x' && isxdigit(cp[2])) {
            return 16;
        } else {
            return 8;
        }
    } else {
        return 10;
    }
}

/**
 * simple_strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base)
{
    unsigned long long result = 0;

    if (!base) {
        base = simple_guess_base(cp);
    }

    if (base == 16 && cp[0] == '0' && TOLOWER(cp[1]) == 'x') {
        cp += 2;
    }

    while (isxdigit(*cp)) {
        unsigned int value;

        value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
        if (value >= base) {
            break;
        }
        result = result * base + value;
        cp++;
    }
    if (endp) {
        *endp = (char *)cp;
    }

    return result;
}

/**
 * simple_strtoll - convert a string to a signed long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long long simple_strtoll(const char *cp, char **endp, unsigned int base)
{
    if (*cp == '-') {
        return -simple_strtoull(cp + 1, endp, base);
    }

    return simple_strtoull(cp, endp, base);
}

/**
 * simple_strtoul - convert a string to an unsigned long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoull(cp, endp, base);
}

/**
 * simple_strtol - convert a string to a signed long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long simple_strtol(const char *cp, char **endp, unsigned int base)
{
    if (*cp == '-') {
        return -simple_strtoul(cp + 1, endp, base);
    }

    return simple_strtoul(cp, endp, base);
}



/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
int vsscanf(const char *buf, const char *fmt, va_list args)
{
    const char *str = buf;
    char *next;
    char digit;
    int num = 0;
    unsigned char qualifier;
    unsigned char base;
    int field_width;
    bool is_sign;
    bool is_float;
    while (*fmt && *str) {
        /* skip any white space in format */
        /* white space in format matchs any amount of
         * white space, including none, in the input.
         */

        if (isspace(*fmt) && *(++fmt)) {

            fmt = skip_spaces(fmt);
            str = skip_spaces(str);
        }
        if (!*fmt) {
            break;
        }

        /* anything that is not a conversion must match exactly */
        if (*fmt != '%' && *fmt) {
            if (*fmt++ != *str++) {
                break;
            }
            continue;
        }

        if (!*fmt) {
            break;
        }
        ++fmt;

        /* skip this conversion.
         * advance both strings to next white space
         */
        if (*fmt == '*') {
            while (!isspace(*fmt) && *fmt != '%' && *fmt) {
                fmt++;
            }
            while (!isspace(*str) && *str) {
                str++;
            }
            continue;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt)) {
            field_width = skip_atoi(&fmt);
        }

        /* get conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || TOLOWER(*fmt) == 'l' ||
            TOLOWER(*fmt) == 'z') {
            qualifier = *fmt++;
            if (unlikely(qualifier == *fmt)) {
                if (qualifier == 'h') {
                    qualifier = 'H';
                    fmt++;
                } else if (qualifier == 'l') {
                    qualifier = 'L';
                    fmt++;
                }
            }
        }

        if (!*fmt || !*str) {
            break;
        }

        base = 10;
        is_sign = 0;
        is_float = 0;

        switch (*fmt++) {
        case 'c': {
            char *s = (char *)va_arg(args, char *);
            if (field_width == -1) {
                field_width = 1;
            }
            do {
                *s++ = *str++;
            } while (--field_width > 0 && *str);
            num++;
        }
        continue;
        case 's': {
            char *s = (char *)va_arg(args, char *);
            if (field_width == -1) {
                field_width = SHORT_MAX;
            }
            /* first, skip leading white space in buffer */
            str = skip_spaces(str);
            if (!s) {
                puts("vsscanf BUG!! dead while(1)\n");
                while (1);
            }
            /* now copy until next white space */
            while (*str && !isspace(*str) && field_width--) {
                *s++ = *str++;
            }
            *s = '\0';
            num++;
        }
        continue;
        case 'n':
            /* return number of characters read so far */
        {
            int *i = (int *)va_arg(args, int *);
            *i = str - buf;
        }
        continue;
        case 'o':
            base = 8;
            break;
        case 'x':
        case 'X':
            base = 16;
            break;
        case 'i':
            base = 0;
        case 'd':
            is_sign = 1;
        case 'u':
            break;
        case 'f':
            is_float = 1;
            break;
        case '%':
            /* looking for '%' in str */
            if (*str++ != '%') {
                return num;
            }
            continue;
        default:
            /* invalid format; stop here */
            return num;
        }

        /* have some sort of integer conversion.
         * first, skip white space in buffer.
         */
        str = skip_spaces(str);

        digit = *str;
        if ((is_sign || is_float) && digit == '-') {
            digit = *(str + 1);
        }

        if (!digit
            || (base == 16 && !isxdigit(digit))
            || (base == 10 && !isdigit(digit))
            || (base == 8 && (!isdigit(digit) || digit > '7'))
            || (base == 0 && !isdigit(digit))) {
            break;
        }

        switch (qualifier) {
        case 'H':	/* that's 'hh' in format */
            if (is_sign) {
                signed char *s = (signed char *)va_arg(args, signed char *);
                *s = (signed char)simple_strtol(str, &next, base);
            } else {
                unsigned char *s = (unsigned char *)va_arg(args, unsigned char *);
                *s = (unsigned char)simple_strtoul(str, &next, base);
            }
            break;
        case 'h':
            if (is_sign) {
                short *s = (short *)va_arg(args, short *);
                *s = (short)simple_strtol(str, &next, base);
            } else {
                unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
                *s = (unsigned short)simple_strtoul(str, &next, base);
            }
            break;
        case 'l':
            if (is_sign) {
                long *l = (long *)va_arg(args, long *);
                *l = simple_strtol(str, &next, base);
            } else if (is_float) {
                double *l = (double *)va_arg(args, double *);
                *l = strtod(str, &next);
            } else {
                unsigned long *l = (unsigned long *)va_arg(args, unsigned long *);
                *l = simple_strtoul(str, &next, base);
            }
            break;
        case 'L':
            if (is_sign) {
                long long *l = (long long *)va_arg(args, long long *);
                *l = simple_strtoll(str, &next, base);
            } else {
                unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
                *l = simple_strtoull(str, &next, base);
            }
            break;
        case 'Z':
        case 'z': {
            unsigned int *s = (unsigned int *)va_arg(args, unsigned int *);
            *s = (unsigned int)simple_strtoul(str, &next, base);
        }
        break;
        default:
            if (is_sign) {
                int *i = (int *)va_arg(args, int *);
                *i = (int)simple_strtol(str, &next, base);
            } else if (is_float) {
                double *l = (double *)va_arg(args, double *);
                *l = strtod(str, &next);
            } else {
                unsigned int *i = (unsigned int *)va_arg(args, unsigned int *);
                unsigned int j = (unsigned int)simple_strtoul(str, &next, base);
                unsigned char *bak = (unsigned char *)i;
                bak[0] = (unsigned char)(j & 0xff);
                bak[1] = (unsigned char)((j >> 8) & 0xff);
                bak[2] = (unsigned char)((j >> 16) & 0xff);
                bak[3] = (unsigned char)((j >> 24) & 0xff);
                /*unsigned int *i = (unsigned int *)va_arg(args, unsigned int *);*/
                /**i = (unsigned int)simple_strtoul(str, &next, base);*/
            }
            break;
        }
        num++;

        if (!next) {
            break;
        }
        str = next;
    }

    /*
     * Now we've come all the way through so either the input string or the
     * format ended. In the former case, there can be a %n at the current
     * position in the format that needs to be filled.
     */
    if (*fmt == '%' && *(fmt + 1) == 'n') {
        int *p = (int *)va_arg(args, int *);
        *p = str - buf;
    }

    return num;
}


/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int sscanf(const char *buf, const char *fmt, ...)   //BUG: 多个参数? 最后又空格?
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsscanf(buf, fmt, args);
    va_end(args);

    return i;
}

#if 0
/**
 * sprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf. Use snprintf() or scnprintf() in order to avoid
 * buffer overflows.
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, INT_MAX, fmt, args);
    va_end(args);

    return i;
}
#endif


/* flags */
#define LEFT    0x01
#define PLUS    0x02
#define SPACE   0x04
#define SPECIAL 0x08
#define ZERO    0x10
#define SIGN    0x20 /* signed if set */
#define SMALL   0x40 /* 'abcdef' if set, 'ABCDEF' otherwise */

int get_wide(const char **s)
{
    int res = 0;
    while (isdigit(**s)) {
        res = 10 * res + *((*s)++) - '0';
    }
    return res;
}

#define LONG_STRSIZE_BASE_2 32

void number_to_string(long num, int base, int flags, int wide, int precision, char **s)
{
    char sign;  /* sign printed : '+', '-', ' ', or 0 (no sign) */
    int num_cpy = num;
    unsigned long ul_num = (unsigned long) num; /* for unsigned format */

    /* string representation of num (reversed) */
    char tmp[LONG_STRSIZE_BASE_2];
    int i = 0; /* number of figures in tmp */

    const char *digits = "0123456789ABCDEF";
    if (flags & SMALL) {
        digits = "0123456789abcdef";
    }

    if ((base < 2) || (base > 16)) {
        return;
    }

    if ((flags & SIGN) && (num < 0)) {
        sign = '-';
        num = -num;
    } else {
        sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
    }
    if (sign) {
        wide--;
    }

    if (flags & SPECIAL) {
        if ((base == 16) && (num != 0)) {
            wide -= 2;    /* '0x' or '0X' */
        }
        if (base == 8) {
            wide--;    /* '0' */
            precision--;
        }
    }

    if (num == 0) {
        tmp[i++] = '0';
    }
    /* signed format */
    if (flags & SIGN) {
        while (num != 0) {
            tmp[i++] = digits[num % base];
            num = num / base;
        }
    }
    /* unsigned format */
    else {
        while (ul_num != 0) {
            tmp[i++] = digits[ul_num % base];
            ul_num = ul_num / base;
        }
    }

    if (i > precision) {
        precision = i;
    }
    wide -= precision;

    /* wide = number of padding chars */
    /* precision = number of figures after the sign and the special chars */

    /* right justified and no zeropad : pad with spaces */
    if (!(flags & (LEFT + ZERO))) while (wide-- > 0) {
            *((*s)++) = ' ';
        }

    if (sign) {
        *((*s)++) = sign;
    }
    if ((flags & SPECIAL) && (num_cpy != 0)) {
        if (base == 8) {
            *((*s)++) = '0';
        }
        if (base == 16) {
            *((*s)++) = '0';
            if (flags & SMALL) {
                *((*s)++) = 'x';
            } else {
                *((*s)++) = 'X';
            }
        }
    }

    /* rigth justified and zeropad : pad with 0 */
    if (!(flags & LEFT)) while (wide-- > 0) {
            *((*s)++) = '0';
        }

    /* print num */
    while (i < precision--) {
        *((*s)++) = '0';
    }
    while (i-- > 0) {
        *((*s)++) = tmp[i];
    }

    /* left justfied : pad with spaces */
    while (wide-- > 0) {
        *((*s)++) = ' ';
    }
}


int vsprintf(char *str, const char *format, va_list ap)
{
    char c;
    char *start = str;
    int flags;
    int wide;
    int precision;
    int qualifier;
    char *s;
    int i, len, base;

    while ((c = *format++) != 0) {
        if (c != '%') {
            *str++ = c;
            continue;
        }
        if (*format == '%') {
            *str++ = '%';
            format++;
            continue;
        }

        /* get flags */
        flags = 0;
        while (1) {
            if (*format == '-') {
                flags |= LEFT;
                format++;
                continue;
            }
            if (*format == '+') {
                flags |= PLUS;
                format++;
                continue;
            }
            if (*format == ' ') {
                flags |= SPACE;
                format++;
                continue;
            }
            if (*format == '#') {
                flags |= SPECIAL;
                format++;
                continue;
            }
            if (*format == '0') {
                flags |= ZERO   ;
                format++;
                continue;
            }
            break;
        }

        /* get wide */
        wide = -1;
        if (isdigit(*format)) {
            wide = get_wide((const char **)(&format));
        } else if (*format == '*') {
            wide = va_arg(ap, int);
            format++;
        }

        /* get precision */
        precision = -1;
        if (*format == '.') {
            format++;
            if (isdigit(*format)) {
                precision = get_wide((const char **)(&format));
            } else if (*format == '*') {
                precision = va_arg(ap, int);
                format++;
            } else {
                precision = 0;
            }
        }

        /* get qualifier */
        qualifier = -1;
        if ((*format == 'h') || (*format == 'l')) {
            qualifier = *format++;
        }

        /* get format */
        switch (*format++) {
        case 'i':
        case 'd':
            flags |= SIGN;
            if (precision != -1) {
                flags &= ~ZERO;
            }
            switch (qualifier) {
            case 'h':
                number_to_string((short) va_arg(ap, int), 10, flags,
                                 wide, precision, &str);
                break;
            case 'l':
                number_to_string(va_arg(ap, long), 10, flags,
                                 wide, precision, &str);
                break;
            default:
                number_to_string(va_arg(ap, int), 10, flags,
                                 wide, precision, &str);
                break;
            }
            break;

        case 'u':
            base = 10;
            goto num_to_str_without_sign;

        case 'o':
            base = 8;
            goto num_to_str_without_sign;

        case 'x':
            flags |= SMALL;
        case 'X':
            base = 16;

num_to_str_without_sign:
            flags &= (~PLUS & ~SPACE);
            if (precision != -1) {
                flags &= ~ZERO;
            }
            switch (qualifier) {
            case 'h':
                number_to_string((unsigned short) va_arg(ap, int), \
                                 base, flags, wide, precision, &str);
                break;
            case 'l':
                number_to_string((unsigned long) va_arg(ap, long), \
                                 base, flags, wide, precision, &str);
                break;
            default:
                number_to_string((unsigned int)va_arg(ap, int), \
                                 base, flags, wide, precision, &str);
                break;
            }
            break;

        case 's':
            s = va_arg(ap, char *);
            len = strlen(s);
            if ((precision >= 0) && (len > precision)) {
                len = precision;
            }

            /* rigth justified : pad with spaces */
            if (!(flags & LEFT)) while (len < wide--) {
                    *str++ = ' ';
                }
            for (i = 0; i < len; i++) {
                *str++ = *s++;
            }
            /* left justified : pad with spaces */
            while (len < wide--) {
                *str++ = ' ';
            }
            break;

        case 'c':
            /* rigth justified : pad with spaces */
            if (!(flags & LEFT)) while (1 < wide--) {
                    *str++ = ' ';
                }
            *str++ = (unsigned char) va_arg(ap, int);
            /* left justified : pad with spaces */
            while (1 < wide--) {
                *str++ = ' ';
            }
            break;

        default:
            return -1;
        }
    }
    *str = 0;

    return (int)(str - start);
}
