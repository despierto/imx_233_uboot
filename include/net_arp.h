/**
 * Top network layer: ARP table support
 *
 * Copyright (c) 2014 Alex Winter
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
 
 #ifndef __NET_ARP_H__
 #define __NET_ARP_H__

#include "types.h"
#include "drv_eth.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define ARP_TABLE_SIZE				10
#define ARP_TABLE_TYPE_NONE			0
#define ARP_TABLE_TYPE_ETH			1
#define ARP_TABLE_TYPE_VIRT			2
#define ARP_TABLE_VALID_PERIOD_SEC	30	/* 255 max */
#define ARP_TABLE_TTL_OBSOLETE		0

typedef struct _ARP_TABLE_
{
	IPaddr_t		ip_addr;
	uchar       	hw_addr[ETHER_ADDR_LEN];
	ushort 			type;	
	unsigned int 	reg_time;										/* time in sen when MAC addr wa placed into cache. 0 sec means MAC address is obsolete */
} ARP_TABLE, *PARP_TABLE;


/************************************************
 *               FUNCTIONS                   		                     *
 ************************************************/
int 		arp_table_create(void);
int 		arp_table_destroy(void);
void 		arp_table_info(void);
uchar 		arp_table_check_ip(IPaddr_t ip, char **mac);
void 		arp_table_reg_ip(IPaddr_t ip, char *mac, ushort type, unsigned int sys_time);




#endif /* __NET_ARP_H__ */

