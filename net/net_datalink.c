/**
 * Data link layer
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
#include "net_datalink.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
PDATALINK_CTX pDataLinkCtx = NULL;

U8  NetEtherNullAddr[ETHER_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
U8  NetBcastAddr[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static U32  local_datalink_set_arp_hdr(ARP_t *arp, IPaddr_t dst_ip);
static int  local_datalink_add_arp_req_to_list(PARP_REQ pArpReq);
static int  local_datalink_rem_arp_req_from_list(PARP_REQ pArpReq);
void        local_datalink_arp_timeout_check (void);



/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int datalink_open(void)
{
    int rc = SUCCESS;

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

    //do pool testing
    {
        PTR addr;
        addr = sys_pool_alloc(pDataLinkCtx->arp_reg_pool_ctx);
        if (!addr) {
            print_err("%s", "ARP requests alloc");        
            return FAILURE;
        }
        rc = sys_pool_free(pDataLinkCtx->arp_reg_pool_ctx, addr);
        if (rc) {
            print_err("%s", "ARP requests free");        
            return rc;
        }
    }
    
    pDataLinkCtx->arp_list_head = NULL;
    pDataLinkCtx->arp_list_end = NULL;    
    
    return rc;
}

int datalink_close(void)
{
    int rc = SUCCESS;
    PARP_REQ pArpReq = NULL;

    /* Create ARM table */
    arp_table_destroy();

    //free all ARP requests? list entries and close pool
    pArpReq = pDataLinkCtx->arp_list_head;
    while(pArpReq) {
        if (pArpReq->addr) {
            drv_eth_heap_free((PTR)pArpReq->addr);
        }
        sys_pool_free(pDataLinkCtx->arp_reg_pool_ctx, (PTR)pArpReq);
        pArpReq = (PARP_REQ)pArpReq->next;
    }
    sys_pool_close(pDataLinkCtx->arp_reg_pool_ctx);
    
    /* Stop Ethernt device*/    
    drv_eth_halt();


    return rc;
}

PETH_PKT datalink_tx_alloc(void)
{
    return (PETH_PKT)drv_eth_heap_alloc();
}

