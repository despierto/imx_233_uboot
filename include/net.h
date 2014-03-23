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

 #ifndef __NET_H__
 #define __NET_H__

#include "types.h"
#include "net_arp.h"
#include "net_datalink.h"

/************************************************
 *              PROTOCOL HEADERS DEFINITIONS                *
 ************************************************/



#define VLAN_ETHER_HDR_SIZE 18              /* VLAN Ethernet header size */

#define PROT_IP             0x0800          /* IP protocol */
#define PROT_ARP            0x0806          /* IP ARP protocol */
#define PROT_RARP           0x8035          /* IP ARP protocol */
#define PROT_VLAN           0x8100          /* IEEE 802.1q protocol */

#define IPPROTO_ICMP        1               /* Internet Control Message Protocol */
#define IPPROTO_UDP         17              /* User Datagram Protocol */

#define PROT_IPV4_VERSION   4               /* IPv4 protocol version*/
#define PROT_IPV6_VERSION   6               /* IPv6 protocol version*/


/* Internet Protocol v4 (IPv4) header */
typedef struct {
    uchar       ip_hl_v;                    /* header length and version */
    uchar       ip_tos;                     /* type of service */
    ushort      ip_len;                     /* total length */
    ushort      ip_id;                      /* identification */
    ushort      ip_off;                     /* fragment offset field */
    uchar       ip_ttl;                     /* time to live */
    uchar       ip_p;                       /* protocol */
    ushort      ip_sum;                     /* checksum */
    IPaddr_t    ip_src;                     /* Source IP address */
    IPaddr_t    ip_dst;                     /* Destination IP address */
} IP_t;

/* User Data Protocol (UDP) header */
typedef struct {
    ushort      udp_src;                    /* UDP source port */
    ushort      udp_dst;                    /* UDP destination port */
    ushort      udp_len;                    /* Length of UDP packet */
    ushort      udp_xsum;                   /* Checksum */
} UDP_t;

/* Internet Control Message Protocol (ICMP) header: echo request (ping) */
typedef struct {
    uchar       icmp_type;                  /* ICMP type */
    uchar       icmp_code;                  /* ICMP code */
    ushort      icmp_sum;                   /* ICMP checksum */
    ushort      icmp_id;                    /* ICMP identificatoin */
    ushort      icmp_sn;                    /* ICMP sequence number */    
} ICMP_ECHO_t;

#define IP_OFFS             0x1fff          /* ip offset *= 8 */
#define IP_FLAGS            0xe000          /* first 3 bits */
#define IP_FLAGS_RES        0x8000          /* reserved */
#define IP_FLAGS_DFRAG      0x4000          /* don't fragments */
#define IP_FLAGS_MFRAG      0x2000          /* more fragments */

#define IP_ADDR_LEN         4               /* Length of logical IP address */

#define IP_HDR_SIZE         (sizeof (IP_t))
#define UDP_HDR_SIZE        (sizeof (UDP_t))
#define ICMP_ECHO_HDR_SIZE  (sizeof (ICMP_ECHO_t))

#define ICMP_TYPE_ECHO_REPLY            (0)
#define ICMP_TYPE_DST_UNREACHABLE       (3)
#define ICMP_TYPE_SRC_QUENCH            (4)
#define ICMP_TYPE_REDIRECT_MSG          (5)
#define ICMP_TYPE_ECHO_REQUEST          (8)
#define ICMP_TYPE_ROUTER_ADVERTISEMENT  (9)
#define ICMP_TYPE_ROUTER_SOLICITATION   (10)
#define ICMP_TYPE_TIME_EXCEEDED         (11)
#define ICMP_TYPE_BAD_IP_HDR            (12)
#define ICMP_TYPE_TIMESTAMP_REQUEST     (13)
#define ICMP_TYPE_TIMESTAMP_REPLY       (14)
#define ICMP_TYPE_INFO_REQUEST          (15)
#define ICMP_TYPE_INFO_REPLY            (16)
#define ICMP_TYPE_ADDR_MASK_REQUEST     (17)
#define ICMP_TYPE_ADDR_MASK_REPLY       (18)
#define ICMP_TYPE_TRACEROUTE            (30)

#define IP_PROT_ICMP        0x01            /* Internet Control Message Protocol,  RFC 792*/
#define IP_PROT_UDC         0x11            /* ser Datagram Protocol,  RFC 768*/

/* Address Resolution Protocol (ARP) header. */
typedef struct {
    ushort      ar_htype;                   /* Format of hardware address */
    ushort      ar_ptype;                   /* Format of protocol address */
    uchar       ar_hlen;                    /* Length of hardware address */
    uchar       ar_plen;                    /* Length of protocol address */
    ushort      ar_oper;                    /* Operation */
    uchar       ar_sha[ETHER_ADDR_LEN];     /* Sender hardware address */
    uchar       ar_spa[IP_ADDR_LEN];        /* Sender protocol address */
    uchar       ar_tha[ETHER_ADDR_LEN];     /* Target hardware address */
    uchar       ar_tpa[IP_ADDR_LEN];        /* Target protocol address */
}ARP_t;

#define ARP_HW_TYPE_ETHER   (0x0001)        /* Ethernet  hardware address */

#define ARP_OP_REQUEST      (0x0001)        /* Request  to resolve  address */
#define ARP_OP_REPLY        (0x0002)        /* Response to previous request */
#define RARP_OP_REQUEST     (0x0003)        /* Request  to resolve  address */
#define RARP_OP_REPLY       (0x0004)        /* Response to previous request */
    
#define ARP_HDR_SIZE        (sizeof (ARP_t))

#define ARP_TIMEOUT         5000UL          /* Milliseconds before trying ARP again */
#define ARP_TIMEOUT_COUNT    5              /* # of timeouts before giving up  */

#define NETSTATE_CONTINUE    1
#define NETSTATE_RESTART     2
#define NETSTATE_SUCCESS     3
#define NETSTATE_FAIL        4


/************************************************
 *              FUNCTION HEADERS DEFINITIONS                 *
 ************************************************/
int 	net_init(void);
int	 	net_close(void);
void	net_ping_req(unsigned int timeout_ms, IPaddr_t ip);
void 	net_rx_process(void);



#endif /* __NET_H__ */

