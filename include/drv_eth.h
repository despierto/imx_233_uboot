/**
 * Top layer ethernel driver header file
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

#ifndef __DRV_ETH_H__
#define __DRV_ETH_H__

#include "types.h"
#include "platform.h"
#include "sys_pool.h"
#include "netdefs.h"

#define    GBL_ETH_DIAG_ENA


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

#define ETH_RX_POOL_SIZE        256

typedef struct _ETH_POOL_ {
    U32         addr;
    U32         size;    
}ETH_POOL, *PETH_POOL;

//declarations
typedef struct _ETH_CTX_ {
    ETH_POOL        rx_pool[ETH_RX_POOL_SIZE];        /* queue of incomming packets */
    unsigned int    rx_pool_get;
    unsigned int    rx_pool_put;   
    PSYS_POOL_CTX   pheap_ctx;
}ETH_CTX, *PETH_CTX;



/************************************************
 *              FUNCTIONS                                                  *
 ************************************************/

static inline int drv_eth_mac_is_zero(const uchar *addr)
{
    return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

static inline int drv_eth_mac_is_multicast(const uchar *addr)
{
    return (0x01 & addr[0]);
}

static inline int drv_eth_mac_is_valid(const uchar * addr)
{
    /* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to explicitly check for it here. */
    return !drv_eth_mac_is_multicast(addr) && !drv_eth_mac_is_zero(addr);
}

int         drv_eth_init(void);
void        drv_eth_halt(void);
int         drv_eth_rx(void);
unsigned int drv_eth_rx_get(unsigned int *addr);
int         drv_eth_tx(void *packet, int length);
void        drv_eth_info(void);
PTR         drv_eth_heap_alloc(void);
int         drv_eth_heap_free(PTR ptr);
int         drv_eth_mac_set(uchar * mac);

#endif /* __DRV_ETH_H__ */

