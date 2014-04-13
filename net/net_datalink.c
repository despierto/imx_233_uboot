/**
 * Data link layer
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
#include "net_datalink.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
PDATALINK_CTX pDataLinkCtx = NULL;
static U32 net_datalink_status = NET_DATALINK_STATUS_DIS;
    
U8  NetEtherNullAddr[ETHER_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
U8  NetBcastAddr[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static U32  local_datalink_set_arp_hdr(ARP_t *arp, IPaddr_t dst_ip, U8 *dst_mac_addr, ushort operation);
void        local_datalink_arp_handler (void *param);
void        local_datalink_dump_packet(U32 addr, U32 size, char *caption);
static U32  local_datalink_create_arp_request(IPaddr_t dst_ip, PARP_REQ *pArpReqEx);
static U32  local_datalink_create_arp_replay(IPaddr_t spa, uchar *sha);

/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int datalink_open(void)
{
    int rc = SUCCESS;

    if (net_datalink_status == NET_DATALINK_STATUS_ENA) {
        print_err("%s",NET_DATALINK_ERR_CAPTION_DIS);
        return FAILURE;
    }

    /* Configure Ethernt device*/
    rc = drv_eth_init();
    if (rc)
        return rc;

    /* Create ARM table */
    rc = arp_table_create();
    if (rc)
        return rc;

    arp_table_set_valid_period(ARP_VALID_PERIOD);

    //init dala link ctx
    pDataLinkCtx = (PDATALINK_CTX)malloc(sizeof(DATALINK_CTX));
    assert(pDataLinkCtx);
    memset((void *)pDataLinkCtx, 0, sizeof(DATALINK_CTX));

    //setup mac address
    drv_eth_mac_set(pGblCtx->cfg_mac_addr);

    //allocate pool for storing current ARP requests 
    pDataLinkCtx->arp_reg_pool_ctx = sys_pool_init(ARP_TABLE_SIZE, sizeof(ARP_REQ), (U8 *)&("ARP requests"));
    assert(pDataLinkCtx->arp_reg_pool_ctx);
    assert_rc(sys_pool_test(pDataLinkCtx->arp_reg_pool_ctx));

    //reg eth packet retrieving task -it must fask take packets from eth device and place into the rx queue for future processing
    print_eth("register rx task: class=prio priority=%d", CORE_TASK_PRIO__ETH_RX);
    core_reg_task(drv_eth_rx, NULL, 0, CORE_TASK_TYPE_PRIORITY, CORE_TASK_PRIO__ETH_RX, 0);

    net_datalink_status = NET_DATALINK_STATUS_ENA;
    
    return rc;
}

int datalink_close(void)
{
    int rc = SUCCESS;
    PARP_REQ pArpReq = NULL;

    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return FAILURE;
    }

    /* Create ARM table */
    arp_table_destroy();
    sys_pool_close(pDataLinkCtx->arp_reg_pool_ctx);
    
    /* Stop Ethernt device*/    
    drv_eth_halt();

    return rc;
}

PETH_PKT datalink_tx_alloc(void)
{
    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return NULL;
    }

    return (PETH_PKT)drv_eth_heap_alloc();
}

DATALINK_TX_STATE datalink_tx_send(PETH_PKT pEthPkt, IPaddr_t dst_ip, U32 type, U32 size)
{
    DATALINK_TX_STATE rc;
    uchar *dst_mac;
    ARP_TABLE_STATE arp_status;
    PARP_REQ pArpReq;

    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return DATALINK_TX_ERROR;
    }

    if (pEthPkt == NULL) {
        print_err("%s", "packet to send it null");
        return DATALINK_TX_ERROR;
    }

    if (size < ETHER_HDR_SIZE) {
        print_err("packet size %d is wrong", size);
        return DATALINK_TX_ERROR;
    }

    //check presence of ip address at ARP table
    arp_status = arp_table_get_mac(dst_ip, &dst_mac);

    if (arp_status == ARP_TABLE_STATE_VALID) {
        //set dst mac
        datalink_set_eth_addr(pEthPkt, dst_mac);
        drv_eth_tx ((void *)pEthPkt, size);        
        return DATALINK_TX_SUCCESS;
    } else if (arp_status == ARP_TABLE_STATE_WAIT_ARP_RESPOND) {
        //still waiting for ARM responce
        return DATALINK_TX_ARP_WAIT;
    }

    //no mac addr or it is obsolete: send ARP broadcast request
    rc = local_datalink_create_arp_request(dst_ip, &pArpReq);
    if (rc == DATALINK_TX_ARP_SENT) {
        //update ARP table
        arp_table_reg_ip(dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
        core_reg_task(local_datalink_arp_handler, (void *)pArpReq, ARP_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__ARP, 0);
    }

    return rc;
}

