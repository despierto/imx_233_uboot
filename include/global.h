/**
 * Global definitions
 *
 * Copyright (c) 2014 Alex Winter (eterno.despierto@gmail.com)
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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "types.h"
#include "error.h"
#include "drv_utils.h"
#include "platform.h"
#include "sys_utils.h"
#include "sys_heap.h"
#include "net_datalink.h"
#include "sys_cmgr.h"
#include "sys_pool.h"
#include "core.h"


#define GBL_HEAP_BLOCKS_COUNT    100

extern PHEAPHEADER hGlobalHeap;
#define malloc(size)    sys_heap_alloc(hGlobalHeap, size)
#define free(ptr)        sys_heap_free(hGlobalHeap, ptr)


#define ETH_PKTBUFSRX           4           /* Rx MAX supported by ks8851 is 12KB */
#define ETH_PKTBUFSTX           1           /* Tx MAX supported by ks8851 is 6KB */

//declarations
typedef struct _GBL_CTX_ {
    char            BootFile[CONFIG_BOOTFILE_SIZE];
    unsigned int    linux_load_addr;

    ushort          NetIPID;                /* IP packet ID */
    ushort          ping_req_num;           /* PING request counter */
    ushort          ping_ans_num;           /* PING answer counter */
    uint            ping_reg_time;          /* PING request counter */    
    uint            ping_ans_time;          /* PING answer time: 0 or round trip time per request*/        
    uint            ping_ans_bytes;         /* PING answer bytes*/            
    uint            ping_rtt_sum;           /* PING round trip time sum for average time computation*/            
    IPaddr_t        ping_ans_ttl;           /* PING answer TTL*/    
    IPaddr_t        ping_ip;                /* IP to PING*/

    //system configuration
    uchar           cfg_mac_addr[ETHER_ADDR_LEN];
    IPaddr_t        cfg_ip_addr;
    IPaddr_t        cfg_ip_netmask;
    IPaddr_t        cfg_ip_gateway;    
    IPaddr_t        cfg_ip_server;
    IPaddr_t        cfg_ip_dns;
    IPaddr_t        cfg_ip_vlan;
}GBL_CTX, *PGBL_CTX;

extern PGBL_CTX    pGblCtx;

void         gbl_pring_info(void);


#endif /*_GLOBAL_H_*/
