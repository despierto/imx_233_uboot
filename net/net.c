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
#include "net.h"
#include "net_icmp.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
static unsigned int net_state;
    
void net_ping_task(void *param);


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int net_init(void)
{
    int rc;
    U32 i;
    
    print_net("%s", "Network initialization");

    //set config
    drv_string_to_mac(CONFIG_HW_MAC_ADDR, &pGblCtx->cfg_mac_addr[0]);        // "ethaddr"
    pGblCtx->cfg_ip_addr       = drv_string_to_ip(CONFIG_IPADDR);          // "ipaddr"
    pGblCtx->cfg_ip_netmask    = drv_string_to_ip(CONFIG_NETMASK);         // "netmask"
    pGblCtx->cfg_ip_gateway    = drv_string_to_ip(CONFIG_GATEWAYIP);       // "gatewayip"
    pGblCtx->cfg_ip_server     = drv_string_to_ip(CONFIG_SERVERIP);        // "serverip"
    pGblCtx->cfg_ip_dns        = drv_string_to_ip(CONFIG_DNSIP);           // "dnsip"
    pGblCtx->cfg_ip_vlan       = drv_string_to_ip(CONFIG_VLANIP);          // "vlanip"
    //delay for printing out the print buffer
    sleep_ms(10);

    //reset network variables
    pGblCtx->ping_req_num = 0;
    pGblCtx->ping_ans_num = 0;    

    rc = datalink_open();

    copy_filename(pGblCtx->BootFile, CONFIG_BOOTFILE, CONFIG_BOOTFILE_SIZE);    
    pGblCtx->linux_load_addr = SYS_RAM_LOAD_ADDR;
        
    return rc;
}

int net_close(void)
{
    int rc;
    
    print_net("%s", "Network termination");
    rc = datalink_close();

    return rc;
}


void net_ping_req(IPaddr_t ip_addr)
{
    uchar ip_str[IP_ADDR_STR_LEN];

    if (pGblCtx->ping_req_num) {
        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);
        print_err_cmd("pinging %s is running right now", ip_str);
        return;
    }

    drv_ip_to_string(ip_addr, &ip_str[0]);
    pGblCtx->ping_ip = ip_addr;
    pGblCtx->ping_ans_num = 0;        

    //register task for processing ping request right now
    core_reg_task(net_ping_task, NULL, 0, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);

    return;
}

void net_rx_process(void)
{
#if 0    
    unsigned int size;
    unsigned int addr;    


    //receive all current packets
    drv_eth_rx();

    //get and process every packet
    while((size = drv_eth_rx_get(&addr)) != 0) {
        
        if (addr && size) {
#if 0            
            U8 *pA = (U8 *)addr;
            unsigned int i;            
            print_inf("[net] --- Rx Packet[0x%x, %d]: [ ", addr, size);
            for(i=0; i<size; i++) {
                print_inf("%x ", pA[i]);
            }
            print_inf("] --- \r\n");
#endif            
            drv_eth_heap_free((PTR)addr);
        }
    
    }
#endif
    //datalink_task();

    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

void net_ping_task(void *param)
{
    uchar ip_str[IP_ADDR_STR_LEN];

    if (pGblCtx->ping_req_num == 0) {
        unsigned int data_size = 56;
        unsigned int packet_size = data_size + IP_HDR_SIZE + ICMP_ECHO_HDR_SIZE;

        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);

        //FORMAT: "PING <DNS or incomming IP address> (<IP address>): <size>(<fsize>) bytes of data"
        print_inf("\r\nPING %s (%s) %d(%d) bytes of data.\r\n", ip_str, ip_str, data_size, packet_size);

        pGblCtx->ping_reg_time = get_time_ms();

        //send ping request
        icmp_send_req(pGblCtx->ping_ip);

        //schedule itself
        core_reg_task(net_ping_task, NULL, PING_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);
        //core_reg_task(net_ping_task, NULL, PING_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, PING_REPEAT_NUM);

    } else if (pGblCtx->ping_req_num < (PING_REPEAT_NUM + 1)) {
        print_inf("Request #%d timed out\r\n", pGblCtx->ping_req_num);

        pGblCtx->ping_req_num++;

        //schedule itself again
        core_reg_task(net_ping_task, NULL, PING_TIMEOUT, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);
    
    } else {
        print_inf("Request #%d timed out\r\n", pGblCtx->ping_req_num);

        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);
        
        print_inf("\r\n--- %s ping statistics ---\r\n", ip_str);
        print_inf("%d packets transmitted, %d received, %d%% packet loss, time %dms\r\n", 
            pGblCtx->ping_req_num, pGblCtx->ping_ans_num, (100 - 100*pGblCtx->ping_ans_num/pGblCtx->ping_req_num),
            get_time_diff(pGblCtx->ping_reg_time, get_time_ms()));        

        pGblCtx->ping_req_num = 0;
    }
   
    return;
}