int datalink_rx(void)
{
    int rc = SUCCESS;
    U32 size, addr;    

    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        //print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return FAILURE;
    }
    
    //get and process every packet
    while((size = drv_eth_rx_get(&addr)) != 0) {
        //3 ----------------------------------
        //3 Goal: 
        //4 - extract packet from ethernet queue
        //4 - place packet into network queue
        //4 - process ARP
        //4 - process ICMP ping requests
        //3 ----------------------------------
        
        if (addr && size) {
            PETH_PKT pEthPkt = (PETH_PKT)addr;

            //printf("\n-------------\n");
            //local_datalink_dump_packet(addr, size, "ANY");

            //print_net("type_%x dst_%x:%x:%x:%x:%x:%x src_%x:%x:%x:%x:%x:%x", 
            //    htons(pEthPkt->header.type), 
            //    pEthPkt->header.dst[0], pEthPkt->header.dst[1], pEthPkt->header.dst[2], 
            //    pEthPkt->header.dst[3], pEthPkt->header.dst[4], pEthPkt->header.dst[5],
            //    pEthPkt->header.src[0], pEthPkt->header.src[1], pEthPkt->header.src[2],
            //    pEthPkt->header.src[3], pEthPkt->header.src[4], pEthPkt->header.src[5]);
               
            switch (htons(pEthPkt->header.type))
            {
                case ETH_P_IP:
                    {
                        //local_datalink_dump_packet(addr, size, "IP");

                        drv_eth_heap_free((PTR)addr);
                    }
                    break;

                case ETH_P_ARP:
                    {
                        ARP_t *pArpHdr = (ARP_t *)&pEthPkt->payload[0];
#if 0                        
                        printf("\n-------------\n");
                        print_net("type_%02x dst_%02x:%02x:%02x:%02x:%02x:%02x src_%02x:%02x:%02x:%02x:%02x:%02x", 
                            htons(pEthPkt->header.type), 
                            pEthPkt->header.dst[0], pEthPkt->header.dst[1], pEthPkt->header.dst[2], 
                            pEthPkt->header.dst[3], pEthPkt->header.dst[4], pEthPkt->header.dst[5],
                            pEthPkt->header.src[0], pEthPkt->header.src[1], pEthPkt->header.src[2],
                            pEthPkt->header.src[3], pEthPkt->header.src[4], pEthPkt->header.src[5]);

                        local_datalink_dump_packet((U32)&pEthPkt->payload[0], size - ETHER_HDR_SIZE, "ARP");

                        print_net("ARP: ht_%02x pt_%02x hl_%x pl_%x op_%04x sha_%02x:%02x:%02x:%02x:%02x:%02x spa_%d.%d.%d.%d tha_%02x:%02x:%02x:%02x:%02x:%02x tpa_%d.%d.%d.%d",
                            htons(pArpHdr->ar_htype), htons(pArpHdr->ar_ptype), 
                            pArpHdr->ar_hlen, pArpHdr->ar_plen, htons(pArpHdr->ar_oper),
                            pArpHdr->ar_sha[0], pArpHdr->ar_sha[1], pArpHdr->ar_sha[2], 
                            pArpHdr->ar_sha[3], pArpHdr->ar_sha[4], pArpHdr->ar_sha[5],
                            pArpHdr->ar_spa[0], pArpHdr->ar_spa[1], pArpHdr->ar_spa[2], pArpHdr->ar_spa[3],
                            pArpHdr->ar_tha[0], pArpHdr->ar_tha[1], pArpHdr->ar_tha[2], 
                            pArpHdr->ar_tha[3], pArpHdr->ar_tha[4], pArpHdr->ar_tha[5],
                            pArpHdr->ar_tpa[0], pArpHdr->ar_tpa[1], pArpHdr->ar_tpa[2], pArpHdr->ar_tpa[3]);
#endif
                        switch (htons(pArpHdr->ar_oper))
                        {
                            case ARP_OP_REQUEST:
                                {
                                    if (memcmp(&pArpHdr->ar_tpa[0], &pGblCtx->cfg_ip_addr, IP_ADDR_LEN) == TRUE) {
                                        IPaddr_t spa;
                                        
                                        memcpy(&spa, &pArpHdr->ar_spa[0], IP_ADDR_LEN);
                                                                               
                                        //print_net("Create ARP replay to (%x)", spa);
                                        local_datalink_create_arp_replay(spa, &pArpHdr->ar_sha[0]);
                                        
                                        //now we also know requester mac
                                        arp_table_reg_ip(spa, (char *)&pArpHdr->ar_sha[0], ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_VALID);
                                    }
                                }
                                break;
                                
                            case ARP_OP_REPLY:
                                {
                                    //add to ARP table
                                    IPaddr_t spa;

                                    memcpy(&spa, &pArpHdr->ar_spa[0], IP_ADDR_LEN);

                                    //print_net("Reg ARP: ip_%d.%d.%d.%d mac_%02x:%02x:%02x:%02x:%02x:%02x", 
                                    //    pArpHdr->ar_spa[0], pArpHdr->ar_spa[1], pArpHdr->ar_spa[2], pArpHdr->ar_spa[3], 
                                    //    pArpHdr->ar_sha[0], pArpHdr->ar_sha[1], pArpHdr->ar_sha[2], 
                                    //    pArpHdr->ar_sha[3], pArpHdr->ar_sha[4], pArpHdr->ar_sha[5]);
                                    arp_table_reg_ip(spa, (char *)&pArpHdr->ar_sha[0], ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_VALID);
                                }
                                break;
                                
                            default:
                                break;
                        }
                        
                        drv_eth_heap_free((PTR)addr);                        
                    }
                    break;

                default:
                    drv_eth_heap_free((PTR)addr);
                    break;
            }
        }
    }

    return rc;
}

