/**
 * Network definitions
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
 
 #ifndef __NETDEFS_H__
 #define __NETDEFS_H__

#include "types.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
/* Supported address families. */
typedef enum {
    AF_UNSPEC =     0,
    AF_UNIX =       1,      /* Unix domain sockets  */
    AF_LOCAL =      1,      /* POSIX name for AF_UNIX */
    AF_INET =       2,      /* Internet IP Protocol  */
    AF_AX25 =       3,      /* Amateur Radio AX.25 */
    AF_IPX =        4,      /* Novell IPX */
    AF_APPLETALK =  5,      /* AppleTalk DDP */
    AF_NETROM =     6,      /* Amateur Radio NET/ROM  */
    AF_BRIDGE =     7,      /* Multiprotocol bridge */
    AF_ATMPVC =     8,      /* ATM PVCs */
    AF_X25 =        9,      /* Reserved for X.25 project */
    AF_INET6 =      10,     /* IP version 6 */
    AF_ROSE =       11,     /* Amateur Radio X.25 PLP */
    AF_DECnet =     12,     /* Reserved for DECnet project */
    AF_NETBEUI =    13,     /* Reserved for 802.2LLC project */
    AF_SECURITY =   14,     /* Security callback pseudo AF */
    AF_KEY =        15,     /* PF_KEY key management API */
    AF_NETLINK =    16,
    AF_ROUTE = AF_NETLINK,  /* Alias to emulate 4.4BSD */
    AF_PACKET =     17,     /* Packet family */
    AF_ASH =        18,     /* Ash */
    AF_ECONET =     19,     /* Acorn Econet */
    AF_ATMSVC =     20,     /* ATM SVCs */
    AF_RDS =        21,     /* RDS sockets */
    AF_SNA =        22,     /* Linux SNA Project (nutters!) */
    AF_IRDA =       23,     /* IRDA sockets */
    AF_PPPOX =      24,     /* PPPoX sockets */
    AF_WANPIPE =    25,     /* Wanpipe API Sockets */
    AF_LLC =        26,     /* Linux LLC */
    AF_CAN =        29,     /* Controller Area Network */
    AF_TIPC =       30,     /* TIPC sockets */
    AF_BLUETOOTH =  31,     /* Bluetooth sockets */
    AF_IUCV =       32,     /* IUCV sockets */
    AF_RXRPC =      33,     /* RxRPC sockets */
    AF_ISDN =       34,     /* mISDN sockets */
    AF_PHONET =     35,     /* Phonet sockets */
    AF_IEEE802154 = 36,     /* IEEE802154 sockets */
    AF_MAX =        37      /* For now.. */
} AF;



/******** ETHERNET ******************************/
/* These are the defined Ethernet Protocol ID's. */
#define ETH_P_LOOP              0x0060      /* Ethernet Loopback packet    */
#define ETH_P_PUP               0x0200      /* Xerox PUP packet        */
#define ETH_P_PUPAT             0x0201      /* Xerox PUP Addr Trans packet    */
#define ETH_P_IP                0x0800      /* Internet Protocol packet    */
#define ETH_P_X25               0x0805      /* CCITT X.25            */
#define ETH_P_ARP               0x0806      /* Address Resolution packet    */
#define ETH_P_BPQ               0x08FF      /* G8BPQ AX.25 Ethernet Packet    [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IEEEPUP           0x0a00      /* Xerox IEEE802.3 PUP packet */
#define ETH_P_IEEEPUPAT         0x0a01      /* Xerox IEEE802.3 PUP Addr Trans packet */
#define ETH_P_DEC               0x6000      /* DEC Assigned proto           */
#define ETH_P_DNA_DL            0x6001      /* DEC DNA Dump/Load            */
#define ETH_P_DNA_RC            0x6002      /* DEC DNA Remote Console       */
#define ETH_P_DNA_RT            0x6003      /* DEC DNA Routing              */
#define ETH_P_LAT               0x6004      /* DEC LAT                      */
#define ETH_P_DIAG              0x6005      /* DEC Diagnostics              */
#define ETH_P_CUST              0x6006      /* DEC Customer use             */
#define ETH_P_SCA               0x6007      /* DEC Systems Comms Arch       */
#define ETH_P_TEB               0x6558      /* Trans Ether Bridging        */
#define ETH_P_RARP              0x8035      /* Reverse Addr Res packet    */
#define ETH_P_ATALK             0x809B      /* Appletalk DDP        */
#define ETH_P_AARP              0x80F3      /* Appletalk AARP        */
#define ETH_P_8021Q             0x8100      /* 802.1Q VLAN Extended Header  */
#define ETH_P_IPX               0x8137      /* IPX over DIX            */
#define ETH_P_IPV6              0x86DD      /* IPv6 over bluebook        */
#define ETH_P_PAUSE             0x8808      /* IEEE Pause frames. See 802.3 31B */
#define ETH_P_SLOW              0x8809      /* Slow Protocol. See 802.3ad 43B */
#define ETH_P_WCCP              0x883E      /* Web-cache coordination protocol defined in draft-wilson-wrec-wccp-v2-00.txt */
#define ETH_P_PPP_DISC          0x8863      /* PPPoE discovery messages     */
#define ETH_P_PPP_SES           0x8864      /* PPPoE session messages    */
#define ETH_P_MPLS_UC           0x8847      /* MPLS Unicast traffic        */
#define ETH_P_MPLS_MC           0x8848      /* MPLS Multicast traffic    */
#define ETH_P_ATMMPOA           0x884c      /* MultiProtocol Over ATM    */
#define ETH_P_ATMFATE           0x8884      /* Frame-based ATM Transport over Ethernet */
#define ETH_P_PAE               0x888E      /* Port Access Entity (IEEE 802.1X) */
#define ETH_P_AOE               0x88A2      /* ATA over Ethernet        */
#define ETH_P_TIPC              0x88CA      /* TIPC             */
#define ETH_P_1588              0x88F7      /* IEEE 1588 Timesync */
#define ETH_P_FCOE              0x8906      /* Fibre Channel over Ethernet  */
#define ETH_P_FIP               0x8914      /* FCoE Initialization Protocol */
#define ETH_P_EDSA              0xDADA      /* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */

