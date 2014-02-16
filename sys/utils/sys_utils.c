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

#include "sys_utils.h"


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

/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
int strcmp(const char * cs,const char * ct)
{
    register signed char __res;

    while (1) {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
    }

    return __res;
}

/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int strncmp(const char * cs,const char * ct,size_t count)
{
    register signed char __res = 0;

    while (count) {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count--;
    }

    return __res;
}

void *memcpy(void *s1, const void *s2, int n)
{
    char *dst = s1;
    const char *src = s2;

    while (n-- > 0)
        *dst++ = *src++;

    return s1;
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

int sys_checksum_ok(uchar * ptr, int len)
{
    return !((sys_checksum(ptr, len) + 1) & 0xfffe);
}

uint sys_checksum(uchar * ptr, int len)
{
    ulong   xsum;
    ushort *p = (ushort *)ptr;

    xsum = 0;
    while (len-- > 0)
        xsum += *p++;
    
    xsum = (xsum & 0xffff) + (xsum >> 16);
    xsum = (xsum & 0xffff) + (xsum >> 16);
    
    return (xsum & 0xffff);
}



/************************************************
  *              LOCAL  FUNCTIONS                                      *
  ************************************************/