void datalink_info(void)
{
    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return;
    }

    sys_pool_info(pDataLinkCtx->arp_reg_pool_ctx);
    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/
void local_datalink_dump_packet(U32 addr, U32 size, char *caption)
{
    U8 *pA = (U8 *)addr;
    unsigned int i;            
    
    print_inf("[net] --- Rx %s Packet[0x%x, %d]: [ ", caption, addr, size);
    for(i=0; i<size; i++) {
        print_inf("%x ", pA[i]);
    }
    print_inf("] --- \r\n");
    
    return;
}

void local_datalink_arp_handler (void *param)
{   
    PARP_REQ pArpReq = (PARP_REQ)param;
    U32 do_arpreq_freeing = 0;

    assert(pArpReq);
    
    //print_dbg("pArpReq tx_num_%d", pArpReq->transmission_num);

    if (pArpReq->transmission_num < ARP_TIMEOUT_COUNT) {
        //check if valid IP existing
        if (arp_table_check_mac(pArpReq->dst_ip) != ARP_TABLE_STATE_VALID) {
            //send ARP again
            //print_dbg("ARP re-transmissions #%d: pkt_0x%x len_%d", pArpReqCurr->transmission_num, (unsigned int)pArpReq->addr,  pArpReq->size);                    

            pArpReq->transmission_num++;
            drv_eth_tx ((void *)pArpReq->addr, pArpReq->size);  

            arp_table_reg_ip(pArpReq->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
            core_reg_task(local_datalink_arp_handler, (void *)pArpReq, ARP_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__ARP, 0);
        } else {
            //compete current arp processing
            do_arpreq_freeing = 1;
        }
    } else {
        //ARP is obsolete - kill it
        arp_table_reg_ip(pArpReq->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_INVALID);

        //print_dbg("%s", "ARP killed after 4 re-transmissions: pArpReq_%x pArpReq->addr_%x", (U32)pArpReq, pArpReq->addr);
        do_arpreq_freeing = 1;
    }

    if (do_arpreq_freeing) {
        //free ARP packet
        drv_eth_heap_free((PTR)pArpReq->addr);
        //free ARP requst entity
        //print_dbg("free at %s addr (0x%x)", pDataLinkCtx->arp_reg_pool_ctx->pool_caption, (U32)pArpReq);
        sys_pool_free(pDataLinkCtx->arp_reg_pool_ctx, (PTR)pArpReq);
    }

    return;
}

U32 datalink_prepare_eth_hdr(PETH_PKT pkt, U8 *dst_mac_addr, U16 protocol)
{
#if 1
    
    if (dst_mac_addr) {
        memcpy ((void *)pkt->header.dst, (void *)dst_mac_addr, ETHER_ADDR_LEN);
    }
    memcpy ((void *)pkt->header.src, (void *)pGblCtx->cfg_mac_addr, ETHER_ADDR_LEN);
    pkt->header.type = htons(protocol);

    return ETHER_HDR_SIZE;
#else
    Ethernet_t *et = (Ethernet_t *)pkt;
    ushort myvlanid;

    myvlanid = ntohs(pGblCtx->cfg_ip_vlan);
    if (myvlanid == (ushort)-1) /* ? == 0xffff */
        myvlanid = VLAN_NONE;   /* = 0x1000 */

    if (addr) {
        memcpy ((void *)et->et_dest, (void *)addr, 6);
    }
    memcpy ((void *)et->et_src, (void *)pGblCtx->cfg_mac_addr, 6);
    
    if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
        et->et_protlen = htons(protocol);
        return ETHER_HDR_SIZE;
    } else {
        VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;

        vet->vet_vlan_type = htons(PROT_VLAN);
        vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
        vet->vet_type = htons(prot);
        return VLAN_ETHER_HDR_SIZE;
    }
#endif    
}

