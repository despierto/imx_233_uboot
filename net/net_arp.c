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
static unsigned int arp_table_valid_period_sec = ARP_TABLE_VALID_PERIOD_SEC;


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int arp_table_create(void)
{
	unsigned int i;

	pArpTable = (PARP_TABLE)malloc(ARP_TABLE_SIZE * sizeof(ARP_TABLE));
	assert(pArpTable);
	memset((void *)pArpTable, 0, ARP_TABLE_SIZE * sizeof(ARP_TABLE));
	
	print_net("ARP table creation: pArpTable_0x%x size_%d ", (U32)pArpTable, ARP_TABLE_SIZE);	
	
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		pArpTable[i].type = ARP_TABLE_TYPE_NONE;
	}

	return SUCCESS;
}

int arp_table_destroy(void)
{
	print_net("%s", "ARP table destroying");
	if (pArpTable)	
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
				drv_mac_to_string((uchar *)&mac, pArpTable[i].hw_addr));		
		}
	}
	print_inf("--------------------------------------------------\n");
	
	return;		
}

uchar *arp_table_get_mac(IPaddr_t ip)
{
	unsigned int reg_time = 0;
	uchar *mac = NULL;
	unsigned int i;

	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].ip_addr == ip){
			reg_time = pArpTable[i].reg_time;
			mac = pArpTable[i].hw_addr;
			break;
		}
	}

	if (mac != NULL) {
		unsigned int time_diff = get_time_diff(reg_time, get_time_s());
		if (time_diff > arp_table_valid_period_sec)
			mac = NULL;
	}

	return mac;
}

void arp_table_set_valid_period(unsigned int valid_period_sec)
{
	arp_table_valid_period_sec = valid_period_sec;
	return;
}

void arp_table_reg_ip(IPaddr_t ip, char *mac, ushort type)
{
	unsigned int i;

	//find the empty place
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].type == ARP_TABLE_TYPE_NONE){
			pArpTable[i].type = type;
			pArpTable[i].ip_addr = ip;
			pArpTable[i].reg_time = get_time_s();
			memcpy(pArpTable[i].hw_addr, mac, ETHER_ADDR_LEN);

			//new entry successfully registered into new place
			return;
		}
	}

	//all empty places are used. resync ARP table 
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (get_time_diff(pArpTable[i].reg_time, get_time_s()) > arp_table_valid_period_sec) {
			pArpTable[i].reg_time = 0;
		}
	}
	//and write new entry into first obsolete place
	for (i=0; i< ARP_TABLE_SIZE; i++) {
		if (pArpTable[i].reg_time == 0){
			pArpTable[i].type = type;
			pArpTable[i].ip_addr = ip;
			pArpTable[i].reg_time = get_time_s();
			memcpy(pArpTable[i].hw_addr, mac, ETHER_ADDR_LEN);

			//new entry successfully registered into obsolete place
			return;
		}
	}

	//ARP table still is full, analyse the oldest entry and replace it
	{
		unsigned int delta = 0; //seconds
		unsigned int target_index = 0;
		unsigned int curr_time = get_time_s();
		
		for (i=0; i< ARP_TABLE_SIZE; i++) {
			unsigned int time_diff = get_time_diff(pArpTable[i].reg_time, get_time_s());	
			if (time_diff > delta) {
				delta = time_diff;
				target_index = i;
			}
		}

		pArpTable[target_index].type = type;
		pArpTable[target_index].ip_addr = ip;
		pArpTable[target_index].reg_time = curr_time;
		memcpy(pArpTable[target_index].hw_addr, mac, ETHER_ADDR_LEN);
	}
	
	//new entry successfully registered insteade the oldest entry
	return;
}

