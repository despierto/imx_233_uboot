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

#define ETH_RX_POOL_SIZE        256

typedef struct _ETH_POOL_ {
    U32         addr;
    U32         size;    
}ETH_POOL, *PETH_POOL;


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
    
    uchar           cfg_mac_addr[ETHER_ADDR_LEN];
    IPaddr_t        cfg_ip_addr;
    IPaddr_t        cfg_ip_netmask;
    IPaddr_t        cfg_ip_gateway;    
    IPaddr_t        cfg_ip_server;
    IPaddr_t        cfg_ip_dns;
    IPaddr_t        cfg_ip_vlan;
    
	ETH_POOL		rx_pool[ETH_RX_POOL_SIZE];						/* queue of incomming packets */
	unsigned int	rx_pool_get;
	unsigned int	rx_pool_put;	

	ARP_TABLE		arp_table[ARP_TABLE_SIZE];
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
unsigned int drv_eth_rx_get(unsigned int *addr);
int         drv_eth_tx(volatile void *packet, int length);
void        drv_eth_parse_enetaddr(const char *addr, uchar *enetaddr);
IPaddr_t    drv_string_to_ip(char *s);
char *		drv_mac_to_string(uchar *mac);
char        *drv_ip_to_string(IPaddr_t ip, uchar *buf);
void        drv_eth_info(void);

PTR         drv_eth_heap_alloc(void);
int         drv_eth_heap_free(PTR ptr);

void 		drv_arp_table_info(void);
uchar 		drv_arp_table_check_ip(IPaddr_t ip, char **mac);
void 		drv_arp_table_reg_ip(IPaddr_t ip, char *mac, ushort type, unsigned int sys_time);

#endif /* __DRV_ETH_H__ */
