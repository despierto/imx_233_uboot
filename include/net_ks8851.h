/**
 * Driver for Ethernet PHY KS8851: header file
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

#ifndef __NET_KS8851_H__
#define __NET_KS8851_H__

int     net_ks8851_init(void * ptr); 
void    net_ks8851_halt(void);
int     net_ks8851_rx(void);
int     net_ks8851_tx(volatile void *packet, int length);

#define net_init(ptr)           net_ks8851_init(ptr)
#define net_halt()              net_ks8851_halt()
#define net_rx()                net_ks8851_rx()
#define net_tx(packet, length)  net_ks8851_tx(packet, length)

#endif /* __NET_KS8851_H__ */

