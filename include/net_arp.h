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
#include "netdefs.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define ARP_TABLE_SIZE                  (10)
#define ARP_TABLE_TYPE_ETH              (1)
#define ARP_TABLE_TYPE_VIRT             (2)

typedef enum {
    ARP_TABLE_STATE_INVALID,
    ARP_TABLE_STATE_VALID,
    ARP_TABLE_STATE_WAIT_ARP_RESPOND
} ARP_TABLE_STATE;

typedef struct _ARP_TABLE_
{
    IPaddr_t        ip_addr;
    uchar           hw_addr[ETHER_ADDR_LEN];
    uchar           type;    
    uchar           state;        
    unsigned int    reg_time;                                        /* time in sen when MAC addr wa placed into cache. 0 sec means MAC address is obsolete */
} ARP_TABLE, *PARP_TABLE;


/************************************************
 *               FUNCTIONS                                                *
 ************************************************/
int             arp_table_create(void);
int             arp_table_destroy(void);
void            arp_table_info(void);
ARP_TABLE_STATE arp_table_get_mac(IPaddr_t ip, uchar **mac);
void            arp_table_reg_ip(IPaddr_t ip, char *mac, uchar type, uchar state);
void            arp_table_set_valid_period(unsigned int valid_period_sec);
int             arp_table_update_valid_period(IPaddr_t ip);


#endif /* __NET_ARP_H__ */

