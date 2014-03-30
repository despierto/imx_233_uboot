/**
 * Data link layer header file
 *
 * Copyright (c) 2014 X-boot GITHUB team
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

 #ifndef __NET_DATALINK_H__
 #define __NET_DATALINK_H__

#include "types.h"
#include "platform.h"
#include "drv_eth.h"


/************************************************
 *              DEFINITIONS                                    *
 ************************************************/
typedef enum {
    DATALINK_TX_SUCCESS,    /* packet successfully sent */
    DATALINK_TX_ARP_SENT,   /* packet wasn't sent, ARP request sent */
    DATALINK_TX_ARP_WAIT,   /* packet wasn't sent, ARP request waiting */    
    DATALINK_TX_ERROR,    /* packet error */
} DATALINK_TX_STATE;

typedef struct _ETH_PKT_ {
    ETH_HDR        header;
    U8            *payload;    
}ETH_PKT, *PETH_PKT;

typedef struct _ARP_REQ_ {
    struct _ARP_REQ_    *prev;
    struct _ARP_REQ_    *next;
    U32                 addr;
    U32                 size;    
    U32                 reg_time;
    IPaddr_t            dst_ip;
    U32                 transmission_num;    
}ARP_REQ, *PARP_REQ;

typedef struct _DATALINK_CTX_ {
    //ARP_REQ_POOL    arp_reg_pool[ETH_RX_POOL_SIZE];        /* queue of incomming packets */
    //unsigned int    arp_reg_pool_get;
    //unsigned int    arp_reg_pool_put; 

    PSYS_POOL_CTX   arp_reg_pool;
    PARP_REQ        arp_list_head;
    PARP_REQ        arp_list_end;    
    
}DATALINK_CTX, *PDATALINK_CTX;


/************************************************
 *              FUNCTIONs                                *
 ************************************************/
extern U8   NetEtherNullAddr[];
extern U8   NetBcastAddr[];

int datalink_open(void);
int datalink_close(void);
DATALINK_TX_STATE datalink_tx_send(PETH_PKT pEthPkt, IPaddr_t dst_ip, U32 type, U32 size);
int datalink_task(void);


#endif /* __NET_DATALINK_H__ */

