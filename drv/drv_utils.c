/**
 * Drv utils file
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
#include "drv_utils.h"
#include "sys_utils.h"

void drv_delay(unsigned int us)
{
    unsigned int start , cur;
    start = cur = HW_DIGCTL_MICROSECONDS_RD();

    while (cur < start+us) {

        cur = HW_DIGCTL_MICROSECONDS_RD();
        /*printf("0x%x\r\n",cur);*/
    }
    
    return;
}

IPaddr_t drv_string_to_ip(char *s)
{
    IPaddr_t addr;
    char *e;
    int i;

    if (s == NULL)
        return(0);

    for (addr=0, i=0; i<4; ++i) {
        ulong val = s ? simple_strtoul(s, &e, 10) : 0;
        
        addr <<= 8;
        addr |= (val & 0xFF);
        if (s) {
            s = (*e) ? e+1 : e;
        }
    }

    return (htonl(addr));
}

char *drv_ip_to_string(IPaddr_t ip, uchar *buf)
{
    sprintf((char *)buf, "%03d.%03d.%03d.%03d", (ip & 0xFF), ((ip >> 8) & 0xFF), ((ip >> 16) & 0xFF), ((ip >> 24) & 0xFF));
    return (char *)buf;
}

char *drv_mac_to_string(uchar *mac_out, uchar *mac_in)
{
	//unsigned int len = strnlen(mac, ETHER_ADDR_LEN);
	//if (len != ETHER_ADDR_LEN) {
	//	print_err("unexpected length (%d) of incoming mac address (%s)", len, mac);
	//}
	sprintf((char *)mac_out, "%02X:%02X:%02X:%02X:%02X:%02X", mac_in[0], mac_in[1], mac_in[2], mac_in[3], mac_in[4], mac_in[5]);

    return (char *)mac_out;
}

void drv_string_to_mac(const char *addr, uchar *mac)
{
    char *end;
    int i;
   
    for (i = 0; i < 6; ++i) {
        mac[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
        if (addr)
            addr = (*end) ? end + 1 : end;
    }
}

