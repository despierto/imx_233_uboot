/**
 * Top layer ethernel driver header file
 *
 * Copyright (c) 2014 X-boot GITHUB team
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

 #ifndef __DRV_ETH_H__
 #define __DRV_ETH_H__

#include "net.h"


#define	GBL_ETH_DIAG_ENA


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
//definitions
#define ETH_PKTSIZE             1518
#define ETH_PKTSIZE_ALIGN       1536
#define ETH_PKTALIGN            32
#define ETH_PKTBUFSRX           4                               /* Rx MAX supported by ks8851 is 12KB */
#define ETH_PKTBUFSTX           1                               /* Tx MAX supported by ks8851 is 6KB */

//declarations
typedef struct _ETH_CTX_ {
    volatile uchar *NetTxPackets[ETH_PKTBUFSTX];                        /* Transmit packets */
    volatile uchar *NetRxPackets[ETH_PKTBUFSRX];                        /* Receive packets */

    volatile uchar *NetArpWaitTxPacket;                             /* THE transmit packet */
    unsigned int    NetArpWaitTxPacketSize;

    unsigned int    Status;                                         /* disabled */
    char            BootFile[CONFIG_BOOTFILE_SIZE];
    unsigned int    linux_load_addr;

    uchar           *NetArpWaitPacketMAC;                            /* MAC address of waiting packet's destination */
    IPaddr_t        NetArpWaitPacketIP;
    IPaddr_t        NetArpWaitReplyIP;

    unsigned int    NetArpWaitTimerStart;                           /* in usec */
    unsigned int    NetArpWaitTry;

    ushort          NetIPID;                                        /* IP packet ID */
    ushort          PingSeqNo;                                      /* PING request counter */
    
    uchar           cfg_mac_addr[6];
    IPaddr_t        cfg_ip_addr;
    IPaddr_t        cfg_ip_netmask;
    IPaddr_t        cfg_ip_gateway;    
    IPaddr_t        cfg_ip_server;
    IPaddr_t        cfg_ip_dns;
    IPaddr_t        cfg_ip_vlan;
    

        
}ETH_CTX, *PETH_CTX;

typedef struct _ETH_HEAP_LIST_ {
    U32         *next;
    U32         addr;
    U32         status;    
}ETH_HEAP_LIST, *PETH_HEAP_LIST;

typedef struct _ETH_HEAP_CTX_ 
{
    ETH_HEAP_LIST   list[NET_PKT_COUNT];
    PETH_HEAP_LIST  next_alloc_item;    
    PETH_HEAP_LIST  next_free_item;        
    U32             stats_alloc;
    U32             stats_free;
    U32             stats_balance;
    
}ETH_HEAP_CTX, *PETH_HEAP_CTX;

extern PETH_CTX        pEth;


/************************************************
 *              FUNCTIONS                                                  *
 ************************************************/

/**
 * is_zero_ether_addr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 */
static inline int is_zero_ether_addr(const uchar *addr)
{
    return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline int is_multicast_ether_addr(const uchar *addr)
{
    return (0x01 & addr[0]);
}

/**
 * is_valid_ether_addr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 */
static inline int is_valid_ether_addr(const uchar * addr)
{
    /* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
     * explicitly check for it here. */
    return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}







int         drv_eth_init(void);
void        drv_eth_halt(void);
int         drv_eth_rx(void);
int         drv_eth_tx(volatile void *packet, int length);
void        drv_eth_parse_enetaddr(const char *addr, uchar *enetaddr);
IPaddr_t    drv_string_to_ip(char *s);
char        *drv_ip_to_string(IPaddr_t ip, uchar *buf);
void        drv_eth_info(void);

PTR         eth_heap_alloc(void);
int         eth_heap_free(PTR ptr);


#endif /* __DRV_ETH_H__ */
