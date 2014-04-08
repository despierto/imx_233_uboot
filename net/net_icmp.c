/**
 * Internet Control Message Protocol
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

#include "global.h"
#include "net_icmp.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
static U8 ping_payload[] = 
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};

#define PING_PAYLOAD_SIZE   (56)

int local_icmp_set_ipv4_hdr(IP_t *ip, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst);
int local_icmp_set_icmp_hdr(uchar * pkt, uchar type, uchar code, ushort id);




/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int icmp_send_req(IPaddr_t ip_addr)
{
    PETH_PKT pIcmpEthPkt;
    DATALINK_TX_STATE tx_state;
    U32 icmp_pkt_size = 0;
    

    //prepare icmp packet
    pIcmpEthPkt = (PETH_PKT)drv_eth_heap_alloc();
    if (pIcmpEthPkt == NULL) {
        print_err_cmd("%s", "can't allocate new packet");
        return FAILURE;
    }
    icmp_pkt_size += datalink_prepare_eth_hdr(pIcmpEthPkt, NULL, ETH_P_IP);
    icmp_pkt_size += local_icmp_set_ipv4_hdr((IP_t *)((U32)pIcmpEthPkt + icmp_pkt_size), ICMP_ECHO_HDR_SIZE, IP_FLAGS_DFRAG, 255, IPPROTO_ICMP, ip_addr);
    icmp_pkt_size += local_icmp_set_icmp_hdr((uchar *)((U32)pIcmpEthPkt + icmp_pkt_size), ICMP_TYPE_ECHO_REQUEST, 0, 0);    

    tx_state = datalink_tx_send(pIcmpEthPkt, ip_addr, ETH_P_IP, icmp_pkt_size);        
    if (tx_state == DATALINK_TX_SUCCESS) {
        //print_dbg("ICMP request successfully sent: pkt_0x%x size_%d", (U32)pIcmpEthPkt, icmp_pkt_size);
        drv_eth_heap_free(pIcmpEthPkt);
    } else if (tx_state >= DATALINK_TX_ERROR)  {
        print_err_cmd("unexpected error while sending ICMP request: pkt_0x%x size_%d", (U32)pIcmpEthPkt, icmp_pkt_size);
        drv_eth_heap_free(pIcmpEthPkt);
    } else {
        //ARP request sent => let's wait for ARP
        //print_dbg("wait for ARP respond: pkt_0x%x size_%d", (U32)pIcmpEthPkt, icmp_pkt_size);


        //temporary
        drv_eth_heap_free(pIcmpEthPkt);
    }

    return SUCCESS;
}

/************************************************
 *              LOCAL  FUNCTIONS                                      *
 ************************************************/

int local_icmp_set_ipv4_hdr(IP_t *ip, ushort payload_len, ushort frag_off, uchar ttl, uchar prot, IPaddr_t ip_dst)
{
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

int local_icmp_set_icmp_hdr(uchar * pkt, uchar type, uchar code, ushort id)
{
    int icmp_hdr_size = 0;

    switch (type) {
        case ICMP_TYPE_ECHO_REQUEST:
        {
            ICMP_ECHO_t *icmp  = (ICMP_ECHO_t *)pkt;

            icmp->icmp_type = type;
            icmp->icmp_code = code;
            icmp->icmp_id = id;
            icmp->icmp_sn = htons (pGblCtx->ping_req_num++);
            icmp->icmp_sum = ~(sys_checksum ((ushort *)icmp, ICMP_ECHO_HDR_SIZE / 2));

            icmp_hdr_size = ICMP_ECHO_HDR_SIZE;
            
            break;
        }
        default:
           print_err("unsupported ICMP type (%d)", type);
    }

    return icmp_hdr_size;
}