DATALINK_TX_STATE datalink_tx_send(PETH_PKT pEthPkt, IPaddr_t dst_ip, U32 type, U32 size)
{
    int rc = SUCCESS;
    uchar *dst_mac;
    ARP_TABLE_STATE arp_status;

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
    {
        PETH_PKT pArpEthPkt = (PETH_PKT)drv_eth_heap_alloc();
        U32 arp_pkt_size = 0;
        PARP_REQ pArpReq;

        if (pArpEthPkt == NULL) {
            print_err_cmd("%s", "can't allocate new packet");
            return DATALINK_TX_CANT_ALLOC_PACKET;
        }
        
        arp_pkt_size += datalink_prepare_eth_hdr(pArpEthPkt, NetBcastAddr, ETH_P_ARP);
        arp_pkt_size += local_datalink_set_arp_hdr((ARP_t *)((U32)pArpEthPkt + arp_pkt_size), dst_ip);

        //store request to the ARP req pool
        pArpReq = (PARP_REQ)sys_pool_alloc(pDataLinkCtx->arp_reg_pool_ctx);
        if (pArpReq == NULL) {
            print_err_cmd("%s", "ARP requests queue overflow");
            return DATALINK_TX_ARP_QUEUE_OVERFLOW;
        }
        
        pArpReq->addr = (U32)pArpEthPkt;
        pArpReq->size = arp_pkt_size;
        pArpReq->dst_ip = dst_ip;
        pArpReq->transmission_num = 1;
        
        local_datalink_add_arp_req_to_list(pArpReq);

        print_dbg("ARP request: pkt_0x%x, len_%d", (unsigned int)pArpEthPkt, arp_pkt_size);
        drv_eth_tx ((void *)pArpEthPkt, arp_pkt_size);  

        //update ARP table
        arp_table_reg_ip(dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
    }

    return DATALINK_TX_ARP_SENT;
}

int datalink_rx_get_pkt(void)
{
    int rc = SUCCESS;



    return rc;
}

int datalink_task(void)
{
    int rc = SUCCESS;

   local_datalink_arp_timeout_check();

    

    return rc;
}

void datalink_info(void)
{
    sys_pool_info(pDataLinkCtx->arp_reg_pool_ctx);
    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

void local_datalink_arp_timeout_check (void)
{   
    PARP_REQ pArpReqCurr = pDataLinkCtx->arp_list_head;

    //check if any ARP request is waiting timeout
    if (pArpReqCurr) {
        PARP_REQ pArpReqLast = NULL;
                    
        while (pArpReqCurr && (pArpReqCurr != pArpReqLast)) {
            U32 curr_time = (U32)get_time_ms();
            U32 time_diff = get_time_diff(pArpReqCurr->reg_time, curr_time);

            //print_dbg("ARP-CHK: pArpReqCurr_%x pArpReqNext_%x pArpReqLast_%x regT_%d currT_%d dT_%d", (U32)pArpReqCurr, (U32)pArpReqCurr->next, (U32)pArpReqLast, pArpReqCurr->reg_time, curr_time, time_diff);
        
            if (time_diff > ARP_TIMEOUT) {
                //remove current ARP request from the list
                local_datalink_rem_arp_req_from_list(pArpReqCurr);

                print_dbg("ARP-CHK: pArpReqCurr_%x  regT_%d currT_%d dT_%d", (U32)pArpReqCurr, pArpReqCurr->reg_time, curr_time, time_diff);
                
                if (pArpReqCurr->transmission_num < ARP_TIMEOUT_COUNT) {
                    //send ARP again
                    print_dbg("ARP re-transmissions #%d: pkt_0x%x len_%d", pArpReqCurr->transmission_num, (unsigned int)pArpReqCurr->addr,  pArpReqCurr->size);                    
                    pArpReqCurr->transmission_num++;
                    local_datalink_add_arp_req_to_list(pArpReqCurr);
                    if (pArpReqLast == NULL) {
                        //this is first packet from next checking queue
                        pArpReqLast = pArpReqCurr;
                        print_dbg("---> pArpReqLast_%x", (U32)pArpReqLast);                        
                    }
                    drv_eth_tx ((void *)pArpReqCurr->addr, pArpReqCurr->size);  
                    arp_table_reg_ip(pArpReqCurr->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
                } else {
                    //ARP is obsolete - kill it
                    arp_table_reg_ip(pArpReqCurr->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_INVALID);

                    print_dbg("%s", "ARP killed after 4 re-transmissions: pArpReqCurr_%x pArpReqCurr->addr_%x", (U32)pArpReqCurr, pArpReqCurr->addr);
                    //free ARP packet
                    drv_eth_heap_free((PTR)pArpReqCurr->addr);
                    //free ARP requst entity
                    sys_pool_free(pDataLinkCtx->arp_reg_pool_ctx, (PTR)pArpReqCurr);
                }
                print_inf("%s", "-----------\n");
                pArpReqCurr = pDataLinkCtx->arp_list_head;
            }else {
                pArpReqCurr = pArpReqCurr->next;
            }
        }
        

    }
     
    return;
}

static int local_datalink_add_arp_req_to_list(PARP_REQ pArpReq)
{
    assert(pArpReq);
    pArpReq->reg_time = (U32)get_time_ms();    
    
    //print_inf("ADD BEFORE: h_%x hp_%x hn_%x e_%x ep_%x en_%x new_%x\n", 
    //                (U32)pDataLinkCtx->arp_list_head, (U32)pDataLinkCtx->arp_list_head->prev,  (U32)pDataLinkCtx->arp_list_head->next,
    //                (U32)pDataLinkCtx->arp_list_end, (U32)pDataLinkCtx->arp_list_end->prev,  (U32)pDataLinkCtx->arp_list_end->next, (U32)pArpReq);    

    pArpReq->next = NULL;
    if (pDataLinkCtx->arp_list_head == NULL) {
        pArpReq->prev = NULL;
        pDataLinkCtx->arp_list_head = pArpReq; 
        pDataLinkCtx->arp_list_end = pArpReq;
    } else {
        pArpReq->prev = pDataLinkCtx->arp_list_end;
        pDataLinkCtx->arp_list_end->next = pArpReq;
        pDataLinkCtx->arp_list_end = pArpReq;
    }

    //print_inf("ADD END: h_%x hp_%x hn_%x e_%x ep_%x en_%x\n", 
    //                (U32)pDataLinkCtx->arp_list_head, (U32)pDataLinkCtx->arp_list_head->prev,  (U32)pDataLinkCtx->arp_list_head->next,
    //                (U32)pDataLinkCtx->arp_list_end, (U32)pDataLinkCtx->arp_list_end->prev,  (U32)pDataLinkCtx->arp_list_end->next);    

    return SUCCESS;
}

static int local_datalink_rem_arp_req_from_list(PARP_REQ pArpReq)
{
   assert(pArpReq);

    //print_inf("REM BEFORE: h_%x hp_%x hn_%x e_%x ep_%x en_%x old_%x\n", 
    //                (U32)pDataLinkCtx->arp_list_head, (U32)pDataLinkCtx->arp_list_head->prev,  (U32)pDataLinkCtx->arp_list_head->next,
    //                (U32)pDataLinkCtx->arp_list_end, (U32)pDataLinkCtx->arp_list_end->prev,  (U32)pDataLinkCtx->arp_list_end->next, (U32)pArpReq);    
   
    if ((U32)pArpReq == (U32)pDataLinkCtx->arp_list_head) {
        pDataLinkCtx->arp_list_head = pArpReq->next;
        if (pDataLinkCtx->arp_list_head) {
            pDataLinkCtx->arp_list_head->prev = NULL;
        }
    } else if ((U32)pArpReq == (U32)pDataLinkCtx->arp_list_end) {
    
        pDataLinkCtx->arp_list_end = pArpReq->prev;
        pDataLinkCtx->arp_list_end->next = NULL;
    } else {
        //somewhere at the middle
        pArpReq->prev = pArpReq->next;
    }

    //print_inf("REM END: h_%x hp_%x hn_%x e_%x ep_%x en_%x\n", 
    //                (U32)pDataLinkCtx->arp_list_head, (U32)pDataLinkCtx->arp_list_head->prev,  (U32)pDataLinkCtx->arp_list_head->next,
    //                (U32)pDataLinkCtx->arp_list_end, (U32)pDataLinkCtx->arp_list_end->prev,  (U32)pDataLinkCtx->arp_list_end->next);    

    return SUCCESS;
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

static U32 local_datalink_set_arp_hdr(ARP_t *arp, IPaddr_t dst_ip)
{
    IPaddr_t final_dst_ip;
    
    arp->ar_htype = htons (ARP_HW_TYPE_ETHER);
    arp->ar_ptype = htons (ETH_P_IP);
    arp->ar_hlen = ETHER_ADDR_LEN;
    arp->ar_plen = IP_ADDR_LEN;
    arp->ar_oper = htons (ARP_OP_REQUEST);

    memcpy ((void *)&arp->ar_sha[0], (void *)&pGblCtx->cfg_mac_addr[0], ETHER_ADDR_LEN);
    memcpy ((void *)&arp->ar_spa[0], (void *)&pGblCtx->cfg_ip_addr, IP_ADDR_LEN);
    memcpy ((void *)&arp->ar_tha[0], (void *)&NetEtherNullAddr[0], ETHER_ADDR_LEN);

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


