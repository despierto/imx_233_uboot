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
#include "drv_ks8851.h"
#include "drv_eth.h"

PETH_HEAP_CTX 	pEthHeapCtx = NULL;
PETH_CTX		pEth = NULL;


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

/**
* Definition of usage ks8851 as physical layer
*/
#define drv_net_init(ptr)           ks8851_init(ptr)
#define drv_net_halt()              ks8851_halt()
#define drv_net_rxfc_get()          ks8851_rxfc_get()
#define drv_net_rx(rx_buff)         ks8851_rx(rx_buff) 
#define drv_net_tx(packet, length)  ks8851_tx(packet, length)
#define drv_net_mac_set(ethaddr)    ks8851_mac_set(ethaddr)


static int 			drv_eth_heap_init(void);
static unsigned int drv_eth_rx_put(unsigned int addr, unsigned int size);

/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int drv_eth_init(void)
{
    int ret = 0;
    unsigned int i;
    PTR addr;
	
    drv_eth_heap_init();

    addr = drv_eth_heap_alloc();
    if (!addr) {
        print_err("%s", "network heap allocation test is failed");        
        return FAILURE;
    }
    ret = drv_eth_heap_free(addr);
    if (ret) {
        print_err("%s", "network heap free test is failed");        
        return FAILURE;
    }

    drv_eth_halt();
    
    //eth ctx initialization
    print_eth("%s", "Net environment initialization");
	pEth = (PETH_CTX)malloc(sizeof(ETH_CTX));
	assert(pEth);
    memset(pEth, 0, sizeof(ETH_CTX));
    print_eth(" - Eth CTX base (0x%x) size (%d) bytes", (unsigned int)pEth, sizeof(ETH_CTX));

    ret = drv_net_init(NULL);
	
    if (ret) {
        drv_eth_halt(); 
        pGblCtx->Status = 0;
        print_err("%s", "ethernet initialization wasn't completed");
    } else {
        print_eth("%s", "Ethernel was successfully started");
        pGblCtx->Status = 1;
    }

	//rx pool init
	pEth->rx_pool_get = 0;
	pEth->rx_pool_put = 0;	
		
    return ret;
}

/** return size in case some address is in queue or 0 */
unsigned int drv_eth_rx_get(unsigned int *addr)
{
	unsigned int size = 0;
	unsigned int get = pEth->rx_pool_get;
	
	if (pEth->rx_pool_put != get) {
		*addr = pEth->rx_pool[get].addr;
		size =  pEth->rx_pool[get].size;

		print_eth("GET[%d]: addr_0x%x size_%d", get, *addr, size);
	
		if (++get >= ETH_RX_POOL_SIZE)
			pEth->rx_pool_get = 0;
		else
			pEth->rx_pool_get = get;
	}

	return size;
}

void drv_eth_halt(void)
{
    print_eth("%s", "Halt Ethernet driver");
    drv_net_halt();
    sleep_ms(100);
}

int drv_eth_rx(void)
{
    U32 rx_len, rxfc, i;
    PTR pRxPacket;

    rxfc = drv_net_rxfc_get();
    for (; rxfc != 0; rxfc--) {

        pRxPacket = drv_eth_heap_alloc();
        assert(pRxPacket);
        
        rx_len = drv_net_rx(pRxPacket);
		if (rx_len < NET_HW_RX_HEADER_SIZE) {
			print_eth("WARNING: received packed with unexpected size (%d)", rx_len);
			continue;
		} else {
		 	rx_len = rx_len - NET_HW_RX_HEADER_SIZE;
		}

		if (rx_len) {
			unsigned int real_packet = (unsigned int)pRxPacket + NET_HW_RX_HEADER_SIZE;

			print_eth("RX PUT: addr_0x%x size_%d pkt_%x", real_packet, rx_len, (unsigned int)pRxPacket);
			
			if (drv_eth_rx_put(real_packet, rx_len)) {
				print_eth("WARNING: Killed RX packet, len (%d)", rx_len);
		        drv_eth_heap_free(pRxPacket);
			}
		} else {
			print_eth("%s", "WARNING: received packed with null payload");
		}
#if 0		
        print_eth("-------- RX packet len %d bytes from %d packets -----------", rx_len, rxfc);
        if (pRxPacket && rx_len) {
            U8 *pA = (U8 *)pRxPacket;
            print_inf("[eth] Rx Packet[%x]: [ ", rx_len);
            for(i=0; i<rx_len; i++) {
                print_inf("%x ", pA[i]);
            }
            print_inf("]\r\n");
        }
#endif
    }
       
    return 0;
}

int drv_eth_tx(void *packet, int length)
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

    drv_net_tx(packet, length);
    return 0;
}

void drv_eth_info(void)
{
    print_eth("%s", "Network heap status:");
    print_eth(" - alloc count:  %d", pEthHeapCtx->stats_alloc);
    print_eth(" - free  count:  %d", pEthHeapCtx->stats_free);
    print_eth(" - balance:      %d", pEthHeapCtx->stats_balance);
    
    return;
}

