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
#include "net.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

uchar   NetEtherNullAddr[6] = { 0, 0, 0, 0, 0, 0 };
uchar   NetBcastAddr[6] = /* Ethernet bcast address */
            { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static unsigned int net_state;
    
int local_net_ping_send(IPaddr_t ip_addr);
int local_net_set_eth_hdr(uchar * xet, uchar * addr, uint prot);
int local_net_set_ipv4_hdr(uchar * pkt, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst);
int local_net_set_icmp_hdr(uchar * pkt, uchar type, uchar code, ushort id);
void local_net_arp_request (void);
int local_net_set_arp_hdr(uchar * pkt);
void local_net_arp_timeout_check (void);


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int net_init(void)
{
    int rc;
    U32 i;
    
    print_net("%s", "Network initialization");

    //set config
    drv_string_to_mac(CONFIG_HW_MAC_ADDR, &pGblCtx->cfg_mac_addr[0]);        // "ethaddr"
    pGblCtx->cfg_ip_addr       = drv_string_to_ip(CONFIG_IPADDR);          // "ipaddr"
    pGblCtx->cfg_ip_netmask    = drv_string_to_ip(CONFIG_NETMASK);         // "netmask"
    pGblCtx->cfg_ip_gateway    = drv_string_to_ip(CONFIG_GATEWAYIP);       // "gatewayip"
    pGblCtx->cfg_ip_server     = drv_string_to_ip(CONFIG_SERVERIP);        // "serverip"
    pGblCtx->cfg_ip_dns        = drv_string_to_ip(CONFIG_DNSIP);           // "dnsip"
    pGblCtx->cfg_ip_vlan       = drv_string_to_ip(CONFIG_VLANIP);          // "vlanip"

    //delay for printing out the print buffer
    sleep_ms(100);

    rc = datalink_open();
    
    //setup packet buffers, aligned correctly
    for (i = 0; i < ETH_PKTBUFSTX; i++) {
        pGblCtx->NetTxPackets[i] = (uchar *)drv_eth_heap_alloc();
        //print_eth(" - Net TX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pGblCtx->NetTxPackets[i], ETH_PKTBUFSTX);        
    }
    for (i = 0; i < ETH_PKTBUFSRX; i++) {
        pGblCtx->NetRxPackets[i] = (uchar *)drv_eth_heap_alloc();
        //print_eth(" - Net RX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pGblCtx->NetRxPackets[i], i+1);                
    }

    pGblCtx->NetArpWaitTxPacket = (uchar *)drv_eth_heap_alloc();
    pGblCtx->NetArpWaitTxPacketSize = 0;
    //print_eth(" - Net Arp Tx packet: base (0x%x) count (%d)", (unsigned int)pGblCtx->NetArpWaitTxPacket, 1);
    
    pGblCtx->Status = 0;
    copy_filename(pGblCtx->BootFile, CONFIG_BOOTFILE, CONFIG_BOOTFILE_SIZE);    
    pGblCtx->linux_load_addr = SYS_RAM_LOAD_ADDR;
    
    pGblCtx->NetArpWaitPacketMAC = NULL;
    pGblCtx->NetArpWaitPacketIP = 0;
    pGblCtx->NetArpWaitReplyIP = 0;
    pGblCtx->NetArpWaitTimerStart = 0;
    pGblCtx->NetArpWaitTry = 0;
    
    return rc;
}

int net_close(void)
{
    int rc;
    
    //print_net("%s", "Network termination");
    rc = datalink_close();

    return rc;
}


void net_ping_req(unsigned int timeout_ms, IPaddr_t ip_addr)
{
    uchar ip_str[15];
    unsigned int data_size = 56;
    unsigned int packet_size = data_size + IP_HDR_SIZE + ICMP_ECHO_HDR_SIZE;
    unsigned int i;
    unsigned int mac_reg_time;
    char *dst_mac;
    char mac_out[64];
    
    drv_ip_to_string(ip_addr, &ip_str[0]);

    //FORMAT: "PING <DNS or incomming IP address> (<IP address>): <size>(<fsize>) bytes of data"
    print_net("PING %s (%s) %d(%d) bytes of data.", ip_str, ip_str, data_size, packet_size);



    
    //NetSetTimeout (timeout_ms, PingTimeout);
    //NetSetHandler (PingHandler);

    local_net_ping_send(ip_addr);
    
    net_state = NETSTATE_CONTINUE;

    print_net("%s", "Entry in loop");
 
    while (net_state == NETSTATE_CONTINUE)
    {
        drv_eth_rx();
        local_net_arp_timeout_check();
        
        //sleep_ms(50);
    }

    print_net("Exit: state (%d)", net_state);
    
    return;
}

void net_rx_process(void)
{
    unsigned int size;
    unsigned int addr;    
    
    //receive all current packets
    drv_eth_rx();

    //get and process every packet
    while((size = drv_eth_rx_get(&addr)) != 0) {
        
        if (addr && size) {
#if 0            
            U8 *pA = (U8 *)addr;
            unsigned int i;            
            print_inf("[net] --- Rx Packet[0x%x, %d]: [ ", addr, size);
            for(i=0; i<size; i++) {
                print_inf("%x ", pA[i]);
            }
            print_inf("] --- \r\n");
#endif            
            drv_eth_heap_free((PTR)addr);
        }
    
    }

    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

int local_net_ping_send(IPaddr_t ip_addr)
{
    static uchar    mac[6];
    IP_t   *ip;
    ushort *s;
    uchar           *pkt;

    /*Prepare ARP request */
    memcpy(mac, NetEtherNullAddr, 6);

    pGblCtx->NetArpWaitPacketIP = ip_addr;
    pGblCtx->NetArpWaitPacketMAC = mac;

    pkt = pGblCtx->NetArpWaitTxPacket;
    //print_dbg("pkt (0x%x), NetArpWaitPacket: IP (0x%x) MAC (%s)", 
    //    (unsigned int)pkt, pGblCtx->NetArpWaitPacketIP, pGblCtx->NetArpWaitPacketMAC);

    pkt += local_net_set_eth_hdr(pkt, mac, PROT_IP);
    pkt += local_net_set_ipv4_hdr(pkt, ICMP_ECHO_HDR_SIZE, IP_FLAGS_DFRAG, 255, IP_PROT_ICMP, ip_addr);
    pkt += local_net_set_icmp_hdr(pkt, ICMP_TYPE_ECHO_REQUEST, 0, 0);    

    pGblCtx->NetArpWaitTxPacketSize = ((uint)pkt - (uint)pGblCtx->NetArpWaitTxPacket); 

    /*Do ARM request */
    pGblCtx->NetArpWaitTry = 1;
    pGblCtx->NetArpWaitTimerStart = get_tick();

    print_net("---> NetArpWaitTimerStart 0x%x", pGblCtx->NetArpWaitTimerStart);

    local_net_arp_request();

    return 1; /* waiting */
}

int local_net_set_eth_hdr(uchar * pkt, uchar * addr, uint prot)
{
    Ethernet_t *et = (Ethernet_t *)pkt;
    //ushort myvlanid;

    //myvlanid = ntohs(pGblCtx->cfg_ip_vlan);
    //if (myvlanid == (ushort)-1) /* ? == 0xffff */
    //    myvlanid = VLAN_NONE;   /* = 0x1000 */

    memcpy ((void *)et->et_dest, (void *)addr, 6);
    memcpy ((void *)et->et_src, (void *)pGblCtx->cfg_mac_addr, 6);
    
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

int local_net_set_ipv4_hdr(uchar * pkt, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst)
{
    IP_t *ip  = (IP_t *)pkt;

    ip->ip_hl_v  = 0x45;    /* IP_HDR_SIZE/sizeof(uint) | (PROT_IPV4_VERSION << 4) */
    ip->ip_tos   = 0x0;
    ip->ip_len   = htons (IP_HDR_SIZE + payload_len);
    ip->ip_id    = htons (pGblCtx->NetIPID++);
    ip->ip_off   = htons (frag_off);   /* Don't fragment */
    ip->ip_ttl   = ttl;
    ip->ip_p     = prot;
    memcpy((void*)&ip->ip_src, (void*)pGblCtx->cfg_ip_addr, sizeof(IPaddr_t));
    memcpy((void*)&ip->ip_dst, (void*)&ip_dst, sizeof(IPaddr_t));
    ip->ip_sum   = ~(sys_checksum ((ushort *)ip, IP_HDR_SIZE / 2));    
    
    return IP_HDR_SIZE;
}

int local_net_set_icmp_hdr(uchar * pkt, uchar type, uchar code, ushort id)
{
    int icmp_hdr_size = 0;

    switch (type) {
        case ICMP_TYPE_ECHO_REQUEST:
        {
            ICMP_ECHO_t *icmp  = (ICMP_ECHO_t *)pkt;

            icmp->icmp_type = type;
            icmp->icmp_code = code;
            icmp->icmp_id = id;
            icmp->icmp_sn = htons (pGblCtx->PingSeqNo++);
            icmp->icmp_sum = ~(sys_checksum ((ushort *)icmp, ICMP_ECHO_HDR_SIZE / 2));

            icmp_hdr_size = ICMP_ECHO_HDR_SIZE;
            
            break;
        }
        default:
           print_err("unsupported ICMP type (%d)", type);
    }

    return icmp_hdr_size;
}

int local_net_set_arp_hdr(uchar * pkt)
{
    ARP_t *arp = (ARP_t *)pkt;

    arp->ar_htype = htons (ARP_HW_TYPE_ETHER);
    arp->ar_ptype = htons (PROT_IP);
    arp->ar_hlen = ETHER_ADDR_LEN;
    arp->ar_plen = IP_ADDR_LEN;
    arp->ar_oper = htons (ARP_OP_REQUEST);

    memcpy ((void *)&arp->ar_sha[0], (void *)&pGblCtx->cfg_mac_addr[0], ETHER_ADDR_LEN);
    memcpy ((void *)&arp->ar_spa[0], (void *)&pGblCtx->cfg_ip_addr, IP_ADDR_LEN);
    memcpy ((void *)&arp->ar_tha[0], (void *)&NetEtherNullAddr[0], ETHER_ADDR_LEN);

    if ((pGblCtx->NetArpWaitPacketIP & pGblCtx->cfg_ip_netmask) 
            != (pGblCtx->cfg_ip_addr & pGblCtx->cfg_ip_netmask)) {
        if (pGblCtx->cfg_ip_gateway == 0) {
            print_err ("%s", "## Warning: gatewayip needed but not set");
            pGblCtx->NetArpWaitReplyIP = pGblCtx->NetArpWaitPacketIP;
        } else {
            pGblCtx->NetArpWaitReplyIP = pGblCtx->cfg_ip_gateway;
        }
    } else {
        pGblCtx->NetArpWaitReplyIP = pGblCtx->NetArpWaitPacketIP;
    }
    memcpy ((void *)&arp->ar_tpa[0], (void *)&pGblCtx->NetArpWaitReplyIP, IP_ADDR_LEN);
    
    return ARP_HDR_SIZE;
}

void local_net_arp_request (void)
{
    int i;
    uchar *pkt;
    ARP_t *arp;
    unsigned int len = 0;

    //print_dbg("ARP broadcast %d", pGblCtx->NetArpWaitTry);

    pkt = (uchar *)&pGblCtx->NetTxPackets[0];

    pkt += local_net_set_eth_hdr(pkt, NetBcastAddr, PROT_ARP);
    pkt += local_net_set_arp_hdr(pkt);

    len = (unsigned int)pkt - (unsigned int)&pGblCtx->NetTxPackets[0];

    print_net("local_net_arp_request: len_%d, pkt_0x%x tx_packet_0x%x", len, (unsigned int)pkt, (unsigned int)&pGblCtx->NetTxPackets[0]);
    drv_eth_tx ((void *)&pGblCtx->NetTxPackets[0], len);

    return;
}

void local_net_arp_timeout_check (void)
{   
    unsigned int t;
    
    if (!pGblCtx->NetArpWaitPacketIP)
        return;

    t = get_tick();
    
    /* check for arp timeout */
    if ( (unsigned int)((t - pGblCtx->NetArpWaitTimerStart)/1000) > (unsigned int)ARP_TIMEOUT) {
        pGblCtx->NetArpWaitTry++;

        print_net("---> NetArpWaitTry %d\n", pGblCtx->NetArpWaitTry);

        if (pGblCtx->NetArpWaitTry >= ARP_TIMEOUT_COUNT) {
            print_net("%s", "ARP Retry count exceeded (%d) times");
            pGblCtx->NetArpWaitTry = 0;
            net_state = NETSTATE_RESTART;

            //printf("---> NetStartAgain\n");
            //NetStartAgain();
        } else {
            pGblCtx->NetArpWaitTimerStart = t;
            print_net("---> NetArpWaitTimerStart 0x%x", pGblCtx->NetArpWaitTimerStart);
 
            printf("---> ArpRequest\n");
            local_net_arp_request();
        }
    }

 
    return;
}


