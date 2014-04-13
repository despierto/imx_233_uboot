/**
 * SW Sys utils file
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

unsigned int strlen(const char * s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
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

void *memcpy(void *dst, const void *src, int n)
{
    char *d = dst;
    const char *s = src;

    while (n-- > 0)
        *d++ = *s++;

    return dst;
}

int memcmp(void *src, void *dst, int size)
{
    uchar *s = (uchar *)src;
    uchar *d = (uchar *)dst;
    int rc = TRUE;
    
    while (size--) {
       if (s[size] != d[size]) {
          rc = FALSE;
          break;
       }
    }
    return rc;
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

int sys_checksum_ok(ushort * ptr, int len)
{
    return !((sys_checksum(ptr, len) + 1) & 0xfffe);
}

uint sys_checksum(ushort * ptr, int len)
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

int sys_check_IPv4_string(const char *ip)
{
#if 0
    int i, j = 0, size, dot;
    char sdata[255]= {0};

    size = strnlen(ip, IP_ADDR_STR_LEN+1);
    if (size > IP_ADDR_STR_LEN) {
        print_err("wrong IPv4 lenght, length (%d)", size);
        return FAILURE;
    }

    dot =0;
    for (i = 0; i < size; i++) {
        if((ip[i]>='0') && (ip[i] <= '9')) {
            sdata[j++] = ip[i];

            if (j >= 3) {
                int value = strtol(sdata, NULL, 0);
                if ((value == 0) || (value == LONG_MAX) || (value >= 256)) {
                    print_err("subnet number is out of order: (%d)", value);
                    return FAILURE;
                }
            }
        } else if (ip[i] == '.') {
             if ((i == (size-1)) || (j==0)) {
                 print_err("wrong format of IPv4 address: (%s)", ip);
                 return FAILURE;
             }
             dot++;
             j = 0;
        } else {
            print_err("wrong input char: (%d)", ip[i]);
            return FAILURE;
        }
    }

    if (dot != 3) {
        print_err("wrong count of subnetworks (%d) in IPv4 address: (%s)", dot, ip);
        return FAILURE;
    }
#endif
    return 0;
}

/************************************************
  *              LOCAL  FUNCTIONS                                      *
  ************************************************/


