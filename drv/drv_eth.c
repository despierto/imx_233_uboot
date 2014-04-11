/**
 * Top layer ethernel driver file
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
#include "drv_ks8851.h"
#include "drv_eth.h"

PETH_CTX        pEth = NULL;


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

/**
* Definition of usage ks8851 as physical layer
*/
#define drv_net_init(ptr)           ks8851_init(ptr)
#define drv_net_halt()              ks8851_halt()
#define drv_net_rxfc_get()          ks8851_rxfc_get()
#define drv_net_rx(rx_buff, fc)     ks8851_rx(rx_buff, fc) 
#define drv_net_tx(packet, length)  ks8851_tx(packet, length)
#define drv_net_mac_set(ethaddr)    ks8851_mac_set(ethaddr)


static int             drv_eth_heap_init(void);
static unsigned int drv_eth_rx_put(unsigned int addr, unsigned int size);

/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int drv_eth_init(void)
{
    int ret = 0;
    unsigned int i;
    PTR addr;

    drv_eth_halt();
    
    //eth ctx initialization
    print_eth("%s", "Net environment initialization");
    pEth = (PETH_CTX)malloc(sizeof(ETH_CTX));
    assert(pEth);
    memset(pEth, 0, sizeof(ETH_CTX));
    print_eth(" - Eth CTX base (0x%x) size (%d) bytes", (unsigned int)pEth, sizeof(ETH_CTX));

    pEth->pheap_ctx = sys_pool_init(NET_PKT_COUNT, NET_PKT_MAX_SIZE, (U8 *)&("network heap"));
    assert(pEth->pheap_ctx);
    assert_rc(sys_pool_test(pEth->pheap_ctx));
        
    ret = drv_net_init(NULL);
    
    if (ret) {
        drv_eth_halt(); 
        print_err("%s", "ethernet initialization wasn't completed");
    } else {
        print_eth("%s", "Ethernel was successfully started");
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

        //print_eth("GET[%d]: addr_0x%x size_%d", get, *addr, size);
    
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

void drv_eth_rx(void *param)
{
    U8 rxfc;
    U32 rx_len, i;
    PTR pRxPacket;

    rxfc = drv_net_rxfc_get();
    for (i=0; rxfc != 0; rxfc--) {

        pRxPacket = drv_eth_heap_alloc();
        assert(pRxPacket);
        
        rx_len = drv_net_rx(pRxPacket, ++i);
        if (rx_len < NET_HW_RX_HEADER_SIZE) {
            print_eth("WARNING: received packed with unexpected size (%d)", rx_len);
            continue;
        }

        if (rx_len) {
            unsigned int real_packet = (unsigned int)pRxPacket + NET_HW_RX_HEADER_SIZE;

            //print_eth("RX PUT: addr_0x%x size_%d pkt_%x", real_packet, rx_len, (unsigned int)pRxPacket);
            
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
       
    return;
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
    sys_pool_info(pEth->pheap_ctx);
    return;
}

PTR drv_eth_heap_alloc(void)
{
    return sys_pool_alloc(pEth->pheap_ctx);
}

int drv_eth_heap_free(PTR ptr)
{
    return sys_pool_free(pEth->pheap_ctx, ptr);
}

int drv_eth_mac_set(uchar * mac)
{
    drv_net_mac_set((char *)mac);
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

    //print_eth("PUT[%d]: addr_0x%x size_%d", pEth->rx_pool_put, addr, size);
            
    pEth->rx_pool[pEth->rx_pool_put].addr = addr;
    pEth->rx_pool[pEth->rx_pool_put].size = size;    

    pEth->rx_pool_put = next_put;

    return 0;
}

