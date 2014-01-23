/**
 * Top layer ethernel driver file
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

#include "global.h"
#include "net_ks8851.h"


int drv_eth_init(void * ptr)
{
    int ret = 0;

    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);

    ret = net_init(ptr);
    return ret;
}

void drv_eth_halt(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    net_halt();
}

int drv_eth_rx(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    net_rx();
    return 0;
}

int drv_eth_tx(volatile void *packet, int length)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    net_tx(packet, length);
    return 0;
}

