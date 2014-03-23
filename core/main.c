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
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"
#include "drv_eth.h"
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


static unsigned int system_time_msec = 0;
static void initialization(void);
static void termination(void);
static void rt_process(void);


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

    initialization();

    //viewk system configuration
    drv_eth_info();

    //do a test ping
    net_ping_req(10000UL, pEth->cfg_ip_server);

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
static void initialization(void)
{
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

		print_inf("[sys] %s", "Heap test... ");
		if (heap_test_rc == SUCCESS) {
			print_inf("%s", "PASSED\n");
		} else {
			print_inf("%s", "FAILED\n");
		}
	}

    /* Configure CPU and SSP clocks*/
    init_clocks();
   
    /* Configure SSP1 pins*/
    init_pinmux();

    /* Configure SPI on SSP1*/
    spi_init();

    /* Configure Ethernt device*/
    drv_eth_init();

    return;
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

    drv_eth_halt();

    return;
}

