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

#ifndef __DRV_KS8851_H__
#define __DRV_KS8851_H__

#include "types.h"


/************************************************
 *              FEATURES                                                    *
 ************************************************/

#define NET_RX_FILTER_PERFECT                           1
#define NET_RX_FILTER_INVERSE_PERFECT                   2
#define NET_RX_FILTER_HASH_ONLY                         3
#define NET_RX_FILTER_INVERSE_HASH_ONLY                 4
#define NET_RX_FILTER_HASH_PERFECT                      5
#define NET_RX_FILTER_INVERSE_HASH_PERFECT              6
#define NET_RX_FILTER_PROMISCUOUS                       7
#define NET_RX_FILTER_HASH_ONLY_W_MULTICAST_ADDR_PASSED 8
#define NET_RX_FILTER_PERFECT_W_MULTICAST_ADDR_PASSED   9
#define NET_RX_FILTER_HASH_ONLY_W_PHYS_ADDR_PASSED      10
#define NET_RX_FILTER_PERFECT_W_PHYS_ADDR_PASSED        11

#define NET_HW_RX_HEADER_SIZE							8

/************************************************
 *              APIs                                                            *
 ************************************************/
RESULTCODE  ks8851_init(PTR ptr); 
void        ks8851_halt(void);
U32         ks8851_rxfc_get(void);
U32         ks8851_rx(PTR rx_buff);
RESULTCODE  ks8851_tx(PTR packet, uint length);
void        ks8851_mac_set(const char *ethaddr);

#endif /* __DRV_KS8851_H__ */

