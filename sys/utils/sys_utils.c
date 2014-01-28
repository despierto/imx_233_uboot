/**
 * SW Sys utils file
 *
 * Copyright (c) 2013 X-boot GITHUB team
 *
 * vsprintf.c -- Lars Wirzenius & Linus Torvalds.
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
#include "sys_utils.h"
#include "drv_print.h"


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



static int skip_atoi(const char **s);

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


