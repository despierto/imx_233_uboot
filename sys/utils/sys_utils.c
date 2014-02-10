/**
 * SW Sys utils file
 *
 * Copyright (c) 2013 X-boot GITHUB team
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
#include "registers/regsuartdbg.h"
#include "sys_utils.h"
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



/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/

void memset(void * base, unsigned int c, unsigned int count)
{
    char *ptr = (char *) base;

    while (count--)
        *ptr++ = c;

    return;
}

void copy_filename (char *dst, char *src, int size)
{
    if (*src && (*src == '"')) {
        ++src;
        --size;
    }

    while ((--size > 0) && *src && (*src != '"')) {
        *dst++ = *src++;
    }
    *dst = '\0';

    return;
}

int strnlen(const char *s, unsigned int len)
{
    int n;
    n = 0;
    while (*s++ && n < len)
        n++;

    return (n);
}

void sys_print_assert(const char* filename, const char* funcname, const int nrow)
{
    printf("\r\n  ---- ASSERT ----\r\n\r\n");
    printf("       File:       %s\r\n", filename);
    printf("       Function:   %s\r\n", funcname);     
    printf("       Row:        %d\r\n\r\n", nrow);    

    return;
}

void sys_print_error(const char* filename, const char* funcname, const int nrow)
{
    printf("\r\n  ---- ERROR ----\r\n\r\n");
    printf("       File:       %s\r\n", filename);
    printf("       Function:   %s\r\n", funcname);     
    printf("       Row:        %d\r\n", nrow);    
    printf("       Caption:    ");    

    return;
}




/************************************************
  *              LOCAL  FUNCTIONS                                      *
  ************************************************/


void drv_print_printhex(int data)
{
    int i = 0;
    char c;
    for (i = sizeof(int)*2-1; i >= 0; i--) {
        c = data>>(i*4);
        c &= 0xf;
        if (c > 9)
            drv_print_putc(c-10+'A');
        else
            drv_print_putc(c+'0');
    }
    return;
}
void drv_print_printdec(int data)
{
    int i = 0;
    char s[10]; //max length of U32 dec value

    if (!data) {
        drv_print_putc('0');
    } else {
        while(data) {
            s[i++]= (char)data%10 +'0';
            data = data/10;
        }
        while(i) {
            drv_print_putc(s[--i]);
        }
    }
    return;
}
void drv_print_printstr(const char *s, int precision)
{
    int i;
    int len = strnlen(s, precision);

    for (i = 0; i < len; ++i)
        drv_print_putc(*s++);

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
                    drv_print_putc('%');
                    break;
                case 's':
                    drv_print_printstr(va_arg(args, char *), 255);
                    break;
                default:
                    break;
            }
        } else {
            drv_print_putc(*fmt);
        }

        fmt++;
    }
    va_end(args);
    
    return;
}


