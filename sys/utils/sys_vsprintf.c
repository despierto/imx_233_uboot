/**
 * SW Sys vsprintf  file
 *
 * Copyright (C) 1991, 1992  Linus Torvalds
 * vsprintf.c -- Lars Wirzenius & Linus Torvalds.  
 * 
 * Copyright (c) 2014 Alex Winter (eterno.despierto@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdarg.h>
#include "types.h"
#include "sys_utils.h"
#include "platform.h"
#include "dbguart.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
 
#define ZEROPAD     1       /* pad with zero */
#define SIGN        2       /* unsigned/signed long */
#define PLUS        4       /* show plus */
#define SPACE       8       /* space if plus */
#define LEFT        16      /* left justified */
#define SMALL       32      /* Must be 32 == 0x20 */
#define SPECIAL     64      /* 0x */

#define NUM_TYPE    long

//#define do_div(n, base) ({ \
//                            unsigned int __res; \
//                            __res = (unsigned int)((unsigned NUM_TYPE) n) % base; \
//                            n = ((unsigned NUM_TYPE) n) / base; \
//                            __res; \
//                        })

const char hex_asc[] = "0123456789abcdef";
#define hex_asc_lo(x)   hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)   hex_asc[((x) & 0xf0) >> 4]

/* No inlining helps gcc to use registers better */
#define noinline __attribute__((noinline))

/* Works only for digits and letters, but small and fast */
#define TOLOWER(x) ((x) | 0x20)



int             vsprintf(char *buf, const char *fmt, va_list args);
static int      skip_atoi(const char **s);
static char     *pointer(const char *fmt, char *buf, void *ptr, int field_width, int precision, int flags);
static inline char *pack_hex_byte(char *buf, uchar byte);
unsigned long   strtoul(const char *cp,char **endp,unsigned int base);
long            strtol(const char *cp,char **endp,unsigned int base);
int             ustrtoul(const char *cp, char **endp, unsigned int base);
static char*    put_dec_trunc(char *buf, unsigned q);
static char*    put_dec_full(char *buf, unsigned q);
static noinline char* put_dec(char *buf, unsigned NUM_TYPE num);
static char     *number(char *buf, unsigned NUM_TYPE num, int base, int size, int precision, int type);
static char     *string(char *buf, char *s, int field_width, int precision, int flags);
static char     *mac_address_string(char *buf, uchar *addr, int field_width, int precision, int flags);
/*static char *ip6_addr_string(char *buf, uchar *addr, int field_width, int precision, int flags);*/
static char     *ip4_addr_string(char *buf, uchar *addr, int field_width, int precision, int flags);
int             vsscanf(const char * buf, const char * fmt, va_list args);
long long strtoll(const char *cp, char **endp, unsigned int base);
unsigned long long strtoull(const char *cp, char **endp, unsigned int base);
static unsigned int simple_guess_base(const char *cp);



/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/

/**
 * sprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf.
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int sprintf(char * buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf,fmt,args);
    va_end(args);
    return i;
}

void sys_printf(const char *fmt, ...)
{
    va_list args;
    uint i;
    char printbuffer[CONFIG_SYS_PBSIZE];

    va_start(args, fmt);

    /* For this to work, printbuffer must be larger than
        * anything we ever want to print.
        */
    i = vsprintf(printbuffer, fmt, args);
    va_end(args);

    /* Print the string */
    drv_serial_puts(printbuffer);
}

int sys_sscanf(const char * buf, const char * fmt, ...)
{
    va_list args;
    int i = 0;

    va_start(args, fmt);
    i = vsscanf(buf,fmt,args);
    va_end(args);
    return i;
}

#if 0 //light printf
void drv_print_printhex(int data)
{
    int i = 0;
    char c;
    for (i = sizeof(int)*2-1; i >= 0; i--) {
        c = data>>(i*4);
        c &= 0xf;
        if (c > 9)
            drv_serial_putc(c-10+'A');
        else
            drv_serial_putc(c+'0');
    }
    return;
}
void drv_print_printdec(int data)
{
    int i = 0;
    char s[10]; //max length of U32 dec value

    if (!data) {
        drv_serial_putc('0');
    } else {
        while(data) {
            s[i++]= (char)data%10 +'0';
            data = data/10;
        }
        while(i) {
            drv_serial_putc(s[--i]);
        }
    }
    return;
}
void drv_print_printstr(const char *s, int precision)
{
    int i;
    int len = strnlen(s, precision);

    for (i = 0; i < len; ++i)
        drv_serial_putc(*s++);

}
void drv_print_printf(const char *fmt, ...)
{
    va_list args;
    int one;
    va_start(args, fmt);
    
    while (*fmt) {

        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd':
                    drv_print_printdec(va_arg(args, int));
                    break;
                case 'x':
                case 'X':
                    drv_print_printhex(va_arg(args, int));
                    break;
                case '%':
                    drv_serial_putc('%');
                    break;
                case 's':
                    drv_print_printstr(va_arg(args, char *), 255);
                    break;
                default:
                    break;
            }
        } else {
            drv_serial_putc(*fmt);
        }

        fmt++;
    }
    va_end(args);
    
    return;
}
#endif

