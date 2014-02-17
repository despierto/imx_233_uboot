/**
 * Top layer ethernel driver file
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
#include "net_ks8851.h"
#include "drv_eth.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

//#define SYS_RAM_ETH_ADDR          /* 0x42000000 */
//#define SYS_RAM_ETH_SIZE          /* 0x10000      */
PETH_CTX        pEth = (PETH_CTX)SYS_RAM_ETH_ADDR;



/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int drv_eth_init(void)
{
    int ret = 0;
    unsigned int i;

    drv_eth_halt();
    
    //net driver initialization
    print_eth("%s", "Net environment initialization");
    if (sizeof(ETH_CTX) > (unsigned int)SYS_RAM_ETH_SIZE)
    {
        print_err("Size of ETH CTX (%d) bytes is out of ranges (%d) bytes", sizeof(ETH_CTX), SYS_RAM_ETH_SIZE);        
        return FAILURE;
    }
    memset(pEth, 0, sizeof(ETH_CTX));
    print_eth(" - Eth CTX base (0x%x) size (%d) bytes", (unsigned int)pEth, sizeof(ETH_CTX));
    
    //setup packet buffers, aligned correctly
    for (i = 0; i < ETH_PKTBUFSTX; i++) {
        pEth->NetTxPackets[i] = (volatile uchar *)((unsigned int)&pEth->TxPktBuf[0] + i*ETH_PKTSIZE_ALIGN);
        print_eth(" - Net TX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetTxPackets[i], ETH_PKTBUFSTX);        
    }
    for (i = 0; i < ETH_PKTBUFSRX; i++) {
        pEth->NetRxPackets[i] = (volatile uchar *)((unsigned int)&pEth->RxPktBuf[0] + i*ETH_PKTSIZE_ALIGN);
        print_eth(" - Net RX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetRxPackets[i], ETH_PKTBUFSRX);                
    }

    pEth->NetArpWaitTxPacket = (volatile uchar *)&pEth->NetArpWaitPacketBuf[0];
    pEth->NetArpWaitTxPacketSize = 0;
    print_eth(" - Net Arp Tx packet: base (0x%x) count (%d)", (unsigned int)pEth->NetArpWaitTxPacket, 1);
    
    pEth->Status = 0;
    copy_filename(pEth->BootFile, CONFIG_BOOTFILE, CONFIG_BOOTFILE_SIZE);    
    pEth->linux_load_addr = SYS_RAM_LOAD_ADDR;
    
    pEth->NetArpWaitPacketMAC = NULL;
    pEth->NetArpWaitPacketIP = 0;
    pEth->NetArpWaitReplyIP = 0;
    pEth->NetArpWaitTimerStart = 0;
    pEth->NetArpWaitTry = 0;

    ret = net_init(NULL);

    if (ret) {
        drv_eth_halt(); 
        pEth->Status = 0;
        print_err("%s", "ethernet initialization wasn't completed");
    } else {
        print_eth("%s", "Ethernel was successfully started");
        pEth->Status = 1;
    }

    //delay for printing out the print buffer
    sleep_ms(100);

    drv_eth_parse_enetaddr(CONFIG_HW_MAC_ADDR, &pEth->cfg_mac_addr[0]); // "ethaddr"
    pEth->cfg_ip_addr       = drv_string_to_ip(CONFIG_IPADDR);          // "ipaddr"
    pEth->cfg_ip_netmask    = drv_string_to_ip(CONFIG_NETMASK);         // "netmask"
    pEth->cfg_ip_gateway    = drv_string_to_ip(CONFIG_GATEWAYIP);       // "gatewayip"
    pEth->cfg_ip_server     = drv_string_to_ip(CONFIG_SERVERIP);        // "serverip"
    pEth->cfg_ip_dns        = drv_string_to_ip(CONFIG_DNSIP);           // "dnsip"
    pEth->cfg_ip_vlan       = drv_string_to_ip(CONFIG_VLANIP);          // "vlanip"


        
    return ret;
}

void drv_eth_halt(void)
{
    print_eth("%s", "Halt Ethernet driver");
    net_halt();
    sleep_ms(100);
}

int drv_eth_rx(void)
{
    print_eth("--> %s -> %s : %d", __FILE__, __FUNCTION__, __LINE__);
    
    net_rx();
    return 0;
}

int drv_eth_tx(volatile void *packet, int length)
{

#ifdef GBL_ETH_DIAG_ENA
    print_inf("[dbg] TX (0x%08x|%d):[ ", (unsigned int)packet, length);
    {
        unsigned char *A = (unsigned char *)packet;
        unsigned int i;
        for (i=0; i<length; i++)
        {
            print_inf("%x ", A[i]);
        }
        print_inf("]\r\n");
    }
#endif

    net_tx(packet, length);
    return 0;
}

void drv_eth_parse_enetaddr(const char *addr, uchar *enetaddr)
{
    char *end;
    int i;
   
    for (i = 0; i < 6; ++i) {
        enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
        if (addr)
            addr = (*end) ? end + 1 : end;
    }
}

IPaddr_t drv_string_to_ip(char *s)
{
    IPaddr_t addr;
    char *e;
    int i;

    if (s == NULL)
        return(0);

    for (addr=0, i=0; i<4; ++i) {
        ulong val = s ? simple_strtoul(s, &e, 10) : 0;
        
        addr <<= 8;
        addr |= (val & 0xFF);
        if (s) {
            s = (*e) ? e+1 : e;
        }
    }

    return (htonl(addr));
}

char *drv_ip_to_string(IPaddr_t ip, uchar *buf)
{
    sprintf((char *)buf, "%03d.%03d.%03d.%03d", (ip & 0xFF), ((ip >> 8) & 0xFF), ((ip >> 16) & 0xFF), ((ip >> 24) & 0xFF));
    return (char *)buf;
}

void drv_eth_info(void)
{
    uchar s[15];
    
    print_eth("%s", "Ethernet configuration:");
    print_eth(" - ipaddr:       %s", drv_ip_to_string(pEth->cfg_ip_addr, &s[0]));
    print_eth(" - netmask:      %s", drv_ip_to_string(pEth->cfg_ip_netmask, &s[0]));
    print_eth(" - gatewayip:    %s", drv_ip_to_string(pEth->cfg_ip_gateway, &s[0]));
    print_eth(" - serverip:     %s", drv_ip_to_string(pEth->cfg_ip_server, &s[0]));
    print_eth(" - dnsip:        %s", drv_ip_to_string(pEth->cfg_ip_dns, &s[0]));
    print_eth(" - vlanip:       %s", drv_ip_to_string(pEth->cfg_ip_vlan, &s[0]));

    return;
}


/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/


