/**
 * X-Boot Operation System Entry
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

/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"
#include "drv_eth.h"

#define XBOOT_VERSION_R      0
#define XBOOT_VERSION_RC     2

static unsigned int system_time_msec = 0;
static void initialization(void);
static void termination(void);
static void rt_process(void);

void  _start(void)
{
    unsigned int i, a;

    print_inf("\r\n");
    print_inf("--- IMX-233: X-BOOT initialization ---\r\n");
    print_inf("%s %s\r\n", __DATE__, __TIME__);
    print_inf("Version: %d.%d\r\n\r\n", XBOOT_VERSION_R, XBOOT_VERSION_RC);

    initialization();

    print_log("%s", "Entry to the main loop...");
    while(1)
    {
        if (system_time_msec%10000 == 0)
          print_inf("[%d sec] Next cycle...\r\n", system_time_msec/1000);

        rt_process();
        
        sleep_ms(50);
        system_time_msec+=50;
    }

    termination();
        
    return;
}

/*====================================================
 *            NET
 *====================================================*/
//definitions
#define CONFIG_BOOTFILE         "zlinux"	                    /* Boot file name */
#define CONFIG_SYS_PROMPT       "x-boot> "
#define CONFIG_BOOTOFFSET       (0x8000)
#define CONFIG_SYS_LOAD_ADDR    (SDRAM_BASE + CONFIG_BOOTOFFSET)
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR           192.168.0.200                   /* 10.1.184.112 192.168.0.136 */
#define CONFIG_SERVERIP         192.168.0.105                   /* 10.1.184.188 192.168.0.2 */
#define CONFIG_BOOTDELAY        3                               /* sec */
#define PKTSIZE                 1518
#define PKTSIZE_ALIGN           1536
#define PKTALIGN                32
#define PKTBUFSRX               4                               /* Rx MAX supported by ks8851 is 12KB */
#define PKTBUFSTX               1                               /* Tx MAX supported by ks8851 is 6KB */

//declarations
char            BootFile[128];
unsigned int    linux_load_addr;

uchar          *NetArpWaitPacketMAC;                            /* MAC address of waiting packet's destination */
uchar          *NetArpWaitTxPacket;                             /* THE transmit packet */
int             NetArpWaitTxPacketSize;
uchar           NetArpWaitPacketBuf[PKTSIZE_ALIGN + PKTALIGN];
unsigned int    NetArpWaitTimerStart;
int             NetArpWaitTry;
IPaddr_t        NetArpWaitPacketIP;
IPaddr_t        NetArpWaitReplyIP;
volatile uchar  PktBuf[(PKTBUFSRX+PKTBUFSTX) * PKTSIZE_ALIGN + PKTALIGN];   
                                                                /* 7712bytes: 1 Tx packet + 4 Rx packets */
volatile uchar *NetTxPackets[PKTBUFSTX];                        /* Transmit packets */
volatile uchar *NetRxPackets[PKTBUFSRX];                        /* Receive packets */

unsigned int	gStatusEthernet = 0;                            /* disabled */


static void initialization(void)
{
    unsigned int i;
 
    print_log("%s", "Environment initialization");

   
    //net driver initialization
    print_eth("%s", "Net environment initialization");
    linux_load_addr = CONFIG_SYS_LOAD_ADDR;
    NetArpWaitPacketMAC = NULL;
    NetArpWaitPacketIP = 0;
    NetArpWaitReplyIP = 0;
    copy_filename(BootFile, CONFIG_BOOTFILE, sizeof(BootFile));

    //setup packet buffers, aligned correctly
    NetTxPackets[0] = (volatile uchar *)((unsigned int)&PktBuf[0] + (PKTALIGN - 1));
    NetTxPackets[0] = (volatile uchar *)((unsigned int)NetTxPackets[0] - (unsigned int)NetTxPackets % PKTALIGN);
    for (i = 1; i < PKTBUFSTX; i++) {
        NetTxPackets[i] = (volatile uchar *)((unsigned int)NetTxPackets[0] + i*PKTSIZE_ALIGN);
    }
    for (i = 0; i < PKTBUFSRX; i++) {
        NetRxPackets[i] = (volatile uchar *)((unsigned int)NetTxPackets[0] + (i+PKTBUFSTX)*PKTSIZE_ALIGN);
    }

    print_eth(" - Net Tx packet: base_0x%x count_%d", (unsigned int)NetTxPackets[0], PKTBUFSTX);
    print_eth(" - Net Rx packet: base_0x%x count_%d", (unsigned int)NetRxPackets[0], PKTBUFSRX);    
    
    NetArpWaitTxPacket = (uchar *)((unsigned int)&NetArpWaitPacketBuf[0] + (PKTALIGN - 1));
    NetArpWaitTxPacket = (uchar *)((unsigned int)NetArpWaitTxPacket - (unsigned int)NetArpWaitTxPacket % PKTALIGN);
    NetArpWaitTxPacketSize = 0;

    print_eth(" - Net Arp Tx packet: base_0x%x count_%d", (unsigned int)NetArpWaitTxPacket, 1);

    print_eth("%s", "Halt Ethernel driver whatever its condition...");
    drv_eth_halt();                                             /* do call of common interface function */

    if (drv_eth_init(NULL)) {                                    /* do call of common interface function */
        drv_eth_halt();                                         /* do call of common interface function */
        gStatusEthernet = 0;
        print_err("%s", "ethernet initialization wasn't completed");
    } else {
        print_eth("%s", "Ethernel was successfully started");
        gStatusEthernet = 1;
    }

    return;
}

static void rt_process(void)
{
    //runtime
    

    return;
}
static void termination(void)
{
    print_log("%s", "Environment termination");

    drv_eth_halt();

    return;
}