U32 datalink_set_eth_addr(PETH_PKT pkt, U8 *dst_mac_addr)
{
    Ethernet_t *et = (Ethernet_t *)pkt;

    if (dst_mac_addr) {
        memcpy ((void *)pkt->header.dst, (void *)dst_mac_addr, 6);
    } else {
        return FAILURE;
    }
    
    return SUCCESS;
}

static U32 local_datalink_set_arp_hdr(ARP_t *arp, IPaddr_t dst_ip, U8 *dst_mac_addr, ushort operation)
{
    IPaddr_t final_dst_ip;
    
    arp->ar_htype = htons (ARP_HW_TYPE_ETHER);
    arp->ar_ptype = htons (ETH_P_IP);
    arp->ar_hlen = ETHER_ADDR_LEN;
    arp->ar_plen = IP_ADDR_LEN;
    arp->ar_oper = htons (operation);

    memcpy ((void *)&arp->ar_sha[0], (void *)&pGblCtx->cfg_mac_addr[0], ETHER_ADDR_LEN);
    memcpy ((void *)&arp->ar_spa[0], (void *)&pGblCtx->cfg_ip_addr, IP_ADDR_LEN);
    memcpy ((void *)&arp->ar_tha[0], (void *)&dst_mac_addr[0], ETHER_ADDR_LEN);

    if ((dst_ip & pGblCtx->cfg_ip_netmask) != (pGblCtx->cfg_ip_addr & pGblCtx->cfg_ip_netmask)) {
        if (pGblCtx->cfg_ip_gateway == 0) {
            print_err ("%s", "## Warning: gatewayip needed but not set");
            final_dst_ip = dst_ip;
        } else {
            final_dst_ip = pGblCtx->cfg_ip_gateway;
        }
    } else {
        final_dst_ip = dst_ip;
    }
    memcpy ((void *)&arp->ar_tpa[0], (void *)&final_dst_ip, IP_ADDR_LEN);
    
    return ARP_HDR_SIZE;
}

static U32 local_datalink_create_arp_request(IPaddr_t dst_ip, PARP_REQ *pArpReqEx)
{
    PETH_PKT pArpEthPkt = (PETH_PKT)drv_eth_heap_alloc();
    U32 arp_pkt_size = 0;
    PARP_REQ pArpReq;

    if (pArpEthPkt == NULL) {
        print_err_cmd("%s", "can't allocate new packet");
        return DATALINK_TX_CANT_ALLOC_PACKET;
    }
    
    arp_pkt_size += datalink_prepare_eth_hdr(pArpEthPkt, NetBcastAddr, ETH_P_ARP);
    arp_pkt_size += local_datalink_set_arp_hdr((ARP_t *)((U32)pArpEthPkt + arp_pkt_size), dst_ip, &NetEtherNullAddr[0], ARP_OP_REQUEST);

    //store request to the ARP req pool
    pArpReq = (PARP_REQ)sys_pool_alloc(pDataLinkCtx->arp_reg_pool_ctx);
    if (pArpReq == NULL) {
        drv_eth_heap_free(pArpEthPkt);
        print_err_cmd("%s", "ARP requests queue overflow");
        return DATALINK_TX_ARP_QUEUE_OVERFLOW;
    }
    
    pArpReq->addr = (U32)pArpEthPkt;
    pArpReq->size = arp_pkt_size;
    pArpReq->dst_ip = dst_ip;
    pArpReq->transmission_num = 1;
    
    //local_datalink_add_arp_req_to_list(pArpReq);

    //print_dbg("ARP request: pkt_0x%x, len_%d", (unsigned int)pArpEthPkt, arp_pkt_size);
    drv_eth_tx ((void *)pArpEthPkt, arp_pkt_size);  

    *pArpReqEx = pArpReq;

    return DATALINK_TX_ARP_SENT;
}    

static U32 local_datalink_create_arp_replay(IPaddr_t spa, uchar *sha)
{
    PETH_PKT pArpEthPkt = (PETH_PKT)drv_eth_heap_alloc();
    U32 arp_pkt_size = 0;

    if (pArpEthPkt == NULL) {
        print_err_cmd("%s", "can't allocate new packet");
        return DATALINK_TX_CANT_ALLOC_PACKET;
    }
    
    arp_pkt_size += datalink_prepare_eth_hdr(pArpEthPkt, sha, ETH_P_ARP);
    arp_pkt_size += local_datalink_set_arp_hdr((ARP_t *)((U32)pArpEthPkt + arp_pkt_size), spa, sha, ARP_OP_REPLY);

    //print_dbg("ARP request: pkt_0x%x, len_%d", (unsigned int)pArpEthPkt, arp_pkt_size);
    drv_eth_tx ((void *)pArpEthPkt, arp_pkt_size);  

    return DATALINK_TX_ARP_SENT;
}    


