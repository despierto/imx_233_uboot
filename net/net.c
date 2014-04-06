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
    sleep_ms(100);

    rc = datalink_open();
    
    //setup packet buffers, aligned correctly
    for (i = 0; i < ETH_PKTBUFSTX; i++) {
        pGblCtx->NetTxPackets[i] = (uchar *)drv_eth_heap_alloc();
        assert(pGblCtx->NetTxPackets[i]);
        //print_eth(" - Net TX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pGblCtx->NetTxPackets[i], ETH_PKTBUFSTX);        
    }
    for (i = 0; i < ETH_PKTBUFSRX; i++) {
        pGblCtx->NetRxPackets[i] = (uchar *)drv_eth_heap_alloc();
        assert(pGblCtx->NetRxPackets[i]);
        //print_eth(" - Net RX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pGblCtx->NetRxPackets[i], i+1);                
    }

    pGblCtx->NetArpWaitTxPacket = (uchar *)drv_eth_heap_alloc();
    assert(pGblCtx->NetArpWaitTxPacket);
    pGblCtx->NetArpWaitTxPacketSize = 0;
    //print_eth(" - Net Arp Tx packet: base (0x%x) count (%d)", (unsigned int)pGblCtx->NetArpWaitTxPacket, 1);
    
    pGblCtx->Status = 0;
    copy_filename(pGblCtx->BootFile, CONFIG_BOOTFILE, CONFIG_BOOTFILE_SIZE);    
    pGblCtx->linux_load_addr = SYS_RAM_LOAD_ADDR;
    
    pGblCtx->NetArpWaitPacketMAC = NULL;
    pGblCtx->NetArpWaitPacketIP = 0;
    pGblCtx->NetArpWaitReplyIP = 0;
    pGblCtx->NetArpWaitTimerStart = 0;
    pGblCtx->NetArpWaitTry = 0;
    
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
    unsigned int data_size = 56;
    unsigned int packet_size = data_size + IP_HDR_SIZE + ICMP_ECHO_HDR_SIZE;
    //unsigned int i;
    //unsigned int mac_reg_time;
    //char *dst_mac;
    char mac_out[64];
    
    drv_ip_to_string(ip_addr, &ip_str[0]);

    //FORMAT: "PING <DNS or incomming IP address> (<IP address>): <size>(<fsize>) bytes of data"
    print_inf("PING %s (%s) %d(%d) bytes of data.\r\n", ip_str, ip_str, data_size, packet_size);

    icmp_send_req(ip_addr);

#if 0
  

    
    net_state = NETSTATE_CONTINUE;

    print_net("%s", "Entry in loop");
 
    while (net_state == NETSTATE_CONTINUE)
    {
        drv_eth_rx();
        //local_net_arp_timeout_check();
        
        //sleep_ms(50);
    }

    print_net("Exit: state (%d)", net_state);
#endif    
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
    datalink_task();

    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

