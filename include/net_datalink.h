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
 *              DEFINITIONS         					       *
 ************************************************/
typedef struct _ETH_PKT_ {
    ETH_HDR		header;
    U8			*payload;    
}ETH_PKT, *PETH_PKT;

typedef struct _DATALINK_CTX_ {
	char       	curr_src_mac[ETHER_ADDR_LEN];	/* Destination node */
}DATALINK_CTX, *PDATALINK_CTX;


/************************************************
 *              FUNCTIONs								*
 ************************************************/
int datalink_open(void);
int datalink_close(void);



#endif /* __NET_DATALINK_H__ */

