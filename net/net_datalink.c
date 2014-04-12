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

static U32  local_datalink_set_arp_hdr(ARP_t *arp, IPaddr_t dst_ip);
//static int  local_datalink_add_arp_req_to_list(PARP_REQ pArpReq);
//static int  local_datalink_rem_arp_req_from_list(PARP_REQ pArpReq);
void        local_datalink_arp_handler (void *param);



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
    int rc = SUCCESS;
    uchar *dst_mac;
    ARP_TABLE_STATE arp_status;

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
        
        //local_datalink_add_arp_req_to_list(pArpReq);

        //print_dbg("ARP request: pkt_0x%x, len_%d", (unsigned int)pArpEthPkt, arp_pkt_size);
        drv_eth_tx ((void *)pArpEthPkt, arp_pkt_size);  

        //update ARP table
        arp_table_reg_ip(dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
        
        core_reg_task(local_datalink_arp_handler, (void *)pArpReq, ARP_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__ARP, 0);
    }

    return DATALINK_TX_ARP_SENT;
}

int datalink_rx(void)
{
    int rc = SUCCESS;
    unsigned int size;
    unsigned int addr;    

    if (net_datalink_status == NET_DATALINK_STATUS_DIS) {
        //print_err("%s", NET_DATALINK_ERR_CAPTION_DIS);
        return FAILURE;
    }

    //print_net("%s", "datalink_rx");
    
    //get and process every packet
    while((size = drv_eth_rx_get(&addr)) != 0) {
        
        if (addr && size) {
#if 1            
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
void local_datalink_arp_handler (void *param)
{   
    PARP_REQ pArpReq = (PARP_REQ)param;

    assert(pArpReq);
    
    //print_dbg("pArpReq tx_num_%d", pArpReq->transmission_num);

    if (pArpReq->transmission_num < ARP_TIMEOUT_COUNT) {
        //send ARP again
        //print_dbg("ARP re-transmissions #%d: pkt_0x%x len_%d", pArpReqCurr->transmission_num, (unsigned int)pArpReq->addr,  pArpReq->size);                    

        pArpReq->transmission_num++;
        drv_eth_tx ((void *)pArpReq->addr, pArpReq->size);  

        arp_table_reg_ip(pArpReq->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_WAIT_ARP_RESPOND);
        core_reg_task(local_datalink_arp_handler, (void *)pArpReq, ARP_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__ARP, 0);
    } else {
        //ARP is obsolete - kill it
        arp_table_reg_ip(pArpReq->dst_ip, NULL, ARP_TABLE_TYPE_ETH, ARP_TABLE_STATE_INVALID);

        //print_dbg("%s", "ARP killed after 4 re-transmissions: pArpReq_%x pArpReq->addr_%x", (U32)pArpReq, pArpReq->addr);
        
        //free ARP packet
        drv_eth_heap_free((PTR)pArpReq->addr);
        //free ARP requst entity
        print_dbg("free at %s addr (0x%x)", pDataLinkCtx->arp_reg_pool_ctx->pool_caption, (U32)pArpReq);
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


