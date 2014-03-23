/**
 * Top network layer: ARP table support
 *
 * Copyright (c) 2014 Alex Winter
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
#include "net_arp.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
static PARP_TABLE 	pArpTable = NULL;


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int arp_table_create(void)
{
	unsigned int i;

	print_net("ARP table creation");	
	pArpTable = malloc(ARP_TABLE_SIZE * sizeof(ARP_TABLE));

	assert(pArpTable);
	
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		pArpTable[i].type = ARP_TABLE_TYPE_NONE;
	}

	return SUCCESS;
}

int arp_table_destroy(void)
{
	print_net("ARP table destroying");
	free(pArpTable);

	return SUCCESS;
}


void arp_table_info(void)
{
	unsigned int i;
	uchar s[15];
	char mac[64];

	print_inf("--------------------------------------------------\n");	/* 50 symbols */
	print_inf("ARP info\n");
	print_inf("--------------------------------------------------\n");
	print_inf("Address          HWtype  ST      HWaddress\n");			
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].type != ARP_TABLE_TYPE_NONE){

			print_inf("%s  %s  %6d     %s\n", 
				drv_ip_to_string(pArpTable[i].ip_addr, &s[0]),
				((pArpTable[i].type == ARP_TABLE_TYPE_ETH)?"ether":"virt "),
				pArpTable[i].reg_time,
				drv_mac_to_string(&mac, pArpTable[i].hw_addr));		
		}
	}
	print_inf("--------------------------------------------------\n");
	
	return;		
}

uchar arp_table_check_ip(IPaddr_t ip, char **mac)
{
	uchar reg_time = 0;
	unsigned int i;

	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].ip_addr == ip){
			reg_time = pArpTable[i].reg_time;
			*mac = pArpTable[i].hw_addr;
			break;
		}
	}

	return reg_time;
}

void arp_table_reg_ip(IPaddr_t ip, char *mac, ushort type, unsigned int curr_reg_time)
{
	unsigned int i;

	//find the empty place
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].type == ARP_TABLE_TYPE_NONE){
			pArpTable[i].type = type;
			pArpTable[i].ip_addr = ip;
			pArpTable[i].reg_time = curr_reg_time;
			memcpy(pArpTable[i].hw_addr, mac, ETHER_ADDR_LEN);

			//new entry successfully registered into new place
			return;
		}
	}

	//all empty places are used. resync ARP table 
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if ((curr_reg_time - pArpTable[i].reg_time) >= ARP_TABLE_VALID_PERIOD_SEC) {
			pArpTable[i].reg_time = 0;
		}
	}
	//and write new entry into first obsolete place
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].reg_time == 0){
			pArpTable[i].type = type;
			pArpTable[i].ip_addr = ip;
			pArpTable[i].reg_time = curr_reg_time;
			memcpy(pArpTable[i].hw_addr, mac, ETHER_ADDR_LEN);

			//new entry successfully registered into obsolete place
			return;
		}
	}

	//ARP table still is full, analyse the oldest entry and replace it
	{
		unsigned int delta = 0; //seconds
		unsigned int target_index = 0;
		
		for (i=0; i< ARP_TABLE_SIZE; i++) {
			unsigned int curr_delta = curr_reg_time - pArpTable[i].reg_time;
			if (curr_delta > delta) {
				delta = curr_delta;
				target_index = i;
			}
		}

		pArpTable[target_index].type = type;
		pArpTable[target_index].ip_addr = ip;
		pArpTable[target_index].reg_time = curr_reg_time;
		memcpy(pArpTable[target_index].hw_addr, mac, ETHER_ADDR_LEN);
	}
	
	//new entry successfully registered insteade the oldest entry
	return;
}

