/**
 * Top network layer: ARP table support
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
#include "net_arp.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
static PARP_TABLE     pArpTable = NULL;
static unsigned int arp_table_valid_period_msec = ARP_VALID_PERIOD;

static INLINE void local_arp_table_fill_item(unsigned int index, IPaddr_t ip, char *mac, uchar type, uchar state);
static char *local_arp_table_get_state_str(uchar state);

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
        pArpTable[i].state = ARP_TABLE_STATE_INVALID;        
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

    print_inf("----------------------------------------------------------------------\n");    /* 70 symbols */
    print_inf("ARP info\n");
    print_inf("----------------------------------------------------------------------\n");
    print_inf("Address          HWtype  RegTime     HWaddress           State\n");            
    for (i=0; i< ARP_TABLE_SIZE; i++) {
        //if (pArpTable[i].state != ARP_TABLE_STATE_INVALID){
        if (pArpTable[i].ip_addr){

            print_inf("%s  %s   %010d  %s   %s\n", 
                drv_ip_to_string(pArpTable[i].ip_addr, &s[0]),
                ((pArpTable[i].type == ARP_TABLE_TYPE_ETH)?"ether":"virt "),
                pArpTable[i].reg_time,
                drv_mac_to_string((uchar *)&mac, pArpTable[i].hw_addr), local_arp_table_get_state_str(pArpTable[i].state));        
        }
    }
    print_inf("----------------------------------------------------------------------\n");
    
    return;        
}

ARP_TABLE_STATE arp_table_get_mac(IPaddr_t ip, uchar **mac)
{
    ARP_TABLE_STATE ret_state = ARP_TABLE_STATE_INVALID;
    
    unsigned int i;
    
    for (i=0; i< ARP_TABLE_SIZE; i++) {
        if ((pArpTable[i].state >= ARP_TABLE_STATE_VALID) && (pArpTable[i].ip_addr == ip)){
            unsigned int time_diff = get_time_diff(pArpTable[i].reg_time, get_time_ms());
            
            if (pArpTable[i].state == ARP_TABLE_STATE_VALID) {
                if (time_diff <= arp_table_valid_period_msec) {
                    *mac = (uchar *)pArpTable[i].hw_addr;        
                    ret_state = ARP_TABLE_STATE_VALID;
                } else {
                    pArpTable[i].state = ARP_TABLE_STATE_INVALID;
                }
            } else {
                if (time_diff <= ARP_TIMEOUT) {
                    ret_state = ARP_TABLE_STATE_WAIT_ARP_RESPOND;
                } else {
                    pArpTable[i].state = ARP_TABLE_STATE_INVALID;
                } 
            }
                   
            break;
        }
    }

    return ret_state;
}

void arp_table_set_valid_period(unsigned int valid_period_msec)
{
    arp_table_valid_period_msec = valid_period_msec;
    return;
}

int arp_table_update_valid_period(IPaddr_t ip)
{
    unsigned int i;

    for (i=0; i< ARP_TABLE_SIZE; i++) {
        if ((pArpTable[i].ip_addr == ip) && (pArpTable[i].state >= ARP_TABLE_STATE_VALID)){
            pArpTable[i].reg_time = get_time_ms();
            pArpTable[i].state = ARP_TABLE_STATE_VALID;            

            //ip address info updated
            return SUCCESS;
        }
    }
    return FAILURE;
}

void arp_table_reg_ip(IPaddr_t ip, char *mac, uchar type, uchar state)
{
    unsigned int i, j = ARP_TABLE_SIZE;

    //find the same ip
    for (i=0; i< ARP_TABLE_SIZE; i++) {
        //print_dbg("-->1 pArpTable[%d].ip_addr_%x vs ip_%x", i, pArpTable[i].ip_addr, ip);
        if (pArpTable[i].ip_addr == ip){
            //print_dbg("-->1 Call[%d] local_arp_table_fill_item", i);
            local_arp_table_fill_item(i, ip, mac, type, state);

             print_dbg("ARPT: ip updated: ip_0x%x, t_%d s_%d T_%d", (unsigned int)ip, type, state, pArpTable[i].reg_time);
            //ip address info updated
            return;
        }
    }

    //resync ARP table 
    for (i=0; i < ARP_TABLE_SIZE; i++) {
        //print_dbg("-->2 pArpTable[%d].state_%d >= %d", i, pArpTable[i].state, ARP_TABLE_STATE_VALID);
        if (pArpTable[i].state >= ARP_TABLE_STATE_VALID) {
            unsigned int time_diff = get_time_diff(pArpTable[i].reg_time, get_time_ms());

            //print_dbg("-->2 diff[%d]_%d", i, time_diff);
            
            if (pArpTable[i].state == ARP_TABLE_STATE_VALID) {
                if (time_diff > arp_table_valid_period_msec) {
                    pArpTable[i].state = ARP_TABLE_STATE_INVALID;
                    j = i;
                    break;
                }
            } else {
                if (time_diff > ARP_TIMEOUT) {
                    pArpTable[i].state = ARP_TABLE_STATE_INVALID;
                    j = i;
                    break;
                } 
            }
        } else {
            j = i;
            break;
        }
    }

    //print_dbg("-->3 j_%d", j);
        
    //no ip duplication & table resync => j is valid first index in case table has free paces
    if (j < ARP_TABLE_SIZE) {
        //print_dbg("-->3 Call[%d] local_arp_table_fill_item", j);
        local_arp_table_fill_item(j, ip, mac, type, state);

        print_dbg("ARPT: reg after resync: ip_0x%x, t_%d s_%d T_%d", (unsigned int)ip, type, state, pArpTable[j].reg_time);
        //new entry successfully registered into new place
        return;
    }

    //ARP table still is full, analyse the oldest entry and replace it
    {
        unsigned int delta = 0; //seconds
        unsigned int target_index = 0;
        unsigned int curr_time = get_time_ms();
        
        for (i=0; i< ARP_TABLE_SIZE; i++) {
            unsigned int time_diff = get_time_diff(pArpTable[i].reg_time, curr_time);    
            //print_dbg("-->4 diff[%d]_%d target_index_%d", i, time_diff, target_index);
            if (time_diff > delta) {
                delta = time_diff;
                target_index = i;
            }
        }

        //print_dbg("-->4 Call[%d] local_arp_table_fill_item", target_index);
        local_arp_table_fill_item(target_index, ip, mac, type, state);
        print_dbg("ARPT: reg inst old: ip_0x%x, t_%d s_%d T_%d", (unsigned int)ip, type, state, pArpTable[target_index].reg_time);
    }
    //new entry successfully registered insteade the oldest entry
    return;
}


/************************************************
 *              LOCAL  FUNCTIONS                                      *
 ************************************************/
static INLINE void local_arp_table_fill_item(unsigned int index, IPaddr_t ip, char *mac, uchar type, uchar state)
{
    pArpTable[index].type = type;
    pArpTable[index].ip_addr = ip;
    pArpTable[index].state = state;
    pArpTable[index].reg_time = get_time_ms();
    if (mac != NULL) {
        memcpy(pArpTable[index].hw_addr, mac, ETHER_ADDR_LEN);
    }
    return;
}

static char *local_arp_table_get_state_str(uchar state)
{
    switch(state)
    {
        case ARP_TABLE_STATE_INVALID:
            return "IP is invalid";
        case ARP_TABLE_STATE_VALID:
            return "IP is valid";
        case ARP_TABLE_STATE_WAIT_ARP_RESPOND:
            return "ARP waiting";
        default:
            break;            
    }
    return "unknown";
}

    