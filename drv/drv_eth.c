/**
 * Top layer ethernel driver file
 *
 * Copyright (C) 2013 X-boot GITHUB team
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

int eth_init(void * ptr)
{
    int ret = 0;

    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);

    //ret = ks_init();
    return ret;
}

void eth_halt(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    //ks_disable();
}

int eth_rx(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    //ks_rx_pkts();
    return 0;
}

int eth_send(volatile void *packet, int length)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    //ks_tx_pkt((uchar *)packet, length);
    return 0;
}

