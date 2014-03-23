/**
 * X-Boot Operation System Entry
 *
 * Copyright (c) 2014 X-boot GITHUB team
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
 *                   STRUCTURE                                              *
 *************************************************/
/*

-------------------------------------------
	cmgr:
	- command "ping"
-------------------------------------------
	net_app:
	- net_app_ping
-------------------------------------------
	net:
	- net_alloc_ip
	- net_alloc_icmp
	- net_send_ip
	- net_send_icmp
	- net_rx_task
		|-- ICMP processing
		|-- placing packet to appropriate queue (TCP, UDC, etc)
-------------------------------------------
	net_datalink:					<-- net_arp
	- datalink_open
	- datalink_tx_alloc
	- datalink_tx_send
		|-- add MAC addresses, type
		|-- ARP processing				
		|-- place packet into tx queue		
	- datalink_close
	- datalink_task
		|--process rx queue
		|	|-- process ethernet header
		|	|-- place incoming packet into net rx queue
	- datalink_rx_get_pkt
-------------------------------------------
	drv_eth:
	- drv_eth_init
	- drv_eth_halt
	- drv_eth_rx
	- drv_eth_rx_get
	- drv_eth_tx
	- drv_eth_info
	- drv_eth_heap_alloc
	- drv_eth_heap_free
	- drv_eth_mac_is_valid
	- drv_eth_mac_is_multicast
	- drv_eth_mac_is_zero
-------------------------------------------
	drv_ks8851: 
	- ks8851_init 
	- ks8851_halt
	- ks8851_rxfc_get
	- ks8851_rx
	- ks8851_tx
	- ks8851_mac_set
-------------------------------------------
                         ||
-------------------------------------------
	[Chip  KS8851 ethernet PHY & MAC]

*/
/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"
#include "clkctrl.h"
#include "pinmux.h"
#include "drv_spi.h"
#include "net.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define XBOOT_VERSION_R      1
#define XBOOT_VERSION_RC     1

HEAPHEADER	GlobalHeap;
PHEAPHEADER	hGlobalHeap;

PGBL_CTX	pGblCtx;


static unsigned int system_time_msec = 0;
static int initialization(void);
static void termination(void);
static void rt_process(void);
void 		gbl_pring_info(void);

/************************************************
  *              ENTRY  FUNCTION                                      *
  ************************************************/
void  _start(void)
{
    unsigned int i, a;

    print_inf("\r\n");
    print_inf("--- IMX-233: X-BOOT initialization ---\r\n");
    print_inf("%s %s\r\n", __DATE__, __TIME__);
    print_inf("Version: %d.%d\r\n\r\n", XBOOT_VERSION_R, XBOOT_VERSION_RC);

    if (initialization()) {
		print_err("%s", "stop processing: initialization not competed");
		while(1);
		return;
	}

    //view system configuration
    //gbl_pring_info();

    //do a test ping
    net_ping_req(10000UL, pGblCtx->cfg_ip_server);

    print_log("%s", "Entry to the main loop...");
    while(1)
    {
        if (system_time_msec%5000 == 0)
          print_inf("[%d sec] Next cycle...\r\n", system_time_msec/1000);

        rt_process();
        
        sleep_ms(50);
        system_time_msec+=50;
    }

    termination();
    return;
}



/************************************************
  *              LOCAL  FUNCTIONs                                      *
  ************************************************/
static int initialization(void)
{
	int rc = SUCCESS;
    print_log("%s", "Environment initialization");

	/* Global Heap Initialization */
	hGlobalHeap = sys_heap_init(&GlobalHeap, SYS_RAM_HEAP_ADDR, 
		SYS_RAM_HEAP_SIZE, SYS_CACHE_LINE_BYTES, GBL_HEAP_BLOCKS_COUNT);

	//heap allocationg test
	{
		int	heap_test_rc = SUCCESS;
		void *ptr1 = sys_heap_alloc(hGlobalHeap, 100);
		void *ptr2 = sys_heap_alloc(hGlobalHeap, 1000);
		void *ptr3 = sys_heap_alloc(hGlobalHeap, 10000);
		void *ptr4 = sys_heap_alloc(hGlobalHeap, 100000);
		void *ptr5 = sys_heap_alloc(hGlobalHeap, 1000000);		
		void *ptr6 = sys_heap_alloc(hGlobalHeap, 10000000);				
		void *ptr7 = sys_heap_alloc(hGlobalHeap, 5000000);				

		if (!ptr1 || !ptr2 || !ptr3 || !ptr4 || !ptr5 || !ptr6 || !ptr7)
			heap_test_rc |= FAILURE;
			
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr5);

		ptr5 = sys_heap_alloc(hGlobalHeap, 1000000);
		if (!ptr5)
			heap_test_rc |= FAILURE;

		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr5);
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr3);
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr4);		
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr6);				
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr7);						
		
		ptr6 = sys_heap_alloc(hGlobalHeap, 15000000);
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr6);
		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr1);
 		heap_test_rc |= sys_heap_free(hGlobalHeap, ptr2);		

		//print_inf("[sys] %s", "Heap test... ");
		if (heap_test_rc == SUCCESS) {
			//print_inf("%s", "PASSED\n");
		} else {
			//print_inf("%s", "FAILED\n");
			return FAILURE;
		}
	}

	/* Init global ctx */
	pGblCtx = (PGBL_CTX)malloc(sizeof(GBL_CTX));
	memset((void *)pGblCtx, 0, sizeof(GBL_CTX));

    /* Configure CPU and SSP clocks*/
    rc |= init_clocks();
   
    /* Configure SSP1 pins*/
    init_pinmux();

    /* Configure SPI on SSP1*/
    rc |= spi_init();

   	rc |= net_init();

    return rc;
}

static void rt_process(void)
{
    //runtime
    net_rx_process();

    return;
}
static void termination(void)
{
    print_log("%s", "Environment termination");

	net_close();

    return;
}

void gbl_pring_info(void)
{
    uchar s[15];
    
    print_eth("%s", "Ethernet configuration:");
    print_eth(" - ipaddr:       %s", drv_ip_to_string(pGblCtx->cfg_ip_addr, &s[0]));
    print_eth(" - netmask:      %s", drv_ip_to_string(pGblCtx->cfg_ip_netmask, &s[0]));
    print_eth(" - gatewayip:    %s", drv_ip_to_string(pGblCtx->cfg_ip_gateway, &s[0]));
    print_eth(" - serverip:     %s", drv_ip_to_string(pGblCtx->cfg_ip_server, &s[0]));
    print_eth(" - dnsip:        %s", drv_ip_to_string(pGblCtx->cfg_ip_dns, &s[0]));
    print_eth(" - vlanip:       %s", drv_ip_to_string(pGblCtx->cfg_ip_vlan, &s[0]));

	drv_eth_info();
	arp_table_info();
	
    return;
}