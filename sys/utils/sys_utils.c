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
#include "sys_utils.h"
#include "drv_print.h"

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


