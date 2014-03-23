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



/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int datalink_open(void)
{
	int rc = SUCCESS;

	/* Configure Ethernt device*/
    drv_eth_init();

	/* Create ARM table */
	arp_table_create();


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

int datalink_tx_send(PETH_PKT pEthPkt, IPaddr_t dst_ip)
{
	int rc = SUCCESS;

	if (pEthPkt == NULL) {
		print_err("%s", "packet to send it null");
		return FAILURE;
	}

/*
	//set src ip
	memcpy ((void *)pEthPkt->header.src, (void *)addr, 6);
    memcpy ((void *)et->et_src, (void *)pGblCtx->cfg_mac_addr, 6);

	mac_reg_time = arp_table_check_ip(ip_addr, &dst_mac);

	print_net(" -- ARP MAC status: mac_reg_time_%d mac: %s", mac_reg_time, drv_mac_to_string((uchar *)&mac_out, (uchar *)dst_mac));

	arp_table_reg_ip(pGblCtx->cfg_ip_dns, (char *)pGblCtx->cfg_mac_addr, ARP_TABLE_TYPE_ETH, 2234);

	

*/

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