/*  Non DIX types. Won't clash for 1500 types. */
#define ETH_P_802_3             0x0001      /* Dummy type for 802.3 frames  */
#define ETH_P_AX25              0x0002      /* Dummy protocol id for AX.25  */
#define ETH_P_ALL               0x0003      /* Every packet (be careful!!!) */
#define ETH_P_802_2             0x0004      /* 802.2 frames         */
#define ETH_P_SNAP              0x0005      /* Internal only        */
#define ETH_P_DDCMP             0x0006      /* DEC DDCMP: Internal only     */
#define ETH_P_WAN_PPP           0x0007      /* Dummy type for WAN PPP frames*/
#define ETH_P_PPP_MP            0x0008      /* Dummy type for PPP MP frames */
#define ETH_P_LOCALTALK         0x0009      /* Localtalk pseudo type     */
#define ETH_P_CAN               0x000C      /* Controller Area Network      */
#define ETH_P_PPPTALK           0x0010      /* Dummy type for Atalk over PPP*/
#define ETH_P_TR_802_2          0x0011      /* 802.2 frames         */
#define ETH_P_MOBITEX           0x0015      /* Mobitex (kaz@cafe.net)    */
#define ETH_P_CONTROL           0x0016      /* Card specific control frames */
#define ETH_P_IRDA              0x0017      /* Linux-IrDA            */
#define ETH_P_ECONET            0x0018      /* Acorn Econet            */
#define ETH_P_HDLC              0x0019      /* HDLC frames            */
#define ETH_P_ARCNET            0x001A      /* 1A for ArcNet :-)            */
#define ETH_P_DSA               0x001B      /* Distributed Switch Arch.    */
#define ETH_P_TRAILER           0x001C      /* Trailer switch tagging    */
#define ETH_P_PHONET            0x00F5      /* Nokia Phonet frames          */
#define ETH_P_IEEE802154        0x00F6      /* IEEE802.15.4 frame        */

#define ETH_PKTSIZE             (1518)
#define ETH_PKTSIZE_ALIGN       (1536)
#define ETH_PKTALIGN            (32)

#define ETHER_ADDR_LEN          (6)         /* Length of ethernet MAC address */
#define ETHER_HDR_SIZE          (14)        /* Ethernet header size */
#define E802_HDR_SIZE           (22)        /* 802 ethernet header size */

/* Ethernet header  */
typedef struct {
    uchar       et_dest[ETHER_ADDR_LEN];    /* Destination node */
    uchar       et_src[ETHER_ADDR_LEN];     /* Source node */
    ushort      et_protlen;                 /* Protocol or length */
    uchar       et_dsap;                    /* 802 DSAP */
    uchar       et_ssap;                    /* 802 SSAP */
    uchar       et_ctl;                     /* 802 control */
    uchar       et_snap1;                   /* SNAP */
    uchar       et_snap2;
    uchar       et_snap3;
    ushort      et_prot;                    /* 802 protocol */
} Ethernet_t;

/* Ethernet 2 header  */
typedef struct tagETH_HDR{
    uchar       dst[ETHER_ADDR_LEN];        /* Destination node */
    uchar       src[ETHER_ADDR_LEN];        /* Source node */
    ushort      type;                       /* Protocol or length */
} ETH_HDR, *PETH_HDR;



/******** VLAN ***********************************/
#define VLAN_NONE               (4095)      /* untagged (0x1000)*/
#define VLAN_IDMASK             (0x0fff)    /* mask of valid vlan id */
#define VLAN_ETHER_HDR_SIZE     (18)        /* VLAN Ethernet header size */

