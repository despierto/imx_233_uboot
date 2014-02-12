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

//definitions
#define PKTSIZE                 1518
#define PKTSIZE_ALIGN           1536
#define PKTALIGN                32
#define PKTBUFSRX               4                               /* Rx MAX supported by ks8851 is 12KB */
#define PKTBUFSTX               1                               /* Tx MAX supported by ks8851 is 6KB */

//declarations
typedef struct _ETH_CTX_ {
    //buffers are aligned to 32 bytes
    volatile uchar  TxPktBuf[PKTBUFSTX * PKTSIZE_ALIGN];       
    volatile uchar  RxPktBuf[PKTBUFSRX * PKTSIZE_ALIGN];       
    volatile uchar  NetArpWaitPacketBuf[PKTSIZE_ALIGN];
    volatile uchar *NetTxPackets[PKTBUFSTX];                        /* Transmit packets */
    volatile uchar *NetRxPackets[PKTBUFSRX];                        /* Receive packets */

    volatile uchar *NetArpWaitTxPacket;                             /* THE transmit packet */
    unsigned int    NetArpWaitTxPacketSize;

    unsigned int    Status;                                         /* disabled */
    char            BootFile[CONFIG_BOOTFILE_SIZE];
    unsigned int    linux_load_addr;

    uchar           *NetArpWaitPacketMAC;                            /* MAC address of waiting packet's destination */
    IPaddr_t        NetArpWaitPacketIP;
    IPaddr_t        NetArpWaitReplyIP;

    unsigned int    NetArpWaitTimerStart;
    unsigned int    NetArpWaitTry;

}ETH_CTX, *PETH_CTX;

//#define SYS_RAM_ETH_ADDR          /* 0x42000000 */
//#define SYS_RAM_ETH_SIZE          /* 0x10000      */
PETH_CTX        pEth = (PETH_CTX)SYS_RAM_ETH_ADDR;

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
    for (i = 0; i < PKTBUFSTX; i++) {
        pEth->NetTxPackets[i] = (volatile uchar *)((unsigned int)&pEth->TxPktBuf[0] + i*PKTSIZE_ALIGN);
        print_eth(" - Net TX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetTxPackets[i], PKTBUFSTX);        
    }
    for (i = 0; i < PKTBUFSRX; i++) {
        pEth->NetRxPackets[i] = (volatile uchar *)((unsigned int)&pEth->RxPktBuf[0] + i*PKTSIZE_ALIGN);
        print_eth(" - Net RX packet[%d]: addr (0x%x) count (%d)", i, (unsigned int)pEth->NetRxPackets[i], PKTBUFSRX);                
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
    print_eth("--> %s -> %s : %d", __FILE__, __FUNCTION__, __LINE__);
    
    net_tx(packet, length);
    return 0;
}

