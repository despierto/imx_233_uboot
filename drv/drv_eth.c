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
#include "net_ks8851.h"
#include "drv_eth.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

//#define SYS_RAM_ETH_ADDR          /* 0x42000000 */
//#define SYS_RAM_ETH_SIZE          /* 0x10000      */
PETH_CTX                pEth = (PETH_CTX)SYS_RAM_ETH_CTX_ADDR;
static PETH_HEAP_CTX    pEthHeapCtx = (PETH_HEAP_CTX)SYS_RAM_ETH_HEAP_ADDR;

static int eth_heap_init(void);


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int drv_eth_init(void)
{
    int ret = 0;
    unsigned int i;
    PTR addr;
    
    eth_heap_init();

    addr = eth_heap_alloc();
    if (!addr) {
        print_err("%s", "network heap allocation test is failed");        
        return FAILURE;
    }
    ret = eth_heap_free(addr);
    if (ret) {
        print_err("%s", "network heap free test is failed");        
        return FAILURE;
    }

    drv_eth_halt();
    
    //net driver initialization
    print_eth("%s", "Net environment initialization");
    if (sizeof(ETH_CTX) > (unsigned int)SYS_RAM_ETH_CTX_SIZE) {
        print_err("Size of ETH CTX (%d) bytes is out of ranges (%d) bytes", sizeof(ETH_CTX), SYS_RAM_ETH_CTX_SIZE);        
        return FAILURE;
    }
    memset(pEth, 0, sizeof(ETH_CTX));
    print_eth(" - Eth CTX base (0x%x) size (%d) bytes", (unsigned int)pEth, sizeof(ETH_CTX));

    
    //setup packet buffers, aligned correctly
    for (i = 0; i < ETH_PKTBUFSTX; i++) {
        pEth->NetTxPackets[i] = (volatile uchar *)eth_heap_alloc();
        print_eth(" - Net TX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetTxPackets[i], ETH_PKTBUFSTX);        
    }
    for (i = 0; i < ETH_PKTBUFSRX; i++) {
        pEth->NetRxPackets[i] = (volatile uchar *)eth_heap_alloc();
        print_eth(" - Net RX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetRxPackets[i], i+1);                
    }

    pEth->NetArpWaitTxPacket = (volatile uchar *)eth_heap_alloc();
    pEth->NetArpWaitTxPacketSize = 0;
    print_eth(" - Net Arp Tx packet: base (0x%x) count (%d)", (unsigned int)pEth->NetArpWaitTxPacket, 1);
    
    pEth->Status = 0;
    copy_filename(pEth->BootFile, CONFIG_BOOTFILE, CONFIG_BOOTFILE_SIZE);    
    pEth->linux_load_addr = SYS_RAM_LOAD_ADDR;
    
    pEth->NetArpWaitPacketMAC = NULL;
    pEth->NetArpWaitPacketIP = 0;
    pEth->NetArpWaitReplyIP = 0;
    pEth->NetArpWaitTimerStart = 0;
    pEth->NetArpWaitTry = 0;

    ret = net_init(NULL);

    if (ret) {
        drv_eth_halt(); 
        pEth->Status = 0;
        print_err("%s", "ethernet initialization wasn't completed");
    } else {
        print_eth("%s", "Ethernel was successfully started");
        pEth->Status = 1;
    }

    //delay for printing out the print buffer
    sleep_ms(100);

    drv_eth_parse_enetaddr(CONFIG_HW_MAC_ADDR, &pEth->cfg_mac_addr[0]); // "ethaddr"
    pEth->cfg_ip_addr       = drv_string_to_ip(CONFIG_IPADDR);          // "ipaddr"
    pEth->cfg_ip_netmask    = drv_string_to_ip(CONFIG_NETMASK);         // "netmask"
    pEth->cfg_ip_gateway    = drv_string_to_ip(CONFIG_GATEWAYIP);       // "gatewayip"
    pEth->cfg_ip_server     = drv_string_to_ip(CONFIG_SERVERIP);        // "serverip"
    pEth->cfg_ip_dns        = drv_string_to_ip(CONFIG_DNSIP);           // "dnsip"
    pEth->cfg_ip_vlan       = drv_string_to_ip(CONFIG_VLANIP);          // "vlanip"


        
    return ret;
}

void drv_eth_halt(void)
{
    print_eth("%s", "Halt Ethernet driver");
    net_halt();
    sleep_ms(100);
}

int drv_eth_rx(void)
{
    U32 rx_len, rxfc, i;
    PTR pRxPacket;

    rxfc = net_rxfc_get();
    //print_eth("-------- RX %d packets -----------", rxfc);
    for (; rxfc != 0; rxfc--) {

        pRxPacket = eth_heap_alloc();
        assert(pRxPacket);
        
        rx_len = net_rx(pRxPacket);
        print_eth("-------- RX packet len %d bytes from %d packets -----------", rx_len, rxfc);
        if (pRxPacket && rx_len) {
            U8 *pA = (U8 *)pRxPacket;
            print_inf("[eth] Rx Packet[%x]: [ ", rx_len);
            for(i=0; i<rx_len; i++) {
                print_inf("%x ", pA[i]);
            }
            print_inf("]\r\n");
        }

        eth_heap_free(pRxPacket);
    }
    
       
    //NetReceive(ks->buff + 8, rxlen);

    return 0;
}

int drv_eth_tx(volatile void *packet, int length)
{

#if 0 //def GBL_ETH_DIAG_ENA
    print_inf("[dbg] TX (0x%08x|%d):[ ", (unsigned int)packet, length);
    {
        unsigned char *A = (unsigned char *)packet;
        unsigned int i;
        for (i=0; i<length; i++) {
            print_inf("%x ", A[i]);
        }
        print_inf("]\r\n");
    }
#endif

    net_tx(packet, length);
    return 0;
}

void drv_eth_parse_enetaddr(const char *addr, uchar *enetaddr)
{
    char *end;
    int i;
   
    for (i = 0; i < 6; ++i) {
        enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
        if (addr)
            addr = (*end) ? end + 1 : end;
    }
}

IPaddr_t drv_string_to_ip(char *s)
{
    IPaddr_t addr;
    char *e;
    int i;

    if (s == NULL)
        return(0);

    for (addr=0, i=0; i<4; ++i) {
        ulong val = s ? simple_strtoul(s, &e, 10) : 0;
        
        addr <<= 8;
        addr |= (val & 0xFF);
        if (s) {
            s = (*e) ? e+1 : e;
        }
    }

    return (htonl(addr));
}

char *drv_ip_to_string(IPaddr_t ip, uchar *buf)
{
    sprintf((char *)buf, "%03d.%03d.%03d.%03d", (ip & 0xFF), ((ip >> 8) & 0xFF), ((ip >> 16) & 0xFF), ((ip >> 24) & 0xFF));
    return (char *)buf;
}

void drv_eth_info(void)
{
    uchar s[15];
    
    print_eth("%s", "Ethernet configuration:");
    print_eth(" - ipaddr:       %s", drv_ip_to_string(pEth->cfg_ip_addr, &s[0]));
    print_eth(" - netmask:      %s", drv_ip_to_string(pEth->cfg_ip_netmask, &s[0]));
    print_eth(" - gatewayip:    %s", drv_ip_to_string(pEth->cfg_ip_gateway, &s[0]));
    print_eth(" - serverip:     %s", drv_ip_to_string(pEth->cfg_ip_server, &s[0]));
    print_eth(" - dnsip:        %s", drv_ip_to_string(pEth->cfg_ip_dns, &s[0]));
    print_eth(" - vlanip:       %s", drv_ip_to_string(pEth->cfg_ip_vlan, &s[0]));

    print_eth("%s", "Network heap status:");
    print_eth(" - alloc count:  %d", pEthHeapCtx->stats_alloc);
    print_eth(" - free  count:  %d", pEthHeapCtx->stats_free);
    print_eth(" - balance:      %d", pEthHeapCtx->stats_balance);
    
    return;
}

PTR         eth_heap_alloc(void)
{
    PTR addr;

    if (pEthHeapCtx->next_alloc_item) {
        addr = (PTR)pEthHeapCtx->next_alloc_item->addr;
        pEthHeapCtx->next_alloc_item->status = 1;
        pEthHeapCtx->next_alloc_item = (PETH_HEAP_LIST)pEthHeapCtx->next_alloc_item->next;
        pEthHeapCtx->stats_alloc++;
        pEthHeapCtx->stats_balance++;
            
        print_eth("[dbg] ---> net heap alloc: addr_%x", (U32)addr);
    } else {
        print_err("network heap is full");
        addr = NULL;
    }
    
    return addr;
}

int         eth_heap_free(PTR ptr)
{
    PETH_HEAP_LIST pList;
    U32 index;

    if (((U32)ptr < SYS_RAM_ETH_STORAGE_ADDR) || ((U32)ptr >= (SYS_RAM_ETH_STORAGE_ADDR + SYS_RAM_ETH_STORAGE_SIZE))) {
        print_err("PTR (0x%x) is out of network heap range");
        return FAILURE;
    }

    index = ((U32)ptr - SYS_RAM_ETH_STORAGE_ADDR)/NET_PKT_MAX_SIZE;
    pList = &pEthHeapCtx->list[index];
    
    if (!pList->status) {
        print_err("double free operation: blocked");
        return FAILURE;
    }

    print_eth("[dbg] ---> net heap free: addr_%x", (U32)ptr);

    pList->next = NULL;
    pList->status = 0;
    
    pEthHeapCtx->next_free_item->next = (U32 *)pList;
    pEthHeapCtx->next_free_item = pList;
    pEthHeapCtx->stats_free++;
    pEthHeapCtx->stats_balance--;
    
    return SUCCESS;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

static int eth_heap_init(void)
{
    U32 i;

    print_eth("%s", "Network heap initialization");

    if (sizeof(ETH_HEAP_CTX) > (unsigned int)SYS_RAM_ETH_HEAP_SIZE) {
        print_err("Size of eth heap ctx (%d) bytes is out of ranges (%d) bytes", sizeof(ETH_HEAP_CTX), SYS_RAM_ETH_HEAP_SIZE);        
        return FAILURE;
    }
    memset(pEthHeapCtx, 0, sizeof(ETH_HEAP_CTX));
    print_eth(" - eth heap ctx base (0x%x) size (%d) bytes", (unsigned int)pEthHeapCtx, sizeof(ETH_HEAP_CTX));

    if (sizeof(ETH_HEAP_CTX) > (unsigned int)SYS_RAM_ETH_HEAP_SIZE) {
        print_err("Size of eth heap ctx (%d) bytes is out of ranges (%d) bytes", sizeof(ETH_HEAP_CTX), SYS_RAM_ETH_HEAP_SIZE);        
        return FAILURE;
    }

    for (i=0; i<(NET_PKT_COUNT - 1); i++) {
        pEthHeapCtx->list[i].addr = (U32)(SYS_RAM_ETH_STORAGE_ADDR + NET_PKT_MAX_SIZE * i);
        pEthHeapCtx->list[i].next = (U32 *)&pEthHeapCtx->list[i+1];
        pEthHeapCtx->list[i].status = 0;
        //print_eth("   [%d] addr_%x next_%x", i, pEthHeapCtx->list[i].addr, (U32)pEthHeapCtx->list[i].next);
    }
    pEthHeapCtx->list[i].addr = (U32)(SYS_RAM_ETH_STORAGE_ADDR + NET_PKT_MAX_SIZE * i);
    pEthHeapCtx->list[i].next = NULL;
    pEthHeapCtx->list[i].status = 0;
    //print_eth("   [%d] addr_%x next_%x", i, pEthHeapCtx->list[i].addr, (U32)pEthHeapCtx->list[i].next);

    pEthHeapCtx->next_alloc_item = &pEthHeapCtx->list[0];
    pEthHeapCtx->next_free_item = &pEthHeapCtx->list[i];

    print_eth(" - next AI[%d]_%x FI[%d]_%x", 0, pEthHeapCtx->next_alloc_item, i, (U32)pEthHeapCtx->next_free_item);

    return SUCCESS;
}

