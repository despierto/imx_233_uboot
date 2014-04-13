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
    
void            net_ping_task(void *param);
void            net_rx_process(void *param);
static void     net_ping_task_print_replay(void);

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

    core_reg_task(net_rx_process, NULL, 50, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__NET_RX, 0xFF);

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

    //register task for processing ping request right now
    core_reg_task(net_ping_task, NULL, 0, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);

    return;
}

void net_rx_process(void *param)
{
    //unsigned int size;
    //unsigned int addr;    

    //print_net("%s", "rx_process");

    datalink_rx();

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
        unsigned int timeout = PING_TIMEOUT;

        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);

        //3 ------------------------------------------------------------------------------------
        //3 FORMAT: "PING <DNS or incomming IP address> (<IP address>): <size>(<fsize>) bytes of data"
        //3 ------------------------------------------------------------------------------------
        print_inf("\r\nPING %s (%s) %d(%d) bytes of data.\r\n", ip_str, ip_str, data_size, packet_size);

        pGblCtx->ping_reg_time = get_time_ms();
        pGblCtx->ping_ans_time = 0;
        pGblCtx->ping_rtt_sum = 0;   
        pGblCtx->ping_ans_num = 0;

        //send ping request
        if (icmp_send_req(pGblCtx->ping_ip)) {
            //increasing timeout due to sending problems: possible ARP is waiting
            timeout = ARP_TIMEOUT;
        }
                
        //schedule itself. core will drop task automatically
        core_reg_task(net_ping_task, NULL, timeout, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);

    } else if (pGblCtx->ping_req_num < (PING_REPEAT_NUM + 1)) {
        unsigned int timeout = PING_TIMEOUT;

        net_ping_task_print_replay();

        //send ping request
        pGblCtx->ping_reg_time = get_time_ms();        
        if (icmp_send_req(pGblCtx->ping_ip)) {
            //increasing timeout due to sending problems: possible ARP is waiting
            timeout = ARP_TIMEOUT;
        }

        //schedule itself. core will drop task automatically
        core_reg_task(net_ping_task, NULL, timeout, CORE_TASK_TYPE_COMMON, CORE_TASK_PRIO__PING, 0);
    
    } else {
        int avg_rtt;
        
        net_ping_task_print_replay();
        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);

        avg_rtt = (pGblCtx->ping_rtt_sum) ? (pGblCtx->ping_rtt_sum/pGblCtx->ping_ans_num) : -1;
        
        print_inf("\r\n--- %s ping statistics ---\r\n", ip_str);
        print_inf("%d packets transmitted, %d received, %d%% packet loss, arg round trip time %dms\r\n", 
            pGblCtx->ping_req_num, pGblCtx->ping_ans_num, (100 - 100*pGblCtx->ping_ans_num/pGblCtx->ping_req_num),
            avg_rtt);        

        pGblCtx->ping_req_num = 0;
    }
   
    return;
}

static void    net_ping_task_print_replay(void)
{
    uchar ip_str[IP_ADDR_STR_LEN];

    //print_dbg("ECHO REPLAY: num_%d ttl_%d bytes_%d", pGblCtx->ping_ans_num, pGblCtx->ping_ans_ttl, pGblCtx->ping_ans_bytes);
    
    if (pGblCtx->ping_ans_time) {
        uint time = get_time_diff(pGblCtx->ping_reg_time, pGblCtx->ping_ans_time);
        //we had peng replay
        drv_ip_to_string(pGblCtx->ping_ip, &ip_str[0]);

        print_inf("Replay from %s ", ip_str);
        
        if ( pGblCtx->ping_ans_bytes == 0xFFFFFFFF) {
            print_inf("with broken payload ");
        } else {
            print_inf("bytes=%d ", pGblCtx->ping_ans_bytes);
        } 
        
        if (time) {
            print_inf("time=%dms ", time);
        } else {
            print_inf("%s", "time<1ms ");
        }

        print_inf("TTL=%d\r\n", pGblCtx->ping_ans_ttl);

        pGblCtx->ping_rtt_sum += time;
        //printf("-----> ping_rtt_sum_%d time_%d\n", pGblCtx->ping_rtt_sum, time);
    
        pGblCtx->ping_ans_time = 0;

    } else {
        print_inf("Request #%d timed out\r\n", pGblCtx->ping_req_num);
    }
    return;
}

