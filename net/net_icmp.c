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
    U32 rc = FAILURE;    

    //prepare icmp packet
    pIcmpEthPkt = (PETH_PKT)drv_eth_heap_alloc();
    if (pIcmpEthPkt == NULL) {
        print_err_cmd("%s", "can't allocate new packet");
        return rc;
    }
    icmp_pkt_size += datalink_prepare_eth_hdr(pIcmpEthPkt, NULL, ETH_P_IP);
    icmp_pkt_size += local_icmp_set_ipv4_hdr((IP_t *)((U32)pIcmpEthPkt + icmp_pkt_size), ICMP_ECHO_HDR_SIZE + PING_PAYLOAD_SIZE, IP_FLAGS_DFRAG, 64, IPPROTO_ICMP, ip_addr);
    icmp_pkt_size += local_icmp_set_icmp_hdr((uchar *)((U32)pIcmpEthPkt + icmp_pkt_size), ICMP_TYPE_ECHO_REQUEST, 0, 0);    
    memcpy((void *)((U32)pIcmpEthPkt + icmp_pkt_size), &ping_payload[0], PING_PAYLOAD_SIZE);
    icmp_pkt_size += PING_PAYLOAD_SIZE;
    
    tx_state = datalink_tx_send(pIcmpEthPkt, ip_addr, ETH_P_IP, icmp_pkt_size);        
    if (tx_state == DATALINK_TX_ERROR) {
        print_err_cmd("unexpected error while sending ICMP request: pkt_0x%x size_%d", (U32)pIcmpEthPkt, icmp_pkt_size);
    } else if (tx_state == DATALINK_TX_SUCCESS) {
        rc = SUCCESS;
    }

    //packet was sent (it doesn't matter is it sent icmp packet of ARP request. Processing of respond is doing at higher layer)
    drv_eth_heap_free(pIcmpEthPkt);
       
    return rc;
}

int icmp_rx_handler(PETH_PKT pEthPkt, U32 pkt_size, PIP_PKT pIpPkt)
{
    PICMP_ECHO_PKT pIcmpPkt = (PICMP_ECHO_PKT)&pIpPkt->payload[0];
 
    switch (pIcmpPkt->icmp_type)
    {
        case ICMP_TYPE_ECHO_REPLY:
            {
                //let's process replay per out request
                if (memcmp(&pIpPkt->ip_src[0], &pGblCtx->ping_ip, IP_ADDR_LEN) == TRUE) {
                    uint len = htons(pIpPkt->ip_len);

                    pGblCtx->ping_ans_num++;
                    pGblCtx->ping_ans_ttl = pIpPkt->ip_ttl;
                    pGblCtx->ping_ans_time = get_time_ms();
                    
                    if (len >= (IP_HDR_SIZE - ICMP_ECHO_HDR_SIZE)) {
                        pGblCtx->ping_ans_bytes = len - IP_HDR_SIZE - ICMP_ECHO_HDR_SIZE;
                        if (!((pGblCtx->ping_ans_bytes == PING_PAYLOAD_SIZE)
                            && (memcmp(&pIcmpPkt->payload[0], &ping_payload[0], PING_PAYLOAD_SIZE) == TRUE))) {
                            //set rx packet as broken
                            pGblCtx->ping_ans_bytes = 0xFFFFFFFF; 
                        }
                    } else {
                        //set rx packet as broken
                        pGblCtx->ping_ans_bytes = 0xFFFFFFFF; 
                    }
                    //print_dbg("ECHO REPLAY: num_%d ttl_%d bytes_%d", pGblCtx->ping_ans_num, pGblCtx->ping_ans_ttl, pGblCtx->ping_ans_bytes);
                                        
                } else {
                    //print_net("wow! somebody from ip %d.%d.%d.%d sent us replay...", 
                    //    pIpPkt->ip_src[0], pIpPkt->ip_src[1], pIpPkt->ip_src[2], pIpPkt->ip_src[3]);
                }
            }
            break;

        case ICMP_TYPE_ECHO_REQUEST:
            {
                DATALINK_TX_STATE tx_state;
                IPaddr_t ip_src;
                                        
                memcpy(&ip_src, &pIpPkt->ip_src[0], IP_ADDR_LEN);
                
                //update some fields at packet and send it back w/o pauload touching
                //upcate icmp header
                pIcmpPkt->icmp_type = ICMP_TYPE_ECHO_REPLY;
                pIcmpPkt->icmp_sum  = (ushort)(~(sys_checksum ((ushort *)pIcmpPkt, ICMP_ECHO_HDR_SIZE / 2)));
                //update ip header
                pIpPkt->ip_ttl      = pIpPkt->ip_ttl/2;
                pIpPkt->ip_id       = htons (pGblCtx->NetIPID++);
                memcpy((void*)&pIpPkt->ip_dst[0], (void*)&pIpPkt->ip_src[0], IP_ADDR_LEN);
                memcpy((void*)&pIpPkt->ip_src[0], (void*)&pGblCtx->cfg_ip_addr, IP_ADDR_LEN);
                pIpPkt->ip_sum   = (ushort)(~(sys_checksum ((ushort *)pIpPkt, IP_HDR_SIZE / 2))); 
                //update eth header
                memcpy ((void *)&pEthPkt->src[0], (void *)&pGblCtx->cfg_mac_addr[0], ETHER_ADDR_LEN);

                //send packet
                tx_state = datalink_tx_send(pEthPkt, ip_src, ETH_P_IP, pkt_size);        
                if (tx_state == DATALINK_TX_ERROR) {
                    print_err_cmd("unexpected error while sending ICMP replay: pkt_0x%x size_%d", (U32)pEthPkt, pkt_size);
                }

                //packet freeing to be done at upper layer
            }
            
        default:
            break;
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
    memcpy((void*)&ip->ip_src[0], (void*)&pGblCtx->cfg_ip_addr, IP_ADDR_LEN);
    memcpy((void*)&ip->ip_dst[0], (void*)&ip_dst, IP_ADDR_LEN);
    ip->ip_sum   = (ushort)(~(sys_checksum ((ushort *)ip, IP_HDR_SIZE / 2)));    
    
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
            icmp->icmp_sum = (ushort)(~(sys_checksum ((ushort *)icmp, ICMP_ECHO_HDR_SIZE / 2)));

            icmp_hdr_size = ICMP_ECHO_HDR_SIZE;
            
            break;
        }
        default:
           print_err("unsupported ICMP type (%d)", type);
    }

    return icmp_hdr_size;
}




