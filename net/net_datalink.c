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

	arp_table_set_valid_period(ARP_TABLE_VALID_PERIOD_SEC);

	//init dala link ctx
	pDataLinkCtx = (PDATALINK_CTX)malloc(sizeof(DATALINK_CTX));
	assert(pDataLinkCtx);
	memset((void *)pDataLinkCtx, 0, sizeof(DATALINK_CTX));

	//setup mac address
	memcpy((void *)pDataLinkCtx->curr_src_mac, (void *)pGblCtx->cfg_mac_addr, ETHER_ADDR_LEN);
	drv_eth_mac_set(pDataLinkCtx->curr_src_mac);

	return rc;
}

int datalink_close(void)
{
	int rc = SUCCESS;

	/* Create ARM table */
	arp_table_destroy();

	/* Stop Ethernt device*/	
    drv_eth_halt();


	return rc;
}

PETH_PKT datalink_tx_alloc(void)
{
	return (PETH_PKT)drv_eth_heap_alloc();
}

int datalink_tx_send(PETH_PKT pEthPkt, IPaddr_t dst_ip, ushort type)
{
	int rc = SUCCESS;
	uchar *dst_mac;

	if (pEthPkt == NULL) {
		print_err("%s", "packet to send it null");
		return FAILURE;
	}

	//set src mac
	memcpy ((void *)pEthPkt->header.src, (void *)pDataLinkCtx->curr_src_mac, ETHER_ADDR_LEN);

	//check presence of ip address at ARP table
	dst_mac = arp_table_get_mac(dst_ip);

	if (dst_mac == NULL) {
		//no mac or mac is obsolete: needed send request and update mac
		
		//mac_reg_time = arp_table_get_mac(ip_addr, &dst_mac);
		//arp_table_reg_ip(pGblCtx->cfg_ip_dns, (char *)pGblCtx->cfg_mac_addr, ARP_TABLE_TYPE_ETH, 2234);

		if (dst_mac == NULL)
			return FAILURE;
	} 
	
	//set dst mac
    memcpy ((void *)pEthPkt->header.dst, (void *)dst_mac, ETHER_ADDR_LEN);

	pEthPkt->header.type = type;

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


	return rc;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/