PTR         drv_eth_heap_alloc(void)
{
    PTR addr;

    if (pEthHeapCtx->next_alloc_item) {
        addr = (PTR)pEthHeapCtx->next_alloc_item->addr;
        pEthHeapCtx->next_alloc_item->status = 1;
        pEthHeapCtx->next_alloc_item = (PETH_HEAP_LIST)pEthHeapCtx->next_alloc_item->next;
        pEthHeapCtx->stats_alloc++;
        pEthHeapCtx->stats_balance++;
            
        //print_eth("[dbg] ---> net heap alloc: addr_%x", (U32)addr);
    } else {
        print_err("%s", "network heap is full");
        addr = NULL;
    }
    
    return addr;
}

int         drv_eth_heap_free(PTR ptr)
{
    PETH_HEAP_LIST pList;
    U32 index;

    if (((U32)ptr < pEthHeapCtx->storage_base) || ((U32)ptr >= pEthHeapCtx->storage_end)) {
        print_err("PTR (0x%x) is out of network heap range", (unsigned int)ptr);
        return FAILURE;
    }

    index = ((U32)ptr - pEthHeapCtx->storage_base)/NET_PKT_MAX_SIZE;
    pList = &pEthHeapCtx->list[index];
    
    if (!pList->status) {
        print_err("%s", "double free operation: blocked");
        return FAILURE;
    }

    //print_eth("[dbg] ---> net heap free: addr_%x", (U32)ptr);

    pList->next = NULL;
    pList->status = 0;
    
    pEthHeapCtx->next_free_item->next = (U32 *)pList;
    pEthHeapCtx->next_free_item = pList;
    pEthHeapCtx->stats_free++;
    pEthHeapCtx->stats_balance--;
    
    return SUCCESS;
}

int			drv_eth_mac_set(char * mac)
{
	print_eth("---> mac (%02X:%02X:%02X:%02X:%02X:%02X)", 
		mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);

	drv_net_mac_set(mac);
	return SUCCESS;
}

/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

/** return 0 in case of success or error */
static unsigned int drv_eth_rx_put(unsigned int addr, unsigned int size)
{
	unsigned int next_put = pEth->rx_pool_put + 1;

	if (next_put >= ETH_RX_POOL_SIZE)
		next_put = 0;

	if (next_put == pEth->rx_pool_get) {
		print_err("%s", "eth rx pool overflow");
		return 1;
	}

	print_eth("PUT[%d]: addr_0x%x size_%d", pEth->rx_pool_put, addr, size);
			
	pEth->rx_pool[pEth->rx_pool_put].addr = addr;
	pEth->rx_pool[pEth->rx_pool_put].size = size;	

	pEth->rx_pool_put = next_put;

	return 0;
}

static int drv_eth_heap_init(void)
{
    U32 i;
	U32 storage_base;

    print_eth("%s", "Network heap initialization");

	pEthHeapCtx = (PETH_HEAP_CTX)malloc(sizeof(ETH_HEAP_CTX));
	assert(pEthHeapCtx);
    memset(pEthHeapCtx, 0, sizeof(ETH_HEAP_CTX));
    print_eth(" - eth heap ctx base (0x%x) size (%d) bytes", (unsigned int)pEthHeapCtx, sizeof(ETH_HEAP_CTX));

	storage_base = (U32)malloc(NET_PKT_MAX_SIZE * (NET_PKT_COUNT + 1));

	//align to 0x800
	storage_base = storage_base + (NET_PKT_MAX_SIZE -1);	
	storage_base -= storage_base % NET_PKT_MAX_SIZE;
	
	pEthHeapCtx->storage_base = storage_base;
	pEthHeapCtx->storage_end = storage_base + NET_PKT_MAX_SIZE * NET_PKT_COUNT;	

    for (i=0; i<(NET_PKT_COUNT - 1); i++) {
        pEthHeapCtx->list[i].addr = (U32)(storage_base + NET_PKT_MAX_SIZE * i);
        pEthHeapCtx->list[i].next = (U32 *)&pEthHeapCtx->list[i+1];
        pEthHeapCtx->list[i].status = 0;
        //print_eth("   [%d] addr_%x next_%x", i, pEthHeapCtx->list[i].addr, (U32)pEthHeapCtx->list[i].next);
    }
    pEthHeapCtx->list[i].addr = (U32)(storage_base + NET_PKT_MAX_SIZE * i);
    pEthHeapCtx->list[i].next = NULL;
    pEthHeapCtx->list[i].status = 0;
    //print_eth("   [%d] addr_%x next_%x", i, pEthHeapCtx->list[i].addr, (U32)pEthHeapCtx->list[i].next);

    pEthHeapCtx->next_alloc_item = &pEthHeapCtx->list[0];
    pEthHeapCtx->next_free_item = &pEthHeapCtx->list[i];

    print_eth(" - next AI[%d]_%x FI[%d]_%x", 0, pEthHeapCtx->next_alloc_item, i, (U32)pEthHeapCtx->next_free_item);

    return SUCCESS;
}

