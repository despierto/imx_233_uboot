/**
 * HW Print Driver header file
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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "types.h"
#include "error.h"
#include "drv_utils.h"
#include "sys_console.h"
#include "platform.h"
#include "sys_utils.h"
#include "sys_heap.h"
#include "net_datalink.h"


#define GBL_HEAP_BLOCKS_COUNT    100

extern PHEAPHEADER hGlobalHeap;
#define malloc(size)    sys_heap_alloc(hGlobalHeap, size)
#define free(ptr)        sys_heap_free(hGlobalHeap, ptr)


#define ETH_PKTBUFSRX           4           /* Rx MAX supported by ks8851 is 12KB */
#define ETH_PKTBUFSTX           1           /* Tx MAX supported by ks8851 is 6KB */

//declarations
typedef struct _GBL_CTX_ {
    uchar *NetTxPackets[ETH_PKTBUFSTX];         /* Transmit packets */
    uchar *NetRxPackets[ETH_PKTBUFSRX];           /* Receive packets */

    uchar *NetArpWaitTxPacket;                   /* THE transmit packet */
    int    NetArpWaitTxPacketSize;

    unsigned int    Status;                             /* disabled */
    char            BootFile[CONFIG_BOOTFILE_SIZE];
    unsigned int    linux_load_addr;

    uchar           *NetArpWaitPacketMAC;               /* MAC address of waiting packet's destination */
    IPaddr_t        NetArpWaitPacketIP;
    IPaddr_t        NetArpWaitReplyIP;

    unsigned int    NetArpWaitTimerStart;                 /* in usec */
    unsigned int    NetArpWaitTry;

    ushort          NetIPID;                              /* IP packet ID */
    ushort          PingSeqNo;                            /* PING request counter */

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



#endif /*_GLOBAL_H_*/
