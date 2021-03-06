/**
 * Drv utils header file
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


#ifndef __DRV_UTILS_H__
#define __DRV_UTILS_H__

#include "registers/regsdigctl.h"
#include "types.h"

void drv_delay(unsigned int us);

#undef  delay
#define delay       drv_delay
#define sleep(s)    drv_delay(s*1000000)
#define sleep_ms(ms)    drv_delay(ms*1000)
#define sleep_us(us)    drv_delay(us)

static inline unsigned int get_tick(void) 
{
    /* in usec */
    return (unsigned int)HW_DIGCTL_MICROSECONDS_RD();
}


IPaddr_t    drv_string_to_ip(char *s);
char 		*drv_mac_to_string(uchar *mac_out, uchar *mac_in);
char        *drv_ip_to_string(IPaddr_t ip, uchar *buf);
void        drv_string_to_mac(const char *addr, uchar *mac);


#endif /*__DRV_UTILS_H__*/