/************************************************
  *              LOCAL  FUNCTIONS                                      *
  ************************************************/

static inline char *pack_hex_byte(char *buf, uchar byte)
{
    *buf++ = hex_asc_hi(byte);
    *buf++ = hex_asc_lo(byte);
    return buf;
}

unsigned long strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;

    if (*cp == '0') {
        cp++;
        if ((*cp == 'x') && isxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if (!base) {
            base = 8;
        }
    }
    if (!base) {
        base = 10;
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
        ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp) {
        *endp = (char *)cp;
    }
    
    return result;
}

long strtol(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -strtoul(cp+1,endp,base);
    return strtoul(cp,endp,base);
}

static unsigned int simple_guess_base(const char *cp)
{
    if (cp[0] == '0') {
        if (TOLOWER(cp[1]) == 'x' && isxdigit(cp[2]))
            return 16;
        else
            return 8;
    } else {
        return 10;
    }
}

/**
 * strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long long strtoull(const char *cp, char **endp, unsigned int base)
{
    unsigned long long result = 0;

    if (!base)
        base = simple_guess_base(cp);

    if (base == 16 && cp[0] == '0' && TOLOWER(cp[1]) == 'x')
        cp += 2;

    while (isxdigit(*cp)) {
        unsigned int value;

        value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
        if (value >= base)
            break;
        result = result * base + value;
        cp++;
    }

    if (endp)
        *endp = (char *)&cp;
    return result;
}

/**
 * strtoll - convert a string to a signed long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long long strtoll(const char *cp, char **endp, unsigned int base)
{
    if(*cp=='-')
        return -strtoull(cp + 1, endp, base);
    return strtoull(cp, endp, base);
}

/**
 * vsprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * This function follows C99 vsprintf, but has some extensions:
 * %pS output the name of a text symbol
 * %pF output the name of a function pointer
 * %pR output the address range in a struct resource
 *
 * The function returns the number of characters written into @buf.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want sprintf() instead.
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
    unsigned NUM_TYPE num;
    int base;
    char *str;

    int flags;              /* flags to number() */

    int field_width;        /* width of output field */
    int precision;          /* min. # of digits for integers; max number of chars for from string */
    int qualifier;          /* 'h', 'l', or 'L' for integer fields */
                            /* 'z' support added 23/7/1999 S.H.    */
                            /* 'z' changed to 'Z' --davidm 1/25/99 */
                            /* 't' added for ptrdiff_t */
    str = buf;

    for (; *fmt ; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        /* process flags */
        flags = 0;
        repeat:
            ++fmt;          /* this also skips first '%' */
            switch (*fmt) {
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
                case '%': *str++ = *fmt; continue;
            }

        /* get field width */
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
            *fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
                qualifier = *fmt;
                ++fmt;
                if (qualifier == 'l' && *fmt == 'l') {
                    qualifier = 'L';
                    ++fmt;
            }
        }

        /* default base */
        base = 10;

        switch (*fmt) {
        case 'c':
            if (!(flags & LEFT))
                while (--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char) va_arg(args, int);
            while (--field_width > 0)
                *str++ = ' ';
            continue;

        case 's':
            str = string(str, va_arg(args, char *), field_width, precision, flags);
            continue;

        case 'p':
            str = pointer(fmt+1, str, va_arg(args, void *), 
                field_width, precision, flags);
            /* Skip all alphanumeric pointer suffixes */
            while (isalnum(fmt[1]))
                fmt++;
            continue;

        case 'n':
            if (qualifier == 'l') {
                long * ip = va_arg(args, long *);
                *ip = (str - buf);
            } else {
                int * ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;

        case '%':
            *str++ = '%';
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'x':
            flags |= SMALL;
        case 'X':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            *str++ = '%';
            if (*fmt)
                *str++ = *fmt;
            else
                --fmt;
            continue;
        }

        if (qualifier == 'l') {
            num = va_arg(args, unsigned long);
            if (flags & SIGN)
                num = (signed long) num;
        } else if (qualifier == 'Z' || qualifier == 'z') {
            num = va_arg(args, size_t);
        } else if (qualifier == 't') {
            num = va_arg(args, ptrdiff_t);
        } else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
                num = (signed short) num;
        } else {
            num = va_arg(args, unsigned int);
            if (flags & SIGN)
                num = (signed int) num;
        }
        str = number(str, num, base, field_width, precision, flags);
    }
    *str = '\0';
    return str-buf;
}

