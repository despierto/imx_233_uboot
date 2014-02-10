/**
 * HW Debug UART Driver file
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

 //#include <stdio.h>
//#include <stdarg.h>
//#include "types.h"
//#include "error.h"

#include "registers/regsuartdbg.h"
#include "dbguart.h"

void drv_print_putc(char ch)
{
    int loop = 0;
    while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF) {
        loop++;
        if (loop > 10000)
            break;
    };

    /* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
    HW_UARTDBGDR_WR(ch);
    
    if (ch == '\n')
        drv_print_putc('\r');
    
    return;
}

void drv_print_puts(const char *s)
{
    while (*s) {
        drv_print_putc(*s++);
    }
}



