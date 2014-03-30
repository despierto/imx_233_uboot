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
#include "net_arp.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
PDATALINK_CTX pDataLinkCtx = NULL;

U8  NetEtherNullAddr[ETHER_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
U8  NetBcastAddr[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static U32  local_datalink_set_eth_hdr(PETH_PKT pkt, U8 *addr, U16 protocol);
static U32  local_datalink_set_arp_hdr(ARP_t *arp);
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
    pDataLinkCtx->arp_reg_pool = sys_pool_init(ARP_TABLE_SIZE, sizeof(ARP_REQ), (U8 *)&("ARP requests"));
    assert(pDataLinkCtx->arp_reg_pool);

    //do pool testing
    {
        PTR addr;
        addr = sys_pool_alloc(pDataLinkCtx->arp_reg_pool);
        if (!addr) {
            print_err("%s", "ARP requests alloc");        
            return FAILURE;
        }
        rc = sys_pool_free(pDataLinkCtx->arp_reg_pool, addr);
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
        sys_pool_free(pDataLinkCtx->arp_reg_pool, (PTR)pArpReq);
        pArpReq = (PARP_REQ)pArpReq->next;
    }
    sys_pool_close(pDataLinkCtx->arp_reg_pool);
    
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
        //set src mac
        memcpy ((void *)pEthPkt->header.src, (void *)&pGblCtx->cfg_mac_addr[0], ETHER_ADDR_LEN);
        //set dst mac
        memcpy ((void *)pEthPkt->header.dst, (void *)dst_mac, ETHER_ADDR_LEN);
        pEthPkt->header.type = type;

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
        
        arp_pkt_size += local_datalink_set_eth_hdr(pArpEthPkt, NetBcastAddr, ETH_P_ARP);
        arp_pkt_size += local_datalink_set_arp_hdr((ARP_t *)((U32)pArpEthPkt + arp_pkt_size));

        //store request to the ARP req pool
        pArpReq = (PARP_REQ)sys_pool_alloc(pDataLinkCtx->arp_reg_pool);
        pArpReq->addr = (U32)pArpEthPkt;
        pArpReq->size = arp_pkt_size;
        pArpReq->dst_ip = dst_ip;
        pArpReq->transmission_num = 1;
        
        local_datalink_add_arp_req_to_list(pArpReq);

        print_net("ARP request: pkt_0x%x, len_%d", (unsigned int)pArpEthPkt, arp_pkt_size);
        drv_eth_tx ((void *)pArpEthPkt, arp_pkt_size);  

        //update ARP table
        arp_table_reg_ip(dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
    }

    return rc;
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


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

void local_datalink_arp_timeout_check (void)
{   
    PARP_REQ pArpReqCurr = pDataLinkCtx->arp_list_head;

    //check if any ARP request is waiting timeout
    if (pArpReqCurr) {
        U32 curr_time = (U32)get_time_s();
        
        while (pArpReqCurr) {
            U32 time_diff = get_time_diff(pArpReqCurr->reg_time, curr_time);
                        
            if (time_diff > ARP_TIMEOUT) {
                //remove current ARP request from the list
                local_datalink_rem_arp_req_from_list(pArpReqCurr);
                
                if (pArpReqCurr->transmission_num < ARP_TIMEOUT_COUNT) {
                    //send ARP again
                    print_net("ARP re-transmissions #%d: pkt_0x%x len_%d", pArpReqCurr->transmission_num, (unsigned int)pArpReqCurr->addr,  pArpReqCurr->size);                    
                    pArpReqCurr->transmission_num++;
                    local_datalink_add_arp_req_to_list(pArpReqCurr);

                    drv_eth_tx ((void *)pArpReqCurr->addr, pArpReqCurr->size);  
                    arp_table_reg_ip(pArpReqCurr->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
                } else {
                    //ARP is obsolete - kill it
                    arp_table_reg_ip(pArpReqCurr->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_INVALID);
                    drv_eth_heap_free((PTR)pArpReqCurr->addr);
                    sys_pool_free(pDataLinkCtx->arp_reg_pool, (PTR)pArpReqCurr);
                    print_net("%s", "ARP killed after 4 re-transmissions");
                }

                //it is next due current request ews remived from head
                pArpReqCurr = pDataLinkCtx->arp_list_head;
            }else {
                //it doesn't make sense to check next requests due they are adding in ascending time manner
                pArpReqCurr = NULL;
            }
        }
    }
     
    return;
}

static int local_datalink_add_arp_req_to_list(PARP_REQ pArpReq)
{
    PARP_REQ pArpReqCurr = pDataLinkCtx->arp_list_head;

    assert(pArpReq);
    pArpReq->reg_time = (U32)get_time_s();    
    
    pArpReq->next = NULL;
        
    if (pArpReqCurr == NULL) {
        pArpReq->prev = NULL;
        pDataLinkCtx->arp_list_head = pArpReq; 
        pDataLinkCtx->arp_list_end = pArpReq;
    } else {
        pArpReq->prev = pDataLinkCtx->arp_list_end;
        pDataLinkCtx->arp_list_end->next = pArpReq;
    }

    return SUCCESS;
}

static int local_datalink_rem_arp_req_from_list(PARP_REQ pArpReq)
{
   assert(pArpReq);
   
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

    return SUCCESS;
}

static U32 local_datalink_set_eth_hdr(PETH_PKT pkt, U8 *addr, U16 protocol)
{
    Ethernet_t *et = (Ethernet_t *)pkt;
    //ushort myvlanid;

    //myvlanid = ntohs(pGblCtx->cfg_ip_vlan);
    //if (myvlanid == (ushort)-1) /* ? == 0xffff */
    //    myvlanid = VLAN_NONE;   /* = 0x1000 */

    memcpy ((void *)et->et_dest, (void *)addr, 6);
    memcpy ((void *)et->et_src, (void *)pGblCtx->cfg_mac_addr, 6);
    
    //if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
        et->et_protlen = htons(protocol);
        return ETHER_HDR_SIZE;
    //} else {
    //    VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;

    //    vet->vet_vlan_type = htons(PROT_VLAN);
    //    vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
    //    vet->vet_type = htons(prot);
    //    return VLAN_ETHER_HDR_SIZE;
    //}
}

static U32 local_datalink_set_arp_hdr(ARP_t *arp)
{
    arp->ar_htype = htons (ARP_HW_TYPE_ETHER);
    arp->ar_ptype = htons (ETH_P_IP);
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


