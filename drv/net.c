/**
 * Top layer ethernel driver file
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

#include "global.h"
#include "drv_eth.h"
#include "drv_utils.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

uchar   NetEtherNullAddr[6] = { 0, 0, 0, 0, 0, 0 };
uchar   NetBcastAddr[6] = /* Ethernet bcast address */
            { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

int local_net_ping_send(IPaddr_t ip_addr);
int local_net_set_eth_hdr(volatile uchar * xet, uchar * addr, uint prot);
int local_net_set_ipv4_hdr(volatile uchar * pkt, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst);
int local_net_set_icmp_hdr(volatile uchar * pkt, uchar type, uchar code, ushort id);
void local_net_arp_request (void);
int local_net_set_arp_hdr(volatile uchar * pkt);


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
void net_ping_req(unsigned int timeout_ms, IPaddr_t ip_addr)
{
    uchar ip_str[15];
    unsigned int data_size = 56;
    unsigned int packet_size = data_size + IP_HDR_SIZE + ICMP_ECHO_HDR_SIZE;
    
    drv_ip_to_string(ip_addr, &ip_str[0]);

    //FORMAT: "PING <DNS or incomming IP address> (<IP address>): <size>(<fsize>) bytes of data"
    print_net("PING %s (%s) %d(%d) bytes of data.", ip_str, ip_str, data_size, packet_size);

    
    //NetSetTimeout (timeout_ms, PingTimeout);
    //NetSetHandler (PingHandler);

    local_net_ping_send(ip_addr);
    
    return;
}



/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

int local_net_ping_send(IPaddr_t ip_addr)
{
    static uchar    mac[6];
    volatile IP_t   *ip;
    volatile ushort *s;
    volatile uchar           *pkt;

    /*Prepare ARP request */
    memcpy(mac, NetEtherNullAddr, 6);

    pEth->NetArpWaitPacketIP = ip_addr;
    pEth->NetArpWaitPacketMAC = mac;

    pkt = pEth->NetArpWaitTxPacket;
    print_dbg("pkt (0x%x), NetArpWaitPacket: IP (0x%x) MAC (%s)", 
        (unsigned int)pkt, pEth->NetArpWaitPacketIP, pEth->NetArpWaitPacketMAC);

    pkt += local_net_set_eth_hdr(pkt, mac, PROT_IP);
    pkt += local_net_set_ipv4_hdr(pkt, ICMP_ECHO_HDR_SIZE, IP_FLAGS_DFRAG, 255, IP_PROT_ICMP, ip_addr);
    pkt += local_net_set_icmp_hdr(pkt, ICMP_TYPE_ECHO_REQUEST, 0, 0);    

    pEth->NetArpWaitTxPacketSize = ((uint)pkt - (uint)pEth->NetArpWaitTxPacket); 

    /*Do ARM request */
    pEth->NetArpWaitTry = 1;
    pEth->NetArpWaitTimerStart = get_tick();


    local_net_arp_request();


    return 1; /* waiting */
}

int local_net_set_eth_hdr(volatile uchar * pkt, uchar * addr, uint prot)
{
    Ethernet_t *et = (Ethernet_t *)pkt;
    //ushort myvlanid;

    //myvlanid = ntohs(pEth->cfg_ip_vlan);
    //if (myvlanid == (ushort)-1) /* ? == 0xffff */
    //    myvlanid = VLAN_NONE;   /* = 0x1000 */

    memcpy ((void *)et->et_dest, (void *)addr, 6);
    memcpy ((void *)et->et_src, (void *)pEth->cfg_mac_addr, 6);
    
    //if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
        et->et_protlen = htons(prot);
        return ETHER_HDR_SIZE;
    //} else {
    //    VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;

    //    vet->vet_vlan_type = htons(PROT_VLAN);
    //    vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
    //    vet->vet_type = htons(prot);
    //    return VLAN_ETHER_HDR_SIZE;
    //}
}

int local_net_set_ipv4_hdr(volatile uchar * pkt, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst)
{
    IP_t *ip  = (IP_t *)pkt;

    ip->ip_hl_v  = 0x45;    /* IP_HDR_SIZE/sizeof(uint) | (PROT_IPV4_VERSION << 4) */
    ip->ip_tos   = 0x0;
    ip->ip_len   = htons (IP_HDR_SIZE + payload_len);
    ip->ip_id    = htons (pEth->NetIPID++);
    ip->ip_off   = htons (frag_off);   /* Don't fragment */
    ip->ip_ttl   = ttl;
    ip->ip_p     = prot;
    memcpy((void*)&ip->ip_src, (void*)pEth->cfg_ip_addr, sizeof(IPaddr_t));
    memcpy((void*)&ip->ip_dst, (void*)&ip_dst, sizeof(IPaddr_t));
    ip->ip_sum   = ~(sys_checksum ((uchar *)ip, IP_HDR_SIZE / 2));    
    
    return IP_HDR_SIZE;
}

int local_net_set_icmp_hdr(volatile uchar * pkt, uchar type, uchar code, ushort id)
{
    int icmp_hdr_size = 0;

    switch (type) {
        case ICMP_TYPE_ECHO_REQUEST:
        {
            ICMP_ECHO_t *icmp  = (ICMP_ECHO_t *)pkt;

            icmp->icmp_type = type;
            icmp->icmp_code = code;
            icmp->icmp_id = id;
            icmp->icmp_sn = htons (pEth->PingSeqNo++);
            icmp->icmp_sum = ~(sys_checksum ((uchar *)icmp, ICMP_ECHO_HDR_SIZE / 2));

            icmp_hdr_size = ICMP_ECHO_HDR_SIZE;
            
            break;
        }
        default:
           print_err("unsupported ICMP type (%d)", type);
    }

    return icmp_hdr_size;
}

int local_net_set_arp_hdr(volatile uchar * pkt)
{
    ARP_t *arp = (ARP_t *)pkt;

    arp->ar_htype = htons (ARP_HW_TYPE_ETHER);
    arp->ar_ptype = htons (PROT_IP);
    arp->ar_hlen = ETHER_ADDR_LEN;
    arp->ar_plen = IP_ADDR_LEN;
    arp->ar_oper = htons (ARP_OP_REQUEST);

    memcpy ((void *)&arp->ar_sha[0], (void *)&pEth->cfg_mac_addr[0], ETHER_ADDR_LEN);
    memcpy ((void *)&arp->ar_spa[0], (void *)&pEth->cfg_ip_addr, IP_ADDR_LEN);
    memcpy ((void *)&arp->ar_tha[0], (void *)&NetEtherNullAddr[0], ETHER_ADDR_LEN);

    if ((pEth->NetArpWaitPacketIP & pEth->cfg_ip_netmask) 
            != (pEth->cfg_ip_addr & pEth->cfg_ip_netmask)) {
        if (pEth->cfg_ip_gateway == 0) {
            print_err ("%s", "## Warning: gatewayip needed but not set");
            pEth->NetArpWaitReplyIP = pEth->NetArpWaitPacketIP;
        } else {
            pEth->NetArpWaitReplyIP = pEth->cfg_ip_gateway;
        }
    } else {
        pEth->NetArpWaitReplyIP = pEth->NetArpWaitPacketIP;
    }
    memcpy ((void *)&arp->ar_tpa[0], (void *)&pEth->NetArpWaitReplyIP, IP_ADDR_LEN);
    
    return ARP_HDR_SIZE;
}

void local_net_arp_request (void)
{
    int i;
    volatile uchar *pkt;
    ARP_t *arp;

    print_dbg("ARP broadcast %d", pEth->NetArpWaitTry);

    pkt = (volatile uchar *)&pEth->NetTxPackets[0];

    pkt += local_net_set_eth_hdr(pkt, NetBcastAddr, PROT_ARP);
    pkt += local_net_set_arp_hdr(pkt);
    
    drv_eth_tx ((void *)&pEth->NetTxPackets[0], (unsigned int)pkt - (unsigned int)&pEth->NetTxPackets[0]);
    return;
}