/* VLAN Ethernet header */
typedef struct {
    uchar       vet_dest[ETHER_ADDR_LEN];   /* Destination node */
    uchar       vet_src[ETHER_ADDR_LEN];    /* Source node */
    ushort      vet_vlan_type;              /* PROT_VLAN */
    ushort      vet_tag;                    /* TAG of VLAN */
    ushort      vet_type;                   /* protocol type */
} VLAN_Ethernet_t;



/******** IP  ************************************/
#define PROT_IPV4_VERSION       (4)         /* IPv4 protocol version*/
#define PROT_IPV6_VERSION       (6)         /* IPv6 protocol version*/

/* Standard IP protocols.  */
typedef enum {
  IPPROTO_IP = 0,           /* Dummy protocol for TCP */
  IPPROTO_ICMP = 1,         /* Internet Control Message Protocol */
  IPPROTO_IGMP = 2,         /* Internet Group Management Protocol */
  IPPROTO_IPIP = 4,         /* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,          /* Transmission Control Protocol */
  IPPROTO_EGP = 8,          /* Exterior Gateway Protocol */
  IPPROTO_PUP = 12,         /* PUP protocol */
  IPPROTO_UDP = 17,         /* User Datagram Protocol */
  IPPROTO_IDP = 22,         /* XNS IDP protocol */
  IPPROTO_DCCP = 33,        /* Datagram Congestion Control Protocol */
  IPPROTO_RSVP = 46,        /* RSVP protocol */
  IPPROTO_GRE = 47,         /* Cisco GRE tunnels (rfc 1701,1702) */

  IPPROTO_IPV6 = 41,        /* IPv6-in-IPv4 tunnelling */

  IPPROTO_ESP = 50,         /* Encapsulation Security Payload protocol */
  IPPROTO_AH = 51,          /* Authentication Header protocol */
  IPPROTO_BEETPH = 94,      /* IP option pseudo header for BEET */
  IPPROTO_PIM = 103,        /* Protocol Independent Multicast */

  IPPROTO_COMP = 108,       /* Compression Header protocol */
  IPPROTO_SCTP = 132,       /* Stream Control Transport Protocol */
  IPPROTO_UDPLITE = 136,    /* UDP-Lite (RFC 3828) */

  IPPROTO_RAW = 255,        /* Raw IP packets */
  IPPROTO_MAX
} IP_PROTOCOL;

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

#define IP_OFFS             0x1fff          /* ip offset *= 8 */
#define IP_FLAGS            0xe000          /* first 3 bits */
#define IP_FLAGS_RES        0x8000          /* reserved */
#define IP_FLAGS_DFRAG      0x4000          /* don't fragments */
#define IP_FLAGS_MFRAG      0x2000          /* more fragments */

#define IP_ADDR_LEN         (4)             /* Length of logical IP address */
#define IP_ADDR_STR_LEN     (15)            /* Length of string of IP address with dots */
#define IP_HDR_SIZE         (sizeof (IP_t))




/******** ARP  ***********************************/

#define ARP_HW_TYPE_ETHER   (0x0001)        /* Ethernet  hardware address */
#define ARP_OP_REQUEST      (0x0001)        /* Request  to resolve  address */
#define ARP_OP_REPLY        (0x0002)        /* Response to previous request */
#define RARP_OP_REQUEST     (0x0003)        /* Request  to resolve  address */
#define RARP_OP_REPLY       (0x0004)        /* Response to previous request */
    
#define ARP_HDR_SIZE        (sizeof (ARP_t))

#define ARP_TIMEOUT         (5*1000)           /* Miliseconds before trying ARP again */
#define ARP_TIMEOUT_COUNT   (5)             /* # of timeouts before giving up  */
#define ARP_VALID_PERIOD    (120*1000)           /* after this period of miliseconds the ARP item is invalid*/

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




/******** UDP  ***********************************/

/* User Data Protocol (UDP) header */
typedef struct {
    ushort      udp_src;                    /* UDP source port */
    ushort      udp_dst;                    /* UDP destination port */
    ushort      udp_len;                    /* Length of UDP packet */
    ushort      udp_xsum;                   /* Checksum */
} UDP_t;

#define UDP_HDR_SIZE        (sizeof (UDP_t))



/******** ICMP  **********************************/
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

/* Internet Control Message Protocol (ICMP) header: echo request (ping) */
typedef struct {
    uchar       icmp_type;                  /* ICMP type */
    uchar       icmp_code;                  /* ICMP code */
    ushort      icmp_sum;                   /* ICMP checksum */
    ushort      icmp_id;                    /* ICMP identificatoin */
    ushort      icmp_sn;                    /* ICMP sequence number */    
} ICMP_ECHO_t;

#define ICMP_ECHO_HDR_SIZE  (sizeof (ICMP_ECHO_t))



#endif /*__NETDEFS_H__ */

