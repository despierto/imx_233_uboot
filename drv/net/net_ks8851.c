/**
 * Driver for Ethernet PHY KS8851
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

int     net_ks8851_init(void * ptr)
{
    int ret = 0;
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);

    return ret;
}

void    net_ks8851_halt(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);

    return;
}

int     net_ks8851_rx(void)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    return 0;
}

int     net_ks8851_tx(volatile void *packet, int length)
{
    printf("--> %s -> %s : %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    
    return 0;
}