/* Function checks string and skips all digits. Atoi converts a text to integer value */
static int skip_atoi(const char **s)
{
    int i=0;

    while (is_digit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way (dot-separated
 *       decimal for v4 and colon separated network-order 16 bit hex for v6)
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses, IPv6 omits the colons, IPv4 is
 *       currently the same
 *
 * Note: The difference between 'S' and 'F' is that on ia64 and ppc64
 * function pointers are really function descriptors, which contain a
 * pointer to the real address.
 */
static char *pointer(const char *fmt, char *buf, void *ptr, int field_width, int precision, int flags)
{
    if (!ptr)
        return string(buf, "(null)", field_width, precision, flags);

    switch (*fmt) {
    case 'm':
        flags |= SPECIAL;
        /* Fallthrough */
    case 'M':
        return mac_address_string(buf, ptr, field_width, precision, flags);
    case 'i':
        flags |= SPECIAL;
        /* Fallthrough */
    case 'I':
        //if (fmt[1] == '6')
        //    return ip6_addr_string(buf, ptr, field_width, precision, flags);
        if (fmt[1] == '4')
            return ip4_addr_string(buf, ptr, field_width, precision, flags);
        flags &= ~SPECIAL;
        break;
    }

    flags |= SMALL;
    if (field_width == -1) {
        field_width = 2*sizeof(void *);
        flags |= ZEROPAD;
    }
    return number(buf, (unsigned long) ptr, 16, field_width, precision, flags);
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
static char* put_dec_trunc(char *buf, unsigned q)
{
    unsigned d3, d2, d1, d0;
    d1 = (q>>4) & 0xf;
    d2 = (q>>8) & 0xf;
    d3 = (q>>12);

    d0 = 6*(d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10*q;
    *buf++ = d0 + '0'; /* least significant digit */
    d1 = q + 9*d3 + 5*d2 + d1;
    if (d1 != 0) {
        q = (d1 * 0xcd) >> 11;
        d1 = d1 - 10*q;
        *buf++ = d1 + '0'; /* next digit */

        d2 = q + 2*d2;
        if ((d2 != 0) || (d3 != 0)) {
            q = (d2 * 0xd) >> 7;
            d2 = d2 - 10*q;
            *buf++ = d2 + '0'; /* next digit */

            d3 = q + 4*d3;
            if (d3 != 0) {
                q = (d3 * 0xcd) >> 11;
                d3 = d3 - 10*q;
                *buf++ = d3 + '0';  /* next digit */
                if (q != 0)
                    *buf++ = q + '0';  /* most sign. digit */
            }
        }
    }
    return buf;
}

/* Same with if's removed. Always emits five digits */
static char* put_dec_full(char *buf, unsigned q)
{
    /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
    /* but anyway, gcc produces better code with full-sized ints */
    unsigned d3, d2, d1, d0;
    d1 = (q>>4) & 0xf;
    d2 = (q>>8) & 0xf;
    d3 = (q>>12);

    /*
     * Possible ways to approx. divide by 10
     * gcc -O2 replaces multiply with shifts and adds
     * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
     * (x * 0x67) >> 10:  1100111
     * (x * 0x34) >> 9:    110100 - same
     * (x * 0x1a) >> 8:     11010 - same
     * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
     */

    d0 = 6*(d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10*q;
    *buf++ = d0 + '0';
    d1 = q + 9*d3 + 5*d2 + d1;
        q = (d1 * 0xcd) >> 11;
        d1 = d1 - 10*q;
        *buf++ = d1 + '0';

        d2 = q + 2*d2;
            q = (d2 * 0xd) >> 7;
            d2 = d2 - 10*q;
            *buf++ = d2 + '0';

            d3 = q + 4*d3;
                q = (d3 * 0xcd) >> 11; /* - shorter code */
                /* q = (d3 * 0x67) >> 10; - would also work */
                d3 = d3 - 10*q;
                *buf++ = d3 + '0';
                    *buf++ = q + '0';
    return buf;
}

/* No inlining helps gcc to use registers better */
static noinline char* put_dec(char *buf, unsigned NUM_TYPE num)
{
    while (1) {
        unsigned int rem;
        if (num < 100000)   // TODO: number is convernig to string ti unexpected 0 in the middle: instead of 16775200 ->167705200
            return put_dec_trunc(buf, num);
        //rem = do_div(num, 100000);
        rem = (unsigned int)(num % 10000);
        num = (unsigned NUM_TYPE)(num / 10000);
        buf = put_dec_full(buf, rem);
    }
}

static char *number(char *buf, unsigned NUM_TYPE num, int base, int size, int precision, int type)
{
    /* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
    static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

    char tmp[66];
    char sign;
    char locase;
    int need_pfx = ((type & SPECIAL) && base != 10);
    int i;

    /* locase = 0 or 0x20. ORing digits or letters with 'locase'
     * produces same digits or (maybe lowercased) letters */
    locase = (type & SMALL);
    if (type & LEFT)
        type &= ~ZEROPAD;
    sign = 0;
    if (type & SIGN) {
        if ((signed NUM_TYPE) num < 0) {
            sign = '-';
            num = - (signed NUM_TYPE) num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }
    if (need_pfx) {
        size--;
        if (base == 16)
            size--;
    }

    /* generate full string in tmp[], in reverse order */
    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    /* Generic code, for any base:
    else do {
        tmp[i++] = (digits[do_div(num,base)] | locase);
    } while (num != 0);
    */
    else if (base != 10) { /* 8 or 16 */
        int mask = base - 1;
        int shift = 3;
        if (base == 16) shift = 4;
        do {
            tmp[i++] = (digits[((unsigned char)num) & mask] | locase);
            num >>= shift;
        } while (num);
    } else { /* base 10 */
        i = put_dec(tmp, num) - tmp;
    }

    /* printing 100 using %2d gives "100", not "00" */
    if (i > precision)
        precision = i;
    /* leading space padding */
    size -= precision;
    if (!(type & (ZEROPAD+LEFT)))
        while(--size >= 0)
            *buf++ = ' ';
    /* sign */
    if (sign)
        *buf++ = sign;
    /* "0x" / "0" prefix */
    if (need_pfx) {
        *buf++ = '0';
        if (base == 16)
            *buf++ = ('X' | locase);
    }
    /* zero or space padding */
    if (!(type & LEFT)) {
        char c = (type & ZEROPAD) ? '0' : ' ';
        while (--size >= 0)
            *buf++ = c;
    }
    /* hmm even more zero padding? */
    while (i <= --precision)
        *buf++ = '0';
    /* actual digits of result */
    while (--i >= 0)
        *buf++ = tmp[i];
    /* trailing space padding */
    while (--size >= 0)
        *buf++ = ' ';
    return buf;
}

static char *string(char *buf, char *s, int field_width, int precision, int flags)
{
    int len, i;

    if (s == 0)
        s = "<NULL>";

    len = strnlen(s, precision);

    if (!(flags & LEFT))
        while (len < field_width--)
            *buf++ = ' ';
    for (i = 0; i < len; ++i)
        *buf++ = *s++;
    while (len < field_width--)
        *buf++ = ' ';
    return buf;
}

static char *mac_address_string(char *buf, uchar *addr, int field_width, int precision, int flags)
{
    char mac_addr[6 * 3]; /* (6 * 2 hex digits), 5 colons and trailing zero */
    char *p = mac_addr;
    int i;

    for (i = 0; i < 6; i++) {
        p = pack_hex_byte(p, addr[i]);
        if (!(flags & SPECIAL) && i != 5)
            *p++ = ':';
    }
    *p = '\0';

    return string(buf, mac_addr, field_width, precision, flags & ~SPECIAL);
}

/*
static char *ip6_addr_string(char *buf, uchar *addr, int field_width, int precision, int flags)
{
    char ip6_addr[8 * 5]; // (8 * 4 hex digits), 7 colons and trailing zero
    char *p = ip6_addr;
    int i;

    for (i = 0; i < 8; i++) {
        p = pack_hex_byte(p, addr[2 * i]);
        p = pack_hex_byte(p, addr[2 * i + 1]);
        if (!(flags & SPECIAL) && i != 7)
            *p++ = ':';
    }
    *p = '\0';

    return string(buf, ip6_addr, field_width, precision, flags & ~SPECIAL);
}
*/

static char *ip4_addr_string(char *buf, uchar *addr, int field_width, int precision, int flags)
{
    char ip4_addr[4 * 4]; /* (4 * 3 decimal digits), 3 dots and trailing zero */
    char temp[3];    /* hold each IP quad in reverse order */
    char *p = ip4_addr;
    int i, digits;

    for (i = 0; i < 4; i++) {
        digits = put_dec_trunc(temp, addr[i]) - temp;
        /* reverse the digits in the quad */
        while (digits--)
            *p++ = temp[digits];
        if (i != 3)
            *p++ = '.';
    }
    *p = '\0';

    return string(buf, ip4_addr, field_width, precision, flags & ~SPECIAL);
}

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:    input buffer
 * @fmt:    format of buffer
 * @args:    arguments
 */
int vsscanf(const char * buf, const char * fmt, va_list args)
{
    const char *str = buf;
    char *next;
    char digit;
    int num = 0;
    int qualifier;
    int base;
    int field_width;
    int is_sign = 0;

    while(*fmt && *str) {
        /* skip any white space in format */
        /* white space in format matchs any amount of
                * white space, including none, in the input.
                */
        if (isspace(*fmt)) {
            while (isspace(*fmt))
                ++fmt;
            while (isspace(*str))
                ++str;
        }

        /* anything that is not a conversion must match exactly */
        if (*fmt != '%' && *fmt) {
            if (*fmt++ != *str++)
                break;
            continue;
        }

        if (!*fmt)
            break;
        ++fmt;
        
        /* skip this conversion.
         * advance both strings to next white space
         */
        if (*fmt == '*') {
            while (!isspace(*fmt) && *fmt)
                fmt++;
            while (!isspace(*str) && *str)
                str++;
            continue;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);

        /* get conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
            *fmt == 'Z' || *fmt == 'z') {
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
        base = 10;
        is_sign = 0;

        if (!*fmt || !*str)
            break;

        switch(*fmt++) {
        case 'c':
        {
            char *s = (char *) va_arg(args,char*);
            if (field_width == -1)
                field_width = 1;
            do {
                *s++ = *str++;
            } while (--field_width > 0 && *str);
            num++;
        }
        continue;
        case 's':
        {
            char *s = (char *) va_arg(args, char *);
            if(field_width == -1)
                field_width = INT_MAX;
            /* first, skip leading white space in buffer */
            while (isspace(*str))
                str++;

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
            int *i = (int *)va_arg(args,int*);
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
        case '%':
            /* looking for '%' in str */
            if (*str++ != '%') 
                return num;
            continue;
        default:
            /* invalid format; stop here */
            return num;
        }

        /* have some sort of integer conversion.
         * first, skip white space in buffer.
         */
        while (isspace(*str))
            str++;

        digit = *str;
        if (is_sign && digit == '-')
            digit = *(str + 1);

        if (!digit
                    || (base == 16 && !isxdigit(digit))
                    || (base == 10 && !isdigit(digit))
                    || (base == 8 && (!isdigit(digit) || digit > '7'))
                    || (base == 0 && !isdigit(digit)))
                break;

        switch(qualifier) {
        case 'H':    /* that's 'hh' in format */
            if (is_sign) {
                signed char *s = (signed char *) va_arg(args,signed char *);
                *s = (signed char) strtol(str,&next,base);
            } else {
                unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);
                *s = (unsigned char) strtoul(str, &next, base);
            }
            break;
        case 'h':
            if (is_sign) {
                short *s = (short *) va_arg(args,short *);
                *s = (short) strtol(str,&next,base);
            } else {
                unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);
                *s = (unsigned short) strtoul(str, &next, base);
            }
            break;
        case 'l':
            if (is_sign) {
                long *l = (long *) va_arg(args,long *);
                *l = strtol(str,&next,base);
            } else {
                unsigned long *l = (unsigned long*) va_arg(args,unsigned long*);
                *l = strtoul(str,&next,base);
            }
            break;
        case 'L':
            if (is_sign) {
                long long *l = (long long*) va_arg(args,long long *);
                *l = strtoll(str,&next,base);
            } else {
                unsigned long long *l = (unsigned long long*) va_arg(args,unsigned long long*);
                *l = strtoull(str,&next,base);
            }
            break;
        case 'Z':
        case 'z':
        {
            size_t *s = (size_t*) va_arg(args,size_t*);
            *s = (size_t) strtoul(str,&next,base);
        }
        break;
        default:
            if (is_sign) {
                int *i = (int *) va_arg(args, int*);
                *i = (int) strtol(str,&next,base);
            } else {
                unsigned int *i = (unsigned int*) va_arg(args, unsigned int*);
                *i = (unsigned int) strtoul(str,&next,base);
            }
            break;
        }
        num++;

        if (!next)
            break;
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
